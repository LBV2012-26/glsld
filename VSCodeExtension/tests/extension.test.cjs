const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { createRequire } = require('node:module');
const { test } = require('node:test');

const extensionFile = path.resolve(__dirname, '../out/extension.js');
const extensionSource = fs.readFileSync(extensionFile, 'utf8');
const extensionRequire = createRequire(extensionFile);

class Disposable {
  constructor(callback = () => {}) { this.callback = callback; }
  dispose() { this.callback(); }
}

function event() {
  const callbacks = new Set();
  return {
    subscribe(callback) { callbacks.add(callback); return new Disposable(() => callbacks.delete(callback)); },
    async fire(value) { await Promise.all([...callbacks].map(callback => callback(value))); },
  };
}

async function activateFixture(t, options = {}) {
  const root = path.join(__dirname, 'virtual-storage');
  const state = {
    config: { ...options.config },
    installed: new Map(options.installed ? [['v0.9.5', options.installed]] : []),
    latestVersion: options.latestVersion ?? 'v0.9.5', currentVersion: 'v0.9.5', checks: 0,
    checkError: options.checkError,
    commands: new Map(), clients: [], sidebarClients: [], starts: [],
    errors: [], information: [], output: [], progress: [],
    downloads: 0, restarts: 0, stops: 0, compilations: 0, cancelProgress: false,
    onConfiguration: event(), onOpenDocument: event(),
  };
  const disposableEvent = () => new Disposable();
  const vscode = {
    Disposable,
    CancellationError: class CancellationError extends Error {},
    ProgressLocation: { Notification: 15 },
    commands: {
      registerCommand(name, callback) {
        assert.equal(state.commands.has(name), false, `Command registered twice: ${name}`);
        state.commands.set(name, callback);
        return new Disposable(() => state.commands.delete(name));
      },
      executeCommand(name, ...args) { return Promise.resolve(state.commands.get(name)?.(...args)); },
    },
    window: {
      activeTextEditor: undefined,
      visibleTextEditors: [],
      createOutputChannel: () => ({ appendLine: line => state.output.push(line), show() {}, dispose() {} }),
      createTextEditorDecorationType: () => new Disposable(),
      registerTreeDataProvider: () => new Disposable(),
      onDidChangeActiveTextEditor: disposableEvent,
      onDidChangeVisibleTextEditors: disposableEvent,
      showInformationMessage: async message => { state.information.push(message); },
      showErrorMessage: async message => { state.errors.push(message); },
      withProgress: async (_options, callback) => callback({ report: progress => state.progress.push(progress) }, {
        isCancellationRequested: state.cancelProgress,
        onCancellationRequested: disposableEvent,
      }),
    },
    workspace: {
      textDocuments: options.withGlsl ? [{ languageId: 'glsl', uri: { toString: () => 'file:///shader.glsl' } }] : [],
      workspaceFolders: [],
      getConfiguration: () => ({ get: (key, fallback) => state.config[key] ?? fallback }),
      createFileSystemWatcher: () => new Disposable(),
      onDidChangeConfiguration: callback => state.onConfiguration.subscribe(callback),
      onDidOpenTextDocument: callback => state.onOpenDocument.subscribe(callback),
      onDidChangeTextDocument: disposableEvent,
      onDidCloseTextDocument: disposableEvent,
    },
  };
  class LanguageClient {
    running = false;
    notifications = [];
    constructor(_id, _name, serverOptions) { this.serverOptions = serverOptions; state.clients.push(this); }
    isRunning() { return this.running; }
    async start() {
      state.starts.push({ command: this.serverOptions.run.command, env: { ...this.serverOptions.run.options.env } });
      if (state.failNextStart) { state.failNextStart = false; throw new Error('Failed to launch test server'); }
      this.running = true;
    }
    async stop() { state.stops++; this.running = false; }
    async restart() { state.restarts++; await this.stop(); await this.start(); }
    async sendNotification(method, payload) { assert.equal(this.running, true); this.notifications.push({ method, payload }); }
    async sendRequest() { assert.equal(this.running, true); return { data: [] }; }
  }
  class ServerManager {
    constructor(storage) { assert.equal(storage, root); }
    async resolveServerPath(configuredPath, extensionPath) {
      if (configuredPath === 'missing') throw new Error('Invalid custom executable');
      if (configuredPath) return path.resolve(extensionPath, configuredPath);
      return state.installed.get(state.currentVersion);
    }
    async getInstalledServer() {
      const executablePath = state.installed.get(state.currentVersion);
      return executablePath ? { version: state.currentVersion, executablePath } : undefined;
    }
    async checkForUpdates(options) {
      options.signal.throwIfAborted();
      state.checks++;
      if (state.checkError) throw state.checkError;
      const installed = state.installed.has(state.latestVersion);
      if (installed) state.currentVersion = state.latestVersion;
      return { version: state.latestVersion, installed };
    }
    async downloadServer(options) {
      state.downloads++;
      options.signal.throwIfAborted();
      if (state.download) await state.download(options);
      const tag = state.latestVersion;
      const executablePath = path.join(root, 'servers', tag, 'win32-x64', 'glsld.exe');
      state.installed.set(tag, executablePath);
      state.currentVersion = tag;
      return { version: tag, target: 'win32-x64', executablePath };
    }
  }
  const mocks = {
    vscode,
    'vscode-languageclient/node': { LanguageClient, TransportKind: { stdio: 0 } },
    './serverManager': { ServerManager },
    './sidebar': {
      activateSidebar: (_context, client) => state.sidebarClients.push(client),
      getActiveVariants: () => ({}), getShaderConfigs: () => ({}),
    },
    './compile': { compileWorkspace: () => state.compilations++ },
    './shaderConfig': { ShaderConfigProvider: class { refresh() {} } },
  };
  const sandbox = {
    exports: {},
    require: name => mocks[name] ?? extensionRequire(name),
    console: { log() {} }, process: { env: {} },
    setTimeout, clearTimeout, AbortController, Error,
  };
  vm.runInNewContext(extensionSource, sandbox, { filename: extensionFile });
  const context = {
    globalStorageUri: { fsPath: root },
    extensionPath: path.join(__dirname, 'virtual-extension'),
    extension: { packageJSON: { version: '0.9.5' } },
    subscriptions: [],
  };
  state.extension = sandbox.exports;
  t.after(async () => {
    await sandbox.exports.deactivate();
    for (const disposable of context.subscriptions) disposable.dispose();
  });
  await sandbox.exports.activate(context);
  return { state, vscode, context };
}

test('first activation without a binary still registers commands and views without downloading', async t => {
  const { state, vscode } = await activateFixture(t, { withGlsl: true });
  assert.equal(state.starts.length, 0);
  assert.equal(state.downloads, 0);
  assert.equal(state.checks, 1);
  assert.equal(state.errors.length, 0);
  assert.equal(state.information.length, 1);
  assert.ok(state.commands.has('glsld.downloadServer'));
  assert.ok(state.commands.has('glsld.restartServer'));
  assert.ok(state.commands.has('glsld.addShaderConfig'));
  assert.equal(state.sidebarClients.length, 1);
  await vscode.commands.executeCommand('glsld.compileWorkspace');
  assert.equal(state.compilations, 1);
  await state.extension.pushConfiguration();
  state.extension.notifyRemoveConfig('shader.glsl');
});

test('the download command installs to global storage and starts the existing client without reloading', async t => {
  const { state, vscode, context } = await activateFixture(t);
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.equal(state.downloads, 1);
  assert.equal(state.starts.length, 1);
  assert.ok(state.starts[0].command.startsWith(context.globalStorageUri.fsPath + path.sep));
  assert.equal(state.starts[0].command.includes('virtual-extension'), false);
  assert.equal(state.clients.length, 1);
  assert.equal(state.clients[0], state.sidebarClients[0]);
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.restarts, 1);
  assert.equal(state.clients.length, 1);
  assert.equal(state.errors.length, 0);
});

test('download failures leave commands usable and a later retry starts the server', async t => {
  const { state, vscode } = await activateFixture(t);
  state.download = async () => { throw new Error('Release v0.9.5 has not been published'); };
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.match(state.errors[0], /has not been published/);
  assert.equal(state.starts.length, 0);
  state.download = undefined;
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.equal(state.downloads, 2);
  assert.equal(state.starts.length, 1);
});

test('cancellation does not produce an error or start the server and can be retried', async t => {
  const { state, vscode } = await activateFixture(t);
  state.cancelProgress = true;
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.equal(state.starts.length, 0);
  assert.equal(state.errors.length, 0);
  state.cancelProgress = false;
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.equal(state.starts.length, 1);
});

test('a custom path remains active after downloading and invalid custom paths can be corrected', async t => {
  const { state, vscode, context } = await activateFixture(t, { config: { 'server.path': 'local.exe' } });
  assert.equal(state.starts[0].command, path.join(context.extensionPath, 'local.exe'));
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.equal(state.starts.length, 1);
  assert.equal(state.config['server.path'], 'local.exe');
  assert.ok(state.information.some(message => message.includes('Clear glsld.server.path')));
  state.config['server.path'] = 'missing';
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.match(state.errors.at(-1), /Invalid custom executable/);
  assert.equal(state.clients[0].isRunning(), true);
  state.config['server.path'] = '';
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.starts.at(-1).command.startsWith(context.globalStorageUri.fsPath + path.sep), true);
});

test('startup and restart check GitHub and downloading updates an already running cached server', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'old-server.exe' });
  assert.equal(state.checks, 1);
  state.latestVersion = 'v0.9.6';
  state.config['server.version'] = '0.1.0'; // A setting left by an older extension cannot pin downloads.
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.checks, 2);
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.starts.at(-1).command, 'old-server.exe');
  assert.ok(state.information.some(message => message.includes('v0.9.6')));
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.ok(state.starts.at(-1).command.includes(path.join('servers', 'v0.9.6')));
  assert.equal(state.clients.length, 1);
});

test('a failed update check keeps a cached server running without a startup error', async t => {
  const { state, vscode } = await activateFixture(t, {
    installed: 'cached-server.exe', checkError: new Error('Network unavailable'),
  });
  assert.equal(state.clients[0].isRunning(), true);
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.checks, 2);
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.starts.at(-1).command, 'cached-server.exe');
  assert.equal(state.errors.length, 0);
  assert.ok(state.output.some(message => message.includes('Network unavailable')));
});

test('background index extra extensions are sent at startup, on changes, and when cleared', async t => {
  const { state } = await activateFixture(t, {
    installed: 'local-server.exe',
    config: { 'backgroundIndex.extraExtensions': ['.glh', '.ggg'] },
  });
  const sentIndexSettings = () => JSON.parse(JSON.stringify(
    state.clients[0].notifications.filter(notification => notification.method === 'workspace/didChangeConfiguration')
      .at(-1).payload.settings.glsld.backgroundIndex,
  ));
  assert.deepEqual(sentIndexSettings(), { roots: [], extraExtensions: ['.glh', '.ggg'] });
  state.config['backgroundIndex.extraExtensions'] = ['.glh'];
  const event = { affectsConfiguration: name => ['glsld', 'glsld.backgroundIndex.extraExtensions'].includes(name) };
  await state.onConfiguration.fire(event);
  assert.deepEqual(sentIndexSettings().extraExtensions, ['.glh']);
  delete state.config['backgroundIndex.extraExtensions'];
  await state.onConfiguration.fire(event);
  assert.deepEqual(sentIndexSettings().extraExtensions, []);
  assert.equal(state.restarts, 0);
});

test('launch failures can be retried and mimalloc settings are refreshed on restart', async t => {
  const { state, vscode } = await activateFixture(t);
  state.failNextStart = true;
  await vscode.commands.executeCommand('glsld.downloadServer');
  assert.match(state.errors[0], /Failed to launch/);
  state.config['mimalloc.verbose'] = true;
  state.config['mimalloc.largePages.enabled'] = true;
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.starts.at(-1).env.MIMALLOC_VERBOSE, '1');
  assert.equal(state.starts.at(-1).env.MIMALLOC_ALLOW_LARGE_OS_PAGES, '1');
  assert.equal(state.clients[0].isRunning(), true);
});

test('deactivation cancels an in-flight download and prevents starting after shutdown', async t => {
  const { state, vscode } = await activateFixture(t);
  let entered;
  const downloading = new Promise(resolve => { entered = resolve; });
  state.download = options => new Promise((_resolve, reject) => {
    options.signal.addEventListener('abort', () => reject(options.signal.reason), { once: true });
    entered();
  });
  const command = vscode.commands.executeCommand('glsld.downloadServer');
  await downloading;
  await state.extension.deactivate();
  await command;
  assert.equal(state.starts.length, 0);
  assert.equal(state.errors.length, 0);
});
