# glsld

glsld is a language server and Visual Studio Code extension for GLSL. It provides preprocessing-aware completion, semantic analysis, navigation, diagnostics, formatting, background indexing, and SPIR-V compilation support for shader projects.

> [!WARNING]
> glsld is still in early development. Features, configuration, and protocol details may change between releases.

> [!NOTE]
> This extension is an independent project and is not affiliated with the other Visual Studio Code extension named `glsld` at https://github.com/daiyousei-qz/glsld. The name collision was discovered while preparing this extension for publication.

## Getting started

Install the extension, open a folder containing GLSL source files, and configure the directories that should be indexed:

```json
{
    "glsld.backgroundIndex.roots": ["${workspaceFolder}"]
}
```

Use `${workspaceFolder}/Shaders` to index a subdirectory instead. If the list is empty or absent, background indexing is disabled. The index cache is stored at `${workspaceFolder}/.glsld/BlobIndex.idx`.

The Windows extension package includes the language server at `bin/Win64/glsld.exe`. Leave `glsld.server.path` empty to use the bundled executable, or provide a custom executable path.

## Completion

glsld completes GLSL keywords, types, variables, functions, structure members, and other symbols available at the cursor.

![Code completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CodeCompletion.gif)

### Extension completion

Extension names and behaviors are completed inside `#extension` directives.

![Extension completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/ExtensionCompletion.gif)

### Include completion

Include paths are completed relative to the current file, workspace, and configured system include directories.

![Include completion](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/IncludeCompletion.gif)

### Function overloads

When a function has multiple signatures, glsld presents the available overloads with their parameter and return types.

![Function overloads](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/FunctionOverloads.png)

### Signature help

Signature help follows the active call, selects the matching overload, and highlights the parameter currently being entered.

![Signature help](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/SignatureHelp.gif)

## Formatting

glsld invokes `clang-format` to format GLSL source files. Formatting can be triggered manually or automatically on save. Need to configure the path to `clang-format` in `glsld.formatting.clangFormatPath` or install it separately.

![Formatting](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Formatting.gif)

## Diagnostics

Syntax and semantic errors are reported while editing. Diagnostics can be enabled or disabled with `glsld.diagnostics.enabled`.

![Diagnostics](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Diagnostics.png)

## Navigation

### Go to definition

Jump from a symbol use to its declaration or definition, including symbols declared in included files.

![Go to definition](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/GotoDefinition.gif)

### Go to include

Open an included shader directly from its `#include` directive.

![Go to include](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/GotoInclude.gif)

### Find references

Find symbol references across the workspace. Background indexing includes references from files that have not been opened in the editor.

![Find references](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Reference.png)

References are also tracked across include boundaries.

![Cross-file references](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CrossReference.png)

Also support rename symbols across the workspace.

![Rename symbol](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/Rename.gif)

## Hover information

Hovering over a symbol displays its declaration and relevant type information.

### Variables and types

![Hover information for a symbol](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/TypeHover.png)
![Hover information for a symbol](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/SymbolHover.png)

### Functions

![Hover information for a function](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/FunctionHover.png)

## Inlay hints

Inlay hints expose parameter names and other information that makes shader code easier to read. They are controlled by VS Code's editor inlay-hint settings.

![Inlay hints](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/InlayHints.png)

## Document symbols

The document outline lists declarations in the current shader and supports quick navigation through large files.

![Document symbols](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/DocumentSymbols.png)

## Preprocessor macros

Global and per-file macros are managed from the glsld sidebar. Each macro has an independent checkbox, and enabled per-file macros are applied on top of enabled global macros. Changes are sent to the language server immediately so affected documents can be reprocessed.

![Preprocessor macro configuration](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/MacroVariant.gif)

## SPIR-V compilation

Compile groups collect shaders using source patterns, command templates, output paths, macro variants, and concurrency settings. Groups can be compiled directly from the glsld sidebar, and their configuration is stored in `.glsld/config.json`.

![Compile groups](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CompileGroups.png)

## Configuration

User-controlled settings belong in `.vscode/settings.json`:

```json
{
    "glsld.backgroundIndex.roots": ["${workspaceFolder}/Shaders", "${workspaceFolder}/Include"],
    "glsld.diagnostics.enabled": true
}
```

Index paths may use `${workspaceFolder}` explicitly. System include directories, the language server executable, `glslc`, `clang-format`, mimalloc options, diagnostics, and language capabilities can also be configured through the Visual Studio Code Settings editor.

The extension-generated `.glsld/config.json` stores shader overrides, macro state, and compile groups. It is separate from ordinary Visual Studio Code settings and should normally be managed through the glsld sidebar.

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

glsld is licensed under the GNU General Public License v3.0 only (`GPL-3.0-only`). The Visual Studio Code extension is licensed under the MIT License.
