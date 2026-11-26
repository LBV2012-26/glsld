const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { createRequire } = require('node:module');
const { fileURLToPath, pathToFileURL } = require('node:url');
const { test } = require('node:test');

const root = path.resolve(__dirname, 'project');
const uri = value => {
  const fsPath = path.resolve(root, value);
  return { fsPath, toString: () => pathToFileURL(fsPath).toString() };
};
const group = { name: 'Shaders', outputPath: 'build/shaders', options: '-O', targetEnv: 'vulkan1.3', targetSpv: 'spv1.6' };

async function compileFixture(options = {}) {
  const state = { calls: [], directories: [], output: [], information: [], warnings: [] };
  const mocks = {
    vscode: {
      window: {
        createOutputChannel: () => ({ show() {}, appendLine: line => state.output.push(line) }),
        showWarningMessage: message => state.warnings.push(message),
        showInformationMessage: message => state.information.push(message),
      },
      workspace: {
        workspaceFolders: options.folders ?? [{ name: 'project', uri: uri(root) }],
        getConfiguration: () => ({ get: (key, fallback) => key === 'glslc.path' ? 'configured-glslc.exe' : key === 'compileMaxConcurrency' ? 1 : fallback }),
        findFiles: () => { throw new Error('Compile Groups must not scan the workspace'); },
        asRelativePath: file => path.relative(root, file.fsPath).replace(/\\/g, '/'),
      },
      Uri: { parse: value => uri(fileURLToPath(value)), file: uri },
    },
    fs: {
      existsSync: () => false,
      readFileSync: () => { throw new Error('Do not scan source code for stages'); },
      promises: { mkdir: async (directory, settings) => {
        state.directories.push({ directory, settings });
        if (options.mkdirError) throw new Error('Output directory is not writable');
      } },
    },
    child_process: {
      execFile: (executable, args, settings, callback) => {
        state.calls.push({ executable, args, settings });
        callback(options.compilerError ? { code: 1 } : null, '', options.compilerError ? 'Shader compile error' : '');
      },
    },
    './sidebar': {
      getCompileGroups: () => options.groups ?? [{ ...group, fileStages: options.stages ?? { 'Shaders/shared.glsl': ['mesh', 'frag'] } }],
      getActiveGlobalMacros: () => options.defines ?? {},
      getFileMacrosData: () => options.fileMacros ?? {},
    },
  };
  const file = path.resolve(__dirname, '../out/compile.js');
  const localRequire = createRequire(file);
  const sandbox = { exports: {}, require: name => mocks[name] ?? localRequire(name), process: { env: {} } };
  vm.runInNewContext(fs.readFileSync(file, 'utf8'), sandbox, { filename: file });
  await sandbox.exports.compileWorkspace();
  return state;
}

test('only manually added files are compiled with their own selected stages and group settings', async () => {
  const state = await compileFixture({ stages: {
    'Shaders/shared.glsl': ['mesh', 'frag'], 'Shaders/post.glsl': ['comp'],
  } });
  assert.equal(state.calls.length, 3);
  assert.deepEqual(state.calls.map(call => call.args[0]), ['-fshader-stage=mesh', '-fshader-stage=frag', '-fshader-stage=comp']);
  for (const call of state.calls) {
    assert.equal(call.executable, 'configured-glslc.exe');
    assert.equal(call.settings.cwd, root);
    assert.equal(call.settings.windowsHide, true);
    assert.ok(call.args.includes('-O'));
    assert.ok(call.args.includes('--target-env=vulkan1.3'));
    assert.ok(call.args.includes('--target-spv=spv1.6'));
  }
  assert.ok(state.output.some(line => line.includes('2 stage(s): mesh, frag')));
  assert.equal(state.warnings.length, 0);
  assert.equal(state.directories.length, 3);
  assert.ok(state.directories.every(item => item.directory === path.join(root, 'build/shaders/Shaders') && item.settings.recursive));
});

test('enabled global and file macros are merged while disabled macros and other stages are excluded', async () => {
  const state = await compileFixture({
    stages: { 'Shaders/shared.glsl': ['mesh'] },
    defines: { COMMON: '1', OVERRIDE: 'global', GL_FRAGMENT_SHADER: '1' },
    fileMacros: { [uri('Shaders/shared.glsl').toString()]: {
      OVERRIDE: { value: 'file', enabled: true }, DISABLED: { value: '1', enabled: false },
    } },
  });
  const args = state.calls[0].args;
  assert.ok(args.includes('-DCOMMON=1'));
  assert.ok(args.includes('-DOVERRIDE=file'));
  assert.ok(args.includes('-DGL_MESH_SHADER_EXT=1'));
  assert.equal(args.some(arg => arg.includes('DISABLED') || arg.includes('GL_FRAGMENT_SHADER')), false);
});

test('an invalid file stage is reported while other manually configured files still compile', async () => {
  const state = await compileFixture({ stages: {
    'Shaders/shared.glsl': ['invalid'], 'Shaders/post.glsl': ['comp'],
  } });
  assert.equal(state.calls.length, 1);
  assert.ok(state.output.some(line => line.includes('Unknown compile stage: invalid')));
  assert.ok(state.warnings.some(message => message.includes('1 succeeded, 1 failed')));
});

test('overlapping groups cannot overwrite the same output and create no directories', async () => {
  const duplicate = { ...group, fileStages: { 'Shaders/shared.glsl': ['mesh', 'frag'] } };
  const state = await compileFixture({ groups: [duplicate, duplicate] });
  assert.equal(state.calls.length, 0);
  assert.equal(state.directories.length, 0);
  assert.ok(state.warnings[0].includes('would overwrite'));
});

test('the same file can use different stages or output settings in different groups', async () => {
  const state = await compileFixture({ groups: [
    { ...group, name: 'Mesh pass', fileStages: { 'Shaders/shared.glsl': ['mesh'] } },
    { ...group, name: 'Fragment pass', fileStages: { 'Shaders/shared.glsl': ['frag'] } },
    { ...group, name: 'Debug mesh', outputPath: 'debug', options: '-g', targetEnv: 'vulkan1.2', fileStages: { 'Shaders/shared.glsl': ['mesh'] } },
  ] });
  assert.deepEqual(state.calls.map(call => call.args[0]), ['-fshader-stage=mesh', '-fshader-stage=frag', '-fshader-stage=mesh']);
  assert.ok(state.calls[2].args.includes('--target-env=vulkan1.2'));
  assert.ok(state.calls[2].args.at(-1).startsWith(path.join(root, 'debug')));
  assert.equal(state.warnings.length, 0);
});

test('empty groups prompt to add files and legacy globs never select extra sources', async () => {
  const empty = await compileFixture({ groups: [{ ...group, include: ['**/*.glsl'], fileStages: {} }] });
  assert.equal(empty.calls.length, 0);
  assert.ok(empty.warnings[0].includes('No files'));
  const legacy = await compileFixture({ groups: [{ ...group, include: ['**/*.glsl'], command: 'ignored', fileStages: { 'Shaders/only.glsl': ['vert'] } }] });
  assert.equal(legacy.calls.length, 1);
  assert.ok(legacy.calls[0].args.includes(path.join(root, 'Shaders/only.glsl')));
  const unconfigured = await compileFixture({ groups: [{ name: 'Legacy', command: 'glslc "{}"', fileStages: { 'Shaders/only.glsl': ['vert'] } }] });
  assert.equal(unconfigured.calls.length, 0);
  assert.ok(unconfigured.output.some(line => line.includes('set its output directory')));
});

test('output-directory and compiler failures produce a failed summary instead of stopping the command', async () => {
  const directoryError = await compileFixture({ mkdirError: true });
  assert.equal(directoryError.calls.length, 0);
  assert.ok(directoryError.output.some(line => line.includes('Output directory is not writable')));
  assert.ok(directoryError.warnings[0].includes('0 succeeded, 2 failed'));
  const compilerError = await compileFixture({ compilerError: true });
  assert.equal(compilerError.calls.length, 2);
  assert.ok(compilerError.output.some(line => line.includes('Shader compile error')));
  assert.ok(compilerError.warnings[0].includes('0 succeeded, 2 failed'));
});

test('absolute paths and file URIs are supported for manually added files', async () => {
  const absolute = path.resolve(root, '../external/absolute.glsl');
  const fileUri = uri('Shaders/uri file.glsl').toString();
  const state = await compileFixture({ stages: { [absolute]: ['vert'], [fileUri]: ['frag'] } });
  assert.ok(state.calls[0].args.includes(absolute));
  assert.ok(state.calls[1].args.includes(uri('Shaders/uri file.glsl').fsPath));
  assert.equal(state.warnings.length, 0);
});

test('file keys with a workspace-folder prefix resolve in a multi-root workspace', async () => {
  const secondRoot = path.resolve(root, '../second');
  const state = await compileFixture({
    folders: [{ name: 'first', uri: uri(root) }, { name: 'second', uri: uri(secondRoot) }],
    stages: { 'first/Shaders/main.glsl': ['vert'], 'second/Shaders/main.glsl': ['frag'] },
  });
  assert.ok(state.calls[0].args.includes(path.join(root, 'Shaders/main.glsl')));
  assert.ok(state.calls[1].args.includes(path.join(secondRoot, 'Shaders/main.glsl')));
  assert.equal(state.warnings.length, 0);
});
