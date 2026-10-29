import { createHash, randomUUID } from 'node:crypto';
import { createWriteStream } from 'node:fs';
import * as fs from 'node:fs/promises';
import * as path from 'node:path';
import { Readable, Transform } from 'node:stream';
import { pipeline } from 'node:stream/promises';
import * as yauzl from 'yauzl';

const repository = 'LBV2012-26/glsld';
const maxArchiveBytes = 512 * 1024 * 1024;
const maxExtractedBytes = 1024 * 1024 * 1024;
const installManifest = '.install.json';

export interface ServerTarget {
	name: string;
	executable: string;
}

export interface InstalledServer {
	version: string;
	target: string;
	executablePath: string;
}

export interface DownloadOptions {
	signal?: AbortSignal;
	onProgress?: (progress: { message: string; increment?: number }) => void;
}

interface ReleaseAsset {
	name: string;
	browser_download_url: string;
	size: number;
	digest?: string | null;
}

interface GitHubRelease {
	tag_name: string;
	assets: ReleaseAsset[];
}

export function getServerTarget(platform: string = process.platform, architecture: string = process.arch): ServerTarget {
	if (!['win32', 'linux', 'darwin'].includes(platform) || !['x64', 'arm64'].includes(architecture)) {
		throw new Error(`No glsld download is supported for ${platform}-${architecture}. Configure glsld.server.path to use a local executable.`);
	}
	return { name: `${platform}-${architecture}`, executable: platform === 'win32' ? 'glsld.exe' : 'glsld' };
}

function isReleaseVersion(version: unknown): version is string {
	return typeof version === 'string' && /^v?\d+\.\d+\.\d+(?:-[0-9A-Za-z.-]+)?(?:\+[0-9A-Za-z.-]+)?$/.test(version);
}

/** Downloads are kept outside the extension installation, under globalStorageUri. */
export class ServerManager {
	private readonly fetch: typeof globalThis.fetch;
	private readonly platform: string;
	private readonly architecture: string;
	private readonly downloads = new Map<string, Promise<InstalledServer>>();

	constructor(
		private readonly storagePath: string,
		options: { fetch?: typeof globalThis.fetch; platform?: string; architecture?: string } = {},
	) {
		this.fetch = options.fetch ?? globalThis.fetch;
		this.platform = options.platform ?? process.platform;
		this.architecture = options.architecture ?? process.arch;
	}

	async resolveServerPath(configuredPath: string, extensionPath: string): Promise<string | undefined> {
		if (configuredPath.trim()) {
			const executablePath = path.resolve(extensionPath, configuredPath.trim());
			if (!await isExecutableFile(executablePath, this.platform)) {
				throw new Error(`glsld.server.path does not point to an executable file: ${executablePath}`);
			}
			return executablePath;
		}
		return (await this.getInstalledServer())?.executablePath;
	}

	async getInstalledServer(version?: string): Promise<InstalledServer | undefined> {
		const target = getServerTarget(this.platform, this.architecture);
		if (version === undefined) { return this.getCurrentInstallation(target); }
		if (!isReleaseVersion(version)) { return undefined; }
		const directory = this.installationDirectory(version, target);
		try {
			const manifest: unknown = JSON.parse(await fs.readFile(path.join(directory, installManifest), 'utf8'));
			if (!manifest || typeof manifest !== 'object' ||
				!('version' in manifest) || manifest.version !== version ||
				!('target' in manifest) || manifest.target !== target.name) {
				return undefined;
			}
			const executablePath = await findServerExecutable(directory, target);
			return executablePath && await isExecutableFile(executablePath, this.platform)
				? { version, target: target.name, executablePath }
				: undefined;
		} catch (error) {
			if (isFileError(error, 'ENOENT') || error instanceof SyntaxError) {
				return undefined;
			}
			throw error;
		}
	}

	/** Every check queries GitHub, including when a server is already cached. */
	async checkForUpdates(options: DownloadOptions = {}): Promise<{ version: string; installed: boolean }> {
		const release = await this.getLatestRelease(options.signal);
		const installed = await this.getInstalledServer(release.tag_name);
		if (installed) { await this.selectInstallation(installed); }
		return { version: release.tag_name, installed: installed !== undefined };
	}

	downloadServer(options: DownloadOptions = {}): Promise<InstalledServer> {
		const target = getServerTarget(this.platform, this.architecture);
		const key = target.name;
		const pending = this.downloads.get(key);
		if (pending) { return pending; }
		const download = this.install(target, options).finally(() => this.downloads.delete(key));
		this.downloads.set(key, download);
		return download;
	}

	private installationDirectory(version: string, target: ServerTarget): string {
		return path.join(this.storagePath, 'servers', version, target.name);
	}

	private async getCurrentInstallation(target: ServerTarget): Promise<InstalledServer | undefined> {
		const root = path.join(this.storagePath, 'servers');
		try {
			const current = JSON.parse(await fs.readFile(path.join(root, `current-${target.name}.json`), 'utf8'));
			if (current && isReleaseVersion(current.version)) {
				const installed = await this.getInstalledServer(current.version);
				if (installed) { return installed; }
			}
		} catch (error) {
			if (!isFileError(error, 'ENOENT') && !(error instanceof SyntaxError)) { throw error; }
		}
		// Older installations have no current-version record. Recover them for offline use.
		let entries;
		try { entries = await fs.readdir(root, { withFileTypes: true }); } catch (error) {
			if (isFileError(error, 'ENOENT')) { return undefined; }
			throw error;
		}
		const candidates: { server: InstalledServer; modified: number }[] = [];
		for (const entry of entries) {
			if (!entry.isDirectory() || !isReleaseVersion(entry.name)) { continue; }
			const installed = await this.getInstalledServer(entry.name);
			if (installed) {
				const stat = await fs.stat(path.join(this.installationDirectory(entry.name, target), installManifest));
				candidates.push({ server: installed, modified: stat.mtimeMs });
			}
		}
		return candidates.sort((left, right) => right.modified - left.modified)[0]?.server;
	}

	private async selectInstallation(installed: InstalledServer): Promise<InstalledServer> {
		const root = path.join(this.storagePath, 'servers');
		const temporary = path.join(root, `.current-${randomUUID()}.json`);
		await fs.mkdir(root, { recursive: true });
		try {
			await fs.writeFile(temporary, JSON.stringify({ version: installed.version }), { flag: 'wx' });
			await fs.rename(temporary, path.join(root, `current-${installed.target}.json`));
		} finally {
			await fs.rm(temporary, { force: true });
		}
		return installed;
	}

	private async getLatestRelease(cancellation?: AbortSignal): Promise<GitHubRelease> {
		const signal = AbortSignal.any([
			...(cancellation ? [cancellation] : []), AbortSignal.timeout(10_000),
		]);
		const response = await this.request(`https://api.github.com/repos/${repository}/releases/latest`, signal);
		if (response.status === 404) {
			await response.body?.cancel();
			throw new Error('No latest GitHub release has been published. Retry after a release is available, or configure glsld.server.path to use a local executable.');
		}
		await checkResponse(response);
		const release = JSON.parse(await readText(response, signal)) as GitHubRelease;
		if (!release || !isReleaseVersion(release.tag_name) || !Array.isArray(release.assets) ||
			release.assets.some(asset => !asset || typeof asset.name !== 'string' || typeof asset.browser_download_url !== 'string')) {
			throw new Error('GitHub returned invalid latest release information.');
		}
		return release;
	}

	private async install(target: ServerTarget, options: DownloadOptions): Promise<InstalledServer> {
		const signal = AbortSignal.any([
			...(options.signal ? [options.signal] : []),
			AbortSignal.timeout(5 * 60 * 1000),
		]);
		signal.throwIfAborted();
		options.onProgress?.({ message: `Checking the latest GitHub release for ${target.name}…` });
		const release = await this.getLatestRelease(signal);
		const version = release.tag_name;
		const installed = await this.getInstalledServer(version);
		if (installed) { return this.selectInstallation(installed); }
		const assetName = `glsld-${target.name}.zip`;
		const asset = release.assets.find(candidate => candidate.name === assetName);
		if (!asset) {
			if (release.assets.some(candidate => candidate.name === `glsld-${target.name}.7z`)) {
				throw new Error(`7z archives are not supported. Publish ${assetName} as a ZIP archive instead.`);
			}
			throw new Error(`Release ${version} does not contain ${assetName}. This platform may not have a published build; configure glsld.server.path to use a local executable.`);
		}
		if (!Number.isSafeInteger(asset.size) || asset.size <= 0 || asset.size > maxArchiveBytes) {
			throw new Error(`Invalid download size for ${assetName}.`);
		}
		const checksum = await this.getChecksum(release, asset, signal);
		const directory = this.installationDirectory(version, target);
		await fs.mkdir(path.dirname(directory), { recursive: true });
		const staging = await fs.mkdtemp(path.join(path.dirname(directory), '.download-'));
		try {
			const archivePath = path.join(staging, 'server.zip');
			const response = await this.request(assetUrl(asset, version), signal, 'application/octet-stream');
			await checkResponse(response);
			if (!response.body) { throw new Error('The server download was empty.'); }
			let downloaded = 0;
			const hash = createHash('sha256');
			const progress = new Transform({
				transform(chunk: Buffer, _encoding, callback) {
					downloaded += chunk.length;
					if (downloaded > asset.size || downloaded > maxArchiveBytes) {
						callback(new Error('The server download exceeded its expected size.'));
						return;
					}
					hash.update(chunk);
					options.onProgress?.({ message: `Downloading ${assetName}…`, increment: chunk.length / asset.size * 100 });
					callback(null, chunk);
				},
			});
			await pipeline(Readable.fromWeb(response.body), progress, createWriteStream(archivePath, { flags: 'wx' }), { signal });
			if (downloaded !== asset.size || hash.digest('hex') !== checksum) {
				throw new Error(`SHA-256 verification failed for ${assetName}. The download was not installed; retry the download.`);
			}

			options.onProgress?.({ message: 'Extracting and installing the language server…' });
			const extracted = path.join(staging, 'package');
			await fs.mkdir(extracted);
			await extractArchive(archivePath, extracted, signal);
			const stagedExecutable = await findServerExecutable(extracted, target);
			if (!stagedExecutable) {
				throw new Error(`${assetName} must contain ${serverExecutablePaths(target).join(' or ')}.`);
			}
			const executableRelativePath = path.relative(extracted, stagedExecutable);
			if (this.platform !== 'win32') { await fs.chmod(stagedExecutable, 0o755); }
			await fs.writeFile(path.join(extracted, installManifest), JSON.stringify({ version, target: target.name, sha256: checksum }), { flag: 'wx' });
			signal.throwIfAborted();
			try {
				await fs.rename(extracted, directory);
			} catch (error) {
				// Another VS Code window may have installed this same version meanwhile.
				const otherInstallation = await this.getInstalledServer(version);
				if (otherInstallation) { return this.selectInstallation(otherInstallation); }
				// Preserve an incomplete previous installation until its replacement is ready.
				const previous = path.join(staging, 'previous');
				try { await fs.rename(directory, previous); } catch { throw error; }
				try {
					await fs.rename(extracted, directory);
				} catch (installError) {
					await fs.rename(previous, directory);
					throw installError;
				}
			}
			return await this.selectInstallation({ version, target: target.name, executablePath: path.join(directory, executableRelativePath) });
		} finally {
			await fs.rm(staging, { recursive: true, force: true });
		}
	}

	private async getChecksum(release: GitHubRelease, asset: ReleaseAsset, signal: AbortSignal): Promise<string> {
		if (typeof asset.digest === 'string' && /^sha256:[0-9a-f]{64}$/i.test(asset.digest)) {
			return asset.digest.slice(7).toLowerCase();
		}
		const checksums = release.assets.find(candidate => candidate.name === 'SHA256SUMS');
		if (!checksums) {
			throw new Error(`Release ${release.tag_name} must provide a SHA-256 asset digest or a SHA256SUMS file.`);
		}
		const response = await this.request(assetUrl(checksums, release.tag_name), signal, 'text/plain');
		await checkResponse(response);
		for (const line of (await readText(response, signal)).split(/\r?\n/)) {
			const match = /^([0-9a-f]{64})\s+\*?(.+)$/i.exec(line);
			if (match?.[2] === asset.name) { return match[1].toLowerCase(); }
		}
		throw new Error(`SHA256SUMS does not contain a checksum for ${asset.name}.`);
	}

	private async request(url: string, signal: AbortSignal, accept = 'application/vnd.github+json'): Promise<Response> {
		for (let redirects = 0; redirects <= 5; redirects++) {
			signal.throwIfAborted();
			if (new URL(url).protocol !== 'https:') { throw new Error('Server downloads require HTTPS.'); }
			const response = await this.fetch(url, {
				redirect: 'manual', signal,
				headers: { Accept: accept, 'User-Agent': 'glsld-vscode', 'Cache-Control': 'no-cache' },
			});
			if (![301, 302, 303, 307, 308].includes(response.status)) { return response; }
			const location = response.headers.get('location');
			await response.body?.cancel();
			if (!location) { throw new Error('GitHub returned a redirect without a download location.'); }
			url = new URL(location, url).href;
		}
		throw new Error('GitHub returned too many download redirects.');
	}
}

function serverExecutablePaths(target: ServerTarget): string[] {
	// Preserve the Windows build directory and the DLLs next to its executable.
	// Flat archives remain valid for previously installed releases.
	return target.name.startsWith('win32-')
		? [path.join('Win64', target.executable), target.executable]
		: [target.executable];
}

async function findServerExecutable(directory: string, target: ServerTarget): Promise<string | undefined> {
	for (const relativePath of serverExecutablePaths(target)) {
		const executablePath = path.join(directory, relativePath);
		if (await isRegularFile(executablePath)) { return executablePath; }
	}
	return undefined;
}

function assetUrl(asset: ReleaseAsset, version: string): string {
	const url = new URL(asset.browser_download_url);
	const parts = url.pathname.split('/').map(decodeURIComponent);
	if (url.protocol !== 'https:' || url.hostname !== 'github.com' ||
		parts.slice(1, 5).join('/').toLowerCase() !== `${repository}/releases/download`.toLowerCase() ||
		parts.length !== 7 || parts[5] !== version || parts[6] !== asset.name) {
		throw new Error(`GitHub returned an unexpected download URL for ${asset.name}.`);
	}
	return url.href;
}

async function checkResponse(response: Response): Promise<void> {
	if (response.ok) { return; }
	await response.body?.cancel();
	if (response.status === 403 && response.headers.get('x-ratelimit-remaining') === '0') {
		throw new Error('GitHub API rate limit reached. Retry later, or configure glsld.server.path to use a local executable.');
	}
	throw new Error(`GitHub download failed (HTTP ${response.status}). Check your connection and VS Code proxy settings, then retry.`);
}

async function readText(response: Response, signal: AbortSignal): Promise<string> {
	if (!response.body) { throw new Error('GitHub returned an empty response.'); }
	const reader = response.body.getReader();
	const chunks: Uint8Array[] = [];
	let size = 0;
	try {
		while (true) {
			signal.throwIfAborted();
			const { value, done } = await reader.read();
			if (done) { break; }
			size += value.length;
			if (size > 2 * 1024 * 1024) { throw new Error('GitHub release metadata exceeded the size limit.'); }
			chunks.push(value);
		}
		return Buffer.concat(chunks).toString('utf8');
	} finally {
		await reader.cancel();
		reader.releaseLock();
	}
}

async function extractArchive(archivePath: string, directory: string, signal: AbortSignal): Promise<void> {
	const zip = await yauzl.openPromise(archivePath, { strictFileNames: true, validateEntrySizes: true });
	let extractedBytes = 0;
	try {
		if (zip.entryCount > 5000) { throw new Error('The server archive contains too many files.'); }
		for await (const entry of zip.eachEntry()) {
			signal.throwIfAborted();
			const mode = entry.externalFileAttributes >>> 16;
			const fileType = mode & 0o170000;
			const parts = entry.fileName.replace(/\/$/, '').split('/');
			if (parts.some(part => !part || part === '.' || part === '..' || /[:\\\x00]/.test(part)) ||
				![0, 0o100000, 0o040000].includes(fileType) || entry.isEncrypted()) {
				throw new Error(`Unsafe entry in the server archive: ${entry.fileName}`);
			}
			const destination = path.resolve(directory, ...parts);
			const relative = path.relative(directory, destination);
			if (!relative || relative.startsWith(`..${path.sep}`) || path.isAbsolute(relative)) {
				throw new Error(`Unsafe path in the server archive: ${entry.fileName}`);
			}
			extractedBytes += entry.uncompressedSize;
			if (extractedBytes > maxExtractedBytes) { throw new Error('The extracted server exceeds the size limit.'); }
			if (entry.fileName.endsWith('/')) {
				await fs.mkdir(destination, { recursive: true });
				continue;
			}
			await fs.mkdir(path.dirname(destination), { recursive: true });
			const stream = await zip.openReadStreamPromise(entry);
			await pipeline(stream, createWriteStream(destination, { flags: 'wx', mode: mode & 0o111 ? 0o755 : 0o644 }), { signal });
		}
	} finally {
		zip.close();
	}
}

async function isRegularFile(filePath: string): Promise<boolean> {
	try {
		const stat = await fs.stat(filePath);
		return stat.isFile() && stat.size > 0;
	} catch (error) {
		if (isFileError(error, 'ENOENT')) { return false; }
		throw error;
	}
}

async function isExecutableFile(filePath: string, platform: string): Promise<boolean> {
	if (!await isRegularFile(filePath)) { return false; }
	try {
		await fs.access(filePath, platform === 'win32' ? fs.constants.R_OK : fs.constants.X_OK);
		return true;
	} catch { return false; }
}

function isFileError(error: unknown, code: string): boolean {
	return error instanceof Error && 'code' in error && error.code === code;
}
