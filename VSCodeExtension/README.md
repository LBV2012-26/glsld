# glsld

> [!WARNING]
> glsld is in early development. It is currently Windows only, and Linux builds are on the way. Features, configuration, and protocol details may change between releases.

> [!NOTE]
> This extension is an independent project and is not affiliated with another Visual Studio Code extension that also uses the name `glsld` (https://github.com/daiyousei-qz/glsld). I was the one who discovered the name collision only while preparing this extension for publication on the Visual Studio Code Marketplace.

glsld is a language server and Visual Studio Code extension for GLSL. It provides language-aware editing features for shader projects, including preprocessing, semantic analysis, cross-file navigation, and background indexing.

## Setup

Install the extension, open a folder containing GLSL source files, and configure the directories that should be indexed:

```json
{
    "glsld.backgroundIndex.roots": ["Shaders"]
}
```

The Windows extension package is intended to include the glsld server. A custom server executable can also be selected with `glsld.server.path`.

## Code completion

glsld completes GLSL keywords, types, variables, functions, structure members, and other symbols available at the cursor.

![Code completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CodeCompletion.png)

### Extended completion

Extended completion supplies context-aware candidates beyond direct symbol-name matching, helping complete common GLSL constructs and declarations.

![Extended completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/ExtensionCompletion.png)

### Include completion

Include paths are completed relative to the current file and the configured include directories.

![Include completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/IncludeCompletion.png)

### Overload resolve

When a function has multiple signatures, glsld presents its overloads so that the appropriate parameter list and return type can be selected.

![Overload resolve](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/FunctionOverloads.png)

### Signature help

Signature help shows the active overload and highlights the parameter currently being entered.

![Signature help](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/SignatureHelp.png)

## Diagnostics

Syntax and semantic errors are reported while editing. Diagnostics can be enabled or disabled with `glsld.diagnostics.enabled`.

![Diagnostics](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Diagnostics.png)

## Go to definition

Jump from a symbol use to its declaration or definition, including symbols declared in included files.

| Symbol use | Definition target |
| --- | --- |
| ![Go to definition from a symbol use](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Goto1.png) | ![Go to definition target](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Goto2.png) |

## Find references

Find symbol references across the workspace. Background indexing allows references in files that have not been opened in the editor to participate in the result.

![Find references](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Reference.png)

References are tracked across include boundaries as well as within the current file.

![Cross-file references](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CrossReference.png)

## Hover

Hovering over a symbol displays its declaration and relevant type information.

| Symbol information | Function information |
| --- | --- |
| ![Hover information for a symbol](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Hover1.png) | ![Hover information for a function](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Hover2.png) |

## Inlay hints

glsld can display inline hints that make shader code easier to read. They can be toggled with `glsld.capabilities.inlayHints`.

![Inlay hints](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/InlayHints.png)

## Document symbols

The document outline exposes declarations in the current shader and supports quick navigation through the file.

## Preprocessor variants

Macro variants can be configured for the whole workspace or for an individual shader. The extension sends the active variant state to the server and refreshes affected documents when it changes.

| Variant configuration | Variant applied to source code |
| --- | --- |
| ![Macro variant configuration](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/MacroVariant1.png) | ![Macro variant result](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/MacroVariant2.png) |

## Compile groups

Compile groups collect shaders using configurable source patterns, command templates, output paths, macro variants, and concurrency settings. They can be compiled directly from the glsld sidebar.

![Compile groups](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CompileGroups.png)

## Configuration

User-controlled settings belong in `.vscode/settings.json`:

```json
{
    "glsld.backgroundIndex.roots": ["Shaders", "Include"],
    "glsld.capabilities.inlayHints": true,
    "glsld.diagnostics.enabled": true,
    "glsld.server.path": "bin/Win64/glsld.exe"
}
```

The `.glsld/config.json` file stores shader and variant state generated by the extension. It is not intended to replace ordinary VS Code settings.

If `glsld.backgroundIndex.roots` is empty or absent, glsld does not scan any directory for background indexing.

## Building the extension

```powershell
cd VSCodeExtension
npm ci
npm run compile
npx @vscode/vsce package --target win32-x64
```

## Current limitations

- Only Windows packages are currently available.
- Linux builds are still in development.
- The project is under active development, so incomplete behavior and breaking changes should be expected.

## License

glsld is licensed under the GNU General Public License v3.0 only (`GPL-3.0-only`), and the extension is licensed under the MIT License.
