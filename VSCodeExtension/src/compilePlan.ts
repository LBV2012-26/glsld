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

export interface CompileCommand {
	executable: string;
	args: string[];
	variantName: string;
	stage?: string;
	outputPath?: string;
}

export function parseCompileCommand(command: string): { executable: string; args: string[] } {
	const args: string[] = [];
	let current = '';
	let quote: string | undefined;
	for (const character of command) {
		if (quote) {
			if (character === quote) { quote = undefined; }
			else { current += character; }
		} else if (character === '"' || character === "'") { quote = character; }
		else if (/\s/.test(character)) {
			if (current) { args.push(current); current = ''; }
		} else { current += character; }
	}
	if (current) { args.push(current); }
	return { executable: args.shift() ?? 'glslc.exe', args };
}

function outputArgument(args: string[]): { value: string; index: number; joined: boolean } | undefined {
	let output;
	for (let index = 0; index < args.length; index++) {
		if (args[index] === '-o' && index + 1 < args.length) { output = { value: args[++index], index, joined: false }; }
		else if (args[index].startsWith('-o') && args[index].length > 2) { output = { value: args[index].slice(2), index, joined: true }; }
	}
	return output;
}

export function createCompileCommands(
	template: string,
	filename: string,
	defines: Record<string, string>,
	variants: { name: string; defines: Record<string, string> }[] = [],
	selectedStages?: string[],
): CompileCommand[] {
	if (selectedStages !== undefined && !Array.isArray(selectedStages)) { throw new Error('File compile stages must be an array.'); }
	const stages: (ShaderStage | undefined)[] = [...new Set(selectedStages ?? [])].map(name => {
		const stage = typeof name === 'string' ? byName.get(name) : undefined;
		if (!stage) { throw new Error(`Unknown compile stage: ${name}`); }
		return stage;
	});
	if (!stages.length) {
		if (template.includes('{stage}')) { throw new Error('Select stages for this file inside its Compile Group before using {stage}.'); }
		stages.push(undefined);
	}
	const outputTemplate = outputArgument(parseCompileCommand(template).args)?.value ?? '';
	const commands: CompileCommand[] = [];
	for (const variant of variants.length ? variants : [{ name: '', defines: {} }]) {
		for (const stage of stages) {
			const merged = { ...defines, ...variant.defines };
			if (stage) {
				for (const macro of byMacro.keys()) { delete merged[macro]; }
				merged[stage.macro] = '1';
			}
			const directory = path.dirname(filename);
			const name = path.basename(filename, path.extname(filename));
			const output = stage || variant.name
				? path.join(directory, [name, variant.name, stage?.stage, 'spv'].filter(Boolean).join('.')) : `${filename}.spv`;
			const values: Record<string, string> = {
				'': filename, dir: directory, name, variant: variant.name, stage: stage?.stage ?? '', output,
				defines: Object.entries(merged).map(([key, value]) => `-D ${key}=${value}`).join(' '),
			};
			const parsed = parseCompileCommand(template.replace(/\{(defines|variant|stage|output|dir|name|)\}/g, (_match, key: string) => values[key]));
			if (stage) {
				// Stage options must precede the input. Isolate stage defines even if written literally in the template.
				const args: string[] = [];
				for (let index = 0; index < parsed.args.length; index++) {
					const arg = parsed.args[index];
					if (arg.startsWith('-fshader-stage=')) {
						if (byName.get(arg.slice('-fshader-stage='.length))?.stage !== stage.stage) {
							throw new Error('The fixed -fshader-stage conflicts with this file\'s stages. Remove the fixed flag or use -fshader-stage={stage}.');
						}
						continue;
					}
					const definition = arg === '-D' ? parsed.args[index + 1] : arg.startsWith('-D') ? arg.slice(2) : undefined;
					if (definition && byMacro.has(definition.split('=')[0])) {
						if (arg === '-D') { index++; }
						continue;
					}
					args.push(arg);
				}
				parsed.args = [`-fshader-stage=${stage.stage}`, `-D${stage.macro}=1`, ...args];
			}
			let destination = outputArgument(parsed.args);
			if (stage && !destination) {
				parsed.args.push('-o', output);
				destination = outputArgument(parsed.args);
			} else if (stage && destination && destination.value !== '-' && !outputTemplate.includes('{output}')) {
				// Existing group commands also get a distinct output for each configured stage/variant.
				const suffix = [
					variants.length > 1 && !outputTemplate.includes('{variant}') ? variant.name : '',
					!outputTemplate.includes('{stage}') ? stage.stage : '',
				].filter(Boolean);
				if (suffix.length) {
					const file = path.parse(destination.value);
					destination.value = path.join(file.dir, `${file.name}.${suffix.join('.')}${file.ext}`);
					parsed.args[destination.index] = (destination.joined ? '-o' : '') + destination.value;
				}
			}
			commands.push({ ...parsed, variantName: variant.name, stage: stage?.stage,
				outputPath: destination && destination.value !== '-' ? path.resolve(directory, destination.value) : undefined });
		}
	}
	return commands;
}

export function findCompileOutputConflict(commands: CompileCommand[]): string | undefined {
	const outputs = new Map<string, CompileCommand>();
	for (const command of commands) {
		if (!command.outputPath) { continue; }
		const key = process.platform === 'win32' ? command.outputPath.toLowerCase() : command.outputPath;
		const previous = outputs.get(key);
		if (previous && (previous.stage || command.stage)) { return command.outputPath; }
		outputs.set(key, command);
	}
	return undefined;
}
