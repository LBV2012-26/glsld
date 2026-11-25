const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { createRequire } = require('node:module');
const { test } = require('node:test');
const { configFileChildren, configFileParent } = require('../out/configFileTree');

const plain = value => JSON.parse(JSON.stringify(value));

function fixture() {
  const root = path.resolve(__dirname, 'project');
  const uri = relative => {
    const fsPath = path.isAbsolute(relative) ? relative : path.join(root, relative);
    return { scheme: 'file', fsPath, toString: () => `file:///${fsPath.replace(/\\/g, '/')}` };
  };
  const paths = ['Shaders/Lighting/shared.glsl', 'Shaders/Post/shared.glsl', 'root.glsl'];
  const state = {
    commands: new Map(), views: new Map(), picks: [], dialogs: [], inputs: [], pickRequests: [], writes: [],
    data: {
      globalMacros: { KEEP: { value: '1', enabled: true } },
      fileMacros: Object.fromEntries(paths.map(key => [uri(key).toString(), { TEST: { value: '2', enabled: true } }])),
      shaderConfigs: Object.fromEntries(paths.map(key => [key, { shaderStage: 'frag' }])),
      compileGroups: [
        { name: 'Main', command: 'glslc -o "{output}" "{}"', include: ['**/*.glsl'], fileStages: {
          'Shaders/Lighting/shared.glsl': ['mesh', 'frag'], 'Shaders/Post/shared.glsl': ['comp'], 'root.glsl': ['vert'],
        } },
        { name: 'Other', command: 'glslc -o "{output}" "{}"', include: ['**/*.glsl'], fileStages: { 'Shaders/Lighting/shared.glsl': ['vert'] } },
      ],
    },
  };
  const disposable = () => ({ dispose() {} });
  class EventEmitter {
    callbacks = new Set();
    event = callback => { this.callbacks.add(callback); return disposable(); };
    fire = value => { for (const callback of this.callbacks) callback(value); };
  }
  class TreeItem { constructor(label, collapsibleState) { Object.assign(this, { label, collapsibleState }); } }
  class ThemeIcon { static File = 'file'; constructor(id) { this.id = id; } }
  const vscode = {
    EventEmitter, TreeItem, ThemeIcon,
    TreeItemCollapsibleState: { None: 0, Collapsed: 1, Expanded: 2 },
    TreeItemCheckboxState: { Unchecked: 0, Checked: 1 },
    Uri: { parse: value => uri(value.replace(/^file:\/\/\//, '')), file: uri },
    commands: { registerCommand: (name, callback) => { state.commands.set(name, callback); return disposable(); } },
    window: {
      activeTextEditor: { document: { languageId: 'glsl', uri: uri(paths[0]) } },
      registerTreeDataProvider: (name, provider) => { state.views.set(name, provider); return disposable(); },
      createTreeView: (name, options) => {
        state.views.set(name, options.treeDataProvider);
        return { dispose() {}, onDidChangeCheckboxState: disposable };
      },
      showInformationMessage() {}, showWarningMessage: async () => 'Remove',
      showQuickPick: async (items, options) => {
        state.pickRequests.push({ items, options });
        const next = state.picks.shift();
        return typeof next === 'function' ? next(items, options) : next;
      },
      showOpenDialog: async () => state.dialogs.shift(),
      showInputBox: async () => state.inputs.shift(),
    },
    workspace: {
      workspaceFolders: [{ uri: uri(root) }], textDocuments: [],
      getConfiguration: () => ({ get: (_key, fallback) => fallback }),
      asRelativePath: file => path.relative(root, file.fsPath).replace(/\\/g, '/'),
    },
  };
  const out = path.resolve(__dirname, '../out');
  const modules = new Map();
  function load(name) {
    if (modules.has(name)) return modules.get(name);
    const file = path.join(out, `${name}.js`);
    const localRequire = createRequire(file);
    const exports = {};
    modules.set(name, exports);
    const mocks = {
      vscode,
      fs: {
        existsSync: () => true,
        readFileSync: () => JSON.stringify(state.data),
        writeFileSync: (_file, value) => { state.data = JSON.parse(value); state.writes.push(state.data); },
      },
      './extension': { pushConfiguration() {}, notifyRemoveConfig() {} },
    };
    vm.runInNewContext(fs.readFileSync(file, 'utf8'), {
      exports,
      require: target => mocks[target] ?? (['./sidebar', './shaderConfig'].includes(target)
        ? load(target.slice(2)) : localRequire(target)),
      process: { env: {} }, console,
    }, { filename: file });
    return exports;
  }
  const sidebar = load('sidebar');
  sidebar.activateSidebar({ globalStorageUri: uri(root), subscriptions: [] }, { isRunning: () => false });
  const { ShaderConfigProvider } = load('shaderConfig');
  state.views.set('glsld.shaderConfig', new ShaderConfigProvider());
  return { state, vscode, sidebar, uri };
}

test('directory entries group Windows/POSIX separators and sort folders before files', () => {
  const files = [
    { key: 'first', displayPath: 'Shaders\\Lighting\\same.glsl' },
    { key: 'second', displayPath: 'Shaders/Post/same.glsl' },
    { key: 'root', displayPath: 'root.glsl' },
  ];
  const root = configFileChildren(files);
  assert.deepEqual(root.map(node => node.label), ['Shaders', 'root.glsl']);
  const shaders = configFileChildren(files, 'Shaders');
  assert.deepEqual(shaders.map(node => node.label), ['Lighting', 'Post']);
  assert.equal(configFileChildren(files, 'Shaders/Lighting')[0].key, 'first');
  assert.equal(configFileChildren(files, 'Shaders/Post')[0].key, 'second');
  assert.deepEqual(configFileParent('Shaders/Post/same.glsl'), { kind: 'configFolder', folderPath: 'Shaders/Post', label: 'Post' });
  assert.equal(configFileParent('root.glsl'), undefined);
});

test('folders and file stages are contained inside Compile Groups without changing other views', () => {
  const { state } = fixture();
  assert.equal(state.views.has('glsld.compileStages'), false);
  for (const view of ['glsld.compileGroups']) {
    const provider = state.views.get(view);
    const roots = view === 'glsld.compileGroups' ? provider.getChildren({ kind: 'compileGroup', index: 0 }) : provider.getChildren();
    const shaders = roots.find(node => node.kind === 'configFolder');
    assert.equal(provider.getTreeItem(shaders).label, 'Shaders');
    assert.equal(provider.getTreeItem(shaders).collapsibleState, 1);
    const lighting = provider.getChildren(shaders).find(node => node.label === 'Lighting');
    const file = provider.getChildren(lighting)[0];
    assert.equal(provider.getTreeItem(file).label, 'shared.glsl');
    assert.equal(provider.getParent(file).folderPath, 'Shaders/Lighting');
    const properties = provider.getChildren(file);
    assert.ok(properties.length > 0);
    assert.equal(provider.getParent(properties[0]).kind, file.kind);
    assert.equal(provider.getParent(lighting).folderPath, 'Shaders');
    if (view === 'glsld.compileGroups') { assert.deepEqual(plain(provider.getParent(shaders)), { kind: 'compileGroup', index: 0 }); }
    else { assert.equal(provider.getParent(shaders), undefined); }
  }
  for (const view of ['glsld.fileVariant', 'glsld.shaderConfig']) {
    const provider = state.views.get(view);
    const roots = provider.getChildren();
    assert.equal(roots.length, 3);
    assert.equal(roots.some(node => node.kind === 'configFolder'), false);
  }
  assert.equal(state.writes.length, 0, 'Rendering directory trees must not rewrite stored file keys');
});

test('adding/editing file stages persists a multi-selection and preserves all other configuration', async () => {
  const { state, vscode, sidebar, uri } = fixture();
  const originalMacros = plain(state.data.fileMacros);
  const originalShaderConfigs = plain(state.data.shaderConfigs);
  const otherGroup = plain(state.data.compileGroups[1]);
  vscode.window.activeTextEditor.document.uri = uri('Shaders/New/new.glsl');
  state.picks.push(items => items.filter(item => ['mesh', 'frag'].includes(item.label)));
  await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
  assert.deepEqual(plain(sidebar.getCompileGroups()[0].fileStages['Shaders/New/new.glsl']), ['frag', 'mesh']);
  assert.deepEqual(state.data.fileMacros, originalMacros);
  assert.deepEqual(state.data.shaderConfigs, originalShaderConfigs);
  assert.deepEqual(state.data.compileGroups[0].fileStages['Shaders/New/new.glsl'], ['frag', 'mesh']);
  assert.deepEqual(state.data.compileGroups[1], otherGroup);
  assert.equal(state.data.compileStages, undefined);
  assert.equal(state.pickRequests.at(-1).options.canPickMany, true);
  state.picks.push(items => {
    assert.deepEqual(items.filter(item => item.picked).map(item => item.label).sort(), ['frag', 'mesh']);
    return items.filter(item => item.label === 'comp');
  });
  await state.commands.get('glsld.editCompileStages')({ kind: 'compileGroupFile', groupIndex: 0, key: 'Shaders/New/new.glsl' });
  assert.deepEqual(state.data.compileGroups[0].fileStages['Shaders/New/new.glsl'], ['comp']);
});

test('cancelling stage selection leaves configuration unchanged and clearing removes only that file', async () => {
  const { state } = fixture();
  const file = { kind: 'compileGroupFile', groupIndex: 0, key: 'Shaders/Lighting/shared.glsl' };
  state.picks.push(undefined);
  await state.commands.get('glsld.editCompileStages')(file);
  assert.equal(state.writes.length, 0);
  state.picks.push([]);
  await state.commands.get('glsld.editCompileStages')(file);
  assert.equal(state.data.compileGroups[0].fileStages[file.key], undefined);
  assert.deepEqual(state.data.compileGroups[0].fileStages['Shaders/Post/shared.glsl'], ['comp']);
  assert.deepEqual(state.data.compileGroups[1].fileStages[file.key], ['vert']);
  assert.deepEqual(state.data.shaderConfigs[file.key], { shaderStage: 'frag' });
});

test('stage configuration can be added through the file browser and removed through its context action', async () => {
  const { state, vscode, uri } = fixture();
  vscode.window.activeTextEditor = undefined;
  state.dialogs.push([uri('Shaders/Browse/new.glsl')]);
  state.picks.push(items => items.filter(item => item.label === 'vert'));
  await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
  assert.deepEqual(state.data.compileGroups[0].fileStages['Shaders/Browse/new.glsl'], ['vert']);
  await state.commands.get('glsld.removeCompileStages')({ kind: 'compileGroupFile', groupIndex: 0, key: 'Shaders/Browse/new.glsl' });
  assert.equal(state.data.compileGroups[0].fileStages['Shaders/Browse/new.glsl'], undefined);
});

test('refresh reloads file stages edited externally in config.json', () => {
  const { state } = fixture();
  state.data.compileGroups[0].fileStages = { 'External/new.glsl': ['task', 'mesh'] };
  state.commands.get('glsld.refreshCompileGroups')();
  const provider = state.views.get('glsld.compileGroups');
  const folder = provider.getChildren({ kind: 'compileGroup', index: 0 }).find(node => node.kind === 'configFolder');
  assert.equal(folder.label, 'External');
  const file = provider.getChildren(folder)[0];
  assert.deepEqual(plain(provider.getChildren(file).map(node => node.stage)), ['task', 'mesh']);
});

test('editing group settings preserves its file stages and folders have group-specific parents and IDs', async () => {
  const { state } = fixture();
  const original = plain(state.data.compileGroups[0].fileStages);
  const provider = state.views.get('glsld.compileGroups');
  const first = provider.getChildren({ kind: 'compileGroup', index: 0 }).find(node => node.kind === 'configFolder');
  const second = provider.getChildren({ kind: 'compileGroup', index: 1 }).find(node => node.kind === 'configFolder');
  assert.notEqual(provider.getTreeItem(first).id, provider.getTreeItem(second).id);
  assert.equal(provider.getParent(second).index, 1);
  state.inputs.push('Renamed', '**/*.glsl', 'glslc -o "{output}" "{}"', '');
  await state.commands.get('glsld.editCompileGroup')({ kind: 'compileGroup', index: 0 });
  assert.equal(state.data.compileGroups[0].name, 'Renamed');
  assert.deepEqual(state.data.compileGroups[0].fileStages, original);
});
