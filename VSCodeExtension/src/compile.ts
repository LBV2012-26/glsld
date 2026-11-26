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
	stage: string;
	fileUri: vscode.Uri;
	command: CompileCommand;
}

interface CompileResult {
	uri: string;
	groupName: string;
	stage: string;
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

async function executeCompile(glslcPath: string, command: CompileCommand): Promise<{ success: boolean; stdout: string; stderr: string }> {
	await fs.promises.mkdir(path.dirname(command.outputPath), { recursive: true });
	return new Promise((resolve) => {
		cp.execFile(glslcPath, command.args, {
			cwd: command.cwd,
			windowsHide: true,
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

function resolveGroupFile(key: string): vscode.Uri {
	if (key.startsWith('file:')) { return vscode.Uri.parse(key); }
	if (path.isAbsolute(key)) { return vscode.Uri.file(key); }
	const folders = vscode.workspace.workspaceFolders;
	if (!folders?.length) { throw new Error(`Open a workspace or use an absolute file path: ${key}`); }
	const normalized = key.replace(/\\/g, '/');
	// asRelativePath prefixes the folder name when more than one workspace folder is open.
	const folder = folders.length > 1 ? folders.find(folder => normalized.startsWith(`${folder.name}/`)) : undefined;
	return vscode.Uri.file(path.resolve((folder ?? folders[0]).uri.fsPath, folder ? normalized.slice(folder.name.length + 1) : normalized));
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
	if (!groups.some(group => Object.keys(group.fileStages ?? {}).length > 0)) {
		vscode.window.showWarningMessage('No files in compile groups. Use the + action on a Compile Group to add a file and select its stages.');
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

	// Only explicitly added files are compiled.
	channel.appendLine('Preparing Compile Group files...\n');

	const jobs: CompileJob[] = [];
	let preparationFailures = 0;

	for (const group of groups) {
		const files = Object.entries(group.fileStages ?? {});
		channel.appendLine(`  ${group.name}: ${files.length} file(s)`);
		for (const [key, stages] of files) {
			try {
				const uri = resolveGroupFile(key);
				const perFileDefines = fileMacros[uri.toString()] ?? {};
				const enabledPerFile: Record<string, string> = {};
				for (const [name, entry] of Object.entries(perFileDefines)) {
					if (entry.enabled) { enabledPerFile[name] = entry.value; }
				}
				const workspaceDirectory = vscode.workspace.workspaceFolders?.[0].uri.fsPath ?? path.dirname(uri.fsPath);
				const commands = createCompileCommands(group, uri.fsPath,
					{ ...activeMacros, ...enabledPerFile }, stages, workspaceDirectory);
				const names = commands.map(command => command.stage);
				channel.appendLine(`    ${vscode.workspace.asRelativePath(uri)}: ${names.length} stage(s): ${names.join(', ')}`);
				for (const command of commands) {
					jobs.push({ groupName: group.name, fileUri: uri, command, stage: command.stage });
				}
			} catch (error) {
				preparationFailures++;
				channel.appendLine(`    Could not prepare ${key}: ${error instanceof Error ? error.message : error}`);
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
		const message = `Multiple compilation jobs would overwrite ${conflictingOutput}. Set a different output directory for each group that compiles the same file and stage.`;
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
			const label = `${job.groupName}: ${relPath}[${job.stage}]`;
			channel.appendLine(`[${i + 1}/${jobs.length}] ${label} ...`);
		},
		async (job) => {
			const start = Date.now();
			let result: { success: boolean; stdout: string; stderr: string };
			try { result = await executeCompile(glslcPath, job.command); }
			catch (error) { result = { success: false, stdout: '', stderr: error instanceof Error ? error.message : String(error) }; }
			return {
				uri: job.fileUri.toString(),
				groupName: job.groupName,
				stage: job.stage,
				...result,
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
		const label = `${r.groupName}: ${relPath} [${r.stage}]`;
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
