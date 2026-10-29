const assert = require('node:assert/strict');
const { createHash } = require('node:crypto');
const fs = require('node:fs/promises');
const os = require('node:os');
const path = require('node:path');
const { test } = require('node:test');
const { deflateRawSync } = require('node:zlib');
const { ServerManager, getServerTarget } = require('../out/serverManager');

const version = 'v0.9.5';
const assetName = 'glsld-win32-x64.zip';
const releaseUrl = 'https://api.github.com/repos/LBV2012-26/glsld/releases/latest';
const assetUrl = `https://github.com/LBV2012-26/glsld/releases/download/${version}/${assetName}`;
const checksumUrl = `https://github.com/LBV2012-26/glsld/releases/download/${version}/SHA256SUMS`;

// Generate small real ZIP fixtures, including malformed entries the extractor must reject.
function zipArchive(files) {
  const chunks = [];
  const central = [];
  let offset = 0;
  for (const file of files) {
    const name = Buffer.from(file.name);
    const data = Buffer.from(file.data ?? 'server fixture');
    const compressed = deflateRawSync(data);
    let crc = 0xffffffff;
    for (const byte of data) {
      crc ^= byte;
      for (let bit = 0; bit < 8; bit++) crc = (crc >>> 1) ^ (crc & 1 ? 0xedb88320 : 0);
    }
    crc = (crc ^ 0xffffffff) >>> 0;
    const local = Buffer.alloc(30);
    local.writeUInt32LE(0x04034b50);
    local.writeUInt16LE(20, 4);
    local.writeUInt16LE(8, 8);
    local.writeUInt32LE(crc, 14);
    local.writeUInt32LE(compressed.length, 18);
    local.writeUInt32LE(file.size ?? data.length, 22);
    local.writeUInt16LE(name.length, 26);
    const directory = Buffer.alloc(46);
    directory.writeUInt32LE(0x02014b50);
    directory.writeUInt16LE(0x0314, 4);
    directory.writeUInt16LE(20, 6);
    directory.writeUInt16LE(8, 10);
    directory.writeUInt32LE(crc, 16);
    directory.writeUInt32LE(compressed.length, 20);
    directory.writeUInt32LE(file.size ?? data.length, 24);
    directory.writeUInt16LE(name.length, 28);
    directory.writeUInt32LE(((file.mode ?? 0o100644) << 16) >>> 0, 38);
    directory.writeUInt32LE(offset, 42);
    chunks.push(local, name, compressed);
    central.push(directory, name);
    offset += local.length + name.length + compressed.length;
  }
  const end = Buffer.alloc(22);
  end.writeUInt32LE(0x06054b50);
  end.writeUInt16LE(files.length, 8);
  end.writeUInt16LE(files.length, 10);
  end.writeUInt32LE(Buffer.concat(central).length, 12);
  end.writeUInt32LE(offset, 16);
  return Buffer.concat([...chunks, ...central, end]);
}

async function fixture(t, options = {}) {
  const storage = await fs.mkdtemp(path.join(os.tmpdir(), 'glsld-download-test-'));
  t.after(() => fs.rm(storage, { recursive: true, force: true }));
  const archive = options.archive ?? zipArchive([{ name: 'glsld.exe' }, { name: 'runtime.dll', data: 'runtime fixture' }]);
  const sha256 = createHash('sha256').update(archive).digest('hex');
  const release = {
    tag_name: version,
    assets: [{ name: assetName, size: archive.length, digest: `sha256:${sha256}`, browser_download_url: assetUrl }],
  };
  const requests = [];
  const fetch = async (url, init) => {
    init.signal.throwIfAborted();
    requests.push({ url: String(url), init });
    if (options.respond) return options.respond(String(url), init, { archive, release, sha256 });
    if (String(url) === releaseUrl) return Response.json(release);
    if (String(url) === assetUrl) return new Response(archive);
    if (String(url) === checksumUrl) return new Response(`${sha256} *${assetName}\r\n`);
    throw new Error(`Unexpected request: ${url}`);
  };
  const manager = new ServerManager(storage, { fetch, platform: 'win32', architecture: 'x64', ...options.manager });
  return { storage, manager, archive, release, sha256, requests, fetch };
}

test('invalid and unsafe release tags are rejected before creating an installation', async t => {
  for (const value of ['', '../0.9.5', 'v0.9.5/../../outside', 'C:\\server']) {
    const { manager, release, storage } = await fixture(t);
    release.tag_name = value;
    await assert.rejects(manager.downloadServer(), /invalid latest release information/);
    assert.deepEqual(await fs.readdir(storage), []);
  }
});

test('platform selection uses the extension host and rejects unsupported targets', () => {
  assert.deepEqual(getServerTarget('win32', 'x64'), { name: 'win32-x64', executable: 'glsld.exe' });
  assert.deepEqual(getServerTarget('linux', 'arm64'), { name: 'linux-arm64', executable: 'glsld' });
  assert.deepEqual(getServerTarget('darwin', 'arm64'), { name: 'darwin-arm64', executable: 'glsld' });
  assert.throws(() => getServerTarget('linux', 'riscv64'), /local executable/);
});

test('custom absolute and relative paths take priority even on an unsupported download platform', async t => {
  const { manager, storage, requests } = await fixture(t, { manager: { platform: 'unsupported' } });
  const executable = path.join(storage, 'local.exe');
  await fs.writeFile(executable, 'local binary');
  await fs.chmod(executable, 0o755);
  assert.equal(await manager.resolveServerPath(executable, storage), executable);
  assert.equal(await manager.resolveServerPath('local.exe', storage), executable);
  assert.equal(requests.length, 0);
  await assert.rejects(manager.resolveServerPath('missing.exe', storage), /does not point to an executable file/);
  await assert.rejects(manager.resolveServerPath(storage, storage), /does not point to an executable file/);
});

test('a missing cache does not trigger a network request', async t => {
  const { manager, storage, requests } = await fixture(t);
  assert.equal(await manager.resolveServerPath('', storage), undefined);
  assert.equal(requests.length, 0);
});

test('downloads, verifies, installs under global storage, and reuses the cache offline', async t => {
  const { manager, storage, requests } = await fixture(t);
  const progress = [];
  const installed = await manager.downloadServer({ onProgress: update => progress.push(update) });
  assert.equal(installed.executablePath, path.join(storage, 'servers', version, 'win32-x64', 'glsld.exe'));
  assert.equal(await fs.readFile(installed.executablePath, 'utf8'), 'server fixture');
  assert.equal(await fs.readFile(path.join(path.dirname(installed.executablePath), 'runtime.dll'), 'utf8'), 'runtime fixture');
  assert.ok(Math.abs(progress.reduce((total, item) => total + (item.increment ?? 0), 0) - 100) < 0.001);
  assert.deepEqual(await fs.readdir(path.join(storage, 'servers', version)), ['win32-x64']);
  const offline = new ServerManager(storage, { platform: 'win32', architecture: 'x64', fetch: () => { throw new Error('offline'); } });
  assert.deepEqual(await offline.getInstalledServer(), installed);
  await assert.rejects(offline.checkForUpdates(), /offline/);
  await assert.rejects(offline.downloadServer(), /offline/);
  assert.equal(await offline.resolveServerPath('', '/unused-extension'), installed.executablePath);
  assert.deepEqual(await manager.downloadServer(), installed);
  assert.deepEqual(requests.map(request => request.url), [releaseUrl, assetUrl, releaseUrl]);
  assert.equal(requests.at(-1).init.headers['Cache-Control'], 'no-cache');
});

test('Windows releases preserve Win64/glsld.exe and its runtime files and can be reused offline', async t => {
  const { manager, storage } = await fixture(t, { archive: zipArchive([
    { name: 'Win64/', data: '', mode: 0o040755 },
    { name: 'Win64/glsld.exe' },
    { name: 'Win64/runtime.dll', data: 'runtime fixture' },
  ]) });
  const installed = await manager.downloadServer();
  assert.equal(installed.executablePath, path.join(storage, 'servers', version, 'win32-x64', 'Win64', 'glsld.exe'));
  assert.equal(await fs.readFile(installed.executablePath, 'utf8'), 'server fixture');
  assert.equal(await fs.readFile(path.join(path.dirname(installed.executablePath), 'runtime.dll'), 'utf8'), 'runtime fixture');
  const offline = new ServerManager(storage, {
    platform: 'win32', architecture: 'x64', fetch: () => { throw new Error('offline'); },
  });
  assert.deepEqual(await offline.getInstalledServer(), installed);
  assert.equal(await offline.resolveServerPath('', '/unused-extension'), installed.executablePath);
  await assert.rejects(offline.downloadServer(), /offline/);
});

test('a release containing only a 7z asset explains that ZIP is required', async t => {
  const { manager, release } = await fixture(t);
  release.assets[0].name = 'glsld-win32-x64.7z';
  await assert.rejects(manager.downloadServer(), /7z archives are not supported.*glsld-win32-x64.zip/);
});

test('SHA256SUMS is used when GitHub does not provide a digest', async t => {
  const { manager, release, requests } = await fixture(t);
  release.assets[0].digest = null;
  release.assets.push({ name: 'SHA256SUMS', browser_download_url: checksumUrl });
  await manager.downloadServer();
  assert.ok(requests.some(request => request.url === checksumUrl));
});

test('downloads without a checksum are rejected before fetching the archive', async t => {
  const { manager, release, requests } = await fixture(t);
  delete release.assets[0].digest;
  await assert.rejects(manager.downloadServer(), /SHA-256.*SHA256SUMS/);
  assert.equal(requests.length, 1);
});

test('an absent checksum entry is rejected', async t => {
  const { manager, release } = await fixture(t, {
    respond: (url, _init, { release, sha256 }) => url === releaseUrl ? Response.json(release) : new Response(`${sha256}  other.zip`),
  });
  release.assets[0].digest = null;
  release.assets.push({ name: 'SHA256SUMS', browser_download_url: checksumUrl });
  await assert.rejects(manager.downloadServer(), /does not contain a checksum/);
});

test('checksum failures clean staging and preserve earlier versions', async t => {
  const { manager, release, storage } = await fixture(t);
  const original = await manager.downloadServer();
  release.tag_name = 'v0.9.6';
  release.assets[0].digest = `sha256:${'0'.repeat(64)}`;
  release.assets[0].browser_download_url = assetUrl.replace(version, 'v0.9.6');
  const failing = new ServerManager(storage, {
    platform: 'win32', architecture: 'x64',
    fetch: async url => String(url).includes('api.github.com') ? Response.json(release) : new Response(zipArchive([{ name: 'glsld.exe' }, { name: 'runtime.dll', data: 'runtime fixture' }])),
  });
  await assert.rejects(failing.downloadServer(), /SHA-256 verification failed/);
  assert.deepEqual(await fs.readdir(path.join(storage, 'servers', 'v0.9.6')), []);
  assert.deepEqual(await manager.getInstalledServer(), original);
  assert.equal(await fs.readFile(original.executablePath, 'utf8'), 'server fixture');
});

test('missing releases, missing platform assets, rate limits and HTTP errors are actionable', async t => {
  for (const [status, headers, expected] of [
    [404, {}, /No latest GitHub release has been published/],
    [403, { 'x-ratelimit-remaining': '0' }, /rate limit reached/],
    [500, {}, /HTTP 500/],
  ]) {
    const { manager } = await fixture(t, { respond: () => new Response('', { status, headers }) });
    await assert.rejects(manager.downloadServer(), expected);
  }
  const { manager, release } = await fixture(t);
  release.assets = [];
  await assert.rejects(manager.downloadServer(), /does not contain glsld-win32-x64.zip/);
});

test('the release tag comes from GitHub and is not tied to the extension version or a v prefix', async t => {
  const { manager, release, storage } = await fixture(t, {
    respond: (url, _init, { release, archive }) => url === releaseUrl ? Response.json(release) : new Response(archive),
  });
  release.tag_name = '9.9.9';
  release.assets[0].browser_download_url = assetUrl.replace(version, '9.9.9');
  const installed = await manager.downloadServer();
  assert.equal(installed.version, '9.9.9');
  assert.equal(installed.executablePath, path.join(storage, 'servers', '9.9.9', 'win32-x64', 'glsld.exe'));
});

test('GitHub asset redirects are followed without allowing an HTTPS downgrade', async t => {
  const redirected = 'https://release-assets.githubusercontent.com/signed-fixture.zip';
  const { manager, requests } = await fixture(t, {
    respond: (url, _init, { release, archive }) => {
      if (url === releaseUrl) return Response.json(release);
      if (url === assetUrl) return new Response(null, { status: 302, headers: { location: redirected } });
      assert.equal(url, redirected);
      return new Response(archive);
    },
  });
  await manager.downloadServer();
  assert.equal(requests.at(-1).url, redirected);
  const downgraded = await fixture(t, {
    respond: (url, _init, { release }) => url === releaseUrl ? Response.json(release)
      : new Response(null, { status: 302, headers: { location: 'http://example.com/server.zip' } }),
  });
  await assert.rejects(downgraded.manager.downloadServer(), /require HTTPS/);
});

test('unrelated asset URLs and excessive redirects are rejected', async t => {
  const { manager, release } = await fixture(t);
  release.assets[0].browser_download_url = 'https://example.com/server.zip';
  await assert.rejects(manager.downloadServer(), /unexpected download URL/);
  const loop = await fixture(t, {
    respond: () => new Response(null, { status: 302, headers: { location: releaseUrl } }),
  });
  await assert.rejects(loop.manager.downloadServer(), /too many download redirects/);
});

test('oversized and truncated archives are not installed', async t => {
  const huge = await fixture(t);
  huge.release.assets[0].size = 1024 * 1024 * 1024;
  await assert.rejects(huge.manager.downloadServer(), /Invalid download size/);
  const truncated = await fixture(t, {
    respond: (url, _init, { release, archive }) => url === releaseUrl ? Response.json(release) : new Response(archive.subarray(0, 15)),
  });
  await assert.rejects(truncated.manager.downloadServer(), /SHA-256 verification failed/);
  assert.equal(await truncated.manager.getInstalledServer(), undefined);
});

test('unsafe, duplicate, incomplete, and oversized ZIP contents are rejected and staging is cleaned', async t => {
  const cases = [
    [[{ name: '../escaped.exe' }], /invalid relative path|Unsafe/],
    [[{ name: 'C:/escaped.exe' }], /absolute path|Unsafe/],
    [[{ name: 'glsld.exe', mode: 0o120777 }], /Unsafe entry/],
    [[{ name: 'glsld.exe' }, { name: 'glsld.exe' }], /EEXIST/],
    [[{ name: 'nested/glsld.exe' }], /must contain/],
    [[{ name: 'glsld.exe', size: 2 * 1024 * 1024 * 1024 }], /size limit/],
  ];
  for (const [files, expected] of cases) {
    const { manager, storage } = await fixture(t, { archive: zipArchive(files) });
    await assert.rejects(manager.downloadServer(), expected);
    assert.equal(await manager.getInstalledServer(), undefined);
    assert.deepEqual(await fs.readdir(path.join(storage, 'servers', version)), []);
    await assert.rejects(fs.stat(path.join(storage, 'servers', 'escaped.exe')), { code: 'ENOENT' });
  }
});

test('a pre-cancelled download does not touch the network or create an installation', async t => {
  const { manager, requests, storage } = await fixture(t);
  const abort = new AbortController();
  abort.abort();
  await assert.rejects(manager.checkForUpdates({ signal: abort.signal }), /abort/i);
  await assert.rejects(manager.downloadServer({ signal: abort.signal }), /abort/i);
  assert.equal(requests.length, 0);
  assert.deepEqual(await fs.readdir(storage), []);
});

test('cached versions never skip Latest checks and a newly published release is downloaded', async t => {
  const { manager, release, requests, storage } = await fixture(t, {
    respond: (url, _init, { release, archive }) => {
      if (url === releaseUrl) return Response.json(release);
      assert.equal(url, release.assets[0].browser_download_url);
      return new Response(archive);
    },
  });
  const previous = await manager.downloadServer();
  release.tag_name = 'v12.34.56';
  release.assets[0].browser_download_url = assetUrl.replace(version, release.tag_name);
  assert.deepEqual(await manager.checkForUpdates(), { version: 'v12.34.56', installed: false });
  assert.deepEqual(await manager.getInstalledServer(), previous);
  assert.equal(requests.filter(request => request.url !== releaseUrl).length, 1);

  const latest = await manager.downloadServer();
  assert.equal(latest.version, 'v12.34.56');
  assert.deepEqual(await manager.getInstalledServer(), latest);
  assert.deepEqual(await manager.getInstalledServer(version), previous);
  const record = JSON.parse(await fs.readFile(path.join(storage, 'servers', 'current-win32-x64.json'), 'utf8'));
  assert.equal(record.version, latest.version);
  assert.equal(requests.filter(request => request.url === releaseUrl).length, 3);

  // GitHub can mark an older release Latest; use its answer, not a semver maximum.
  release.tag_name = version;
  release.assets[0].browser_download_url = assetUrl;
  assert.deepEqual(await manager.checkForUpdates(), { version, installed: true });
  assert.deepEqual(await manager.getInstalledServer(), previous);
  assert.equal(requests.filter(request => request.url !== releaseUrl).length, 2);
});

test('legacy installations and a damaged current-version record are usable offline', async t => {
  const { manager, storage } = await fixture(t);
  const installed = await manager.downloadServer();
  const recordPath = path.join(storage, 'servers', 'current-win32-x64.json');
  await fs.rm(recordPath);
  assert.deepEqual(await manager.getInstalledServer(), installed);
  await fs.writeFile(recordPath, '{invalid');
  assert.deepEqual(await manager.getInstalledServer(), installed);
  await fs.writeFile(recordPath, JSON.stringify({ version: '../../outside' }));
  assert.deepEqual(await manager.getInstalledServer(), installed);
});

test('cancelling an active download removes partial files and allows retry', async t => {
  const abort = new AbortController();
  let firstAttempt = true;
  const { manager, storage } = await fixture(t, {
    respond: (url, _init, { release, archive }) => {
      if (url === releaseUrl) return Response.json(release);
      if (!firstAttempt) return new Response(archive);
      firstAttempt = false;
      return new Response(new ReadableStream({
        start(controller) { controller.enqueue(archive.subarray(0, 10)); },
      }));
    },
  });
  await assert.rejects(manager.downloadServer({ signal: abort.signal, onProgress: update => {
    if (update.increment) abort.abort();
  } }), /abort/i);
  assert.deepEqual(await fs.readdir(path.join(storage, 'servers', version)), []);
  assert.ok(await manager.downloadServer());
});

test('concurrent requests share one download and an incomplete cache can be repaired', async t => {
  const { manager, requests, storage } = await fixture(t);
  const directory = path.join(storage, 'servers', version, 'win32-x64');
  await fs.mkdir(directory, { recursive: true });
  await fs.writeFile(path.join(directory, '.install.json'), '{broken');
  const first = manager.downloadServer();
  const second = manager.downloadServer();
  const [left, right] = await Promise.all([first, second]);
  assert.deepEqual(left, right);
  assert.equal(requests.filter(request => request.url === assetUrl).length, 1);
  assert.deepEqual(await fs.readdir(path.dirname(directory)), ['win32-x64']);
});

test('separate VS Code windows can install the same version into shared global storage', async t => {
  const { manager, storage, fetch } = await fixture(t);
  const otherWindow = new ServerManager(storage, { fetch, platform: 'win32', architecture: 'x64' });
  const [first, second] = await Promise.all([manager.downloadServer(), otherWindow.downloadServer()]);
  assert.deepEqual(first, second);
  assert.equal(await fs.readFile(first.executablePath, 'utf8'), 'server fixture');
  assert.deepEqual(await fs.readdir(path.join(storage, 'servers', version)), ['win32-x64']);
});

test('Unix downloads use the requested host target and make the server executable', async t => {
  const archive = zipArchive([{ name: 'glsld' }]);
  const sha256 = createHash('sha256').update(archive).digest('hex');
  const { storage } = await fixture(t);
  const manager = new ServerManager(storage, {
    platform: 'linux', architecture: 'arm64',
    fetch: async url => String(url) === releaseUrl ? Response.json({
      tag_name: version,
      assets: [{ name: 'glsld-linux-arm64.zip', size: archive.length, digest: `sha256:${sha256}`, browser_download_url: assetUrl.replace(assetName, 'glsld-linux-arm64.zip') }],
    }) : new Response(archive),
  });
  const installed = await manager.downloadServer();
  assert.equal(installed.executablePath, path.join(storage, 'servers', version, 'linux-arm64', 'glsld'));
  if (process.platform !== 'win32') assert.equal((await fs.stat(installed.executablePath)).mode & 0o777, 0o755);
  assert.equal(await manager.resolveServerPath('', storage), installed.executablePath);
});
