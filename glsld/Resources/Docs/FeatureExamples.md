# Feature examples

[Installation and setup](../VSCodeExtension/README.md)

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

Format GLSL source with `clang-format`. Set `glsld.clangFormat.path` if it is not on your PATH.

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

Rename symbols across the workspace.

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

Manage global and per-file preprocessor macros from the glsld sidebar.

![Preprocessor macro configuration](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/MacroVariant.gif)

## SPIR-V compilation

Configure shader compile groups and build SPIR-V with `glslc` from the glsld sidebar.

![Compile groups](https://raw.githubusercontent.com/LBV2012-26/glsld/main/glsld/Resources/ThumbNails/CompileGroups.png)
