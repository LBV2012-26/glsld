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
    commands: new Map(), views: new Map(), checkboxHandlers: new Map(), picks: [], dialogs: [], inputs: [], pickRequests: [], inputRequests: [], writes: [],
    removedConfigs: [], configurationPushes: 0,
    data: {
      globalMacros: { KEEP: { value: '1', enabled: true } },
      fileMacros: Object.fromEntries(paths.map(key => [uri(key).toString(), { TEST: { value: '2', enabled: true } }])),
      shaderConfigs: Object.fromEntries(paths.map(key => [key, { shaderStage: 'frag' }])),
      compileGroups: [
        { name: 'Main', outputPath: 'build/shaders', options: '-O', targetEnv: 'vulkan1.3', targetSpv: '', fileStages: {
          'Shaders/Lighting/shared.glsl': ['mesh', 'frag'], 'Shaders/Post/shared.glsl': ['comp'], 'root.glsl': ['vert'],
        } },
        { name: 'Other', outputPath: 'other', options: '-g', targetEnv: 'vulkan1.2', fileStages: { 'Shaders/Lighting/shared.glsl': ['vert'] } },
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
        return { dispose() {}, onDidChangeCheckboxState: callback => {
          state.checkboxHandlers.set(name, callback);
          return disposable();
        } };
      },
      showInformationMessage() {}, showWarningMessage: async (_message, _options, action) => action,
      showQuickPick: async (items, options) => {
        state.pickRequests.push({ items, options });
        const next = state.picks.shift();
        return typeof next === 'function' ? next(items, options) : next;
      },
      showOpenDialog: async () => state.dialogs.shift(),
      showInputBox: async options => { state.inputRequests.push(options); return state.inputs.shift(); },
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
      './extension': {
        pushConfiguration() { state.configurationPushes++; },
        notifyRemoveConfig(key) { state.removedConfigs.push(key); },
      },
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

test('all file configuration views group files into folders and retain their property children and parents', () => {
  const { state } = fixture();
  assert.equal(state.views.has('glsld.compileStages'), false);
  for (const view of ['glsld.compileGroups', 'glsld.fileVariant', 'glsld.shaderConfig']) {
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
  assert.equal(state.writes.length, 0, 'Rendering directory trees must not rewrite stored file keys');
});

test('nested macro files retain URI keys, context actions and checkbox toggles even with matching basenames', async () => {
  const { state, uri } = fixture();
  const originalConfigs = plain(state.data.shaderConfigs);
  const originalGroups = plain(state.data.compileGroups);
  const provider = state.views.get('glsld.fileVariant');
  const shaders = provider.getChildren().find(node => node.kind === 'configFolder');
  const folders = provider.getChildren(shaders);
  const lighting = provider.getChildren(folders.find(node => node.label === 'Lighting'))[0];
  const post = provider.getChildren(folders.find(node => node.label === 'Post'))[0];
  const lightingKey = uri('Shaders/Lighting/shared.glsl').toString();
  const postKey = uri('Shaders/Post/shared.glsl').toString();
  assert.equal(provider.getTreeItem(lighting).label, provider.getTreeItem(post).label);
  assert.notEqual(provider.getTreeItem(lighting).id, provider.getTreeItem(post).id);
  assert.equal(provider.getTreeItem(lighting).contextValue, 'fileMacroFile');
  assert.equal(provider.getTreeItem(lighting).tooltip, 'Shaders/Lighting/shared.glsl');

  state.inputs.push('EXTRA', '9');
  await state.commands.get('glsld.addFileMacro')(post);
  assert.deepEqual(state.data.fileMacros[postKey].EXTRA, { value: '9', enabled: true });
  assert.equal(state.data.fileMacros[lightingKey].EXTRA, undefined);
  const testMacro = provider.getChildren(lighting)[0];
  assert.equal(provider.getTreeItem(testMacro).contextValue, 'macro');
  state.inputs.push('RENAMED', '4');
  await state.commands.get('glsld.editFileMacro')(testMacro);
  assert.equal(state.data.fileMacros[lightingKey].TEST, undefined);
  assert.deepEqual(state.data.fileMacros[lightingKey].RENAMED, { value: '4', enabled: true });
  const renamed = provider.getChildren(lighting)[0];
  state.checkboxHandlers.get('glsld.fileVariant')({ items: [[renamed, 0]] });
  assert.equal(state.data.fileMacros[lightingKey].RENAMED.enabled, false);
  assert.equal(provider.getTreeItem(provider.getChildren(lighting)[0]).checkboxState, 0);
  await state.commands.get('glsld.deleteFileMacro')(renamed);
  assert.equal(state.data.fileMacros[lightingKey], undefined);
  await state.commands.get('glsld.removeFileFromList')(post);
  assert.equal(state.data.fileMacros[postKey], undefined);
  assert.equal(Object.keys(state.data.fileMacros).length, 1);
  assert.deepEqual(state.data.shaderConfigs, originalConfigs);
  assert.deepEqual(state.data.compileGroups, originalGroups);
});

test('nested shader configuration properties edit and remove the original keys, independently of matching basenames', async () => {
  const { state } = fixture();
  const originalMacros = plain(state.data.fileMacros);
  const originalGroups = plain(state.data.compileGroups);
  const provider = state.views.get('glsld.shaderConfig');
  const shaders = provider.getChildren().find(node => node.kind === 'configFolder');
  const folders = provider.getChildren(shaders);
  const lighting = provider.getChildren(folders.find(node => node.label === 'Lighting'))[0];
  const post = provider.getChildren(folders.find(node => node.label === 'Post'))[0];
  assert.equal(provider.getTreeItem(lighting).label, provider.getTreeItem(post).label);
  assert.notEqual(provider.getTreeItem(lighting).id, provider.getTreeItem(post).id);
  assert.equal(provider.getTreeItem(lighting).contextValue, 'fileConfig');
  assert.equal(provider.getTreeItem(lighting).tooltip, lighting.key);
  const stage = provider.getChildren(lighting)[0];
  assert.equal(provider.getTreeItem(stage).contextValue, 'configProp');
  state.picks.push('mesh');
  await provider.editProperty(stage);
  assert.deepEqual(state.data.shaderConfigs[lighting.key], { shaderStage: 'mesh' });
  assert.deepEqual(state.data.shaderConfigs[post.key], { shaderStage: 'frag' });
  await provider.removeConfig(post);
  assert.equal(state.data.shaderConfigs[post.key], undefined);
  assert.deepEqual(state.removedConfigs, [post.key]);
  assert.equal(state.configurationPushes, 2);
  assert.deepEqual(state.data.fileMacros, originalMacros);
  assert.deepEqual(state.data.compileGroups, originalGroups);
});

test('stage suffixes skip the stage picker for both the current editor and browsed files', async () => {
  for (const mode of ['editor', 'browse']) {
    const { state, vscode, uri } = fixture();
    const otherGroup = plain(state.data.compileGroups[1]);
    for (const stage of ['vert', 'frag', 'comp', 'geom', 'tesc', 'tese', 'mesh', 'task', 'rgen', 'rahit', 'rchit', 'rmiss', 'rint', 'rcall']) {
      const key = 'Shaders/New/shader.' + stage.toUpperCase();
      if (mode === 'editor') vscode.window.activeTextEditor.document.uri = uri(key);
      else {
        vscode.window.activeTextEditor = undefined;
        state.dialogs.push([uri(key)]);
      }
      await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
      assert.deepEqual(state.data.compileGroups[0].fileStages[key], [stage]);
    }
    assert.equal(state.pickRequests.length, 0);
    assert.deepEqual(state.data.compileGroups[1], otherGroup);
  }
});

test('inferred stages remain editable and re-adding a file preserves explicit stage selections', async () => {
  const { state, vscode, uri } = fixture();
  const key = 'Shaders/New/shader.frag';
  vscode.window.activeTextEditor.document.uri = uri(key);
  await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
  state.picks.push(items => {
    assert.deepEqual(items.filter(item => item.picked).map(item => item.label), ['frag']);
    return items.filter(item => ['frag', 'mesh'].includes(item.label));
  });
  await state.commands.get('glsld.editCompileStages')({ kind: 'compileGroupFile', groupIndex: 0, key });
  const writes = state.writes.length;
  await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
  assert.deepEqual(state.data.compileGroups[0].fileStages[key], ['frag', 'mesh']);
  assert.equal(state.pickRequests.length, 1);
  assert.equal(state.writes.length, writes);
});

test('only the final extension is used for inference and glsl files still allow multiple stages', async () => {
  const { state, vscode, uri } = fixture();
  const key = 'Shaders/New/shader.frag.glsl';
  vscode.window.activeTextEditor.document.uri = uri(key);
  state.picks.push(items => items.filter(item => ['mesh', 'frag'].includes(item.label)));
  await state.commands.get('glsld.addCompileStages')({ kind: 'compileGroup', index: 0 });
  assert.deepEqual(state.data.compileGroups[0].fileStages[key], ['frag', 'mesh']);
  assert.equal(state.pickRequests.length, 1);
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
  state.inputs.push('Renamed', 'new output', '-O -g');
  state.picks.push(items => items.find(item => item.value === 'vulkan1.4'), items => items.find(item => item.value === 'spv1.6'));
  await state.commands.get('glsld.editCompileGroup')({ kind: 'compileGroup', index: 0 });
  assert.equal(state.data.compileGroups[0].name, 'Renamed');
  assert.equal(state.data.compileGroups[0].outputPath, 'new output');
  assert.equal(state.data.compileGroups[0].options, '-O -g');
  assert.equal(state.data.compileGroups[0].targetEnv, 'vulkan1.4');
  assert.equal(state.data.compileGroups[0].targetSpv, 'spv1.6');
  assert.deepEqual(state.data.compileGroups[0].fileStages, original);
});

test('adding a group saves only output, compiler and target settings and starts with an empty file list', async () => {
  const { state } = fixture();
  state.inputs.push('New Group', 'build/new', '-O');
  state.picks.push(items => items.find(item => item.value === 'vulkan1.3'), items => items.find(item => item.value === ''));
  await state.commands.get('glsld.addCompileGroup')();
  assert.deepEqual(state.data.compileGroups[2], {
    name: 'New Group', outputPath: 'build/new', options: '-O', targetEnv: 'vulkan1.3', targetSpv: '', fileStages: {},
  });
  assert.equal(state.inputRequests[1].value, 'build/shaders');
  assert.ok(state.inputRequests[1].validateInput(' '));
  assert.equal(state.inputRequests[2].validateInput('-O -g'), undefined);
  assert.ok(state.inputRequests[2].validateInput('-o ignored.spv'));
  const provider = state.views.get('glsld.compileGroups');
  const properties = provider.getChildren({ kind: 'compileGroup', index: 2 });
  assert.deepEqual(plain(properties.map(item => item.label)), ['Output Directory', 'Compiler Options', 'Target Environment', 'Target SPIR-V']);
});

test('cancelling any step in group setup does not save a partial group', async () => {
  for (let step = 0; step < 5; step++) {
    const { state } = fixture();
    state.inputs.push(...['New Group', 'build/new', '-O'].slice(0, step));
    if (step >= 3) state.picks.push(...Array.from({ length: step - 3 }, () => items => items[0]));
    await state.commands.get('glsld.addCompileGroup')();
    assert.equal(state.writes.length, 0, 'No save when cancelling step ' + step);
    assert.equal(state.data.compileGroups.length, 2);
  }
});

test('clicking a group property edits just that setting and retains its files and other groups', async () => {
  const { state } = fixture();
  const provider = state.views.get('glsld.compileGroups');
  const properties = provider.getChildren({ kind: 'compileGroup', index: 0 });
  const output = properties.find(item => item.setting === 'outputPath');
  const original = plain(state.data);
  state.inputs.push('generated/shaders');
  const command = provider.getTreeItem(output).command;
  await state.commands.get(command.command)(...command.arguments);
  original.compileGroups[0].outputPath = 'generated/shaders';
  assert.deepEqual(state.data, original);
  assert.equal(state.inputRequests.length, 1);
  assert.equal(state.pickRequests.length, 0);
  const spv = properties.find(item => item.setting === 'targetSpv');
  state.picks.push(items => items.find(item => item.value === 'spv1.5'));
  await state.commands.get('glsld.editCompileGroup')(spv);
  assert.equal(state.data.compileGroups[0].targetSpv, 'spv1.5');
  state.picks.push(items => items.find(item => item.value === ''));
  await state.commands.get('glsld.editCompileGroup')(spv);
  assert.equal(state.data.compileGroups[0].targetSpv, '');
});

test('editing a legacy group retains its manually added files and replaces the old command model', async () => {
  const { state } = fixture();
  const files = plain(state.data.compileGroups[0].fileStages);
  state.data.compileGroups[0] = { name: 'Legacy', include: ['**/*.glsl'], command: 'glslc "{}"', variants: [], fileStages: files };
  state.commands.get('glsld.refreshCompileGroups')();
  state.inputs.push('Legacy', 'build/legacy', '-g');
  state.picks.push(items => items.find(item => item.value === 'vulkan1.3'), items => items.find(item => item.value === ''));
  await state.commands.get('glsld.editCompileGroup')({ kind: 'compileGroup', index: 0 });
  assert.deepEqual(state.data.compileGroups[0], {
    name: 'Legacy', outputPath: 'build/legacy', options: '-g', targetEnv: 'vulkan1.3', targetSpv: '', fileStages: files,
  });
});
