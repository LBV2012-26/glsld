export interface ConfigFolderNode {
	kind: 'configFolder';
	folderPath: string;
	label: string;
}

interface ConfigFileEntry {
	kind: 'configFile';
	key: string;
	label: string;
}

function normalize(displayPath: string): string {
	return displayPath.replace(/\\/g, '/').split('/').filter(part => part && part !== '.').join('/');
}

export function configFileName(displayPath: string): string {
	return normalize(displayPath).split('/').at(-1) ?? displayPath;
}

/** Build one directory level without changing the original configuration keys. */
export function configFileChildren(
	files: { key: string; displayPath: string }[],
	folderPath = '',
): (ConfigFolderNode | ConfigFileEntry)[] {
	const prefix = folderPath ? `${folderPath}/` : '';
	const folders = new Map<string, ConfigFolderNode>();
	const children: ConfigFileEntry[] = [];
	for (const file of files) {
		const displayPath = normalize(file.displayPath);
		if (!displayPath.startsWith(prefix)) { continue; }
		const remainder = displayPath.slice(prefix.length);
		if (!remainder) { continue; }
		const separator = remainder.indexOf('/');
		if (separator < 0) { children.push({ kind: 'configFile', key: file.key, label: remainder }); }
		else {
			const label = remainder.slice(0, separator);
			const key = prefix + label;
			folders.set(key, { kind: 'configFolder', folderPath: key, label });
		}
	}
	const compare = (a: { label: string }, b: { label: string }) => a.label.localeCompare(b.label, undefined, { numeric: true });
	return [...[...folders.values()].sort(compare), ...children.sort(compare)];
}

export function configFileParent(displayPath: string): ConfigFolderNode | undefined {
	const normalized = normalize(displayPath);
	const separator = normalized.lastIndexOf('/');
	if (separator < 0) { return undefined; }
	const folderPath = normalized.slice(0, separator);
	return { kind: 'configFolder', folderPath, label: configFileName(folderPath) };
}
