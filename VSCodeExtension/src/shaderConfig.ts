/* --------------------------------------------------------------------------------------------
 * Copyright (c) glsld authors. All rights reserved.
 * Licensed under the MIT License.
 * ------------------------------------------------------------------------------------------ */

import * as vscode from 'vscode';
import { getShaderConfigs, putShaderConfigs, getTemplates, putTemplates } from './sidebar';
import { pushConfiguration, notifyRemoveConfig } from './extension';

// ============================================================
// Types
// ============================================================

const ALL_SHADER_STAGES = [
	'vert', 'frag', 'comp', 'geom',
	'tesc', 'tese', 'mesh', 'task',
	'rgen', 'rahit', 'rchit', 'rmiss', 'rint', 'rcall',
] as const;

interface Template {
	name: string;
	shaderStage?: string;
	version?: string;
	targetEnv?: string;
	targetSpv?: string;
}

interface FileConfig {
	shaderStage?: string;
	version?: string;
	targetEnv?: string;
	targetSpv?: string;
}

const PROPERTY_OPTIONS: Record<string, { values: string[]; isEnum: boolean }> = {
	shaderStage: { isEnum: true, values: [...ALL_SHADER_STAGES] },
	targetEnv:   { isEnum: true, values: ['vulkan1.0', 'vulkan1.1', 'vulkan1.2', 'vulkan1.3', 'vulkan1.4'] },
	targetSpv:   { isEnum: true, values: ['spv1.0', 'spv1.1', 'spv1.2', 'spv1.3', 'spv1.4', 'spv1.5', 'spv1.6'] },
	version:     { isEnum: false, values: [] },
};

// ============================================================
// Tree node types
// ============================================================

type ShaderConfigNode = FileConfigNode | PropertyNode;

interface FileConfigNode {
	kind: 'fileConfig';
	key: string;
}

interface PropertyNode {
	kind: 'configProp';
	fileKey: string;
	property: string;
	value: string;
}

// ============================================================
// Provider
// ============================================================

export class ShaderConfigProvider implements vscode.TreeDataProvider<ShaderConfigNode> {
	private _onDidChangeTreeData = new vscode.EventEmitter<ShaderConfigNode | undefined | void>();
	readonly onDidChangeTreeData = this._onDidChangeTreeData.event;

	public refresh(): void { this._onDidChangeTreeData.fire(); }

	public getTreeItem(element: ShaderConfigNode): vscode.TreeItem {
		if (element.kind === 'fileConfig') {
			const item = new vscode.TreeItem(element.key, vscode.TreeItemCollapsibleState.Expanded);
			item.iconPath = vscode.ThemeIcon.File;
			item.contextValue = 'fileConfig';

			const sc = getShaderConfigs();
			const cfg = sc[element.key];
			if (cfg) {
				const parts: string[] = [];
				if (cfg.shaderStage) { parts.push(cfg.shaderStage); }
				if (cfg.version)     { parts.push('#version ' + cfg.version); }
				if (cfg.targetEnv)   { parts.push(cfg.targetEnv); }
				if (cfg.targetSpv)   { parts.push(cfg.targetSpv); }
				item.description = parts.join(' / ') || 'no properties';
			} else {
				item.description = '(deleted)';
			}
			return item;
		}

		if (element.kind === 'configProp') {
			const item = new vscode.TreeItem(element.property, vscode.TreeItemCollapsibleState.None);
			item.description = element.value || '(empty)';
			item.iconPath = new vscode.ThemeIcon('symbol-property');
			item.contextValue = 'configProp';
			return item;
		}

		throw new Error('Unknown node kind');
	}

	public getChildren(element?: ShaderConfigNode): ShaderConfigNode[] {
		if (!element) {
			const sc = getShaderConfigs();
			return Object.keys(sc).map(key => ({ kind: 'fileConfig' as const, key }));
		}
		if (element.kind === 'fileConfig') {
			const sc = getShaderConfigs()[element.key];
			if (!sc) { return []; }
			return Object.entries(sc)
				.filter(([, v]) => v !== undefined && v !== '')
				.map(([prop, value]) => ({
					kind: 'configProp' as const,
					fileKey: element.key,
					property: prop,
					value: String(value),
				}));
		}
		return [];
	}

	public getParent(element: ShaderConfigNode): vscode.ProviderResult<ShaderConfigNode> {
		if (element.kind === 'configProp') {
			return { kind: 'fileConfig', key: element.fileKey };
		}
		return null;
	}

	// ---- Actions ----

	/** Generate one template per shader stage with user-chosen settings. */
	public async generateConfigs(): Promise<void> {
		// 1. Pick target environment
		const envs = ['vulkan1.0', 'vulkan1.1', 'vulkan1.2', 'vulkan1.3', 'vulkan1.4', 'opengl'];
		const targetEnv = await vscode.window.showQuickPick(envs, {
			placeHolder: 'Select target environment',
			title: 'Generate Shader Config Templates',
		});
		if (!targetEnv) { return; }

		// 2. Pick SPIR-V version (Vulkan only)
		let targetSpv: string | undefined;
		if (targetEnv.startsWith('vulkan')) {
			const spvs = ['spv1.0', 'spv1.1', 'spv1.2', 'spv1.3', 'spv1.4', 'spv1.5', 'spv1.6'];
			targetSpv = await vscode.window.showQuickPick(spvs, {
				placeHolder: 'Select SPIR-V version',
				title: 'Generate Shader Config Templates',
			});
			if (!targetSpv) { return; }
		}

		// 3. Pick GLSL version (optional)
		const version = await vscode.window.showInputBox({
			prompt: 'GLSL version (e.g. 460, leave empty to skip)',
			placeHolder: '460',
			title: 'Generate Shader Config Templates',
		});
		if (version === undefined) { return; } // cancelled

		// 4. Generate
		const label = targetEnv + (targetSpv ? ' / ' + targetSpv : '');
		const existing = getTemplates();

		const newTemplates: Template[] = [];
		for (const stage of ALL_SHADER_STAGES) {
			const name = stage.charAt(0).toUpperCase() + stage.slice(1) + ' (' + label + ')';
			if (existing.some((t: Template) => t.name === name)) { continue; }
			const t: Template = { name, shaderStage: stage, targetEnv };
			if (targetSpv) { t.targetSpv = targetSpv; }
			if (version && version.trim()) { t.version = version.trim(); }
			newTemplates.push(t);
		}

		if (newTemplates.length === 0) {
			vscode.window.showInformationMessage('All templates for ' + label + ' already exist.');
			return;
		}

		const all = [...existing, ...newTemplates];
		putTemplates(all);
			pushConfiguration();

		vscode.window.showInformationMessage(
			'Generated ' + newTemplates.length + ' templates: ' + label,
		);
	}

	/** Add shader config: pick template → pick file → apply */
	public async addConfig(): Promise<void> {
		const templates = getTemplates();
		if (templates.length === 0) {
			const gen = await vscode.window.showWarningMessage(
				'No templates yet. Generate them now?',
				{ modal: true }, 'Generate',
			);
			if (gen === 'Generate') {
				await this.generateConfigs();
			}
			return;
		}

		const templatePick = await vscode.window.showQuickPick(
			templates.map((t: any) => ({ label: t.name, template: t })),
			{ placeHolder: 'Select a template to apply' },
		);
		if (!templatePick) { return; }

		const key = await this.pickTargetFile();
		if (!key) { return; }

		await this.applyTemplateToFile(templatePick.template, key);
	}

	/** Apply template from context menu */
	public async applyTemplate(node?: FileConfigNode): Promise<void> {
		const templates = getTemplates();
		if (templates.length === 0) {
			vscode.window.showWarningMessage('No templates. Run "Generate Shader Configs" first.');
			return;
		}

		const templatePick = await vscode.window.showQuickPick(
			templates.map((t: any) => ({ label: t.name, template: t })),
			{ placeHolder: 'Select a template to apply' },
		);
		if (!templatePick) { return; }

		const key = node?.key ?? await this.pickTargetFile();
		if (!key) { return; }

		await this.applyTemplateToFile(templatePick.template, key);
	}

	/** Edit single property */
	public async editProperty(node: PropertyNode): Promise<void> {
		const opts = PROPERTY_OPTIONS[node.property];
		let newValue: string | undefined;

		if (opts?.isEnum) {
			newValue = await vscode.window.showQuickPick(opts.values, {
				placeHolder: 'Select ' + node.property + ' for ' + node.fileKey,
				title: node.property,
			});
		} else {
			newValue = await vscode.window.showInputBox({
				prompt: node.property + ' for ' + node.fileKey,
				value: node.value,
				placeHolder: 'e.g. 460',
			});
		}

		if (newValue === undefined) { return; }

		const sc = { ...getShaderConfigs() };
		if (!sc[node.fileKey]) { sc[node.fileKey] = {}; }
		(sc[node.fileKey] as any)[node.property] = newValue;

		putShaderConfigs(sc);
		pushConfiguration();
		this.refresh();
	}

	/** Manually edit all properties */
	public async editConfigProps(key?: string): Promise<void> {
		let fileKey = key;
		if (!fileKey) {
			const sc = getShaderConfigs();
			const keys = Object.keys(sc);
			if (keys.length === 0) {
				vscode.window.showInformationMessage('No shader configs yet.');
				return;
			}
			fileKey = await vscode.window.showQuickPick(keys, { placeHolder: 'Select a file to edit' });
			if (!fileKey) { return; }
		}

		const all = { ...getShaderConfigs() };
		const existing = all[fileKey] ? { ...all[fileKey] } : {};

		const props = ['shaderStage', 'version', 'targetEnv', 'targetSpv'] as const;
		for (const prop of props) {
			const opts = PROPERTY_OPTIONS[prop];
			const currentVal = (existing as any)[prop] ?? '';

			let newVal: string | undefined;
			if (opts?.isEnum) {
				const items = [
					{ label: '$(close) (unset)', description: currentVal },
					...opts.values.map(v => ({
						label: v === currentVal ? '$(pass-filled) ' + v : '  ' + v,
					})),
				];
				const pick = await vscode.window.showQuickPick(items, {
					placeHolder: 'Set ' + prop + ' (Esc to skip)',
					title: 'Configure ' + fileKey,
				});
				if (!pick) { continue; }
				newVal = pick === items[0] ? undefined : pick.label.replace(/^\S+\s+/, '');
			} else {
				newVal = await vscode.window.showInputBox({
					prompt: prop + ' (Enter to skip)',
					value: currentVal,
					placeHolder: 'e.g. 460',
				});
				if (newVal === undefined) { continue; }
				if (newVal.trim() === '') { newVal = undefined; }
			}

			if (newVal !== undefined) {
				(existing as any)[prop] = newVal;
			} else {
				delete (existing as any)[prop];
			}
		}

		if (Object.keys(existing).length > 0) {
			all[fileKey] = existing;
		} else {
			delete all[fileKey];
		}

		putShaderConfigs(all);
		pushConfiguration();
		this.refresh();
	}

	/** Remove config */
	public async removeConfig(node: FileConfigNode): Promise<void> {
		const choice = await vscode.window.showWarningMessage(
			'Remove shader config for "' + node.key + '"?',
			{ modal: true }, 'Remove',
		);
		if (choice !== 'Remove') { return; }

		const all = { ...getShaderConfigs() };
		delete all[node.key];

		putShaderConfigs(all);
		notifyRemoveConfig(node.key);
		pushConfiguration();
		this.refresh();
	}

	// ---- Helpers ----

	private async pickTargetFile(): Promise<string | undefined> {
		const editor = vscode.window.activeTextEditor;
		const currentFile = (editor && editor.document.languageId === 'glsl')
			? vscode.workspace.asRelativePath(editor.document.uri)
			: undefined;

		const sc = getShaderConfigs();
		const keys = Object.keys(sc);
		const items: (vscode.QuickPickItem & { key?: string })[] = [];

		if (currentFile) {
			items.push({
				label: '$(circle-filled) ' + currentFile,
				description: keys.includes(currentFile) ? '(overwrite)' : 'Current file',
				key: currentFile,
			});
		}
		for (const k of keys) {
			if (k === currentFile) { continue; }
			items.push({ label: k, key: k });
		}
		items.push({ label: '$(folder-opened) Browse for file...' });

		const pick = await vscode.window.showQuickPick(items, { placeHolder: 'Select target file' });
		if (!pick) { return undefined; }
		if (pick.key) { return pick.key; }

		const uris = await vscode.window.showOpenDialog({
			canSelectMany: false,
			filters: { 'GLSL Shaders': ['glsl', 'vert', 'frag', 'comp', 'geom', 'tesc', 'tese', 'mesh', 'task', 'rgen', 'rahit', 'rchit', 'rmiss', 'rint', 'rcall', 'inc', 'h'] },
			title: 'Select GLSL file',
		});
		if (!uris || uris.length === 0) { return undefined; }
		return vscode.workspace.asRelativePath(uris[0]);
	}

	private async applyTemplateToFile(template: Template, key: string): Promise<void> {
		const all = { ...getShaderConfigs() };
		const cfg: FileConfig = {};
		if (template.shaderStage) { cfg.shaderStage = template.shaderStage; }
		if (template.version)     { cfg.version = template.version; }
		if (template.targetEnv)   { cfg.targetEnv = template.targetEnv; }
		if (template.targetSpv)   { cfg.targetSpv = template.targetSpv; }
		all[key] = cfg as Record<string, string>;

		putShaderConfigs(all);
		pushConfiguration();
		this.refresh();
		vscode.window.showInformationMessage('"' + template.name + '" applied to "' + key + '"');
	}
}
