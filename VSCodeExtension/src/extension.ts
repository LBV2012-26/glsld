/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import {
	CancellationError,
	commands,
	Disposable,
	ExtensionContext,
	Position,
	ProgressLocation,
	Range,
	Selection,
	TextDocument,
	TextDocumentChangeEvent,
	TextEditorDecorationType,
	Uri,
	window,
	workspace
} from 'vscode';

import { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } from 'vscode-languageclient/node';

import { activateSidebar, getActiveVariants, getShaderConfigs } from './sidebar';
import { compileWorkspace } from './compile';
import * as path from 'path';
import { ShaderConfigProvider } from './shaderConfig';
import { ServerManager } from './serverManager';

let client: LanguageClient;
let inactiveDecorationDisposable: TextEditorDecorationType | undefined;
const inactiveRangesByUri = new Map<string, Range[]>();
const scheduledUpdates = new Map<string, NodeJS.Timeout>();
const runtimeDisposables: Disposable[] = [];
let shuttingDown = false;
let downloadAbort: AbortController | undefined;
let updateAbort: AbortController | undefined;

export async function activate(context: ExtensionContext) {
	shuttingDown = false;
	updateAbort = new AbortController();
	const serverManager = new ServerManager(context.globalStorageUri.fsPath);
	const serverOutput = window.createOutputChannel('glsld');
	context.subscriptions.push(serverOutput);
	const resolveServerPath = () => {
		const config = workspace.getConfiguration('glsld');
		return serverManager.resolveServerPath(config.get<string>('server.path', ''), context.extensionPath);
	};
	// Update this executable before each launch while keeping one client for the views.
	// Let LanguageClient own the child process so stop/restart can also terminate it.
	const serverExecutable = {
		command: '', transport: TransportKind.stdio,
		options: { env: { ...process.env }, windowsHide: true },
	};
	const serverOptions: ServerOptions = { run: serverExecutable, debug: serverExecutable };
	const fileWatcher = workspace.createFileSystemWatcher('**/.clientrc');
	context.subscriptions.push(fileWatcher);

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		outputChannel: serverOutput,
		// Register the server for plain text documents
		documentSelector: [{ scheme: 'file', language: 'glsl' }],
		synchronize: {
			// Notify the server about file changes to '.clientrc files contained in the workspace
			fileEvents: fileWatcher
		},
		middleware: {
			provideDocumentSemanticTokens: async (document, token, next) => {
				const result = await next(document, token);
				if (document.languageId === 'glsl' && result) {
					applyInactiveFromSemanticTokenData(document, result.data);
				}
				return result;
			},
			provideDocumentSemanticTokensEdits: async (document, previousResultId, token, next) => {
				const result = await next(document, previousResultId, token);
				if (document.languageId === 'glsl') {
					scheduleInactiveUpdate(document, 0);
				}
				return result;
			},
			provideDocumentRangeSemanticTokens: async (document, range, token, next) => {
				const result = await next(document, range, token);
				if (document.languageId === 'glsl' && result) {
					applyInactiveFromSemanticTokenData(document, result.data);
					scheduleInactiveUpdate(document, 0);
				}
				return result;
			}
		}
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'glsld',
		'glsld',
		serverOptions,
		clientOptions
	);

	let serverOperation = Promise.resolve();
	let downloadTask: Promise<void> | undefined;
	let missingServerPromptShown = false;
	const openServerSettings = () => commands.executeCommand('workbench.action.openSettings', 'glsld.server');
	const reportError = (error: unknown) => {
		const message = error instanceof Error ? error.message : String(error);
		serverOutput.appendLine(`[glsld] ${message}`);
		if (!shuttingDown) {
			void window.showErrorMessage(`glsld: ${message}`, 'Open Settings', 'Show Output').then(choice => {
				if (choice === 'Open Settings') { return openServerSettings(); }
				if (choice === 'Show Output') { serverOutput.show(); }
			});
		}
	};
	const startServer = (restart = false, promptIfMissing = true): Promise<void> => {
		serverOperation = serverOperation.then(async () => {
			if (shuttingDown) { return; }
			const serverPath = await resolveServerPath();
			if (shuttingDown) { return; }
			if (!serverPath) {
				if (restart && client.isRunning()) {
					clearInactiveHighlights();
					await client.stop();
				}
				if (promptIfMissing && !missingServerPromptShown && workspace.textDocuments.some(doc => doc.languageId === 'glsl')) {
					missingServerPromptShown = true;
					void window.showInformationMessage('Download the glsld language server to enable language features, or configure glsld.server.path to use a local executable.', 'Download', 'Open Settings').then(choice => {
						if (choice === 'Download') { return commands.executeCommand('glsld.downloadServer'); }
						if (choice === 'Open Settings') { return openServerSettings(); }
					});
				}
				return;
			}
			if (client.isRunning() && !restart) { return; }
			serverExecutable.command = serverPath;
			serverExecutable.options.env = createServerEnvironment(workspace.getConfiguration('glsld'));
			serverOutput.appendLine(`[glsld] Server executable: ${serverPath}`);
			serverOutput.appendLine(formatMimallocLaunchOptions(serverExecutable.options.env));
			if (client.isRunning()) {
				clearInactiveHighlights();
				await client.restart();
			} else {
				await client.start();
			}
			missingServerPromptShown = false;
			await pushConfiguration();
			for (const document of workspace.textDocuments) {
				if (document.languageId === 'glsl') { scheduleInactiveUpdate(document, 0); }
			}
		}).catch(reportError);
		return serverOperation;
	};
	const downloadServer = (): Promise<void> => {
		if (downloadTask) { return downloadTask; }
		downloadTask = (async () => {
			const installed = await window.withProgress({
				location: ProgressLocation.Notification,
				title: 'Downloading glsld language server', cancellable: true,
			}, async (progress, token) => {
				const abort = new AbortController();
				downloadAbort = abort;
				const cancellation = token.onCancellationRequested(() => abort.abort(new CancellationError()));
				if (token.isCancellationRequested || shuttingDown) { abort.abort(new CancellationError()); }
				try {
					return await serverManager.downloadServer({ signal: abort.signal, onProgress: update => progress.report(update) });
				} catch (error) {
					if (abort.signal.aborted) { return undefined; }
					throw error;
				} finally {
					cancellation.dispose();
					downloadAbort = undefined;
				}
			});
			if (!installed || shuttingDown) { return; }
			serverOutput.appendLine(`[glsld] Installed ${installed.version} for ${installed.target}: ${installed.executablePath}`);
			if (workspace.getConfiguration('glsld').get<string>('server.path', '').trim()) {
				void window.showInformationMessage(`glsld ${installed.version} was downloaded. Clear glsld.server.path to use it.`, 'Open Settings').then(choice => {
					if (choice === 'Open Settings') { return openServerSettings(); }
				});
			} else {
				await startServer(true, false);
			}
		})().catch(reportError).finally(() => { downloadTask = undefined; });
		return downloadTask;
	};
	let notifiedVersion: string | undefined;
	const checkForUpdates = async (): Promise<void> => {
		try {
			const latest = await serverManager.checkForUpdates({ signal: updateAbort?.signal });
			if (shuttingDown) { return; }
			serverOutput.appendLine(`[glsld] Latest GitHub release: ${latest.version}`);
			const customPath = workspace.getConfiguration('glsld').get<string>('server.path', '').trim();
			if (!customPath && !latest.installed && notifiedVersion !== latest.version && await serverManager.getInstalledServer()) {
				notifiedVersion = latest.version;
				void window.showInformationMessage(`glsld ${latest.version} is available. Download the latest language server?`, 'Download').then(choice => {
					if (choice === 'Download') { return commands.executeCommand('glsld.downloadServer'); }
				});
			}
		} catch (error) {
			if (!shuttingDown) {
				const message = error instanceof Error ? error.message : String(error);
				serverOutput.appendLine(`[glsld] Could not check the latest release: ${message} Using an installed server if available.`);
			}
		}
	};
	// Register these before attempting startup: a missing server must not prevent installation.
	context.subscriptions.push(
		commands.registerCommand('glsld.downloadServer', downloadServer),
		commands.registerCommand('glsld.restartServer', async () => {
			await checkForUpdates();
			await startServer(true);
		}),
		new Disposable(() => {
			downloadAbort?.abort(new CancellationError());
			updateAbort?.abort(new CancellationError());
		}),
	);
	initializeInactiveTokenDimming();
	activateSidebar(context, client);

	// Shader Config view (per-file stage/version overrides)

	const shaderConfigProvider = new ShaderConfigProvider();

	context.subscriptions.push(
		window.registerTreeDataProvider('glsld.shaderConfig', shaderConfigProvider),

		commands.registerCommand('glsld.addShaderConfig', () => shaderConfigProvider.addConfig()),
		commands.registerCommand('glsld.editShaderConfigProp', (node: any) => {
			if (node?.kind === 'configProp') { shaderConfigProvider.editProperty(node); }
			else { shaderConfigProvider.editConfigProps(); }
		}),


		commands.registerCommand('glsld.removeShaderConfig', (node: any) => {
			if (node?.kind === 'fileConfig') { shaderConfigProvider.removeConfig(node); }
		}),

		commands.registerCommand('glsld.generateShaderConfigs', () => shaderConfigProvider.generateConfigs()),
		commands.registerCommand('glsld.applyShaderConfigTemplate', () => shaderConfigProvider.applyTemplate()),

	);


	// Register compile-to-SPIR-V command
	context.subscriptions.push(
		commands.registerCommand('glsld.compileWorkspace', () => {
			compileWorkspace();
		})
	);

	// Push configuration to server when settings change
	context.subscriptions.push(
		workspace.onDidChangeConfiguration(async (e) => {
			if (e.affectsConfiguration('glsld')) {
				if (e.affectsConfiguration('glsld.server') || e.affectsConfiguration('glsld.mimalloc')) {
					if (e.affectsConfiguration('glsld.server')) { await checkForUpdates(); }
					await startServer(true);
				} else {
					await pushConfiguration();
				}
				shaderConfigProvider.refresh();
			}
		})
	);

	// Re-push shader config when a GLSL file is opened, so the server
	// can apply stored per-file configs to the now-open document.
	context.subscriptions.push(
		workspace.onDidOpenTextDocument(async (doc) => {
			if (doc.languageId === 'glsl') {
				await startServer();
				await pushConfiguration();
			}
		})
	);

	await checkForUpdates();
	await startServer();

}

function createServerEnvironment(config: ReturnType<typeof workspace.getConfiguration>): NodeJS.ProcessEnv {
	const environment: NodeJS.ProcessEnv = { ...process.env };
	const mimallocVerbose = config.get<boolean>('mimalloc.verbose', false);
	if (mimallocVerbose) {
		environment.MIMALLOC_VERBOSE = '1';
	} else {
		delete environment.MIMALLOC_VERBOSE;
	}

	const largePagesEnabled = config.get<boolean>('mimalloc.largePages.enabled', false);
	environment.MIMALLOC_ALLOW_LARGE_OS_PAGES = largePagesEnabled ? '1' : '0';
	environment.MIMALLOC_ARENA_EAGER_COMMIT = largePagesEnabled ? '1' : '0';

	if (!largePagesEnabled) {
		delete environment.MIMALLOC_ARENA_RESERVE;
		return environment;
	}

	const configuredReserveMiB = config.get<number>('mimalloc.largePages.reserveMiB', 256);
	const reserveMiB = Number.isFinite(configuredReserveMiB)
		? Math.min(Math.max(Math.trunc(configuredReserveMiB), 2), 4096)
		: 256;
	environment.MIMALLOC_ARENA_RESERVE = String(reserveMiB * 1024 * 1024);
	return environment;
}

function formatMimallocLaunchOptions(environment: NodeJS.ProcessEnv): string {
	const largePages = environment.MIMALLOC_ALLOW_LARGE_OS_PAGES === '1';
	const reserveBytes = environment.MIMALLOC_ARENA_RESERVE;
	const reserveMiB = reserveBytes === undefined ? undefined : Number(reserveBytes) / (1024 * 1024);
	const reserveDescription = Number.isFinite(reserveMiB) ? `${reserveMiB} MiB` : 'mimalloc default';
	return `[glsld] Launching glsld with mimalloc large-page allocation ${largePages ? 'requested' : 'disabled'}; arena reserve: ${reserveDescription}.`;
}

export function deactivate(): Thenable<void> | undefined {
	shuttingDown = true;
	downloadAbort?.abort(new CancellationError());
	updateAbort?.abort(new CancellationError());
	for (const handle of scheduledUpdates.values()) {
		clearTimeout(handle);
	}
	scheduledUpdates.clear();
	inactiveRangesByUri.clear();
	inactiveDecorationDisposable?.dispose();
	inactiveDecorationDisposable = undefined;
	while (runtimeDisposables.length > 0) {
		runtimeDisposables.pop()?.dispose();
	}

	if (!client) {
		return undefined;
	}
	return client.stop();
}

function initializeInactiveTokenDimming(): void {
	inactiveDecorationDisposable = window.createTextEditorDecorationType({
		opacity: '0.55'
	});

	runtimeDisposables.push(
		workspace.onDidChangeTextDocument((event) => {
			if (event.document.languageId !== 'glsl') {
				return;
			}
			void autoCloseIncludeAngleBracket(event);
			scheduleInactiveUpdate(event.document);
		}),
		workspace.onDidOpenTextDocument((document) => {
			if (document.languageId !== 'glsl') {
				return;
			}
			scheduleInactiveUpdate(document, 0);
		}),
		workspace.onDidCloseTextDocument((document) => {
			const uri = document.uri.toString();
			inactiveRangesByUri.delete(uri);
			const timeout = scheduledUpdates.get(uri);
			if (timeout) {
				clearTimeout(timeout);
				scheduledUpdates.delete(uri);
			}
		}),
		window.onDidChangeActiveTextEditor((editor) => {
			if (!editor || editor.document.languageId !== 'glsl') {
				return;
			}
			scheduleInactiveUpdate(editor.document, 0);
		}),
		window.onDidChangeVisibleTextEditors((editors) => {
			const seen = new Set<string>();
			for (const editor of editors) {
				if (editor.document.languageId !== 'glsl') {
					continue;
				}
				const uri = editor.document.uri.toString();
				if (seen.has(uri)) {
					continue;
				}
				seen.add(uri);
				scheduleInactiveUpdate(editor.document, 0);
			}
		})
	);

	for (const editor of window.visibleTextEditors) {
		if (editor.document.languageId === 'glsl') {
			scheduleInactiveUpdate(editor.document, 0);
		}
	}
}

/**
 * GLSL uses '<' and '>' extensively for comparisons and shifts, so they must
 * not be configured as a global auto-closing pair.  Includes are the one
 * unambiguous context in which typing '<' should produce a matching '>'.
 */
async function autoCloseIncludeAngleBracket(event: TextDocumentChangeEvent): Promise<void> {
	if (event.contentChanges.length !== 1) {
		return;
	}

	const change = event.contentChanges[0];
	if (change.text !== '<' || change.rangeLength !== 0) {
		return;
	}

	const editor = window.activeTextEditor;
	if (!editor || editor.document !== event.document) {
		return;
	}

	const closePosition = new Position(change.range.start.line, change.range.start.character + 1);
	const lineText = event.document.lineAt(closePosition.line).text;
	const textBeforeCursor = lineText.slice(0, closePosition.character);
	if (!/^\s*#\s*include\s*<$/.test(textBeforeCursor) || lineText.charAt(closePosition.character) === '>') {
		return;
	}

	const wasInserted = await editor.edit(
		(editBuilder) => editBuilder.insert(closePosition, '>'),
		{ undoStopBefore: false, undoStopAfter: false }
	);
	if (!wasInserted || window.activeTextEditor !== editor || editor.document !== event.document) {
		return;
	}

	const caret = new Selection(closePosition, closePosition);
	editor.selection = caret;
	await commands.executeCommand('editor.action.triggerSuggest');
}

function scheduleInactiveUpdate(document: TextDocument, delayMs = 120): void {
	if (!client?.isRunning()) { return; }
	const uri = document.uri.toString();
	const existing = scheduledUpdates.get(uri);
	if (existing) {
		clearTimeout(existing);
	}

	const timeout = setTimeout(() => {
		scheduledUpdates.delete(uri);
		void updateInactiveRanges(document);
	}, delayMs);

	scheduledUpdates.set(uri, timeout);
}

function clearInactiveHighlights(): void {
	for (const handle of scheduledUpdates.values()) { clearTimeout(handle); }
	scheduledUpdates.clear();
	inactiveRangesByUri.clear();
	if (inactiveDecorationDisposable) {
		for (const editor of window.visibleTextEditors) { editor.setDecorations(inactiveDecorationDisposable, []); }
	}
}

export function refreshHighlights(document: TextDocument): void {
	scheduleInactiveUpdate(document, 0);
}

async function updateInactiveRanges(document: TextDocument): Promise<void> {
	if (!client?.isRunning()) { return; }
	try {
		const response = await client.sendRequest<{ data?: number[] }>('textDocument/semanticTokens/full', {
			textDocument: {
				uri: document.uri.toString()
			}
		});

		applyInactiveFromSemanticTokenData(document, response?.data ?? []);
	} catch {
		applyInactiveRanges(document, []);
	}
}

function applyInactiveFromSemanticTokenData(document: TextDocument, data: ArrayLike<number>): void {
	const legend = getSemanticTokensLegend();
	if (!legend) {
		applyInactiveRanges(document, []);
		return;
	}

	const inactiveIndex = legend.tokenModifiers.indexOf('inactive');
	if (inactiveIndex < 0) {
		applyInactiveRanges(document, []);
		return;
	}

	const ranges = decodeInactiveRanges(data, inactiveIndex);
	applyInactiveRanges(document, ranges);
}

function applyInactiveRanges(document: TextDocument, ranges: Range[]): void {
	if (!inactiveDecorationDisposable) {
		return;
	}

	const uri = document.uri.toString();
	inactiveRangesByUri.set(uri, ranges);

	for (const editor of window.visibleTextEditors) {
		if (editor.document.uri.toString() !== uri) {
			continue;
		}

		editor.setDecorations(inactiveDecorationDisposable, ranges);
	}
}

function decodeInactiveRanges(data: ArrayLike<number>, inactiveModifierIndex: number): Range[] {
	const ranges: Range[] = [];
	let line = 0;
	let character = 0;
	const inactiveMask = 2 ** inactiveModifierIndex;

	for (let i = 0; i + 4 < data.length; i += 5) {
		const deltaLine = data[i];
		const deltaStart = data[i + 1];
		const tokenLength = data[i + 2];
		const tokenModifiers = data[i + 4];

		line += deltaLine;
		character = deltaLine === 0 ? character + deltaStart : deltaStart;

		if (tokenLength <= 0) {
			continue;
		}

		if ((tokenModifiers & inactiveMask) === 0) {
			continue;
		}

		ranges.push(new Range(line, character, line, character + tokenLength));
	}

	return ranges;
}

function getSemanticTokensLegend(): { tokenModifiers: string[] } | undefined {
	const provider = client?.initializeResult?.capabilities.semanticTokensProvider as
		| { legend?: { tokenModifiers?: string[] } }
		| undefined;

	if (!provider?.legend?.tokenModifiers || !Array.isArray(provider.legend.tokenModifiers)) {
		return undefined;
	}

	return {
		tokenModifiers: provider.legend.tokenModifiers
	};
}

export function notifyRemoveConfig(key: string): void {
	if (!client?.isRunning()) { return; }
	const uri = resolveKey(key).toString();
	client.sendNotification('glsld/removeConfiguration', {
		uri,
	});
}

export async function pushConfiguration(): Promise<void> {
	if (!client?.isRunning()) { return; }
	const config = workspace.getConfiguration('glsld');
	const shaderExtensions = getShaderConfigs(); // from .glsld/config.json
	const diagnosticsEnabled = config.get<boolean>('diagnostics.enabled', true);
	const glslcPath = config.get<string>('glslc.path', '').trim();
	const clangFormatPath = config.get<string>('clangFormat.path', 'clang-format.exe').trim() || 'clang-format.exe';
	const backgroundIndexRoots = getBackgroundIndexRoots(config.get<string[]>('backgroundIndex.roots', []));
	const backgroundIndexExtraExtensions = config.get<string[]>('backgroundIndex.extraExtensions', []);
	const systemIncludeDirectories = getSystemIncludeDirectories();
	const activeVariants = getActiveVariants();

	const shaderConfigs: Record<string, object> = {};
	if (shaderExtensions) {
		for (const [filePath, cfg] of Object.entries(shaderExtensions)) {
			if (cfg && typeof cfg === 'object') {
				shaderConfigs[resolveKey(filePath).toString()] = cfg;
			}
		}
	}

	// Strip VSCode config proxies before JSON-RPC serialization
	const payload = JSON.parse(JSON.stringify({
		settings: {
			glsld: {
				shaderConfigs,
				activeVariants,
				diagnosticsEnabled,
				glslcPath,
				clangFormatPath,
				backgroundIndex: { roots: backgroundIndexRoots, extraExtensions: backgroundIndexExtraExtensions },
				systemIncludeDirectories
			}
		}
	}));
	console.log('[glsld] pushConfiguration sending: %o', payload);
	await client.sendNotification('workspace/didChangeConfiguration', payload);
}

function getBackgroundIndexRoots(configuredRoots: string[]): string[] {
	const roots = new Set<string>();
	const folders = workspace.workspaceFolders ?? [];
	const workspaceFolderVariable = '${workspaceFolder}';

	for (const configuredRoot of configuredRoots) {
		if (typeof configuredRoot !== 'string') {
			continue;
		}

		const root = configuredRoot.trim();
		if (root.length === 0) {
			continue;
		}

		if (path.isAbsolute(root)) {
			roots.add(path.normalize(root));
			continue;
		}

		if (root === workspaceFolderVariable || root.startsWith(`${workspaceFolderVariable}/`) || root.startsWith(`${workspaceFolderVariable}\\`)) {
			const relativePath = root.slice(workspaceFolderVariable.length).replace(/^[/\\]+/, '');
			for (const folder of folders) {
				roots.add(path.resolve(folder.uri.fsPath, relativePath));
			}
			continue;
		}

		if (folders.length > 0) {
			roots.add(path.resolve(folders[0].uri.fsPath, root));
		}
	}

	return [...roots];
}

function getSystemIncludeDirectories(): string[] {
        const directories = new Set<string>();
        const folders = workspace.workspaceFolders;
        const workspaceFolderVariable = '${workspaceFolder}';

	if (!folders || folders.length === 0) {
		for (const directory of workspace.getConfiguration('glsld').get<string[]>('systemIncludeDirectories', [])) {
			if (typeof directory === 'string' && directory.trim().length > 0) {
				directories.add(path.normalize(directory.trim()));
			}
		}
		return [...directories];
	}

	for (const folder of folders) {
		const configuredDirectories = workspace
			.getConfiguration('glsld', folder.uri)
			.get<string[]>('systemIncludeDirectories', []);

		for (const directory of configuredDirectories) {
			if (typeof directory !== 'string' || directory.trim().length === 0) {
				continue;
                        }

                        const trimmed = directory.trim();

                        if (trimmed === workspaceFolderVariable ||
                            trimmed.startsWith(`${workspaceFolderVariable}/`) ||
                            trimmed.startsWith(`${workspaceFolderVariable}\\`)) {
                                const relativePath = trimmed
                                        .slice(workspaceFolderVariable.length)
                                        .replace(/^[/\\]+/, '');
                                directories.add(path.resolve(folder.uri.fsPath, relativePath));
                                continue;
                        }

                        directories.add(path.isAbsolute(trimmed)
                                ? path.normalize(trimmed)
                                : path.resolve(folder.uri.fsPath, trimmed));
		}
	}

	return [...directories];
}

/**
 * Resolve a user config key to a file:// URI.
 *   - "file:///..."  already a URI, pass-through
 *   - "/abs/path"    convert to Uri.file()
 *   - "rel/path"     resolve against workspace root
 */
function resolveKey(key: string): Uri {
	// Already a URI
	if (/^[a-z]+:\/\//i.test(key)) { return Uri.parse(key); }
	// Absolute path
	if (path.isAbsolute(key)) { return Uri.file(key); }
	// Workspace-relative
	const folders = workspace.workspaceFolders;
	if (folders && folders.length > 0) {
		return Uri.joinPath(folders[0].uri, key);
	}
	return Uri.file(key); // fallback
}
