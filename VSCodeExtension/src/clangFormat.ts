import { execFile } from 'node:child_process';
import { constants } from 'node:fs';
import * as fs from 'node:fs/promises';
import * as path from 'node:path';

export interface ClangFormatStatus {
	command: string;
	configured: boolean;
	available: boolean;
	executablePath?: string;
	error?: string;
}

function unquote(value: string): string {
	const trimmed = value.trim();
	return trimmed.startsWith('"') && trimmed.endsWith('"') ? trimmed.slice(1, -1) : trimmed;
}

/** Resolve paths from the workspace; bare executable names are looked up in PATH. */
export function getClangFormatCommand(
	configuredPath: string,
	baseDirectory = process.cwd(),
	platform = process.platform,
): string {
	const configured = unquote(configuredPath).replace(/\$\{workspaceFolder\}/g, () => baseDirectory);
	if (!configured) { return platform === 'win32' ? 'clang-format.exe' : 'clang-format'; }
	return path.isAbsolute(configured) || /[/\\]/.test(configured) || (platform === 'win32' && configured.includes(':'))
		? path.resolve(baseDirectory, configured) : configured;
}

export async function checkClangFormat(
	configuredPath: string,
	options: { baseDirectory?: string; env?: NodeJS.ProcessEnv; signal?: AbortSignal } = {},
): Promise<ClangFormatStatus> {
	const baseDirectory = options.baseDirectory ?? process.cwd();
	const environment = options.env ?? process.env;
	const command = getClangFormatCommand(configuredPath, baseDirectory);
	const status: ClangFormatStatus = { command, configured: unquote(configuredPath).length > 0, available: false };
	options.signal?.throwIfAborted();
	const searchPath = process.platform === 'win32'
		? Object.entries(environment).find(([key]) => key.toLowerCase() === 'path')?.[1]
		: environment.PATH;
	const names = process.platform === 'win32' && !path.extname(command) ? [command, `${command}.exe`] : [command];
	const candidates = path.isAbsolute(command)
		? names
		: (searchPath ?? '').split(path.delimiter).map(unquote).filter(Boolean)
			.flatMap(directory => names.map(name => path.resolve(baseDirectory, directory, name)));
	for (const candidate of candidates) {
		options.signal?.throwIfAborted();
		try {
			if (!(await fs.stat(candidate)).isFile()) { continue; }
			await fs.access(candidate, process.platform === 'win32' ? constants.R_OK : constants.X_OK);
		} catch { continue; }
		status.executablePath = candidate;
		try {
			const output = await new Promise<string>((resolve, reject) => {
				execFile(candidate, ['--version'], {
					cwd: baseDirectory, env: environment, windowsHide: true, shell: false,
					timeout: 5_000, maxBuffer: 64 * 1024, encoding: 'utf8', signal: options.signal,
				}, (error, stdout, stderr) => error ? reject(error) : resolve(`${stdout}\n${stderr}`));
			});
			options.signal?.throwIfAborted();
			if (!/\bclang-format\b/i.test(output)) { throw new Error('The executable did not identify itself as clang-format.'); }
			return { ...status, available: true };
		} catch (error) {
			options.signal?.throwIfAborted();
			return { ...status, error: error instanceof Error ? error.message : String(error) };
		}
	}
	return { ...status, error: status.configured ? 'The configured executable was not found or is not accessible.' : 'clang-format was not found in PATH or is not accessible.' };
}
