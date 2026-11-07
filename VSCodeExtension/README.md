# glsld

glsld adds GLSL language support to Visual Studio Code, including completion, diagnostics, hover information, signature help, navigation, rename, formatting, etc.

- Find definitions and references across shader files with background indexing.
- Manage global and per-file preprocessor macros from the sidebar.
- Compile shader groups to SPIR-V with `glslc`.

[Feature examples and detailed documentation](https://github.com/LBV2012-26/glsld/blob/main/Docs/FeatureExamples.md)

## Getting started

Install the extension and accept the **Download** prompt to install the language server. You can also run **glsld: Download Language Server**, or set `glsld.server.path` to a local executable. Downloaded servers update automatically.

To enable workspace-wide navigation, configure the directories to index:

```json
{
    "glsld.backgroundIndex.roots": ["${workspaceFolder}"]
}
```

You can also add your custom shader extension names for indexing using `glsld.backgroundIndex.extraExtensions`.

Diagnostics and SPIR-V compilation require `glslc` from the Vulkan SDK or your PATH. Formatting requires `clang-format`; set `glsld.clangFormat.path` if needed. Other options are available in VS Code Settings under **glsld**.

The project is under active development. This extension is independent of [daiyousei-qz/glsld](https://github.com/daiyousei-qz/glsld).

## License

The VS Code extension is licensed under MIT. The glsld language server is licensed under GPL-3.0-only.
