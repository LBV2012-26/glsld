/* --------------------------------------------------------------------------------------------
 * Copyright (c) glsld authors. All rights reserved.
 * Licensed under the MIT License.
 * ------------------------------------------------------------------------------------------ */

import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';
import { LanguageClient } from 'vscode-languageclient/node';

// ============================================================
// Variant storage
// ============================================================

interface VariantsFile {
	variants: Record<string, Record<string, string>>;
}

let gStorageUri: vscode.Uri | null = null;

function storagePath(): string {
	const folders = vscode.workspace.workspaceFolders;
	if (folders && folders.length > 0) {
		return path.join(folders[0].uri.fsPath, '.glsld', 'variants.json');
	}
	if (gStorageUri) {
		return path.join(gStorageUri.fsPath, 'variants.json');
	}
	const home = process.env.USERPROFILE ?? process.env.HOME ?? '';
	return path.join(home, '.glsld', 'variants.json');
}

function loadVariants(): Record<string, Record<string, string>> {
	const filePath = storagePath();
	if (!fs.existsSync(filePath)) {
		const config = vscode.workspace.getConfiguration('glsld');
		return config.get<Record<string, Record<string, string>>>('shaderVariants', {});
	}
	try {
		const raw  = fs.readFileSync(filePath, 'utf-8');
		const data = JSON.parse(raw) as VariantsFile;
		return data.variants ?? {};
	} catch {
		return {};
	}
}

function saveVariants(variants: Record<string, Record<string, string>>): void {
	const filePath = storagePath();
	const dir      = path.dirname(filePath);
	if (!fs.existsSync(dir)) {
		fs.mkdirSync(dir, { recursive: true });
	}
	fs.writeFileSync(filePath, JSON.stringify({ variants }, null, '\t'), 'utf-8');
}

let cachedVariants: Record<string, Record<string, string>> | null = null;

function getVariants(): Record<string, Record<string, string>> {
	if (cachedVariants === null) {
		cachedVariants = loadVariants();
	}
	return cachedVariants;
}

function putVariants(v: Record<string, Record<string, string>>): void {
	cachedVariants = v;
	saveVariants(v);
}

// ============================================================
// Shared state
// ============================================================

let globalActive: string | null  = null;
let fileActive: string | null    = null;
let activeFileUri: string | null = null;
let selectedVariantName: string | null = null;

const onEvent = new vscode.EventEmitter<void>();

function glslEditor(): vscode.TextEditor | null {
	const e = vscode.window.activeTextEditor;
	return (e && e.document.languageId === 'glsl') ? e : null;
}

function sendSelect(client: LanguageClient, name: string | null, scope: string): void {
	const editor = glslEditor();
	if (!editor) {
		return;
	}
	const uri = editor.document.uri.toString();

	if (name === null) {
		client.sendNotification('glsld/removeVariant', {
			textDocument: { uri }, scope
		});
		return;
	}

	const variants = getVariants();
	const macros   = variants[name];
	if (!macros) {
		vscode.window.showErrorMessage(`Variant "${name}" has no macros defined.`);
		return;
	}

	client.sendNotification('glsld/selectVariant', {
		textDocument: { uri }, variant: name, macros, scope
	});
}

function refreshAll(globalProvider: GlobalVariantProvider, fileProvider: FileVariantProvider): void {
	cachedVariants = null;
	globalProvider.refresh();
	fileProvider.refresh();
	onEvent.fire();
}

// ============================================================
// Global Variant provider
// ============================================================

class GlobalVariantProvider implements vscode.TreeDataProvider<vscode.TreeItem> {
	private _onDidChangeTreeData = new vscode.EventEmitter<vscode.TreeItem | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;
	private _client: LanguageClient;

	constructor(client: LanguageClient) {
		this._client = client;
	}

	public refresh(): void { this._onDidChangeTreeData.fire(); }
	public getTreeItem(element: vscode.TreeItem): vscode.TreeItem { return element; }

	public getChildren(): vscode.TreeItem[] {
		const variants = getVariants();
		const names    = Object.keys(variants);
		const items: vscode.TreeItem[] = [];

		{
			const isActive = (globalActive === null);
			const item = new vscode.TreeItem(
				isActive ? '● Default' : '○ Default',
				vscode.TreeItemCollapsibleState.None);
			item.command = { command: 'glsld.selectVariant', title: '', arguments: [null] };
			item.description = 'No macros injected';
			items.push(item);
		}

		for (const name of names) {
			const isActive = (name === globalActive);
			const item = new vscode.TreeItem(
				isActive ? `● ${name}` : `○ ${name}`,
				vscode.TreeItemCollapsibleState.None);
			item.command = { command: 'glsld.selectVariant', title: '', arguments: [name] };
			item.tooltip = this.tooltip(name, variants[name]);
			item.contextValue = 'variant';
			items.push(item);
		}

		return items;
	}

	private tooltip(name: string, macros: Record<string, string> | undefined): string {
		if (!macros || Object.keys(macros).length === 0) { return name; }
		const lines = Object.entries(macros).map(([k, v]) => `#define ${k} ${v}`);
		return `${name}\n${lines.join('\n')}`;
	}

	public async select(name: string | null): Promise<void> {
		globalActive = name;
		this.refresh();
		onEvent.fire();
		sendSelect(this._client, name, 'global');
	}

	public async selectFile(name: string): Promise<void> {
		fileActive = name;
		onEvent.fire();
		sendSelect(this._client, name, 'file');
	}
}

// ============================================================
// File Variant provider
// ============================================================

class FileVariantProvider implements vscode.TreeDataProvider<vscode.TreeItem> {
	private _onDidChangeTreeData = new vscode.EventEmitter<vscode.TreeItem | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;
	private _client: LanguageClient;

	constructor(client: LanguageClient) { this._client = client; }
	public refresh(): void { this._onDidChangeTreeData.fire(); }
	public getTreeItem(element: vscode.TreeItem): vscode.TreeItem { return element; }

	public getChildren(): vscode.TreeItem[] {
		const variants = getVariants();
		const names    = Object.keys(variants);
		const items: vscode.TreeItem[] = [];

		if (!activeFileUri) {
			return [this.leaf('No GLSL file active')];
		}

		// 当前状态
		if (fileActive !== null) {
			items.push(this.leaf(`Overriding with: ● ${fileActive}`));
			const clear = new vscode.TreeItem('Clear Override', vscode.TreeItemCollapsibleState.None);
			clear.command = { command: 'glsld.clearFileVariant', title: '', arguments: [] };
			clear.iconPath = new vscode.ThemeIcon('close');
			items.push(clear);
		} else if (globalActive !== null) {
			items.push(this.leaf(`↳ ${globalActive} (global)`));
		} else {
			items.push(this.leaf('↳ Default (global)'));
		}

		if (names.length > 0) {
			items.push(this.leaf('──────────────'));
		}

		// 可选列表 — 点击设为文件级覆盖
		for (const name of names) {
			const isActive = (name === fileActive);
			const label = isActive ? `● ${name}` : `○ ${name}`;
			const item = new vscode.TreeItem(label, vscode.TreeItemCollapsibleState.None);
			item.command = { command: 'glsld.selectFileVariant', title: '', arguments: [name] };
			item.tooltip = variants[name]
				? Object.entries(variants[name]).map(([k, v]) => `#define ${k} ${v}`).join('\n')
				: name;
			items.push(item);
		}

		return items;
	}

	private leaf(text: string): vscode.TreeItem {
		return new vscode.TreeItem(text, vscode.TreeItemCollapsibleState.None);
	}

	public async clear(): Promise<void> {
		fileActive = null;
		this.refresh();
		sendSelect(this._client, null, 'file');
	}
}

// ============================================================
// Variant editing
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
		prompt: 'Variant name', placeHolder: 'e.g. Debug, Release, Vulkan14'
	});
	if (!name) { return; }

	const macros = await inputMacros(name);
	if (!macros) { return; }

	const all = getVariants();
	all[name] = macros;
	putVariants(all);
	onEvent.fire();
	vscode.window.showInformationMessage(`Variant "${name}" saved.`);
}

async function editVariant(name?: string): Promise<void> {
	const target = name ?? await pickVariantName('Select variant to edit');
	if (!target) { return; }

	const all = getVariants();
	const existing = all[target];
	if (!existing) { return; }

	const macros = await inputMacros(target, existing);
	if (!macros) { return; }

	all[target] = macros;
	putVariants(all);
	onEvent.fire();
	vscode.window.showInformationMessage(`Variant "${target}" updated.`);
}

async function removeVariant(name?: string): Promise<void> {
	const target = name ?? await pickVariantName('Select variant to delete');
	if (!target) { return; }

	const choice = await vscode.window.showWarningMessage(
		`Delete variant "${target}"?`, { modal: true }, 'Delete'
	);
	if (choice !== 'Delete') { return; }

	const all = getVariants();
	delete all[target];
	putVariants(all);
	if (globalActive === target) { globalActive = null; }
	if (fileActive === target)   { fileActive = null; }
	onEvent.fire();
}

async function inputMacros(
	name: string, existing?: Record<string, string>
): Promise<Record<string, string> | undefined> {
	const initialLines = existing
		? Object.entries(existing).map(([k, v]) => `${k}=${v}`)
		: ['MACRO_NAME=1'];

	const raw = await vscode.window.showInputBox({
		prompt: `Macros for "${name}" — one per line, NAME=replacement`,
		placeHolder: 'MACRO_NAME=replacement', value: initialLines.join('\n')
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

	const globalProvider = new GlobalVariantProvider(client);
	const fileProvider   = new FileVariantProvider(client);

	const editor = glslEditor();
	activeFileUri = editor ? editor.document.uri.toString() : null;

	context.subscriptions.push(
		vscode.window.registerTreeDataProvider('glsld.shaderVariant', globalProvider),
		vscode.window.registerTreeDataProvider('glsld.fileVariant', fileProvider),

		vscode.commands.registerCommand('glsld.selectVariant', (name: string | null) => {
			selectedVariantName = name;
			globalProvider.select(name);
		}),
		vscode.commands.registerCommand('glsld.selectFileVariant', (name: string) => {
			selectedVariantName = name;
			globalProvider.selectFile(name);
		}),
		vscode.commands.registerCommand('glsld.clearFileVariant', () => fileProvider.clear()),
		vscode.commands.registerCommand('glsld.addVariant', () => {
			addVariant().then(() => refreshAll(globalProvider, fileProvider));
		}),
		vscode.commands.registerCommand('glsld.editVariant', () => {
			editVariant(selectedVariantName ?? undefined).then(() => refreshAll(globalProvider, fileProvider));
		}),
		vscode.commands.registerCommand('glsld.removeVariantCmd', () => {
			removeVariant(selectedVariantName ?? undefined).then(() => refreshAll(globalProvider, fileProvider));
		}),
		vscode.commands.registerCommand('glsld.refreshVariants', () => refreshAll(globalProvider, fileProvider)),

		onEvent.event(() => { globalProvider.refresh(); fileProvider.refresh(); }),

		vscode.window.onDidChangeActiveTextEditor((editor) => {
			activeFileUri = (editor && editor.document.languageId === 'glsl')
				? editor.document.uri.toString() : null;
			fileActive = null;
			globalProvider.refresh();
			fileProvider.refresh();
		})
	);
}
