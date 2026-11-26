import * as path from 'path';

export interface ShaderStage {
	stage: string;
	macro: string;
}

// Keep these macros consistent with the language server's MetadataManager.
export const shaderStages: ShaderStage[] = [
	{ stage: 'vert', macro: 'GL_VERTEX_SHADER' },
	{ stage: 'frag', macro: 'GL_FRAGMENT_SHADER' },
	{ stage: 'comp', macro: 'GL_COMPUTE_SHADER' },
	{ stage: 'geom', macro: 'GL_GEOMETRY_SHADER' },
	{ stage: 'tesc', macro: 'GL_TESSELLATION_CONTROL_SHADER' },
	{ stage: 'tese', macro: 'GL_TESSELLATION_EVALUATION_SHADER' },
	{ stage: 'mesh', macro: 'GL_MESH_SHADER_EXT' },
	{ stage: 'task', macro: 'GL_TASK_SHADER_EXT' },
	{ stage: 'rgen', macro: 'GL_RAY_GENERATION_SHADER_EXT' },
	{ stage: 'rahit', macro: 'GL_ANY_HIT_SHADER_EXT' },
	{ stage: 'rchit', macro: 'GL_CLOSEST_HIT_SHADER_EXT' },
	{ stage: 'rmiss', macro: 'GL_MISS_SHADER_EXT' },
	{ stage: 'rint', macro: 'GL_INTERSECTION_SHADER_EXT' },
	{ stage: 'rcall', macro: 'GL_CALLABLE_SHADER_EXT' },
];
const byMacro = new Map(shaderStages.map(stage => [stage.macro, stage]));
const byName = new Map(shaderStages.map(stage => [stage.stage, stage]));
for (const [alias, shortName] of Object.entries({
	vertex: 'vert', fragment: 'frag', compute: 'comp', geometry: 'geom',
	tesscontrol: 'tesc', tesseval: 'tese', raygen: 'rgen', anyhit: 'rahit',
	closest: 'rchit', closesthit: 'rchit', miss: 'rmiss', intersect: 'rint', intersection: 'rint', callable: 'rcall',
})) { byName.set(alias, byName.get(shortName)!); }

export interface CompileSettings {
	/** Output directory, absolute or relative to the workspace folder. */
	outputPath: string;
	options?: string;
	targetEnv?: string;
	targetSpv?: string;
}

export const targetEnvironments = ['vulkan1.0', 'vulkan1.1', 'vulkan1.2', 'vulkan1.3', 'vulkan1.4', 'opengl'];
export const targetSpirvVersions = ['spv1.0', 'spv1.1', 'spv1.2', 'spv1.3', 'spv1.4', 'spv1.5', 'spv1.6'];
export const defaultTargetEnv = 'vulkan1.3';
export const defaultOutputPath = 'build/shaders';

export interface CompileCommand {
	args: string[];
	stage: string;
	outputPath: string;
	cwd: string;
}

/** Split options into argv without running a shell or changing Windows path separators. */
export function parseCompileOptions(options: string): string[] {
	const args: string[] = [];
	let current = '';
	let quote: string | undefined;
	let started = false;
	for (const character of options) {
		if (quote) {
			if (character === quote) { quote = undefined; }
			else { current += character; }
		} else if (character === '"' || character === "'") { quote = character; started = true; }
		else if (/\s/.test(character)) {
			if (started) { args.push(current); current = ''; started = false; }
		} else { current += character; started = true; }
	}
	if (quote) { throw new Error('Unclosed quote in compiler options.'); }
	if (started) { args.push(current); }
	for (const arg of args) {
		if (arg.startsWith('-o') || /^(-fshader-stage|--target-env|--target-spv)(=|$)/.test(arg)) {
			throw new Error(`Set ${arg.split('=')[0]} using the group's output/target settings or the file's stages.`);
		}
	}
	return args;
}

export function createCompileCommands(
	settings: CompileSettings,
	filename: string,
	defines: Record<string, string>,
	selectedStages: string[],
	workspaceDirectory: string,
): CompileCommand[] {
	if (!settings.outputPath?.trim()) { throw new Error('Edit this Compile Group and set its output directory.'); }
	if (!Array.isArray(selectedStages) || !selectedStages.length) { throw new Error('Select at least one stage for this file in its Compile Group.'); }
	const stages = [...new Set(selectedStages.map(name => {
		const stage = typeof name === 'string' ? byName.get(name) : undefined;
		if (!stage) { throw new Error(`Unknown compile stage: ${name}`); }
		return stage;
	}))];
	const targetEnv = settings.targetEnv || defaultTargetEnv;
	if (!targetEnvironments.includes(targetEnv)) { throw new Error(`Unknown target environment: ${targetEnv}`); }
	if (settings.targetSpv && !targetSpirvVersions.includes(settings.targetSpv)) { throw new Error(`Unknown SPIR-V target: ${settings.targetSpv}`); }
	const options = parseCompileOptions(settings.options ?? '');
	const isolatedOptions: string[] = [];
	for (let index = 0; index < options.length; index++) {
		const arg = options[index];
		const definition = arg === '-D' || arg === '-U' ? options[index + 1] : /^-[DU]/.test(arg) ? arg.slice(2) : undefined;
		if (definition && byMacro.has(definition.split('=')[0])) {
			if (arg === '-D' || arg === '-U') { index++; }
		} else { isolatedOptions.push(arg); }
	}
	const commonDefines = Object.entries(defines).filter(([name]) => !byMacro.has(name)).map(([name, value]) => `-D${name}=${value}`);
	const outputDirectory = path.resolve(workspaceDirectory,
		settings.outputPath.trim().replace(/\$\{workspaceFolder\}/g, () => workspaceDirectory));
	const relative = path.relative(workspaceDirectory, filename);
	const insideWorkspace = !path.isAbsolute(relative) && relative !== '..' && !relative.startsWith(`..${path.sep}`);
	const source = path.parse(insideWorkspace ? relative : path.basename(filename));
	const extension = options.includes('-S') ? 'spvasm' : 'spv';
	return stages.map(stage => {
		const outputPath = path.join(outputDirectory, source.dir, `${source.name}.${stage.stage}.${extension}`);
		return {
			stage: stage.stage, outputPath, cwd: workspaceDirectory,
			args: [`-fshader-stage=${stage.stage}`, `--target-env=${targetEnv}`,
				...(settings.targetSpv ? [`--target-spv=${settings.targetSpv}`] : []),
				...commonDefines, ...isolatedOptions, `-D${stage.macro}=1`, filename, '-o', outputPath],
		};
	});
}

export function findCompileOutputConflict(commands: CompileCommand[]): string | undefined {
	const outputs = new Map<string, CompileCommand>();
	for (const command of commands) {
		const key = process.platform === 'win32' ? command.outputPath.toLowerCase() : command.outputPath;
		const previous = outputs.get(key);
		if (previous) { return command.outputPath; }
		outputs.set(key, command);
	}
	return undefined;
}
