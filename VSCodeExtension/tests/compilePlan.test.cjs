const assert = require('node:assert/strict');
const path = require('node:path');
const { test } = require('node:test');
const { createCompileCommands, findCompileOutputConflict, parseCompileOptions } = require('../out/compilePlan');

const root = path.resolve('workspace with spaces');
const filename = path.join(root, 'Shaders', 'shared shader.glsl');
const settings = { outputPath: 'build/shaders', options: '-O -g', targetEnv: 'vulkan1.3', targetSpv: 'spv1.6' };
const plan = (overrides = {}, stages = ['mesh', 'frag'], defines = {}) =>
  createCompileCommands({ ...settings, ...overrides }, filename, defines, stages, root);

test('each selected stage has its own output and corresponding isolated macro', () => {
  const commands = plan({}, ['mesh', 'frag'], { GL_FRAGMENT_SHADER: '0', GL_COMPUTE_SHADER: '1', COMMON: 'two words' });
  assert.deepEqual(commands.map(command => command.stage), ['mesh', 'frag']);
  assert.deepEqual(commands.map(command => command.outputPath),
    ['mesh', 'frag'].map(stage => path.join(root, 'build/shaders/Shaders', 'shared shader.' + stage + '.spv')));
  for (const command of commands) {
    assert.equal(command.cwd, root);
    assert.ok(command.args.includes(filename));
    assert.ok(command.args.includes('--target-env=vulkan1.3'));
    assert.ok(command.args.includes('--target-spv=spv1.6'));
    assert.ok(command.args.includes('-O'));
    assert.ok(command.args.includes('-g'));
    assert.ok(command.args.includes('-DCOMMON=two words'));
    assert.deepEqual(command.args.slice(-2), ['-o', command.outputPath]);
    assert.equal(command.args.filter(arg => arg.startsWith('-DGL_')).length, 1);
  }
  assert.ok(commands[0].args.includes('-DGL_MESH_SHADER_EXT=1'));
  assert.ok(commands[1].args.includes('-DGL_FRAGMENT_SHADER=1'));
  assert.equal(findCompileOutputConflict(commands), undefined);
});

test('compiler options preserve quoted paths, Windows separators and macro values', () => {
  const options = '-I "C:\\Shader Includes\\" -D \'MESSAGE=hello world\' -DMODE=2';
  assert.deepEqual(parseCompileOptions(options), ['-I', 'C:\\Shader Includes\\', '-D', 'MESSAGE=hello world', '-DMODE=2']);
  const [command] = plan({ options }, ['comp']);
  assert.ok(command.args.includes('C:\\Shader Includes\\'));
  assert.ok(command.args.includes('MESSAGE=hello world'));
  assert.throws(() => parseCompileOptions('-I "unfinished'), /Unclosed quote/);
});

test('literal stage defines and undefines in compiler options cannot override selected stages', () => {
  const [command] = plan({ options: '-DGL_FRAGMENT_SHADER=1 -D GL_COMPUTE_SHADER -UGL_MESH_SHADER_EXT -U GL_VERTEX_SHADER -DKEEP=3' }, ['mesh']);
  assert.deepEqual(command.args.filter(arg => arg.includes('GL_')), ['-DGL_MESH_SHADER_EXT=1']);
  assert.ok(command.args.includes('-DKEEP=3'));
});

test('managed output, stage and target flags must use the matching UI settings', () => {
  for (const options of ['-o bad.spv', '-obad.spv', '-fshader-stage=frag', '--target-env=opengl', '--target-spv=spv1.0']) {
    assert.throws(() => plan({ options }), /group's output\/target settings/);
  }
});

test('absolute output directories and the workspaceFolder variable are supported', () => {
  const absolute = path.resolve('elsewhere', 'output');
  assert.equal(plan({ outputPath: absolute })[0].outputPath, path.join(absolute, 'Shaders/shared shader.mesh.spv'));
  assert.equal(plan({ outputPath: '${workspaceFolder}/generated' })[0].outputPath,
    path.join(root, 'generated/Shaders/shared shader.mesh.spv'));
});

test('an external input never escapes the chosen output directory', () => {
  const external = path.resolve(root, '..', 'external', 'outside.glsl');
  const [command] = createCompileCommands(settings, external, {}, ['vert'], root);
  assert.equal(command.outputPath, path.join(root, 'build/shaders/outside.vert.spv'));
});

test('matching basenames in different source folders have different outputs', () => {
  const first = createCompileCommands(settings, path.join(root, 'Lighting/shared.glsl'), {}, ['frag'], root);
  const second = createCompileCommands(settings, path.join(root, 'Post/shared.glsl'), {}, ['frag'], root);
  assert.equal(findCompileOutputConflict([...first, ...second]), undefined);
});

test('target environment defaults and automatic SPIR-V target do not add an explicit version', () => {
  const [command] = plan({ targetEnv: undefined, targetSpv: '' }, ['vert']);
  assert.ok(command.args.includes('--target-env=vulkan1.3'));
  assert.equal(command.args.some(arg => arg.startsWith('--target-spv=')), false);
  assert.ok(plan({ targetEnv: 'opengl', targetSpv: '' }, ['frag'])[0].args.includes('--target-env=opengl'));
  assert.throws(() => plan({ targetEnv: 'bad' }), /Unknown target environment/);
  assert.throws(() => plan({ targetSpv: 'bad' }), /Unknown SPIR-V target/);
});

test('invalid or missing stages and missing output directories give actionable errors', () => {
  assert.throws(() => plan({}, ['bad']), /Unknown compile stage: bad/);
  assert.throws(() => plan({}, []), /Select at least one stage/);
  assert.throws(() => plan({}, 'mesh'), /Select at least one stage/);
  assert.throws(() => plan({ outputPath: '' }), /Edit this Compile Group/);
});

test('duplicate stage aliases compile once and assembly outputs use spvasm', () => {
  const commands = plan({ options: '-S' }, ['mesh', 'mesh', 'fragment', 'frag']);
  assert.equal(commands.length, 2);
  assert.deepEqual(commands.map(command => command.stage), ['mesh', 'frag']);
  assert.ok(commands.every(command => command.outputPath.endsWith('.spvasm')));
});

test('duplicate outputs across groups are detected before compilation', () => {
  const commands = plan();
  assert.equal(findCompileOutputConflict([...commands, ...commands]), commands[0].outputPath);
  assert.equal(findCompileOutputConflict([...commands, ...plan({ outputPath: 'other' })]), undefined);
});
