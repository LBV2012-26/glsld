/* --------------------------------------------------------------------------------------------
 * Copyright (c) glsld authors. All rights reserved.
 * Licensed under the MIT License.
 * ------------------------------------------------------------------------------------------ */

import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';
import * as os from 'os';
import * as fs from 'fs';
import {
	getActiveGlobalMacros,
	getCompileGroups,
	getFileMacrosData,
} from './sidebar';
import { CompileCommand, createCompileCommands, findCompileOutputConflict } from './compilePlan';

// ============================================================
// Compile job / result
// ============================================================

interface CompileJob {
	groupName: string;
	variantName: string;
	fileUri: vscode.Uri;
	command: CompileCommand;
}

interface CompileResult {
	uri: string;
	groupName: string;
	variantName: string;
	success: boolean;
	stdout: string;
	stderr: string;
	durationMs: number;
}

// ============================================================
// glslc path resolution (mirrors DiagnosticEngine::FindGlslc)
// ============================================================

function findGlslc(): string {
	const config = vscode.workspace.getConfiguration('glsld');
	const userPath = config.get<string>('glslc.path', '').trim();
	if (userPath) {
		return userPath;
	}
	// Auto-detect from VULKAN_SDK
	const vkSdk = process.env.VULKAN_SDK;
	if (vkSdk) {
		const candidate = path.join(vkSdk, 'Bin', 'glslc.exe');
		if (fs.existsSync(candidate)) {
			return candidate;
		}
	}
	return 'glslc.exe'; // fallback to PATH
}

// ============================================================
// Concurrency-limited parallel execution
// ============================================================

async function runParallel<T, R>(
	items: T[],
	concurrency: number,
	onStart: (item: T, index: number) => void,
	worker: (item: T, index: number) => Promise<R>,
): Promise<R[]> {
	const results: R[] = new Array(items.length);
	let nextIndex = 0;

	async function runner(): Promise<void> {
		while (true) {
			const i = nextIndex++;
			if (i >= items.length) { return; }
			onStart(items[i], i);
			results[i] = await worker(items[i], i);
		}
	}

	const count = Math.min(concurrency, items.length);
	if (count <= 0) { return results; }

	await Promise.all(Array.from({ length: count }, () => runner()));
	return results;
}

// ============================================================
// Single file compilation
// ============================================================

function executeCompile(glslcPath: string, command: CompileCommand, cwd: string): Promise<{ success: boolean; stdout: string; stderr: string }> {
	return new Promise((resolve) => {
		const { executable: templateExe, args } = command;

		// If template starts with "glslc" (any path variant), use our detected glslc
		const exeName = path.basename(templateExe).toLowerCase();
		const isGlslc = exeName === 'glslc' || exeName === 'glslc.exe';

		const exe = isGlslc ? glslcPath : templateExe;
		cp.execFile(exe, args, {
			cwd,
			maxBuffer: 16 * 1024 * 1024, // 16 MB
			timeout: 120_000,            // 2 min per file
		}, (error, stdout, stderr) => {
			resolve({
				success: !error || error.code === 0,
				stdout: stdout?.trim() ?? '',
				stderr: stderr?.trim() ?? '',
			});
		});
	});
}

// ============================================================
// Main entry — compile workspace
// ============================================================

export async function compileWorkspace(): Promise<void> {
	const config = vscode.workspace.getConfiguration('glsld');
	const groups = getCompileGroups();

	if (!groups || groups.length === 0) {
		vscode.window.showWarningMessage(
			'No compile groups configured. Add one in GLSL Shader > Compile Groups.',
		);
		return;
	}

	// Validate that at least one group has files
	let hasFiles = false;
	for (const g of groups) {
		if (g.include && g.include.length > 0) { hasFiles = true; break; }
	}
	if (!hasFiles) {
		vscode.window.showWarningMessage('No glob patterns in compile groups. Add include patterns.');
		return;
	}

	const channel = vscode.window.createOutputChannel('GLSL Compile');
	channel.show(true);
	channel.appendLine('══════════════════════════════════════');
	channel.appendLine('  GLSL → SPIR-V Compilation');
	channel.appendLine('══════════════════════════════════════\n');

	// Resolve glslc
	const glslcPath = findGlslc();
	channel.appendLine(`glslc: ${glslcPath}\n`);

	// Resolve active variant macros (from sidebar)
	const activeMacros = getActiveGlobalMacros();

	// Resolve per-file macros (from sidebar)
	const fileMacros = getFileMacrosData();

	// Expand globs → build job list
	channel.appendLine('Expanding glob patterns...\n');

	const jobs: CompileJob[] = [];
	let preparationFailures = 0;

	for (const group of groups) {
		if (!group.include || group.include.length === 0) { continue; }
		if (!group.command) { continue; }

		// Build VSCode glob pattern: {**/*.vert,**/*.frag}
		const pattern = `{${group.include.join(',')}}`;
		const uris = await vscode.workspace.findFiles(pattern, null, 10000);

		if (uris.length === 0) {
			channel.appendLine(`  ${group.name}: no files matched`);
			continue;
		}

		const variantCount = group.variants?.length || 1;
		channel.appendLine(`  ${group.name}: ${uris.length} file(s) × ${variantCount} variant(s)`);

		for (const uri of uris) {
			const perFileDefines = fileMacros[uri.toString()] ?? {};
			const enabledPerFile: Record<string, string> = {};
			for (const [k, v] of Object.entries(perFileDefines)) {
				if (v.enabled) { enabledPerFile[k] = v.value; }
			}

			try {
				const stages = group.fileStages?.[vscode.workspace.asRelativePath(uri)] ?? group.fileStages?.[uri.toString()] ?? group.fileStages?.[uri.fsPath];
				const commands = createCompileCommands(group.command, uri.fsPath,
					{ ...activeMacros, ...enabledPerFile }, group.variants, stages);
				if (stages?.length) {
					const names = [...new Set(commands.map(command => command.stage))];
					channel.appendLine(`    ${vscode.workspace.asRelativePath(uri)}: ${names.length} stage(s): ${names.join(', ')}`);
				}
				for (const command of commands) {
					jobs.push({ groupName: group.name, fileUri: uri, command,
						variantName: [command.variantName, command.stage].filter(Boolean).join('/') });
				}
			} catch (error) {
				preparationFailures++;
				channel.appendLine(`    Could not prepare ${uri.fsPath}: ${error instanceof Error ? error.message : error}`);
			}
		}
	}

	if (jobs.length === 0) {
		channel.appendLine('\nNo files to compile.');
		if (preparationFailures) { vscode.window.showWarningMessage(`SPIR-V: ${preparationFailures} file(s) could not be prepared — check Output for details.`); }
		return;
	}
	const conflictingOutput = findCompileOutputConflict(jobs.map(job => job.command));
	if (conflictingOutput) {
		const message = `Multiple compilation jobs would overwrite ${conflictingOutput}. Use -o "{output}" or include {stage} and {variant} in the output path.`;
		channel.appendLine(message);
		vscode.window.showWarningMessage(message);
		return;
	}

	// Concurrency
	const maxConcurrency = config.get<number>('compileMaxConcurrency', 0) || os.cpus().length;

	channel.appendLine(`\nCompiling ${jobs.length} job(s) with ${maxConcurrency} workers...\n`);

	const startTime = Date.now();

	// Run!
	const results = await runParallel<CompileJob, CompileResult>(
		jobs,
		maxConcurrency,
		(job, i) => {
			const relPath = vscode.workspace.asRelativePath(job.fileUri);
			const label = job.variantName ? `${relPath}[${job.variantName}]` : relPath;
			channel.appendLine(`[${i + 1}/${jobs.length}] ${label} ...`);
		},
		async (job) => {
			const start = Date.now();
			const { success, stdout, stderr } = await executeCompile(
				glslcPath,
				job.command,
				path.dirname(job.fileUri.fsPath),
			);
			return {
				uri: job.fileUri.toString(),
				groupName: job.groupName,
				variantName: job.variantName,
				success,
				stdout,
				stderr,
				durationMs: Date.now() - start,
			};
		},
	);

	// Report
	const succeeded = results.filter(r => r.success).length;
	const failed = results.filter(r => !r.success).length + preparationFailures;
	const totalDuration = Date.now() - startTime;

	channel.appendLine('\n──────────────────────────────────────');
	channel.appendLine(`  Done: ${succeeded} succeeded, ${failed} failed, ${jobs.length + preparationFailures} total`);
	channel.appendLine(`  Time:  ${(totalDuration / 1000).toFixed(1)}s`);
	channel.appendLine('──────────────────────────────────────\n');

	// Per-result detail
	for (let i = 0; i < results.length; i++) {
		const r = results[i];
		const relPath = vscode.workspace.asRelativePath(vscode.Uri.parse(r.uri));
		const label = r.variantName ? `${relPath} [${r.variantName}]` : relPath;
		const status = r.success ? '✓' : '✗';
		channel.appendLine(`${status} ${label}  (${r.durationMs}ms)`);

		if (!r.success && r.stderr) {
			// Indent stderr for readability
			for (const line of r.stderr.split('\n')) {
				channel.appendLine(`    ${line}`);
			}
		}
	}

	channel.appendLine('');

	// Summary notification
	if (failed === 0) {
		vscode.window.showInformationMessage(
			`SPIR-V: ${succeeded} succeeded in ${(totalDuration / 1000).toFixed(1)}s`,
		);
	} else {
		vscode.window.showWarningMessage(
			`SPIR-V: ${succeeded} succeeded, ${failed} failed — check Output for details`,
		);
	}
}

// (macro state is read from sidebar.ts exports — see getActiveGlobalMacros / getFileMacrosData)
