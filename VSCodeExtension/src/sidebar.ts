/* --------------------------------------------------------------------------------------------
 * Copyright (c) glsld authors. All rights reserved.
 * Licensed under the MIT License.
 * ------------------------------------------------------------------------------------------ */

import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';
import { LanguageClient } from 'vscode-languageclient/node';
import { ConfigFolderNode, configFileChildren, configFileName, configFileParent } from './configFileTree';
import { CompileSettings, defaultOutputPath, defaultTargetEnv, parseCompileOptions, shaderStages, targetEnvironments, targetSpirvVersions } from './compilePlan';

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

export interface CompileGroup extends CompileSettings {
	/** Display name shown in the Compile Groups view. */
	name: string;
	/** Manually added source files and their selected compile stages. */
	fileStages?: Record<string, string[]>;
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

type FileMacroNode = ConfigFolderNode | FileNode | MacroNode;

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
	if (!client.isRunning()) { return; }
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
	if (!client.isRunning()) { return; }
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
		if (element.kind === 'configFolder') {
			const item = new vscode.TreeItem(element.label, vscode.TreeItemCollapsibleState.Collapsed);
			item.id = `fileMacros:folder:${element.folderPath}`;
			item.iconPath = new vscode.ThemeIcon('folder');
			return item;
		}
		if (element.kind === 'file') {
			const displayPath = vscode.workspace.asRelativePath(element.uri);
			const item = new vscode.TreeItem(
				configFileName(displayPath),
				vscode.TreeItemCollapsibleState.Expanded,
			);
			item.id = `fileMacros:file:${element.uri.toString()}`;
			item.tooltip = displayPath;
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
		if (!element || element.kind === 'configFolder') {
			return configFileChildren(Object.keys(getFileMacros()).map(key => ({
				key, displayPath: vscode.workspace.asRelativePath(vscode.Uri.parse(key)),
			})), element?.folderPath).map(child => child.kind === 'configFolder'
				? child : { kind: 'file', uri: vscode.Uri.parse(child.key) });
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
		return configFileParent(element.kind === 'configFolder' ? element.folderPath : vscode.workspace.asRelativePath(element.uri));
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
		if (this._client.isRunning()) {
			this._client.sendNotification('glsld/removeVariant', {
				textDocument: { uri: node.uri.toString() },
				scope: 'file',
			});
		}

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

type CompileGroupNode = CompileGroupEntry | CompileGroupProperty
	| (ConfigFolderNode & { groupIndex: number })
	| { kind: 'compileGroupFile'; groupIndex: number; key: string }
	| { kind: 'compileGroupStage'; groupIndex: number; fileKey: string; stage: string };

interface CompileGroupEntry {
	kind: 'compileGroup';
	index: number;
}

interface CompileGroupProperty {
	kind: 'compileGroupProperty';
	groupIndex: number;
	setting: keyof CompileSettings;
	label: string;
	value: string;
}

const compileGroupSettings: { setting: keyof CompileSettings; label: string }[] = [
	{ setting: 'outputPath', label: 'Output Directory' },
	{ setting: 'options', label: 'Compiler Options' },
	{ setting: 'targetEnv', label: 'Target Environment' },
	{ setting: 'targetSpv', label: 'Target SPIR-V' },
];

class CompileGroupProvider implements vscode.TreeDataProvider<CompileGroupNode> {
	private readonly onDidChangeTreeDataEmitter = new vscode.EventEmitter<CompileGroupNode | undefined | void>();
	readonly onDidChangeTreeData = this.onDidChangeTreeDataEmitter.event;

	public refresh(): void { this.onDidChangeTreeDataEmitter.fire(); }

	public getTreeItem(element: CompileGroupNode): vscode.TreeItem {
		if (element.kind === 'configFolder') {
			const item = new vscode.TreeItem(element.label, vscode.TreeItemCollapsibleState.Collapsed);
			item.id = `group:${element.groupIndex}:folder:${element.folderPath}`;
			item.iconPath = new vscode.ThemeIcon('folder');
			return item;
		}
		if (element.kind === 'compileGroupFile' || element.kind === 'compileGroupStage') {
			const key = element.kind === 'compileGroupFile' ? element.key : element.fileKey;
			const item = new vscode.TreeItem(element.kind === 'compileGroupFile' ? configFileName(key) : element.stage,
				element.kind === 'compileGroupFile' ? vscode.TreeItemCollapsibleState.Expanded : vscode.TreeItemCollapsibleState.None);
			item.contextValue = element.kind;
			item.command = { command: 'glsld.editCompileStages', title: 'Edit File Stages', arguments: [element] };
			if (element.kind === 'compileGroupFile') {
				item.iconPath = vscode.ThemeIcon.File;
				item.tooltip = key;
				item.description = getCompileGroups()[element.groupIndex]?.fileStages?.[key]?.join(', ');
			} else { item.description = shaderStages.find(stage => stage.stage === element.stage)?.macro; }
			return item;
		}
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
				item.description = `${Object.keys(group.fileStages ?? {}).length} file(s)`;
				item.tooltip = [
					`Output directory: ${group.outputPath || '(edit group to set) '}`,
					`Compiler options: ${group.options || '(none)'}`,
					`Target: ${group.targetEnv || defaultTargetEnv} / ${group.targetSpv || 'Auto'}`,
				].join('\n');
			}
			return item;
		}

		const item = new vscode.TreeItem(element.label, vscode.TreeItemCollapsibleState.None);
		item.description = element.value || '(none)';
		item.iconPath = new vscode.ThemeIcon('symbol-property');
		item.contextValue = 'compileGroupProperty';
		item.command = { command: 'glsld.editCompileGroup', title: `Edit ${element.label}`, arguments: [element] };
		return item;
	}

	public getChildren(element?: CompileGroupNode): CompileGroupNode[] {
		if (!element) {
			return getCompileGroups().map((_, index) => ({ kind: 'compileGroup' as const, index }));
		}
		if (element.kind === 'compileGroupProperty' || element.kind === 'compileGroupStage') { return []; }
		const groupIndex = element.kind === 'compileGroup' ? element.index : element.groupIndex;
		const group = getCompileGroups()[groupIndex];
		if (!group) { return []; }
		if (element.kind === 'compileGroupFile') {
			return (group.fileStages?.[element.key] ?? []).map(stage => ({ kind: 'compileGroupStage', groupIndex, fileKey: element.key, stage }));
		}
		const files: CompileGroupNode[] = configFileChildren(Object.keys(group.fileStages ?? {}).map(key => ({ key, displayPath: key })),
			element.kind === 'configFolder' ? element.folderPath : '').map(child => child.kind === 'configFolder'
				? { ...child, groupIndex } : { kind: 'compileGroupFile', groupIndex, key: child.key });
		if (element.kind === 'configFolder') { return files; }

		const children: CompileGroupProperty[] = compileGroupSettings.map(({ setting, label }) => ({
			kind: 'compileGroupProperty', groupIndex, setting, label,
			value: group[setting] || (setting === 'targetEnv' ? defaultTargetEnv : setting === 'targetSpv' ? 'Auto' : ''),
		}));
		return [...children, ...files];
	}

	public getParent(node: CompileGroupNode): CompileGroupNode | undefined {
		if (node.kind === 'compileGroup') { return undefined; }
		if (node.kind === 'compileGroupStage') { return { kind: 'compileGroupFile', groupIndex: node.groupIndex, key: node.fileKey }; }
		if (node.kind === 'configFolder' || node.kind === 'compileGroupFile') {
			const folder = configFileParent(node.kind === 'configFolder' ? node.folderPath : node.key);
			if (folder) { return { ...folder, groupIndex: node.groupIndex }; }
		}
		return { kind: 'compileGroup', index: node.groupIndex };
	}

	private async pickGroup(node?: CompileGroupNode): Promise<number | undefined> {
		const index = node?.kind === 'compileGroup' ? node.index : node?.groupIndex;
		if (index !== undefined) { return getCompileGroups()[index] ? index : undefined; }
		return (await vscode.window.showQuickPick(getCompileGroups().map((group, groupIndex) => ({ label: group.name, groupIndex })),
			{ placeHolder: 'Select a compile group' }))?.groupIndex;
	}

	public async addFileStages(node?: CompileGroupNode): Promise<void> {
		const groupIndex = await this.pickGroup(node);
		if (groupIndex === undefined) { return; }
		const document = vscode.window.activeTextEditor?.document;
		let key = document?.languageId === 'glsl' ? vscode.workspace.asRelativePath(document.uri) : undefined;
		if (!key) {
			const files = await vscode.window.showOpenDialog({ canSelectMany: false, title: 'Select a shader file' });
			if (!files?.length) { return; }
			key = vscode.workspace.asRelativePath(files[0]);
		}
		const inferred = shaderStages.find(stage => stage.stage === path.extname(key).slice(1).toLowerCase());
		if (inferred) {
			// Re-adding a file must preserve any stages the user explicitly selected.
			if (!getCompileGroups()[groupIndex].fileStages?.[key]?.length) {
				this.saveFileStages(groupIndex, key, [inferred.stage]);
			}
			return;
		}
		await this.editFileStages({ kind: 'compileGroupFile', groupIndex, key });
	}

	public async editFileStages(node?: CompileGroupNode): Promise<void> {
		const groupIndex = await this.pickGroup(node);
		if (groupIndex === undefined) { return; }
		const group = getCompileGroups()[groupIndex];
		const key = node?.kind === 'compileGroupFile' ? node.key : node?.kind === 'compileGroupStage' ? node.fileKey
			: await vscode.window.showQuickPick(Object.keys(group.fileStages ?? {}), { placeHolder: 'Select a file' });
		if (!key) { return; }
		const current = group.fileStages?.[key] ?? [];
		const selected = await vscode.window.showQuickPick(shaderStages.map(stage => ({
			label: stage.stage, description: stage.macro, picked: current.includes(stage.stage),
		})), { canPickMany: true, title: `${group.name}: stages for ${key}`, placeHolder: 'Select all stages to compile' });
		if (selected === undefined) { return; }
		this.saveFileStages(groupIndex, key, selected.map(stage => stage.label));
	}

	private saveFileStages(groupIndex: number, key: string, stages: string[]): void {
		const groups = [...getCompileGroups()];
		const fileStages = { ...groups[groupIndex].fileStages };
		if (stages.length) { fileStages[key] = stages; }
		else { delete fileStages[key]; }
		groups[groupIndex] = { ...groups[groupIndex], fileStages };
		putCompileGroups(groups);
		this.refresh();
	}

	public async removeFileStages(node?: CompileGroupNode): Promise<void> {
		const groupIndex = await this.pickGroup(node);
		if (groupIndex === undefined) { return; }
		const group = getCompileGroups()[groupIndex];
		const key = node?.kind === 'compileGroupFile' ? node.key : node?.kind === 'compileGroupStage' ? node.fileKey
			: await vscode.window.showQuickPick(Object.keys(group.fileStages ?? {}), { placeHolder: 'Select a file' });
		if (!key || await vscode.window.showWarningMessage(`Remove "${key}" from "${group.name}"?`, { modal: true }, 'Remove') !== 'Remove') { return; }
		const groups = [...getCompileGroups()];
		const fileStages = { ...groups[groupIndex].fileStages };
		delete fileStages[key];
		groups[groupIndex] = { ...groups[groupIndex], fileStages };
		putCompileGroups(groups);
		this.refresh();
	}

	public async addGroup(): Promise<void> {
		const group = await this.inputGroup();
		if (!group) { return; }

		const groups = [...getCompileGroups(), group];
		putCompileGroups(groups);
		this.refresh();
		vscode.window.showInformationMessage(`Compile group "${group.name}" saved to .glsld/config.json.`);
	}

	public async editGroup(node?: CompileGroupEntry | CompileGroupProperty): Promise<void> {
		const groups = [...getCompileGroups()];
		let index = node?.kind === 'compileGroup' ? node.index : node?.groupIndex;
		if (index === undefined) {
			const selected = await vscode.window.showQuickPick(
				groups.map((group, groupIndex) => ({
					label: group.name,
					description: `${Object.keys(group.fileStages ?? {}).length} file(s)`,
					groupIndex,
				})),
				{ placeHolder: 'Select a compile group to edit' },
			);
			index = selected?.groupIndex;
		}
		if (index === undefined || !groups[index]) { return; }

		let group: CompileGroup | undefined;
		if (node?.kind === 'compileGroupProperty') {
			const value = await this.inputSetting(node.setting, groups[index]);
			if (value === undefined) { return; }
			group = { ...groups[index], [node.setting]: value };
		} else { group = await this.inputGroup(groups[index], index); }
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

		const settings: CompileSettings = { outputPath: existing?.outputPath ?? defaultOutputPath };
		for (const { setting } of compileGroupSettings) {
			const value = await this.inputSetting(setting, { ...existing, ...settings });
			if (value === undefined) { return undefined; }
			settings[setting] = value;
		}
		return { name: name.trim(), ...settings, fileStages: existing?.fileStages ?? {} };
	}

	private async inputSetting(setting: keyof CompileSettings, existing: Partial<CompileSettings>): Promise<string | undefined> {
		const title = `Compile Group: ${compileGroupSettings.find(item => item.setting === setting)!.label}`;
		if (setting === 'targetEnv' || setting === 'targetSpv') {
			const current = existing[setting] ?? (setting === 'targetEnv' ? defaultTargetEnv : '');
			const values = setting === 'targetEnv' ? targetEnvironments : ['', ...targetSpirvVersions];
			const items = values.map(value => ({ label: value || 'Auto', value,
				description: value === current ? 'Current' : undefined }));
			items.sort((a, b) => Number(b.value === current) - Number(a.value === current));
			return (await vscode.window.showQuickPick(items, { title,
				placeHolder: setting === 'targetEnv' ? 'Select target environment' : 'Auto uses the target environment default' }))?.value;
		}
		const value = await vscode.window.showInputBox({
			title,
			prompt: setting === 'outputPath' ? 'Output directory (absolute or relative to the workspace folder)'
				: 'Additional glslc options; output, target and stages use their own settings',
			placeHolder: setting === 'outputPath' ? defaultOutputPath : '-O -g -I "Shaders/Includes"',
			value: existing[setting] ?? (setting === 'outputPath' ? defaultOutputPath : ''),
			validateInput: value => {
				if (setting === 'outputPath') { return value.trim() ? undefined : 'Output directory cannot be empty.'; }
				try { parseCompileOptions(value); return undefined; }
				catch (error) { return error instanceof Error ? error.message : String(error); }
			},
		});
		return value?.trim();
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
		vscode.commands.registerCommand('glsld.addCompileStages', node => compileProvider.addFileStages(node)),
		vscode.commands.registerCommand('glsld.editCompileStages', node => compileProvider.editFileStages(node)),
		vscode.commands.registerCommand('glsld.removeCompileStages', node => compileProvider.removeFileStages(node)),
		vscode.commands.registerCommand('glsld.addCompileGroup', () => compileProvider.addGroup()),
		vscode.commands.registerCommand('glsld.editCompileGroup', (node?: CompileGroupEntry | CompileGroupProperty) =>
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
		vscode.commands.registerCommand('glsld.addFileMacro', async (node?: FileNode) => {
			if (node?.kind === 'file') {
				await fileProvider.addMacro(node.uri);
				return;
			}
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
				return fileProvider.editMacro(node);
			}
		}),

		// --- Delete a file macro (context menu) ---
		vscode.commands.registerCommand('glsld.deleteFileMacro', (node: MacroNode) => {
			if (node && node.kind === 'macro') {
				return fileProvider.deleteMacro(node);
			}
		}),

		// --- Remove a file from File view ---
		vscode.commands.registerCommand('glsld.removeFileFromList', (node: FileNode) => {
			if (node && node.kind === 'file') {
				return fileProvider.removeFile(node);
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
