const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { createRequire } = require('node:module');
const { test } = require('node:test');

async function compileFixture(options = {}) {
  const root = path.resolve(__dirname, 'project');
  const files = (options.files ?? ['Shaders/shared.glsl']).map(file => {
    const fsPath = path.join(root, file);
    return { fsPath, toString: () => `file:///${fsPath.replace(/\\/g, '/')}` };
  });
  const state = { calls: [], output: [], information: [], warnings: [] };
  const template = options.command ?? 'glslc {defines} -o "{output}" "{}"';
  const mocks = {
    vscode: {
      window: {
        createOutputChannel: () => ({ show() {}, appendLine: line => state.output.push(line) }),
        showWarningMessage: message => state.warnings.push(message),
        showInformationMessage: message => state.information.push(message),
      },
      workspace: {
        getConfiguration: () => ({ get: (key, fallback) => key === 'glslc.path' ? 'configured-glslc.exe' : key === 'compileMaxConcurrency' ? 1 : fallback }),
        findFiles: async () => files,
        asRelativePath: uri => path.relative(root, uri.fsPath).replace(/\\/g, '/'),
      },
      Uri: { parse: value => files.find(file => file.toString() === value) },
    },
    fs: { existsSync: () => false }, // No source reads: stages are explicitly configured.
    child_process: {
      execFile: (executable, args, settings, callback) => {
        state.calls.push({ executable, args, settings });
        callback(null, '', '');
      },
    },
    './sidebar': {
      getCompileGroups: () => options.groups ?? [{ name: 'Shaders', command: template, include: ['**/*.glsl'], variants: options.variants, fileStages: options.stages }],
      getActiveGlobalMacros: () => options.defines ?? {},
      getFileMacrosData: () => ({}),
    },
  };
  const file = path.resolve(__dirname, '../out/compile.js');
  const localRequire = createRequire(file);
  const sandbox = { exports: {}, require: name => mocks[name] ?? localRequire(name), process: { env: {} } };
  vm.runInNewContext(fs.readFileSync(file, 'utf8'), sandbox, { filename: file });
  await sandbox.exports.compileWorkspace();
  return state;
}

test('Compile Groups use each file\'s selected stages, not a group-wide stage list', async () => {
  const state = await compileFixture({ files: ['Shaders/shared.glsl', 'Shaders/post.glsl'], stages: {
    'Shaders/shared.glsl': ['mesh', 'frag'], 'Shaders/post.glsl': ['comp'],
  } });
  assert.equal(state.calls.length, 3);
  assert.deepEqual(state.calls.map(call => call.args[0]), ['-fshader-stage=mesh', '-fshader-stage=frag', '-fshader-stage=comp']);
  assert.ok(state.calls.every(call => call.executable === 'configured-glslc.exe'));
  assert.ok(state.output.some(line => line.includes('2 stage(s): mesh, frag')));
  assert.equal(state.warnings.length, 0);
});

test('manual group variants work without per-file stages or source reads', async () => {
  const state = await compileFixture({
    command: 'glslc -fshader-stage={variant} {defines} -o "{name}.{variant}.spv" "{}"',
    variants: [{ name: 'vertex', defines: { VERTEX: '1' } }, { name: 'fragment', defines: { FRAGMENT: '1' } }],
  });
  assert.equal(state.calls.length, 2);
  assert.equal(state.calls[0].args[0], '-fshader-stage=vertex');
  assert.equal(state.calls[1].args[0], '-fshader-stage=fragment');
  assert.equal(state.warnings.length, 0);
});

test('an invalid file stage is reported while other configured files can still compile', async () => {
  const state = await compileFixture({ files: ['Shaders/shared.glsl', 'Shaders/post.glsl'], stages: {
    'Shaders/shared.glsl': ['invalid'], 'Shaders/post.glsl': ['comp'],
  } });
  assert.equal(state.calls.length, 1);
  assert.ok(state.output.some(line => line.includes('Unknown compile stage: invalid')));
  assert.ok(state.warnings.some(message => message.includes('1 succeeded, 1 failed')));
});

test('overlapping staged groups cannot overwrite the same output', async () => {
  const group = { name: 'Shaders', command: 'glslc -o "{output}" "{}"', include: ['**/*.glsl'], fileStages: { 'Shaders/shared.glsl': ['mesh', 'frag'] } };
  const state = await compileFixture({ groups: [group, group] });
  assert.equal(state.calls.length, 0);
  assert.ok(state.warnings[0].includes('would overwrite'));
});

test('the same file can have different stages in different Compile Groups', async () => {
  const base = { command: 'glslc -o "{output}" "{}"', include: ['**/*.glsl'] };
  const state = await compileFixture({ groups: [
    { ...base, name: 'Mesh pass', fileStages: { 'Shaders/shared.glsl': ['mesh'] } },
    { ...base, name: 'Fragment pass', fileStages: { 'Shaders/shared.glsl': ['frag'] } },
  ] });
  assert.deepEqual(state.calls.map(call => call.args[0]), ['-fshader-stage=mesh', '-fshader-stage=frag']);
  assert.equal(state.warnings.length, 0);
});
