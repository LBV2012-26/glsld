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
    installed: new Map(options.installations ?? (options.installed ? [['v0.9.5', options.installed]] : [])),
    latestVersion: options.latestVersion ?? 'v0.9.5', currentVersion: options.currentVersion ?? 'v0.9.5', checks: 0,
    checkError: options.checkError, onCheck: options.onCheck, download: options.download,
    commands: new Map(), executedCommands: [], clients: [], sidebarClients: [], starts: [],
    errors: [], information: [], informationActions: [], warnings: [], warningActions: [], output: [], progress: [],
    downloads: 0, restarts: 0, stops: 0, compilations: 0, cancelProgress: false,
    cleanupCalls: [], cleanupError: options.cleanupError,
    formatterChecks: [], formatterAvailable: options.formatterAvailable ?? true,
    formatterExecutable: options.formatterExecutable ?? path.join(root, 'tools', 'clang-format.exe'),
    onFormatterCheck: options.onFormatterCheck,
    onConfiguration: event(), onOpenDocument: event(),
  };
  const disposableEvent = () => new Disposable();
  const vscode = {
    Disposable,
    CancellationError: class CancellationError extends Error {},
    ProgressLocation: { Notification: 15, Window: 10 },
    commands: {
      registerCommand(name, callback) {
        assert.equal(state.commands.has(name), false, `Command registered twice: ${name}`);
        state.commands.set(name, callback);
        return new Disposable(() => state.commands.delete(name));
      },
      executeCommand(name, ...args) {
        state.executedCommands.push({ name, args });
        return Promise.resolve(state.commands.get(name)?.(...args));
      },
    },
    window: {
      activeTextEditor: undefined,
      visibleTextEditors: [],
      createOutputChannel: () => ({ appendLine: line => state.output.push(line), show() {}, dispose() {} }),
      createTextEditorDecorationType: () => new Disposable(),
      registerTreeDataProvider: () => new Disposable(),
      onDidChangeActiveTextEditor: disposableEvent,
      onDidChangeVisibleTextEditors: disposableEvent,
      showInformationMessage: async (message, ...actions) => {
        state.information.push(message);
        state.informationActions.push(actions);
        return options.onInformation?.(message, actions, state);
      },
      showErrorMessage: async message => { state.errors.push(message); },
      showWarningMessage: async (message, ...actions) => {
        state.warnings.push(message);
        state.warningActions.push(actions);
        return options.onWarning?.(message, actions, state);
      },
      withProgress: async (_options, callback) => callback({ report: progress => state.progress.push(progress) }, {
        isCancellationRequested: state.cancelProgress,
        onCancellationRequested: disposableEvent,
      }),
    },
    workspace: {
      textDocuments: options.withGlsl ? [{ languageId: 'glsl', uri: { toString: () => 'file:///shader.glsl' } }] : [],
      workspaceFolders: options.workspaceFolders ?? [],
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
    async checkForUpdates(checkOptions) {
      checkOptions.signal.throwIfAborted();
      state.checks++;
      await state.onCheck?.(state);
      checkOptions.signal.throwIfAborted();
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
    async cleanupOldVersions(executablePath, options) {
      options.signal.throwIfAborted();
      assert.equal(state.clients[0].isRunning(), true, 'Cleanup requires a successfully started server');
      assert.equal(executablePath, state.installed.get(state.currentVersion));
      state.cleanupCalls.push({ executablePath, version: state.currentVersion, restarts: state.restarts });
      if (state.cleanupError) throw state.cleanupError;
      const removed = [];
      for (const version of state.installed.keys()) {
        if (version !== state.currentVersion) { removed.push(version); state.installed.delete(version); }
      }
      return removed;
    }
  }
  const mocks = {
    vscode,
    'vscode-languageclient/node': { LanguageClient, TransportKind: { stdio: 0 } },
    './serverManager': { ServerManager },
    './clangFormat': {
      getClangFormatCommand: extensionRequire('./clangFormat').getClangFormatCommand,
      checkClangFormat: async (configuredPath, checkOptions) => {
        checkOptions.signal.throwIfAborted();
        state.formatterChecks.push({ configuredPath, baseDirectory: checkOptions.baseDirectory });
        const result = {
          command: extensionRequire('./clangFormat').getClangFormatCommand(configuredPath, checkOptions.baseDirectory),
          configured: Boolean(configuredPath), available: state.formatterAvailable,
          executablePath: state.formatterAvailable ? state.formatterExecutable : undefined,
          error: state.formatterAvailable ? undefined : 'Formatter is not accessible',
        };
        await state.onFormatterCheck?.(configuredPath, state);
        checkOptions.signal.throwIfAborted();
        return result;
      },
    },
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
    console: { log() {} }, process: { env: {}, cwd: () => path.join(__dirname, 'virtual-workspace') },
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

test('activation without a binary offers download even without an open GLSL document', async t => {
  const { state, vscode } = await activateFixture(t);
  assert.equal(state.starts.length, 0);
  assert.equal(state.downloads, 0);
  assert.equal(state.checks, 1);
  assert.equal(state.errors.length, 0);
  assert.equal(state.information.length, 1);
  assert.match(state.information[0], /not found.*latest release from GitHub/);
  assert.deepEqual(state.informationActions[0], ['Download', 'Open Settings']);
  assert.ok(state.commands.has('glsld.downloadServer'));
  assert.ok(state.commands.has('glsld.checkForUpdates'));
  assert.ok(state.commands.has('glsld.restartServer'));
  assert.ok(state.commands.has('glsld.addShaderConfig'));
  assert.equal(state.sidebarClients.length, 1);
  await vscode.commands.executeCommand('glsld.compileWorkspace');
  assert.equal(state.compilations, 1);
  await state.extension.pushConfiguration();
  state.extension.notifyRemoveConfig('shader.glsl');
});

test('the startup Download action installs the latest server and starts it without reloading', async t => {
  const { state, context } = await activateFixture(t, {
    latestVersion: 'v1.2.3',
    onInformation: () => 'Download',
  });
  await new Promise(setImmediate);
  assert.equal(state.downloads, 1);
  assert.equal(state.clients.length, 1);
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.starts.length, 1);
  assert.ok(state.starts[0].command.startsWith(path.join(context.globalStorageUri.fsPath, 'servers', 'v1.2.3')));
  assert.equal(state.information.length, 1);
  assert.equal(state.errors.length, 0);
});

test('dismissing the startup prompt does not download or repeat on document opens and restarts', async t => {
  const { state, vscode } = await activateFixture(t);
  await state.onOpenDocument.fire({ languageId: 'glsl', uri: { toString: () => 'file:///shader.glsl' } });
  await state.onOpenDocument.fire({ languageId: 'glsl' });
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.information.length, 1);
  assert.equal(state.downloads, 0);
  assert.equal(state.starts.length, 0);
});

test('the startup Open Settings action opens server settings without downloading', async t => {
  const { state } = await activateFixture(t, { onInformation: () => 'Open Settings' });
  assert.deepEqual(state.executedCommands, [{ name: 'workbench.action.openSettings', args: ['glsld.server'] }]);
  assert.equal(state.downloads, 0);
  assert.equal(state.starts.length, 0);
});

test('the missing-server prompt is shown before a slow or failed GitHub check completes', async t => {
  const { state } = await activateFixture(t, {
    onCheck: async state => {
      assert.equal(state.information.length, 1);
      await new Promise(setImmediate);
      assert.equal(state.downloads, 0);
    },
    checkError: new Error('Network unavailable'),
  });
  assert.equal(state.information.length, 1);
  assert.equal(state.errors.length, 0);
  assert.ok(state.output.some(message => message.includes('Network unavailable')));
});

test('cached and custom servers start without an installation prompt', async t => {
  for (const options of [{ installed: 'cached-server.exe' }, { config: { 'server.path': 'local.exe' } }]) {
    const { state } = await activateFixture(t, options);
    assert.equal(state.clients[0].isRunning(), true);
    assert.equal(state.information.length, 0);
    assert.equal(state.downloads, 0);
  }
});

test('accepting the startup prompt after deactivation cannot start a download', async t => {
  let choose;
  const { state } = await activateFixture(t, {
    onInformation: () => new Promise(resolve => { choose = resolve; }),
  });
  await state.extension.deactivate();
  choose('Download');
  await new Promise(setImmediate);
  assert.equal(state.downloads, 0);
  assert.equal(state.starts.length, 0);
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
  assert.equal(state.cleanupCalls.length, 0);
  assert.ok(state.information.some(message => message.includes('Clear glsld.server.path')));
  state.config['server.path'] = 'missing';
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.match(state.errors.at(-1), /Invalid custom executable/);
  assert.equal(state.clients[0].isRunning(), true);
  state.config['server.path'] = '';
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.starts.at(-1).command.startsWith(context.globalStorageUri.fsPath + path.sep), true);
});

test('restart checks GitHub and automatically updates an already running cached server once', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'old-server.exe' });
  assert.equal(state.checks, 1);
  state.latestVersion = 'v0.9.6';
  state.config['server.version'] = '0.1.0'; // A setting left by an older extension cannot pin downloads.
  await vscode.commands.executeCommand('glsld.restartServer');
  assert.equal(state.checks, 2);
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.downloads, 1);
  assert.equal(state.restarts, 1);
  assert.equal(state.information.length, 0);
  assert.ok(state.starts.at(-1).command.includes(path.join('servers', 'v0.9.6')));
  assert.equal(state.clients.length, 1);
});

test('startup starts the installed server before checking and automatically installing the latest release', async t => {
  const { state } = await activateFixture(t, {
    installed: 'cached-server.exe', latestVersion: 'v1.0.0',
    onCheck: state => {
      assert.equal(state.clients[0].isRunning(), true);
      assert.equal(state.starts[0].command, 'cached-server.exe');
    },
  });
  assert.equal(state.checks, 1);
  assert.equal(state.downloads, 1);
  assert.equal(state.restarts, 1);
  assert.equal(state.clients.length, 1);
  assert.ok(state.starts.at(-1).command.includes(path.join('servers', 'v1.0.0')));
  assert.deepEqual([...state.installed.keys()], ['v1.0.0']);
  assert.equal(state.cleanupCalls.length, 1);
  assert.equal(state.cleanupCalls[0].version, 'v1.0.0');
  assert.equal(state.cleanupCalls[0].restarts, 1);
  assert.equal(state.information.length, 0);
  await state.onOpenDocument.fire({ languageId: 'glsl', uri: { toString: () => 'file:///shader.glsl' } });
  assert.equal(state.checks, 1);
  assert.equal(state.downloads, 1);
});

test('the update command reports an up-to-date server without downloading or restarting', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.checks, 2);
  assert.equal(state.downloads, 0);
  assert.equal(state.restarts, 0);
  assert.equal(state.information.at(-1), 'glsld v0.9.5 is up to date.');
});

test('the update command automatically downloads a new release and reports completion', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  state.latestVersion = 'v1.0.1';
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.downloads, 1);
  assert.equal(state.restarts, 1);
  assert.ok(state.starts.at(-1).command.includes(path.join('servers', 'v1.0.1')));
  assert.equal(state.information.at(-1), 'glsld was updated to v1.0.1.');
});

test('an already cached Latest release is selected and started without downloading again', async t => {
  const { state } = await activateFixture(t, {
    installations: [['v0.9.5', 'old-server.exe'], ['v1.0.0', 'latest-server.exe']],
    latestVersion: 'v1.0.0',
  });
  assert.equal(state.starts[0].command, 'old-server.exe');
  assert.equal(state.starts.at(-1).command, 'latest-server.exe');
  assert.equal(state.downloads, 0);
  assert.equal(state.restarts, 1);
  assert.deepEqual([...state.installed.keys()], ['v1.0.0']);
});

test('startup removes accumulated versions even when no new download is needed', async t => {
  const { state } = await activateFixture(t, {
    installations: [['v0.9.3', 'older-server.exe'], ['v0.9.4', 'old-server.exe'], ['v0.9.5', 'current-server.exe']],
  });
  assert.equal(state.downloads, 0);
  assert.equal(state.restarts, 0);
  assert.deepEqual([...state.installed.keys()], ['v0.9.5']);
  assert.equal(state.cleanupCalls.length, 1);
  assert.ok(state.output.some(message => message.includes('Removed old server versions: v0.9.3, v0.9.4')));
});

test('a failed launch of an updated server preserves previous installations', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  const previousCleanups = state.cleanupCalls.length;
  state.latestVersion = 'v1.0.0';
  state.failNextStart = true;
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.clients[0].isRunning(), false);
  assert.equal(state.cleanupCalls.length, previousCleanups);
  assert.equal(state.installed.get('v0.9.5'), 'cached-server.exe');
  assert.equal(state.installed.has('v1.0.0'), true);
  assert.match(state.errors.at(-1), /Failed to launch/);
});

test('cleanup failures leave the updated server running and are retried by a later check', async t => {
  const { state, vscode } = await activateFixture(t, {
    installed: 'cached-server.exe', latestVersion: 'v1.0.0', cleanupError: new Error('Files are busy'),
  });
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.installed.has('v0.9.5'), true);
  assert.equal(state.errors.length, 0);
  assert.ok(state.output.some(message => message.includes('Could not clean old server versions: Error: Files are busy')));
  state.cleanupError = undefined;
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.deepEqual([...state.installed.keys()], ['v1.0.0']);
  assert.equal(state.restarts, 1);
});

test('a failed automatic download preserves the running server and can be retried manually', async t => {
  const { state, vscode } = await activateFixture(t, {
    installed: 'cached-server.exe', latestVersion: 'v1.0.0',
    download: async () => { throw new Error('SHA-256 verification failed'); },
  });
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.starts.at(-1).command, 'cached-server.exe');
  assert.equal(state.restarts, 0);
  assert.equal(state.currentVersion, 'v0.9.5');
  assert.equal(state.cleanupCalls.length, 0);
  assert.match(state.errors[0], /SHA-256/);
  state.download = undefined;
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.downloads, 2);
  assert.equal(state.restarts, 1);
  assert.ok(state.starts.at(-1).command.includes(path.join('servers', 'v1.0.0')));
});

test('cancelling an automatic update preserves the running version without reporting success', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  const previousCleanups = state.cleanupCalls.length;
  state.latestVersion = 'v1.0.0';
  state.cancelProgress = true;
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.starts.at(-1).command, 'cached-server.exe');
  assert.equal(state.currentVersion, 'v0.9.5');
  assert.equal(state.restarts, 0);
  assert.equal(state.errors.length, 0);
  assert.equal(state.information.length, 0);
  assert.equal(state.cleanupCalls.length, previousCleanups);
});

test('custom servers are never automatically downloaded or replaced and the command explains why', async t => {
  const { state, vscode, context } = await activateFixture(t, {
    config: { 'server.path': 'local.exe' }, installed: 'cached-server.exe', latestVersion: 'v1.0.0',
  });
  assert.equal(state.checks, 1);
  assert.equal(state.downloads, 0);
  assert.equal(state.information.length, 0);
  assert.equal(state.cleanupCalls.length, 0);
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.checks, 2);
  assert.equal(state.downloads, 0);
  assert.equal(state.starts.at(-1).command, path.join(context.extensionPath, 'local.exe'));
  assert.match(state.information.at(-1), /latest glsld release is v1.0.0.*glsld.server.path/);
});

test('the update command reoffers first installation after the initial prompt was dismissed', async t => {
  const { state, vscode } = await activateFixture(t);
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.information.length, 2);
  assert.deepEqual(state.informationActions.at(-1), ['Download', 'Open Settings']);
  assert.equal(state.downloads, 0);
});

test('concurrent manual checks share one check and download and show a single result', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  state.latestVersion = 'v1.0.0';
  let releaseCheck;
  let enteredCheck;
  const checking = new Promise(resolve => { enteredCheck = resolve; });
  state.onCheck = () => new Promise(resolve => { releaseCheck = resolve; enteredCheck(); });
  const first = vscode.commands.executeCommand('glsld.checkForUpdates');
  const second = vscode.commands.executeCommand('glsld.checkForUpdates');
  await checking;
  releaseCheck();
  await Promise.all([first, second]);
  assert.equal(state.checks, 2);
  assert.equal(state.downloads, 1);
  assert.equal(state.restarts, 1);
  assert.deepEqual(state.information, ['glsld was updated to v1.0.0.']);
});

test('a manual check joins a running automatic check and still receives its result', async t => {
  let manualCheck;
  const { state } = await activateFixture(t, {
    installed: 'cached-server.exe', latestVersion: 'v1.0.0',
    onCheck: state => { manualCheck = state.commands.get('glsld.checkForUpdates')(); },
  });
  await manualCheck;
  assert.equal(state.checks, 1);
  assert.equal(state.downloads, 1);
  assert.equal(state.restarts, 1);
  assert.deepEqual(state.information, ['glsld was updated to v1.0.0.']);
});

test('deactivating during the update check prevents a later download or restart', async t => {
  const { state } = await activateFixture(t, {
    installed: 'cached-server.exe', latestVersion: 'v1.0.0',
    onCheck: state => state.extension.deactivate(),
  });
  assert.equal(state.downloads, 0);
  assert.equal(state.restarts, 0);
  assert.equal(state.clients[0].isRunning(), false);
  assert.equal(state.errors.length, 0);
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

test('a failed manual check reports the error and leaves the running version usable', async t => {
  const { state, vscode } = await activateFixture(t, { installed: 'cached-server.exe' });
  state.checkError = new Error('GitHub rate limit exceeded');
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.downloads, 0);
  assert.match(state.errors.at(-1), /rate limit exceeded/);
  state.checkError = undefined;
  await vscode.commands.executeCommand('glsld.checkForUpdates');
  assert.equal(state.information.at(-1), 'glsld v0.9.5 is up to date.');
});

test('an unavailable formatter warns once without preventing the language server from starting', async t => {
  const { state } = await activateFixture(t, { installed: 'local-server.exe', formatterAvailable: false });
  assert.equal(state.clients[0].isRunning(), true);
  assert.equal(state.errors.length, 0);
  assert.equal(state.warnings.length, 1);
  assert.match(state.warnings[0], /clang-format is unavailable in PATH.*glsld.clangFormat.path/);
  assert.deepEqual(state.warningActions[0], ['Open Settings', 'Check Again']);
  await state.onOpenDocument.fire({ languageId: 'glsl', uri: { toString: () => 'file:///shader.glsl' } });
  assert.equal(state.formatterChecks.length, 1);
  assert.equal(state.warnings.length, 1);
});

test('an invalid formatter setting warns with its setting name and can open the matching settings', async t => {
  const { state } = await activateFixture(t, {
    formatterAvailable: false, config: { 'clangFormat.path': 'missing-formatter.exe' },
    onWarning: () => 'Open Settings',
  });
  assert.match(state.warnings[0], /glsld.clangFormat.path.*missing-formatter.exe/);
  assert.ok(state.executedCommands.some(command => command.name === 'workbench.action.openSettings' && command.args[0] === 'glsld.clangFormat.path'));
});

test('the resolved formatter executable is sent to the language server without a warning', async t => {
  const formatter = path.join(__dirname, 'LLVM tools', 'clang-format.exe');
  const { state } = await activateFixture(t, { installed: 'local-server.exe', formatterExecutable: formatter });
  assert.equal(state.warnings.length, 0);
  assert.equal(state.formatterChecks.length, 1);
  assert.equal(state.clients[0].notifications.at(-1).payload.settings.glsld.clangFormatPath, formatter);
});

test('fixing the formatter setting rechecks and updates the server configuration without restarting', async t => {
  const { state } = await activateFixture(t, {
    installed: 'local-server.exe', formatterAvailable: false, config: { 'clangFormat.path': 'missing.exe' },
  });
  state.config['clangFormat.path'] = 'working-formatter.exe';
  state.formatterAvailable = true;
  state.formatterExecutable = path.join(__dirname, 'tools', 'working-formatter.exe');
  await state.onConfiguration.fire({ affectsConfiguration: name => ['glsld', 'glsld.clangFormat.path'].includes(name) });
  assert.equal(state.formatterChecks.length, 2);
  assert.equal(state.warnings.length, 1);
  assert.equal(state.restarts, 0);
  assert.equal(state.clients[0].notifications.at(-1).payload.settings.glsld.clangFormatPath, state.formatterExecutable);
});

test('Check Again retries after installing a formatter and sends its resolved path to the server', async t => {
  const { state } = await activateFixture(t, {
    installed: 'local-server.exe', formatterAvailable: false,
    onWarning: (_message, _actions, state) => { state.formatterAvailable = true; return 'Check Again'; },
  });
  await new Promise(setImmediate);
  assert.equal(state.formatterChecks.length, 2);
  assert.equal(state.warnings.length, 1);
  assert.equal(state.clients[0].notifications.at(-1).payload.settings.glsld.clangFormatPath, state.formatterExecutable);
  assert.equal(state.restarts, 0);
});

test('a slow formatter check cannot overwrite the result of a more recent settings change', async t => {
  const { state } = await activateFixture(t, { installed: 'local-server.exe' });
  let entered;
  let release;
  const checking = new Promise(resolve => { entered = resolve; });
  state.onFormatterCheck = configuredPath => configuredPath === 'old.exe'
    ? new Promise(resolve => { release = resolve; entered(); }) : undefined;
  state.formatterAvailable = false;
  state.config['clangFormat.path'] = 'old.exe';
  const oldChange = state.onConfiguration.fire({ affectsConfiguration: name => ['glsld', 'glsld.clangFormat.path'].includes(name) });
  await checking;
  state.formatterAvailable = true;
  state.formatterExecutable = path.join(__dirname, 'new-formatter.exe');
  state.config['clangFormat.path'] = 'new.exe';
  await state.onConfiguration.fire({ affectsConfiguration: name => ['glsld', 'glsld.clangFormat.path'].includes(name) });
  release();
  await oldChange;
  assert.equal(state.warnings.length, 0);
  assert.equal(state.clients[0].notifications.at(-1).payload.settings.glsld.clangFormatPath, state.formatterExecutable);
});

test('formatter checks use the workspace directory and do not delay language server startup', async t => {
  let serverStarted;
  const folder = { uri: { fsPath: path.join(__dirname, 'project') } };
  const { state } = await activateFixture(t, {
    installed: 'local-server.exe', workspaceFolders: [folder],
    onFormatterCheck: async (_configuredPath, state) => {
      await new Promise(setImmediate);
      serverStarted = state.clients[0].isRunning();
    },
  });
  assert.equal(serverStarted, true);
  assert.equal(state.formatterChecks[0].baseDirectory, folder.uri.fsPath);
});

test('deactivation prevents a pending formatter check from showing a notification', async t => {
  const { state } = await activateFixture(t, {
    formatterAvailable: false, onFormatterCheck: (_configuredPath, state) => state.extension.deactivate(),
  });
  assert.equal(state.warnings.length, 0);
  assert.equal(state.errors.length, 0);
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
