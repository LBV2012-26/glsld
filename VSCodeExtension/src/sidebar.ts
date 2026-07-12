/* --------------------------------------------------------------------------------------------
 * Copyright (c) glsld authors. All rights reserved.
 * Licensed under the MIT License.
 * ------------------------------------------------------------------------------------------ */

import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';
import { LanguageClient } from 'vscode-languageclient/node';

// ============================================================
// Storage types
// ============================================================

interface MacroEntry {
	value: string;
	enabled: boolean;
}

/** Per-file macro overrides: fileUri → { macroName → MacroEntry } */
interface FileMacrosMap {
	[fileUri: string]: { [macroName: string]: MacroEntry };
}

interface ConfigFile {
	variants: Record<string, Record<string, string>>;
	fileMacros?: FileMacrosMap;
	shaderConfigs?: Record<string, Record<string, string>>;
	templates?: { name: string; shaderStage?: string; version?: string; targetEnv?: string; targetSpv?: string }[];
	activeVariant?: string | null;
}

// ============================================================
// Persistent storage
// ============================================================

let gStorageUri: vscode.Uri | null = null;

function storagePath(): string {
	const folders = vscode.workspace.workspaceFolders;
	if (folders && folders.length > 0) {
		return path.join(folders[0].uri.fsPath, '.glsld', 'config.json');
	}
	if (gStorageUri) {
		return path.join(gStorageUri.fsPath, 'config.json');
	}
	const home = process.env.USERPROFILE ?? process.env.HOME ?? '';
	return path.join(home, '.glsld', 'config.json');
}

function loadFile(): ConfigFile {
	const filePath = storagePath();
	if (!fs.existsSync(filePath)) {
		// config read from file only
		return {
			variants: {},
			fileMacros: {},
		};
	}
	try {
		const raw  = fs.readFileSync(filePath, 'utf-8');
		return JSON.parse(raw) as ConfigFile;
	} catch {
		return { variants: {}, fileMacros: {}, shaderConfigs: {}, templates: [] };
	}
}

function saveFile(data: ConfigFile): void {
	const filePath = storagePath();
	const dir      = path.dirname(filePath);
	if (!fs.existsSync(dir)) {
		fs.mkdirSync(dir, { recursive: true });
	}
	fs.writeFileSync(filePath, JSON.stringify(data, null, '\t'), 'utf-8');
}

let cachedData: ConfigFile | null = null;

function getData(): ConfigFile {
	if (cachedData === null) {
		cachedData = loadFile();
	}
	return cachedData;
}

function putData(d: ConfigFile): void {
	cachedData = d;
	saveFile(d);
}

function getVariants(): Record<string, Record<string, string>> {
	return getData().variants;
}

function putVariants(v: Record<string, Record<string, string>>): void {
	const data = getData();
	data.variants = v;
	putData(data);
}

function getFileMacros(): FileMacrosMap {
	return getData().fileMacros ?? {};
}

function putFileMacros(fm: FileMacrosMap): void {
	const data = getData();
	data.fileMacros = fm;
	putData(data);
}

// ============================================================
// VariantTreeItem — carries variant name through context menus
// ============================================================

class VariantTreeItem extends vscode.TreeItem {
	variantName: string | null;

	constructor(
		label: string,
		variantName: string | null,
		collapsibleState: vscode.TreeItemCollapsibleState,
	) {
		super(label, collapsibleState);
		this.variantName = variantName;
	}
}
	function getShaderConfigs(): Record<string, Record<string, string>> { return getData().shaderConfigs ?? {}; }
	function putShaderConfigs(sc: Record<string, Record<string, string>>): void { const data = getData(); data.shaderConfigs = sc; putData(data); }
	function getTemplates() { return getData().templates ?? []; }
	function putTemplates(t: ConfigFile['templates']): void { const data = getData(); data.templates = t; putData(data); }

function resolveVariantName(arg: unknown): string | null | undefined {
	if (arg === null) { return null; }
	if (typeof arg === 'string') { return arg; }
	if (arg instanceof VariantTreeItem) { return arg.variantName; }
	return undefined;
}

// ============================================================
// File-macro tree nodes (discriminated union)
// ============================================================

type FileMacroNode = FileNode | MacroNode;

interface FileNode {
	kind: 'file';
	uri: vscode.Uri;
}

interface MacroNode {
	kind: 'macro';
	fileUri: vscode.Uri;
	macroName: string;
	macroValue: string;
	enabled: boolean;
}

// ============================================================
// Shared state
// ============================================================

let globalActive: string | null = null;
let lastClickedVariant: string | null = null;
const onEvent = new vscode.EventEmitter<void>();

/** Find any open GLSL document (not just visible editor). */
function anyGlslDocument(): vscode.TextDocument | undefined {
	return vscode.workspace.textDocuments.find(d => d.languageId === 'glsl');
}

// ============================================================
// Send macros to LSP server
// ============================================================

function sendGlobalVariant(client: LanguageClient, name: string | null): void {
	const doc = anyGlslDocument();
	if (!doc) { return; } // no GLSL file open yet — will retry on didOpen

	if (name === null) {
		client.sendNotification('glsld/removeVariant', {
			textDocument: { uri: doc.uri.toString() },
			scope: 'global',
		});
		return;
	}

	const variants = getVariants();
	const macros = variants[name];
	if (!macros) {
		vscode.window.showErrorMessage(`Variant "${name}" has no macros defined.`);
		return;
	}

	client.sendNotification('glsld/selectVariant', {
		textDocument: { uri: doc.uri.toString() },
		variant: name, macros, scope: 'global',
	});
}

function sendFileMacros(client: LanguageClient, fileUri: vscode.Uri): void {
	const allFileMacros = getFileMacros();
	const entry = allFileMacros[fileUri.toString()];

	// Collect only enabled macros
	const enabled: Record<string, string> = {};
	if (entry) {
		for (const [name, info] of Object.entries(entry)) {
			if (info.enabled) {
				enabled[name] = info.value;
			}
		}
	}

	if (Object.keys(enabled).length === 0) {
		client.sendNotification('glsld/removeVariant', {
			textDocument: { uri: fileUri.toString() },
			scope: 'file',
		});
	} else {
		client.sendNotification('glsld/selectVariant', {
			textDocument: { uri: fileUri.toString() },
			variant: '__file__',
			macros: enabled,
			scope: 'file',
		});
	}
}

// ============================================================
// Global Variant provider  (radio-button style, same as before)
// ============================================================

class GlobalVariantProvider implements vscode.TreeDataProvider<VariantTreeItem> {
	private _onDidChangeTreeData = new vscode.EventEmitter<VariantTreeItem | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;
	private _client: LanguageClient;

	constructor(client: LanguageClient) { this._client = client; }

	public refresh(): void { this._onDidChangeTreeData.fire(); }
	public getTreeItem(element: VariantTreeItem): VariantTreeItem { return element; }

	public getChildren(): VariantTreeItem[] {
		const variants = getVariants();
		const names    = Object.keys(variants);
		const items: VariantTreeItem[] = [];

		// Default — no global macros
		{
			const isActive = (globalActive === null);
			const item = new VariantTreeItem(
				'Default', null, vscode.TreeItemCollapsibleState.None,
			);
			item.iconPath = new vscode.ThemeIcon(
				isActive ? 'pass-filled' : 'circle-large-outline',
			);
			item.command = { command: 'glsld.selectVariant', title: '', arguments: [null] };
			item.description = 'No macros injected';
			item.contextValue = 'variant';
			items.push(item);
		}

		for (const name of names) {
			const isActive = (name === globalActive);
			const item = new VariantTreeItem(
				name, name, vscode.TreeItemCollapsibleState.None,
			);
			item.iconPath = new vscode.ThemeIcon(
				isActive ? 'pass-filled' : 'circle-large-outline',
			);
			item.command = { command: 'glsld.selectVariant', title: '', arguments: [name] };
			item.tooltip = this.buildTooltip(name, variants[name]);
			item.contextValue = 'variant';
			items.push(item);
		}

		return items;
	}

	private buildTooltip(name: string, macros: Record<string, string> | undefined): string {
		if (!macros || Object.keys(macros).length === 0) { return name; }
		const lines = Object.entries(macros).map(([k, v]) => `#define ${k} ${v}`);
		return `${name}\n${lines.join('\n')}`;
	}

	public async select(name: string | null): Promise<void> {
		globalActive = name;
		// Persist active variant so it survives restart
		const data = getData();
		data.activeVariant = name;
		putData(data);
		this.refresh();
		onEvent.fire();
		sendGlobalVariant(this._client, name);
	}
}

// ============================================================
// File Macro provider  (file list with per-macro checkboxes)
// ============================================================

class FileMacroProvider implements vscode.TreeDataProvider<FileMacroNode> {
	private _onDidChangeTreeData = new vscode.EventEmitter<FileMacroNode | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;
	private _client: LanguageClient;

	constructor(client: LanguageClient) { this._client = client; }

	public refresh(): void { this._onDidChangeTreeData.fire(); }
	public getTreeItem(element: FileMacroNode): vscode.TreeItem {
		if (element.kind === 'file') {
			const item = new vscode.TreeItem(
				vscode.workspace.asRelativePath(element.uri),
				vscode.TreeItemCollapsibleState.Expanded,
			);
			item.resourceUri = element.uri;
			item.iconPath = vscode.ThemeIcon.File;
			item.contextValue = 'fileMacroFile';
			const entry = getFileMacros()[element.uri.toString()];
			const enabledCount = entry
				? Object.values(entry).filter(m => m.enabled).length
				: 0;
			const totalCount = entry ? Object.keys(entry).length : 0;
			item.description = enabledCount > 0
				? `${enabledCount}/${totalCount} enabled`
				: totalCount > 0 ? `${totalCount} macros` : 'no macros';
			return item;
		}

		if (element.kind === 'macro') {
			const label = element.macroValue
				? `${element.macroName}=${element.macroValue}`
				: element.macroName;
			const item = new vscode.TreeItem(
				label, vscode.TreeItemCollapsibleState.None,
			);
			item.checkboxState = element.enabled
				? vscode.TreeItemCheckboxState.Checked
				: vscode.TreeItemCheckboxState.Unchecked;
			item.tooltip = `#define ${element.macroName} ${element.macroValue}`;
			item.contextValue = 'macro';
			return item;
		}

		throw new Error('Unknown node kind');
	}

	public getChildren(element?: FileMacroNode): FileMacroNode[] {
		if (!element) {
			// Root: list all files that have per-file macros
			const fm = getFileMacros();
			const uris = Object.keys(fm);
			if (uris.length === 0) { return []; }

			return uris.map(uri => ({
				kind: 'file' as const,
				uri: vscode.Uri.parse(uri),
			}));
		}

		if (element.kind === 'file') {
			const fm = getFileMacros();
			const entry = fm[element.uri.toString()];
			if (!entry) { return []; }

			return Object.entries(entry).map(([name, info]) => ({
				kind: 'macro' as const,
				fileUri: element.uri,
				macroName: name,
				macroValue: info.value,
				enabled: info.enabled,
			}));
		}

		return [];
	}

	public getParent(element: FileMacroNode): vscode.ProviderResult<FileMacroNode> {
		if (element.kind === 'macro') {
			return { kind: 'file', uri: element.fileUri };
		}
		return null;
	}

	/** Add a macro to a file */
	public async addMacro(fileUri: vscode.Uri): Promise<void> {
		const name = await vscode.window.showInputBox({
			prompt: 'Macro name',
			placeHolder: 'MACRO_NAME',
		});
		if (!name) { return; }

		const value = await vscode.window.showInputBox({
			prompt: `Value for ${name}`,
			placeHolder: '1',
			value: '1',
		});
		if (value === undefined) { return; }

		const fm = getFileMacros();
		const uriStr = fileUri.toString();
		if (!fm[uriStr]) { fm[uriStr] = {}; }
		fm[uriStr][name] = { value, enabled: true };
		putFileMacros(fm);

		this.refresh();
		sendFileMacros(this._client, fileUri);
	}

	/** Edit a macro */
	public async editMacro(node: MacroNode): Promise<void> {
		const name = await vscode.window.showInputBox({
			prompt: 'Macro name',
			value: node.macroName,
			placeHolder: 'MACRO_NAME',
		});
		if (!name) { return; }

		const value = await vscode.window.showInputBox({
			prompt: `Value for ${name}`,
			value: node.macroValue,
			placeHolder: '1',
		});
		if (value === undefined) { return; }

		const fm = getFileMacros();
		const uriStr = node.fileUri.toString();
		const entry = fm[uriStr];
		if (!entry) { return; }

		// If name changed, delete old key
		if (name !== node.macroName) {
			delete entry[node.macroName];
		}
		entry[name] = { value, enabled: node.enabled };
		putFileMacros(fm);

		this.refresh();
		sendFileMacros(this._client, node.fileUri);
	}

	/** Delete a macro */
	public async deleteMacro(node: MacroNode): Promise<void> {
		const choice = await vscode.window.showWarningMessage(
			`Delete macro "${node.macroName}"?`, { modal: true }, 'Delete',
		);
		if (choice !== 'Delete') { return; }

		const fm = getFileMacros();
		const uriStr = node.fileUri.toString();
		const entry = fm[uriStr];
		if (!entry) { return; }

		delete entry[node.macroName];

		// Remove file entry if no macros left
		if (Object.keys(entry).length === 0) {
			delete fm[uriStr];
		}
		putFileMacros(fm);

		this.refresh();
		sendFileMacros(this._client, node.fileUri);
	}

	/** Remove a file and all its macros */
	public async removeFile(node: FileNode): Promise<void> {
		const choice = await vscode.window.showWarningMessage(
			`Remove "${vscode.workspace.asRelativePath(node.uri)}" and all its macros?`,
			{ modal: true }, 'Remove',
		);
		if (choice !== 'Remove') { return; }

		const fm = getFileMacros();
		delete fm[node.uri.toString()];
		putFileMacros(fm);

		// Send removal to server
		this._client.sendNotification('glsld/removeVariant', {
			textDocument: { uri: node.uri.toString() },
			scope: 'file',
		});

		this.refresh();
	}

	/** Toggle a macro's enabled state (called from checkbox change) */
	public toggleMacro(node: MacroNode, checked: boolean): void {
		const fm = getFileMacros();
		const uriStr = node.fileUri.toString();
		const entry = fm[uriStr];
		if (!entry) { return; }

		const macro = entry[node.macroName];
		if (!macro) { return; }

		macro.enabled = checked;
		putFileMacros(fm);

		this.refresh();
		sendFileMacros(this._client, node.fileUri);
	}
}

// ============================================================
// Variant editing (add / edit / delete named variant sets)
// ============================================================

function pickVariantName(title: string): Thenable<string | undefined> {
	const variants = getVariants();
	const names    = Object.keys(variants);
	if (names.length === 0) {
		vscode.window.showInformationMessage('No variants configured. Click + to add one.');
		return Promise.resolve(undefined);
	}
	if (names.length === 1) { return Promise.resolve(names[0]); }
	return vscode.window.showQuickPick(names, { placeHolder: title });
}

async function addVariant(): Promise<void> {
	const name = await vscode.window.showInputBox({
		prompt: 'Variant name', placeHolder: 'e.g. Debug, Release, Vulkan14',
	});
	if (!name) { return; }

	const existing = getVariants();
	if (existing[name]) {
		const overwrite = await vscode.window.showWarningMessage(
			`Variant "${name}" already exists. Overwrite?`, { modal: true }, 'Overwrite',
		);
		if (overwrite !== 'Overwrite') { return; }
	}

	const macros = await inputMacros(name);
	if (!macros) { return; }

	existing[name] = macros;
	putVariants(existing);
	onEvent.fire();
	vscode.window.showInformationMessage(`Variant "${name}" saved.`);
}

async function editVariant(name?: string | null): Promise<void> {
	const target = name ?? await pickVariantName('Select variant to edit');
	if (!target) { return; }

	const all = getVariants();
	const ex = all[target];
	if (!ex) {
		vscode.window.showErrorMessage(`Variant "${target}" not found.`);
		return;
	}

	const macros = await inputMacros(target, ex);
	if (!macros) { return; }

	all[target] = macros;
	putVariants(all);
	onEvent.fire();
	vscode.window.showInformationMessage(`Variant "${target}" updated.`);
}

async function removeVariant(name?: string | null): Promise<void> {
	const target = name ?? await pickVariantName('Select variant to delete');
	if (!target) { return; }

	const choice = await vscode.window.showWarningMessage(
		`Delete variant "${target}"?`, { modal: true }, 'Delete',
	);
	if (choice !== 'Delete') { return; }

	const all = getVariants();
	delete all[target];
	putVariants(all);
	if (globalActive === target) { globalActive = null; }
	onEvent.fire();
}

async function inputMacros(
	name: string, existing?: Record<string, string>,
): Promise<Record<string, string> | undefined> {
	const initialLines = existing
		? Object.entries(existing).map(([k, v]) => `${k}=${v}`)
		: ['MACRO_NAME=1'];

	const raw = await vscode.window.showInputBox({
		prompt: `Macros for "${name}" — one per line, NAME=replacement`,
		placeHolder: 'MACRO_NAME=replacement',
		value: initialLines.join('\n'),
	});
	if (raw === undefined) { return undefined; }

	const macros: Record<string, string> = {};
	for (const line of raw.split('\n')) {
		const trimmed = line.trim();
		if (trimmed.length === 0) { continue; }
		const eq = trimmed.indexOf('=');
		if (eq <= 0) { continue; }
		const key = trimmed.substring(0, eq).trim();
		const value = trimmed.substring(eq + 1).trim();
		if (key.length > 0) { macros[key] = value; }
	}
	return macros;
}

// ============================================================
// Registration
// ============================================================

export function activateSidebar(context: vscode.ExtensionContext, client: LanguageClient): void {
	gStorageUri = context.globalStorageUri;

	// Restore last active variant from config
	const data = getData();
	globalActive = data.activeVariant ?? null;

	const globalProvider = new GlobalVariantProvider(client);
	const fileProvider   = new FileMacroProvider(client);

	// Use createTreeView for File view — we need checkbox events
	const fileTreeView = vscode.window.createTreeView('glsld.fileVariant', {
		treeDataProvider: fileProvider,
	});

	context.subscriptions.push(
		// Global: simple TreeDataProvider is enough (no checkboxes)
		vscode.window.registerTreeDataProvider('glsld.shaderVariant', globalProvider),
		fileTreeView,

		// --- File view checkbox toggle ---
		fileTreeView.onDidChangeCheckboxState((e: vscode.TreeCheckboxChangeEvent<FileMacroNode>) => {
			for (const [node, state] of e.items) {
				if (node.kind === 'macro') {
					const checked = state === vscode.TreeItemCheckboxState.Checked;
					fileProvider.toggleMacro(node, checked);
				}
			}
		}),

		// --- Select global variant ---
		vscode.commands.registerCommand('glsld.selectVariant', (arg: unknown) => {
			const name = resolveVariantName(arg);
			if (name !== undefined) {
				lastClickedVariant = name;
			}
			globalProvider.select(name ?? null);
		}),

		// --- Select file variant (no longer used in new model; kept for compat) ---
		vscode.commands.registerCommand('glsld.selectFileVariant', (_arg: unknown) => {
			// Now handled by checkbox toggles in the File view
			vscode.window.showInformationMessage(
				'Use the File Variant view to toggle per-file macros.',
			);
		}),

		// --- Clear file override (no longer applicable; kept for compat) ---
		vscode.commands.registerCommand('glsld.clearFileVariant', () => {
			vscode.window.showInformationMessage(
				'Use the File Variant view to manage per-file macros.',
			);
		}),

		// --- Add variant (named macro set) ---
		vscode.commands.registerCommand('glsld.addVariant', () => {
			addVariant().then(() => globalProvider.refresh());
		}),

		// --- Edit variant ---
		vscode.commands.registerCommand('glsld.editVariant', (arg: unknown) => {
			const name = resolveVariantName(arg) ?? lastClickedVariant;
			editVariant(name).then(() => globalProvider.refresh());
		}),

		// --- Delete variant ---
		vscode.commands.registerCommand('glsld.removeVariantCmd', (arg: unknown) => {
			const name = resolveVariantName(arg) ?? lastClickedVariant;
			removeVariant(name).then(() => globalProvider.refresh());
		}),

		// --- Refresh views ---
		vscode.commands.registerCommand('glsld.refreshVariants', () => {
			cachedData = null;
			globalProvider.refresh();
			fileProvider.refresh();
			onEvent.fire();
		}),

		// --- Add current file to File view ---
		vscode.commands.registerCommand('glsld.addFileMacro', async () => {
			const editor = vscode.window.activeTextEditor;
			if (!editor || editor.document.languageId !== 'glsl') {
				vscode.window.showWarningMessage('Open a GLSL file first.');
				return;
			}
			await fileProvider.addMacro(editor.document.uri);
		}),

		// --- Edit a file macro (context menu) ---
		vscode.commands.registerCommand('glsld.editFileMacro', (node: MacroNode) => {
			if (node && node.kind === 'macro') {
				fileProvider.editMacro(node);
			}
		}),

		// --- Delete a file macro (context menu) ---
		vscode.commands.registerCommand('glsld.deleteFileMacro', (node: MacroNode) => {
			if (node && node.kind === 'macro') {
				fileProvider.deleteMacro(node);
			}
		}),

		// --- Remove a file from File view ---
		vscode.commands.registerCommand('glsld.removeFileFromList', (node: FileNode) => {
			if (node && node.kind === 'file') {
				fileProvider.removeFile(node);
			}
		}),

		// --- Sync between views ---
		onEvent.event(() => {
			globalProvider.refresh();
			fileProvider.refresh();
		}),

		// When a GLSL file is opened, push macro state to server.
		// This handles the case where config was loaded before any GLSL
		// document existed (e.g. right after extension activation).
		vscode.workspace.onDidOpenTextDocument((doc) => {
			if (doc.languageId === 'glsl') {
				pushMacroState(client, doc.uri);
			}
		}),

		// When switching GLSL files, push per-file macros
		vscode.window.onDidChangeActiveTextEditor(() => {
			const editor = vscode.window.activeTextEditor;
			if (editor && editor.document.languageId === 'glsl') {
				pushMacroState(client, editor.document.uri);
			}
		}),
	);
}

	// ============================================================
	// Public accessors  (used by compile.ts etc.)
	// ============================================================

	/** Returns the currently active global variant name, or null. */
	export function getActiveGlobalVariant(): string | null {
		return globalActive;
	}

	/** Returns the raw per-file macro map. */
	export function getFileMacrosData(): FileMacrosMap {
		return getFileMacros();
	}

	export { getShaderConfigs, putShaderConfigs, getTemplates, putTemplates };
	export type { FileMacrosMap };

	/** Push active variant + per-file macros to server on startup / file open. */
	export function pushMacroState(client: LanguageClient, fileUri?: vscode.Uri): void {
		// Determine which documents to push for
		const targets = fileUri
			? [fileUri]
			: vscode.workspace.textDocuments
				.filter(d => d.languageId === 'glsl')
				.map(d => d.uri);

		if (targets.length === 0) { return; }

		// Global variant — use first available document URI as reference
		if (globalActive !== null) {
			const variants = getVariants();
			const macros = variants[globalActive];
			if (macros) {
				client.sendNotification('glsld/selectVariant', {
					textDocument: { uri: targets[0].toString() },
					variant: globalActive, macros, scope: 'global',
				});
			}
		}

		// Per-file macros
		for (const uri of targets) {
			sendFileMacros(client, uri);
		}
	}