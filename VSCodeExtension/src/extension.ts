/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import {
	commands,
	Disposable,
	Position,
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

let client: LanguageClient;
let inactiveDecorationDisposable: TextEditorDecorationType | undefined;
const inactiveRangesByUri = new Map<string, Range[]>();
const scheduledUpdates = new Map<string, NodeJS.Timeout>();
const runtimeDisposables: Disposable[] = [];

export async function activate(context: any) {
	const config = workspace.getConfiguration('glsld');
	const configuredServerPath = config.get<string>('server.path', 'bin/Win64/glsld.exe').trim();
	const serverPath = path.isAbsolute(configuredServerPath) ? configuredServerPath : context.asAbsolutePath(configuredServerPath);
	const serverEnv = createServerEnvironment(config);
	const serverOutput = window.createOutputChannel('glsld');
	context.subscriptions.push(serverOutput);
	serverOutput.appendLine(formatMimallocLaunchOptions(serverEnv));

	const serverOptions: ServerOptions = {
		run: { command: serverPath, transport: TransportKind.stdio, options: { env: serverEnv } },
		debug: { command: serverPath, transport: TransportKind.stdio, options: { env: serverEnv } },
	};

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		outputChannel: serverOutput,
		// Register the server for plain text documents
		documentSelector: [{ scheme: 'file', language: 'glsl' }],
		synchronize: {
			// Notify the server about file changes to '.clientrc files contained in the workspace
			fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
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

	// Start the client. This will also launch the server
	await client.start();
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
				await pushConfiguration();
				shaderConfigProvider.refresh();
			}
		})
	);

	// Re-push shader config when a GLSL file is opened, so the server
	// can apply stored per-file configs to the now-open document.
	context.subscriptions.push(
		workspace.onDidOpenTextDocument(async (doc) => {
			if (doc.languageId === 'glsl') {
				await pushConfiguration();
			}
		})
	);

	await pushConfiguration();

}

function createServerEnvironment(config: ReturnType<typeof workspace.getConfiguration>): NodeJS.ProcessEnv {
	const environment: NodeJS.ProcessEnv = { ...process.env };
	if (process.platform !== 'win32') {
		return environment;
	}

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

export function refreshHighlights(document: TextDocument): void {
	scheduleInactiveUpdate(document, 0);
}

async function updateInactiveRanges(document: TextDocument): Promise<void> {
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
	const provider = client.initializeResult?.capabilities.semanticTokensProvider as
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
	const uri = resolveKey(key).toString();
	client.sendNotification('glsld/removeConfiguration', {
		uri,
	});
}

export async function pushConfiguration(): Promise<void> {
	const config = workspace.getConfiguration('glsld');
	const shaderExtensions = getShaderConfigs(); // from .glsld/config.json
	const diagnosticsEnabled = config.get<boolean>('diagnostics.enabled', true);
	const glslcPath = config.get<string>('glslc.path', '').trim();
	const clangFormatPath = config.get<string>('clangFormat.path', 'clang-format.exe').trim() || 'clang-format.exe';
	const inlayHints = config.get<boolean>('capabilities.inlayHints', true);
	const backgroundIndexRoots = config.get<string[]>('backgroundIndex.roots', []);
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
				capabilities: { inlayHints },
				backgroundIndex: { roots: backgroundIndexRoots },
				systemIncludeDirectories
			}
		}
	}));
	console.log('[glsld] pushConfiguration sending: %o', payload);
	await client.sendNotification('workspace/didChangeConfiguration', payload);
}

function getSystemIncludeDirectories(): string[] {
	const directories = new Set<string>();
	const folders = workspace.workspaceFolders;

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
