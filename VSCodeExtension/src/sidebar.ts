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
// Shader Variant provider
// ============================================================

class ShaderVariantProvider implements vscode.TreeDataProvider<vscode.TreeItem> {
	private _onDidChangeTreeData = new vscode.EventEmitter<vscode.TreeItem | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;

	private _activeVariant: string | null = null;
	private _client: LanguageClient;

	constructor(client: LanguageClient) {
		this._client = client;
	}

	public refresh(): void {
		cachedVariants = null;
		this._onDidChangeTreeData.fire();
	}

	public getTreeItem(element: vscode.TreeItem): vscode.TreeItem {
		return element;
	}

	public getChildren(): vscode.TreeItem[] {
		const variants = getVariants();
		const names    = Object.keys(variants);
		const items: vscode.TreeItem[] = [];

		{
			const isActive = (this._activeVariant === null);
			const item     = new vscode.TreeItem(
				isActive ? '● Default' : '○ Default',
				vscode.TreeItemCollapsibleState.None);
			item.command   = { command: 'glsld.selectVariant', title: '', arguments: [null] };
			item.description = 'No macros injected';
			items.push(item);
		}

		for (const name of names) {
			const isActive = (name === this._activeVariant);
			const item     = new vscode.TreeItem(
				isActive ? `● ${name}` : `○ ${name}`,
				vscode.TreeItemCollapsibleState.None);
			item.command   = { command: 'glsld.selectVariant', title: '', arguments: [name] };
			item.tooltip   = this.tooltip(name, variants[name]);
			item.contextValue = 'variant';
			items.push(item);
		}

		return items;
	}

	private tooltip(name: string, macros: Record<string, string> | undefined): string {
		if (!macros || Object.keys(macros).length === 0) {
			return name;
		}
		const lines = Object.entries(macros).map(([k, v]) => `#define ${k} ${v}`);
		return `${name}\n${lines.join('\n')}`;
	}

	public async select(name: string | null): Promise<void> {
		this._activeVariant = name;
		this.refresh();

		const editor = vscode.window.activeTextEditor;
		if (!editor || editor.document.languageId !== 'glsl') {
			return;
		}

		if (name === null) {
			await this._client.sendNotification('glsld/removeVariant', {
				textDocument: { uri: editor.document.uri.toString() }
			});
			return;
		}

		const variants = getVariants();
		const macros   = variants[name];
		if (!macros) {
			vscode.window.showErrorMessage(`Variant "${name}" has no macros defined.`);
			return;
		}

		await this._client.sendNotification('glsld/selectVariant', {
			textDocument: { uri: editor.document.uri.toString() },
			variant: name,
			macros
		});
	}

	public async add(): Promise<void> {
		const name = await vscode.window.showInputBox({
			prompt: 'Variant name',
			placeHolder: 'e.g. Debug, Release, Vulkan14'
		});
		if (!name) {
			return;
		}

		const macros = await this.inputMacros(name);
		if (!macros) {
			return;
		}

		const all = getVariants();
		all[name] = macros;
		putVariants(all);
		this.refresh();

		vscode.window.showInformationMessage(`Variant "${name}" saved.`);
	}

	public async edit(name: string): Promise<void> {
		const all = getVariants();
		const existing = all[name];
		if (!existing) {
			return;
		}

		const macros = await this.inputMacros(name, existing);
		if (!macros) {
			return;
		}

		all[name] = macros;
		putVariants(all);
		this.refresh();

		vscode.window.showInformationMessage(`Variant "${name}" updated.`);
	}

	public async remove(name: string): Promise<void> {
		const choice = await vscode.window.showWarningMessage(
			`Delete variant "${name}"?`,
			{ modal: true },
			'Delete'
		);
		if (choice !== 'Delete') {
			return;
		}

		const all = getVariants();
		delete all[name];
		putVariants(all);

		if (this._activeVariant === name) {
			this._activeVariant = null;
		}

		this.refresh();
	}

	private async inputMacros(
		name: string,
		existing?: Record<string, string>
	): Promise<Record<string, string> | undefined> {
		const initialLines = existing
			? Object.entries(existing).map(([k, v]) => `${k}=${v}`)
			: ['MACRO_NAME=1'];

		const raw = await vscode.window.showInputBox({
			prompt: `Macros for "${name}" — one per line, NAME=replacement`,
			placeHolder: 'MACRO_NAME=replacement',
			value: initialLines.join('\n')
		});
		if (raw === undefined) {
			return undefined;
		}

		const macros: Record<string, string> = {};
		for (const line of raw.split('\n')) {
			const trimmed = line.trim();
			if (trimmed.length === 0) {
				continue;
			}

			const eq = trimmed.indexOf('=');
			if (eq <= 0) {
				continue;
			}

			const key   = trimmed.substring(0, eq).trim();
			const value = trimmed.substring(eq + 1).trim();
			if (key.length > 0) {
				macros[key] = value;
			}
		}

		return macros;
	}
}

// ============================================================
// Registration
// ============================================================

export function activateSidebar(context: vscode.ExtensionContext, client: LanguageClient): void {
	gStorageUri = context.globalStorageUri;

	const variantProvider = new ShaderVariantProvider(client);

	context.subscriptions.push(
		vscode.window.registerTreeDataProvider('glsld.shaderVariant', variantProvider),

		vscode.commands.registerCommand('glsld.selectVariant', (name: string | null) => {
			variantProvider.select(name);
		}),
		vscode.commands.registerCommand('glsld.addVariant', () => {
			variantProvider.add();
		}),
		vscode.commands.registerCommand('glsld.editVariant', (name: string) => {
			variantProvider.edit(name);
		}),
		vscode.commands.registerCommand('glsld.removeVariantCmd', (name: string) => {
			variantProvider.remove(name);
		}),
		vscode.commands.registerCommand('glsld.refreshVariants', () => {
			variantProvider.refresh();
		}),

		vscode.window.onDidChangeActiveTextEditor(() => {
			variantProvider.refresh();
		})
	);
}
