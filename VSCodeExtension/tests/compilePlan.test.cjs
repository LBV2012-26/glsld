const assert = require('node:assert/strict');
const path = require('node:path');
const { test } = require('node:test');
const { createCompileCommands, findCompileOutputConflict } = require('../out/compilePlan');

const filename = path.resolve('Shaders', 'shared shader.glsl');
const command = 'glslc --target-env=vulkan1.3 {defines} -o "{output}" "{}"';

test('per-file stages create separate jobs with the selected stage macro and unique outputs', () => {
  const commands = createCompileCommands(command, filename, { GL_FRAGMENT_SHADER: '1', COMMON: '2' }, [], ['mesh', 'frag']);
  assert.deepEqual(commands.map(command => command.stage), ['mesh', 'frag']);
  assert.equal(commands[0].args[0], '-fshader-stage=mesh');
  assert.ok(commands[0].args.includes('-DGL_MESH_SHADER_EXT=1'));
  assert.equal(commands[0].args.some(argument => argument.includes('GL_FRAGMENT_SHADER')), false);
  assert.ok(commands[1].args.includes('-DGL_FRAGMENT_SHADER=1'));
  assert.equal(commands[1].args.some(argument => argument.includes('GL_MESH_SHADER_EXT')), false);
  assert.ok(commands[0].args.includes('COMMON=2'));
  assert.deepEqual(commands.map(command => command.outputPath), ['mesh', 'frag'].map(stage => path.join(path.dirname(filename), `shared shader.${stage}.spv`)));
  assert.equal(findCompileOutputConflict(commands), undefined);
});

test('build variants are combined with file stages and override other common macro values', () => {
  const commands = createCompileCommands(command, filename, { QUALITY: '0' }, [
    { name: 'debug', defines: { QUALITY: '1' } }, { name: 'release', defines: { QUALITY: '2' } },
  ], ['mesh', 'frag']);
  assert.equal(commands.length, 4);
  assert.ok(commands[0].args.includes('QUALITY=1'));
  assert.ok(commands[2].args.includes('QUALITY=2'));
  assert.equal(new Set(commands.map(command => command.outputPath)).size, 4);
  assert.ok(commands[0].outputPath.endsWith('shared shader.debug.mesh.spv'));
});

test('existing output templates get stage and variant suffixes without editing the group command', () => {
  const commands = createCompileCommands('glslc {defines} -o "{}.spv" "{}"', filename, {}, [
    { name: 'debug', defines: {} }, { name: 'release', defines: {} },
  ], ['mesh', 'frag']);
  assert.equal(commands.length, 4);
  assert.equal(commands[0].outputPath, `${filename}.debug.mesh.spv`);
  assert.equal(commands[3].outputPath, `${filename}.release.frag.spv`);
  assert.equal(findCompileOutputConflict(commands), undefined);
});

test('explicit stage and variant output placeholders are respected without duplicate suffixes', () => {
  const commands = createCompileCommands('glslc -fshader-stage={stage} {defines} "{}" -o "{dir}/{name}.{variant}.{stage}.spv"',
    filename, {}, [{ name: 'debug', defines: {} }], ['mesh', 'frag']);
  assert.equal(commands[0].outputPath, path.join(path.dirname(filename), 'shared shader.debug.mesh.spv'));
  assert.deepEqual(commands[0].args.filter(arg => arg.startsWith('-fshader-stage=')), ['-fshader-stage=mesh']);
});

test('stage flags and macros are injected even when the command has no stage or defines placeholders', () => {
  const [job] = createCompileCommands('glslc "{}"', filename, {}, [], ['mesh']);
  assert.ok(job.args.includes('-fshader-stage=mesh'));
  assert.ok(job.args.includes('-DGL_MESH_SHADER_EXT=1'));
  assert.ok(job.outputPath.endsWith('shared shader.mesh.spv'));
});

test('files without stage configuration retain legacy manual commands and original output names', () => {
  const [job] = createCompileCommands('glslc -fshader-stage=vertex {defines} "{}" -o "{}.spv"', filename, { MY_MACRO: '1' });
  assert.equal(job.stage, undefined);
  assert.deepEqual(job.args, ['-fshader-stage=vertex', '-D', 'MY_MACRO=1', filename, '-o', `${filename}.spv`]);
  const [defaultOutput] = createCompileCommands(command, filename, {});
  assert.equal(defaultOutput.outputPath, `${filename}.spv`);
});

test('invalid stages and conflicting fixed stage flags produce actionable errors', () => {
  assert.throws(() => createCompileCommands(command, filename, {}, [], ['not-a-stage']), /Unknown compile stage/);
  assert.throws(() => createCompileCommands(command, filename, {}, [], 'mesh'), /must be an array/);
  assert.throws(() => createCompileCommands('glslc -fshader-stage=vert -o "{output}" "{}"', filename, {}, [], ['mesh', 'frag']), /fixed -fshader-stage/);
  assert.throws(() => createCompileCommands('glslc -fshader-stage={stage} "{}"', filename, {}), /Compile Group/);
});

test('joined output flags, stdout outputs, duplicate selections, and literal replacement characters are handled', () => {
  const literal = path.resolve('Shaders', 'shader$&.glsl');
  const commands = createCompileCommands('glslc "{}" -o"{}.spv"', literal, {}, [], ['mesh', 'mesh', 'frag']);
  assert.equal(commands.length, 2);
  assert.ok(commands[0].args.includes(literal));
  assert.equal(commands[0].outputPath, `${literal}.mesh.spv`);
  const stdout = createCompileCommands('glslc "{}" -o -', filename, {}, [], ['mesh', 'frag']);
  assert.equal(findCompileOutputConflict(stdout), undefined);
  assert.equal(stdout[0].outputPath, undefined);
});

test('cross-group duplicate outputs are detected for staged jobs before compiling', () => {
  const jobs = createCompileCommands(command, filename, {}, [], ['mesh']);
  assert.equal(findCompileOutputConflict([...jobs, ...jobs]), jobs[0].outputPath);
});
