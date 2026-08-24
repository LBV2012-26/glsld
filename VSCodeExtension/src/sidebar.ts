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

type MacrosMap = Record<string, MacroEntry>;

/** Per-file macro overrides: fileUri → { macroName → MacroEntry } */
interface FileMacrosMap {
	[fileUri: string]: { [macroName: string]: MacroEntry };
}

export interface CompileVariant {
	/** Short name inserted by the {variant} command placeholder. */
	name: string;
	/** Macro defines for this compilation variant. */
	defines: Record<string, string>;
}

export interface CompileGroup {
	/** Display name shown in the Compile Groups view. */
	name: string;
	/** glslc command template; see the Compile Groups UI for supported placeholders. */
	command: string;
	/** Workspace glob patterns for the source files in this group. */
	include: string[];
	/** Optional per-group macro variants. */
	variants?: CompileVariant[];
}

interface ConfigFile {
	globalMacros?: MacrosMap;
	variants?: Record<string, Record<string, string>>;
	fileMacros?: FileMacrosMap;
	shaderConfigs?: Record<string, Record<string, string>>;
	templates?: { name: string; shaderStage?: string; version?: string; targetEnv?: string; targetSpv?: string }[];
	activeVariant?: string | null;
	compileGroups?: CompileGroup[];
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

function getGlobalMacros(): MacrosMap {
	return getData().globalMacros ?? {};
}

function putGlobalMacros(macros: MacrosMap): void {
	const data = getData();
	data.globalMacros = macros;
	putData(data);
}

function migrateLegacyGlobalVariant(): void {
	const data = getData();
	if (data.globalMacros !== undefined) { return; }

	data.globalMacros = {};
	for (const [variantName, macros] of Object.entries(data.variants ?? {})) {
		for (const [name, value] of Object.entries(macros)) {
			if (!data.globalMacros[name]) { data.globalMacros[name] = { value, enabled: variantName === data.activeVariant }; }
		}
	}

	const selected = data.activeVariant ? data.variants?.[data.activeVariant] : undefined;
	if (selected) {
		for (const [name, value] of Object.entries(selected)) {
			data.globalMacros[name] = { value, enabled: true };
		}
	}
	delete data.variants;
	delete data.activeVariant;
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

export function getCompileGroups(): CompileGroup[] {
	const groups = getData().compileGroups;
	return Array.isArray(groups) ? groups : [];
}

export function putCompileGroups(groups: CompileGroup[]): void {
	const data = getData();
	data.compileGroups = groups;
	putData(data);
}

/**
 * Move the pre-0.2 setting into the project configuration once.  Keeping this
 * here makes the migration available even after compileGroups disappears from
 * package.json's settings schema.
 */
function migrateLegacyCompileGroups(): number {
	const data = getData();
	if (data.compileGroups !== undefined) { return 0; }

	const legacy = vscode.workspace
		.getConfiguration('glsld')
		.get<CompileGroup[]>('compileGroups', []);
	if (!Array.isArray(legacy) || legacy.length === 0) { return 0; }

	data.compileGroups = legacy;
	putData(data);
	return legacy.length;
}

	function getShaderConfigs(): Record<string, Record<string, string>> { return getData().shaderConfigs ?? {}; }
	function putShaderConfigs(sc: Record<string, Record<string, string>>): void { const data = getData(); data.shaderConfigs = sc; putData(data); }
	function getTemplates() { return getData().templates ?? []; }
	function putTemplates(t: ConfigFile['templates']): void { const data = getData(); data.templates = t; putData(data); }

// ============================================================
// Macro tree nodes
// ============================================================

interface GlobalMacroNode {
	kind: 'globalMacro';
	macroName: string;
	macroValue: string;
	enabled: boolean;
}

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

const onEvent = new vscode.EventEmitter<void>();

/** Find any open GLSL document (not just visible editor). */
function anyGlslDocument(): vscode.TextDocument | undefined {
	return vscode.workspace.textDocuments.find(d => d.languageId === 'glsl');
}

// ============================================================
// Send macros to LSP server
// ============================================================

function sendGlobalMacros(client: LanguageClient): void {
	const doc = anyGlslDocument();
	if (!doc) { return; }

	const macros: Record<string, string> = {};
	for (const [name, entry] of Object.entries(getGlobalMacros())) {
		if (entry.enabled) { macros[name] = entry.value; }
	}

	if (Object.keys(macros).length === 0) {
		client.sendNotification('glsld/removeVariant', {
			textDocument: { uri: doc.uri.toString() },
			scope: 'global',
		});
		return;
	}

	client.sendNotification('glsld/selectVariant', {
		textDocument: { uri: doc.uri.toString() },
		variant: '__global__', macros, scope: 'global',
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

async function inputMacro(existingName = '', existingValue = '1'): Promise<{ name: string; value: string } | undefined> {
	const name = await vscode.window.showInputBox({ prompt: 'Macro name', placeHolder: 'MACRO_NAME', value: existingName });
	if (!name) { return undefined; }

	const value = await vscode.window.showInputBox({ prompt: `Value for ${name}`, placeHolder: '1', value: existingValue });
	if (value === undefined) { return undefined; }
	return { name, value };
}

// ============================================================
// Global macro provider
// ============================================================

class GlobalMacroProvider implements vscode.TreeDataProvider<GlobalMacroNode> {
	private _onDidChangeTreeData = new vscode.EventEmitter<GlobalMacroNode | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;
	private _client: LanguageClient;

	constructor(client: LanguageClient) { this._client = client; }

	public refresh(): void { this._onDidChangeTreeData.fire(); }
	public getTreeItem(element: GlobalMacroNode): vscode.TreeItem {
		const label = element.macroValue ? `${element.macroName}=${element.macroValue}` : element.macroName;
		const item = new vscode.TreeItem(label, vscode.TreeItemCollapsibleState.None);
		item.checkboxState = element.enabled ? vscode.TreeItemCheckboxState.Checked : vscode.TreeItemCheckboxState.Unchecked;
		item.tooltip = `#define ${element.macroName} ${element.macroValue}`;
		item.contextValue = 'globalMacro';
		return item;
	}

	public getChildren(): GlobalMacroNode[] {
		return Object.entries(getGlobalMacros()).map(([name, entry]) => ({ kind: 'globalMacro', macroName: name, macroValue: entry.value, enabled: entry.enabled }));
	}

	public async addMacro(): Promise<void> {
		const input = await inputMacro();
		if (!input) { return; }

		const macros = getGlobalMacros();
		macros[input.name] = { value: input.value, enabled: true };
		putGlobalMacros(macros);
		this.refresh();
		onEvent.fire();
		sendGlobalMacros(this._client);
	}

	public async editMacro(node: GlobalMacroNode): Promise<void> {
		const input = await inputMacro(node.macroName, node.macroValue);
		if (!input) { return; }

		const macros = getGlobalMacros();
		if (input.name !== node.macroName) { delete macros[node.macroName]; }
		macros[input.name] = { value: input.value, enabled: node.enabled };
		putGlobalMacros(macros);
		this.refresh();
		onEvent.fire();
		sendGlobalMacros(this._client);
	}

	public async deleteMacro(node: GlobalMacroNode): Promise<void> {
		const choice = await vscode.window.showWarningMessage(`Delete macro "${node.macroName}"?`, { modal: true }, 'Delete');
		if (choice !== 'Delete') { return; }

		const macros = getGlobalMacros();
		delete macros[node.macroName];
		putGlobalMacros(macros);
		this.refresh();
		onEvent.fire();
		sendGlobalMacros(this._client);
	}

	public toggleMacro(node: GlobalMacroNode, checked: boolean): void {
		const macros = getGlobalMacros();
		const macro = macros[node.macroName];
		if (!macro) { return; }

		macro.enabled = checked;
		putGlobalMacros(macros);
		this.refresh();
		onEvent.fire();
		sendGlobalMacros(this._client);
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
		const input = await inputMacro();
		if (!input) { return; }

		const fm = getFileMacros();
		const uriStr = fileUri.toString();
		if (!fm[uriStr]) { fm[uriStr] = {}; }
		fm[uriStr][input.name] = { value: input.value, enabled: true };
		putFileMacros(fm);

		this.refresh();
		sendFileMacros(this._client, fileUri);
	}

	/** Edit a macro */
	public async editMacro(node: MacroNode): Promise<void> {
		const input = await inputMacro(node.macroName, node.macroValue);
		if (!input) { return; }

		const fm = getFileMacros();
		const uriStr = node.fileUri.toString();
		const entry = fm[uriStr];
		if (!entry) { return; }

		// If name changed, delete old key
		if (input.name !== node.macroName) {
			delete entry[node.macroName];
		}
		entry[input.name] = { value: input.value, enabled: node.enabled };
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
// Compile group provider
// ============================================================

type CompileGroupNode = CompileGroupEntry | CompileGroupProperty;

interface CompileGroupEntry {
	kind: 'compileGroup';
	index: number;
}

interface CompileGroupProperty {
	kind: 'compileGroupProperty';
	groupIndex: number;
	label: string;
	value: string;
}

const DEFAULT_COMPILE_COMMAND = 'glslc --target-env=vulkan1.3 {defines} -o "{}.spv" "{}"';

function splitCompilePatterns(value: string): string[] {
	return value
		.split(/[\n,]/)
		.map(pattern => pattern.trim())
		.filter(pattern => pattern.length > 0);
}

function formatCompileVariants(variants: CompileVariant[] | undefined): string {
	if (!variants || variants.length === 0) { return ''; }
	return variants
		.map(variant => {
			const defines = Object.entries(variant.defines)
				.map(([name, value]) => `${name}=${value}`)
				.join(', ');
			return `${variant.name}: ${defines}`;
		})
		.join('; ');
}

function parseCompileVariants(value: string): CompileVariant[] {
	if (value.trim().length === 0) { return []; }

	return value.split(';').map(rawVariant => {
		const variantText = rawVariant.trim();
		const colon = variantText.indexOf(':');
		if (colon <= 0) {
			throw new Error('Each variant must use the form name: MACRO=value, OTHER=value.');
		}

		const name = variantText.substring(0, colon).trim();
		if (name.length === 0) {
			throw new Error('Variant name cannot be empty.');
		}

		const defines: Record<string, string> = {};
		const rawDefines = variantText.substring(colon + 1).trim();
		if (rawDefines.length > 0) {
			for (const rawDefine of rawDefines.split(',')) {
				const define = rawDefine.trim();
				const equals = define.indexOf('=');
				if (equals <= 0) {
					throw new Error(`Invalid define "${define}" in variant "${name}".`);
				}
				const macro = define.substring(0, equals).trim();
				if (macro.length === 0) {
					throw new Error(`Invalid define in variant "${name}".`);
				}
				defines[macro] = define.substring(equals + 1).trim();
			}
		}

		return { name, defines };
	});
}

class CompileGroupProvider implements vscode.TreeDataProvider<CompileGroupNode> {
	private readonly onDidChangeTreeDataEmitter = new vscode.EventEmitter<CompileGroupNode | undefined | void>();
	readonly onDidChangeTreeData = this.onDidChangeTreeDataEmitter.event;

	public refresh(): void { this.onDidChangeTreeDataEmitter.fire(); }

	public getTreeItem(element: CompileGroupNode): vscode.TreeItem {
		if (element.kind === 'compileGroup') {
			const group = getCompileGroups()[element.index];
			const item = new vscode.TreeItem(
				group?.name ?? '(deleted compile group)',
				vscode.TreeItemCollapsibleState.Expanded,
			);
			item.iconPath = new vscode.ThemeIcon('gear');
			item.contextValue = 'compileGroup';
			item.command = { command: 'glsld.editCompileGroup', title: 'Edit Compile Group', arguments: [element] };
			if (group) {
				const variantCount = group.variants?.length ?? 0;
				item.description = `${group.include?.length ?? 0} pattern(s)${variantCount > 0 ? ` · ${variantCount} variant(s)` : ''}`;
				item.tooltip = [
					`Command: ${group.command}`,
					`Include: ${(group.include ?? []).join(', ')}`,
					variantCount > 0 ? `Variants: ${formatCompileVariants(group.variants)}` : 'Variants: none',
				].join('\n');
			}
			return item;
		}

		const item = new vscode.TreeItem(element.label, vscode.TreeItemCollapsibleState.None);
		item.description = element.value || '(none)';
		item.iconPath = new vscode.ThemeIcon('symbol-property');
		item.contextValue = 'compileGroupProperty';
		return item;
	}

	public getChildren(element?: CompileGroupNode): CompileGroupNode[] {
		if (!element) {
			return getCompileGroups().map((_, index) => ({ kind: 'compileGroup' as const, index }));
		}
		if (element.kind !== 'compileGroup') { return []; }

		const group = getCompileGroups()[element.index];
		if (!group) { return []; }

		const children: CompileGroupProperty[] = [{
			kind: 'compileGroupProperty', groupIndex: element.index,
			label: 'Command', value: group.command,
		}];
		for (const pattern of group.include ?? []) {
			children.push({
				kind: 'compileGroupProperty', groupIndex: element.index,
				label: 'Include', value: pattern,
			});
		}
		if (group.variants && group.variants.length > 0) {
			children.push({
				kind: 'compileGroupProperty', groupIndex: element.index,
				label: 'Variants', value: formatCompileVariants(group.variants),
			});
		}
		return children;
	}

	public async addGroup(): Promise<void> {
		const group = await this.inputGroup();
		if (!group) { return; }

		const groups = [...getCompileGroups(), group];
		putCompileGroups(groups);
		this.refresh();
		vscode.window.showInformationMessage(`Compile group "${group.name}" saved to .glsld/config.json.`);
	}

	public async editGroup(node?: CompileGroupEntry): Promise<void> {
		const groups = [...getCompileGroups()];
		let index = node?.kind === 'compileGroup' ? node.index : undefined;
		if (index === undefined) {
			const selected = await vscode.window.showQuickPick(
				groups.map((group, groupIndex) => ({
					label: group.name,
					description: `${group.include?.length ?? 0} pattern(s)`,
					groupIndex,
				})),
				{ placeHolder: 'Select a compile group to edit' },
			);
			index = selected?.groupIndex;
		}
		if (index === undefined || !groups[index]) { return; }

		const group = await this.inputGroup(groups[index], index);
		if (!group) { return; }
		groups[index] = group;
		putCompileGroups(groups);
		this.refresh();
		vscode.window.showInformationMessage(`Compile group "${group.name}" updated.`);
	}

	public async removeGroup(node?: CompileGroupEntry): Promise<void> {
		const groups = [...getCompileGroups()];
		let index = node?.kind === 'compileGroup' ? node.index : undefined;
		if (index === undefined) {
			const selected = await vscode.window.showQuickPick(
				groups.map((group, groupIndex) => ({ label: group.name, groupIndex })),
				{ placeHolder: 'Select a compile group to remove' },
			);
			index = selected?.groupIndex;
		}
		if (index === undefined || !groups[index]) { return; }

		const group = groups[index];
		const choice = await vscode.window.showWarningMessage(
			`Remove compile group "${group.name}" from .glsld/config.json?`,
			{ modal: true }, 'Remove',
		);
		if (choice !== 'Remove') { return; }

		groups.splice(index, 1);
		putCompileGroups(groups);
		this.refresh();
	}

	private async inputGroup(existing?: CompileGroup, index?: number): Promise<CompileGroup | undefined> {
		const name = await vscode.window.showInputBox({
			title: existing ? 'Edit Compile Group' : 'Add Compile Group',
			prompt: 'Group name',
			placeHolder: 'e.g. Vulkan shaders',
			value: existing?.name ?? '',
			validateInput: value => {
				const trimmed = value.trim();
				if (trimmed.length === 0) { return 'Group name cannot be empty.'; }
				const duplicate = getCompileGroups().some((group, groupIndex) =>
					groupIndex !== index && group.name === trimmed,
				);
				return duplicate ? `A compile group named "${trimmed}" already exists.` : undefined;
			},
		});
		if (name === undefined) { return undefined; }

		const includeText = await vscode.window.showInputBox({
			title: 'Compile Group Source Files',
			prompt: 'Glob patterns, separated by commas',
			placeHolder: '**/*.vert, **/*.frag, **/*.comp',
			value: (existing?.include ?? []).join(', '),
			validateInput: value => splitCompilePatterns(value).length > 0
				? undefined : 'At least one source glob is required.',
		});
		if (includeText === undefined) { return undefined; }

		const command = await vscode.window.showInputBox({
			title: 'Compile Group Command',
			prompt: 'Command template. Supports {}, {defines}, {variant}, {dir}, and {name}.',
			value: existing?.command ?? DEFAULT_COMPILE_COMMAND,
			validateInput: value => value.trim().length > 0 ? undefined : 'Command cannot be empty.',
		});
		if (command === undefined) { return undefined; }

		const variantsText = await vscode.window.showInputBox({
			title: 'Compile Group Variants',
			prompt: 'Optional: name: MACRO=value, OTHER=value; second: MACRO=value',
			placeHolder: 'debug: DEBUG=1; release: OPTIMIZED=1',
			value: formatCompileVariants(existing?.variants),
			validateInput: value => {
				try {
					parseCompileVariants(value);
					return undefined;
				} catch (error) {
					return error instanceof Error ? error.message : 'Invalid variant list.';
				}
			},
		});
		if (variantsText === undefined) { return undefined; }

		return {
			name: name.trim(),
			include: splitCompilePatterns(includeText),
			command: command.trim(),
			variants: parseCompileVariants(variantsText),
		};
	}
}

// ============================================================
// Registration
// ============================================================

export function activateSidebar(context: vscode.ExtensionContext, client: LanguageClient): void {
	gStorageUri = context.globalStorageUri;
	migrateLegacyGlobalVariant();
	const migratedCompileGroups = migrateLegacyCompileGroups();

	const globalProvider = new GlobalMacroProvider(client);
	const fileProvider   = new FileMacroProvider(client);
	const compileProvider = new CompileGroupProvider();

	if (migratedCompileGroups > 0) {
		vscode.window.showInformationMessage(
			`Moved ${migratedCompileGroups} compile group(s) to .glsld/config.json.`,
		);
	}

	const globalTreeView = vscode.window.createTreeView('glsld.shaderVariant', {
		treeDataProvider: globalProvider,
	});
	const fileTreeView = vscode.window.createTreeView('glsld.fileVariant', {
		treeDataProvider: fileProvider,
	});

	context.subscriptions.push(
		vscode.window.registerTreeDataProvider('glsld.compileGroups', compileProvider),
		globalTreeView,
		fileTreeView,

		globalTreeView.onDidChangeCheckboxState((e: vscode.TreeCheckboxChangeEvent<GlobalMacroNode>) => {
			for (const [node, state] of e.items) {
				globalProvider.toggleMacro(node, state === vscode.TreeItemCheckboxState.Checked);
			}
		}),

		// --- File view checkbox toggle ---
		fileTreeView.onDidChangeCheckboxState((e: vscode.TreeCheckboxChangeEvent<FileMacroNode>) => {
			for (const [node, state] of e.items) {
				if (node.kind === 'macro') {
					const checked = state === vscode.TreeItemCheckboxState.Checked;
					fileProvider.toggleMacro(node, checked);
				}
			}
		}),

		// --- Select file variant (no longer used in new model; kept for compat) ---
		vscode.commands.registerCommand('glsld.selectFileVariant', (_arg: unknown) => {
			// Now handled by checkbox toggles in the File view
			vscode.window.showInformationMessage(
				'Use the File Macros view to toggle per-file macros.',
			);
		}),

		// --- Clear file override (no longer applicable; kept for compat) ---
		vscode.commands.registerCommand('glsld.clearFileVariant', () => {
			vscode.window.showInformationMessage(
				'Use the File Macros view to manage per-file macros.',
			);
		}),

		vscode.commands.registerCommand('glsld.addVariant', () => globalProvider.addMacro()),
		vscode.commands.registerCommand('glsld.editVariant', (node: GlobalMacroNode) => {
			if (node?.kind === 'globalMacro') { globalProvider.editMacro(node); }
		}),
		vscode.commands.registerCommand('glsld.removeVariantCmd', (node: GlobalMacroNode) => {
			if (node?.kind === 'globalMacro') { globalProvider.deleteMacro(node); }
		}),

		// --- Compile groups ---
		vscode.commands.registerCommand('glsld.addCompileGroup', () => compileProvider.addGroup()),
		vscode.commands.registerCommand('glsld.editCompileGroup', (node?: CompileGroupEntry) =>
			compileProvider.editGroup(node),
		),
		vscode.commands.registerCommand('glsld.removeCompileGroup', (node?: CompileGroupEntry) =>
			compileProvider.removeGroup(node),
		),
		vscode.commands.registerCommand('glsld.refreshCompileGroups', () => {
			cachedData = null;
			compileProvider.refresh();
		}),

		// --- Refresh views ---
		vscode.commands.registerCommand('glsld.refreshVariants', () => {
			cachedData = null;
			globalProvider.refresh();
			fileProvider.refresh();
			compileProvider.refresh();
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
	);
}

	// ============================================================
	// Public accessors  (used by compile.ts etc.)
	// ============================================================

	export function getActiveGlobalMacros(): Record<string, string> {
		const enabled: Record<string, string> = {};
		for (const [name, entry] of Object.entries(getGlobalMacros())) {
			if (entry.enabled) { enabled[name] = entry.value; }
		}
		return enabled;
	}

	/** Returns the raw per-file macro map. */
	export function getFileMacrosData(): FileMacrosMap {
		return getFileMacros();
	}

	export function getActiveVariants() {
		const activeVariants: Array<{ textDocument?: { uri: string }; variant: string; macros: Record<string, string>; scope: 'global' | 'file' }> = [];
		const globalMacros = getActiveGlobalMacros();
		if (Object.keys(globalMacros).length !== 0) { activeVariants.push({ variant: '__global__', macros: globalMacros, scope: 'global' }); }

		for (const [uri, macros] of Object.entries(getFileMacros())) {
			const enabled: Record<string, string> = {};
			for (const [name, entry] of Object.entries(macros)) {
				if (entry.enabled) {
					enabled[name] = entry.value;
				}
			}

			if (Object.keys(enabled).length !== 0) {
				activeVariants.push({ textDocument: { uri }, variant: '__file__', macros: enabled, scope: 'file' });
			}
		}

		return activeVariants;
	}

	export { getShaderConfigs, putShaderConfigs, getTemplates, putTemplates };
	export type { FileMacrosMap };
