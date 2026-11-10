const assert = require('node:assert/strict');
const cp = require('node:child_process');
const fs = require('node:fs/promises');
const os = require('node:os');
const path = require('node:path');
const { test } = require('node:test');
const { checkClangFormat, getClangFormatCommand } = require('../out/clangFormat');

const defaultName = process.platform === 'win32' ? 'clang-format.exe' : 'clang-format';

async function fixture(t) {
  const root = await fs.mkdtemp(path.join(os.tmpdir(), 'glsld-formatter-test-'));
  t.after(() => fs.rm(root, { recursive: true, force: true }));
  const tools = path.join(root, 'LLVM tools');
  await fs.mkdir(tools);
  const executable = path.join(tools, defaultName);
  await fs.writeFile(executable, 'executable fixture');
  await fs.chmod(executable, 0o755);
  return { root, tools, executable, options: { baseDirectory: root, env: { PATH: tools } } };
}

function mockVersion(t, options = {}) {
  const calls = [];
  t.mock.method(cp, 'execFile', (executable, args, settings, callback) => {
    calls.push({ executable, args, settings });
    queueMicrotask(() => callback(options.error ?? null, options.stdout ?? 'clang-format version 19.1.0', options.stderr ?? ''));
    return {};
  });
  return calls;
}

test('formatter defaults match the extension host and relative settings resolve from the workspace', () => {
  const root = path.resolve('workspace');
  assert.equal(getClangFormatCommand('', root, 'win32'), 'clang-format.exe');
  assert.equal(getClangFormatCommand('  ', root, 'linux'), 'clang-format');
  assert.equal(getClangFormatCommand('', root, 'darwin'), 'clang-format');
  assert.equal(getClangFormatCommand('clang-format-19', root), 'clang-format-19');
  assert.equal(getClangFormatCommand('tools/clang-format', root), path.join(root, 'tools', 'clang-format'));
  assert.equal(getClangFormatCommand('${workspaceFolder}/tools/clang-format', root), path.join(root, 'tools', 'clang-format'));
});

test('PATH lookup uses an absolute executable and verifies --version without a shell', async t => {
  const { executable, options } = await fixture(t);
  const calls = mockVersion(t);
  const result = await checkClangFormat('', options);
  assert.equal(result.available, true);
  assert.equal(result.configured, false);
  assert.equal(result.executablePath, executable);
  assert.equal(calls.length, 1);
  assert.deepEqual(calls[0].args, ['--version']);
  assert.equal(calls[0].settings.shell, false);
  assert.equal(calls[0].settings.windowsHide, true);
  assert.ok(calls[0].settings.timeout > 0);
});

test('a quoted explicit path with spaces takes priority over PATH', async t => {
  const { root, executable, options } = await fixture(t);
  const custom = path.join(root, `custom formatter${path.extname(defaultName)}`);
  await fs.copyFile(executable, custom);
  await fs.chmod(custom, 0o755);
  const calls = mockVersion(t);
  const result = await checkClangFormat(` "${custom}" `, options);
  assert.equal(result.available, true);
  assert.equal(result.configured, true);
  assert.equal(result.executablePath, custom);
  assert.equal(calls[0].executable, custom);
});

test('an invalid explicit setting reports failure even when a working formatter is in PATH', async t => {
  const { root, options } = await fixture(t);
  const calls = mockVersion(t);
  const result = await checkClangFormat(path.join(root, 'missing-formatter.exe'), options);
  assert.equal(result.available, false);
  assert.equal(result.configured, true);
  assert.match(result.error, /configured executable.*not found|not accessible/);
  assert.equal(calls.length, 0);
});

test('configured command names and quoted PATH entries are supported', async t => {
  const { root, tools, executable } = await fixture(t);
  const name = process.platform === 'win32' ? 'clang-format-19.exe' : 'clang-format-19';
  await fs.rename(executable, path.join(tools, name));
  const calls = mockVersion(t);
  const result = await checkClangFormat('clang-format-19', {
    baseDirectory: root,
    env: { [process.platform === 'win32' ? 'pAtH' : 'PATH']: `"${tools}"` },
  });
  assert.equal(result.available, true);
  assert.equal(result.executablePath, path.join(tools, name));
  assert.equal(calls.length, 1);
});

test('workspace-relative paths and workspaceFolder variables refer to the checked executable', async t => {
  const { executable, options } = await fixture(t);
  mockVersion(t);
  for (const configured of [`LLVM tools/${defaultName}`, `\${workspaceFolder}/LLVM tools/${defaultName}`]) {
    const result = await checkClangFormat(configured, options);
    assert.equal(result.available, true);
    assert.equal(result.executablePath, executable);
  }
});

test('an empty PATH does not silently use an executable from the working directory', async t => {
  const { root, executable, options } = await fixture(t);
  await fs.copyFile(executable, path.join(root, defaultName));
  const calls = mockVersion(t);
  const result = await checkClangFormat('', { ...options, env: { PATH: '' } });
  assert.equal(result.available, false);
  assert.match(result.error, /PATH/);
  assert.equal(calls.length, 0);
});

test('a directory cannot be used as clang-format', async t => {
  const { tools, options } = await fixture(t);
  const calls = mockVersion(t);
  const result = await checkClangFormat(tools, options);
  assert.equal(result.available, false);
  assert.equal(calls.length, 0);
});

test('a formatter that exists but cannot run reports the execution error', async t => {
  const { executable, options } = await fixture(t);
  mockVersion(t, { error: Object.assign(new Error('Missing runtime DLL'), { code: 1 }) });
  const result = await checkClangFormat('', options);
  assert.equal(result.available, false);
  assert.equal(result.executablePath, executable);
  assert.match(result.error, /Missing runtime DLL/);
});

test('a program that exits successfully but is not clang-format is rejected using a real process', async t => {
  const { root } = await fixture(t);
  const result = await checkClangFormat(process.execPath, { baseDirectory: root });
  assert.equal(result.available, false);
  assert.match(result.error, /did not identify itself as clang-format/);
});

test('a cancelled check does not run the formatter', async t => {
  const { options } = await fixture(t);
  const calls = mockVersion(t);
  const abort = new AbortController();
  abort.abort();
  await assert.rejects(checkClangFormat('', { ...options, signal: abort.signal }), /abort/i);
  assert.equal(calls.length, 0);
});

test('cancelling an active formatter check aborts rather than reporting a missing executable', async t => {
  const { options } = await fixture(t);
  const abort = new AbortController();
  let entered;
  const executing = new Promise(resolve => { entered = resolve; });
  t.mock.method(cp, 'execFile', (_executable, _args, settings, callback) => {
    settings.signal.addEventListener('abort', () => callback(settings.signal.reason, '', ''), { once: true });
    entered();
    return {};
  });
  const checking = checkClangFormat('', { ...options, signal: abort.signal });
  await executing;
  abort.abort(new Error('Check stopped'));
  await assert.rejects(checking, /Check stopped/);
});
