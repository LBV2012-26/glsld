/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import {
	commands,
	Disposable,
	Range,
	TextDocument,
	TextEditorDecorationType,
	Uri,
	window,
	workspace
} from 'vscode';

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

import { activateSidebar, getShaderConfigs } from './sidebar';
import { compileWorkspace } from './compile';
import * as path from 'path';
import { ShaderConfigProvider } from './shaderConfig';

let client: LanguageClient;
let inactiveDecorationDisposable: TextEditorDecorationType | undefined;
const inactiveRangesByUri = new Map<string, Range[]>();
const scheduledUpdates = new Map<string, NodeJS.Timeout>();
const runtimeDisposables: Disposable[] = [];

export async function activate(context: any) {
	const config     = workspace.getConfiguration('glsld');
	const serverPath = config.get<string>('server.path', 'glsld');

	const serverOptions: ServerOptions = {
		run:   { command: serverPath, transport: TransportKind.stdio },
		debug: { command: serverPath, transport: TransportKind.stdio },
	};

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
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
		'GLSL Language Server',
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

	// Push shader config to server when settings change
	context.subscriptions.push(
		workspace.onDidChangeConfiguration(async (e) => {
			if (e.affectsConfiguration('glsld')) {
				await pushShaderConfig();
				shaderConfigProvider.refresh();
			}
		})
	);
	await pushShaderConfig();
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

export async function pushShaderConfig(): Promise<void> {
	const config = workspace.getConfiguration('glsld');
	const shaderExtensions = getShaderConfigs(); // from .glsld/config.json
	const diagnosticsEnabled = config.get<boolean>('diagnostics.enabled', true);

	const shaderConfig: Record<string, object> = {};
	if (shaderExtensions) {
		for (const [filePath, cfg] of Object.entries(shaderExtensions)) {
			if (cfg && typeof cfg === 'object') {
				shaderConfig[resolveKey(filePath).toString()] = cfg;
			}
		}
	}

		// Strip VSCode config proxies before JSON-RPC serialization
		const payload = JSON.parse(JSON.stringify({
			settings: {
				glsld: {
					shaderConfig,
					diagnosticsEnabled
				}
			}
		}));
		console.log('[glsld] pushShaderConfig sending: %o', payload);
		await client.sendNotification('workspace/didChangeConfiguration', payload);
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