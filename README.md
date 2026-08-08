# glsld

> [!NOTE]
> This extension is an independent project and is not affiliated with another Visual Studio Code extension that also uses the name `glsld` (https://github.com/daiyousei-qz/glsld). I was the one who discovered the name collision only while preparing this extension for publication on the Visual Studio Code Marketplace.

glsld is a language server for GLSL, accompanied by a Visual Studio Code extension. It provides parsing, preprocessing, semantic analysis, cross-file navigation, diagnostics, and workspace-wide background indexing for shader projects.

## Features

- Context-aware code, include-path, and extension completion
- Function overload resolution and signature help
- Syntax and semantic diagnostics through `glslc`
- Hover information and inlay hints
- Go to definition and workspace-wide reference search
- Cross-file symbol discovery through include-aware background indexing
- Shared and per-file preprocessor variants
- Shader compile groups in the Visual Studio Code extension

Screenshots and extension-specific setup instructions are available in the [Visual Studio Code extension documentation](VSCodeExtension/README.md).

## Repository layout

| Path | Description |
| --- | --- |
| `glsld/Sources` | Native glsld language-server source code |
| `glsld/Database` | Built-in GLSL lexical and semantic database |
| `VSCodeExtension` | Visual Studio Code client extension |

## Building the server

Clone the repository together with its submodules:

```powershell
git clone --recursive https://github.com/LBV2012-26/glsld.git
```

### Windows Build

- Requirements: Visual Studio 2026 with C++ workload and vcpkg.

> [!NOTE]
> If you first install vcpkg, you must run `vcpkg integrate install` to make it available to Visual Studio.

Open `glsld.slnx` in Visual Studio, select the desired x64 configuration, and press F7 to build the `glsld` project. The project uses the vcpkg manifest at `glsld/vcpkg.json` for package dependencies.

The `glsld.exe` executable will be located in `glsld/Win64/glsld.exe`.

### Linux Build

- Requirements: CMake, Clang, lld and vcpkg.

> [!NOTE]
> This repo will cause GCC to fail to build due to a internal compiler error. So use Clang instead.
> Make sure VCPKG_ROOT is set to the PATH.

```bash
cd glsld
clang++ ./CMakeGenerator/Sources/main.cpp -o CMakeGen --std=c++26 && ./CMakeGen
```

The `glsld` executable will be located in `glsld/Linux/glsld`.

## Visual Studio Code extension

The extension source is located in `VSCodeExtension`. To install its dependencies and compile it:

```powershell
cd VSCodeExtension
npm ci
npm run compile
```

See the [extension README](VSCodeExtension/README.md) for configuration, features, and VSIX packaging instructions.

## Contributing

If you are considering contributing, thank you for your interest in glsld. However, this project is maintained in limited personal time, and I currently cannot allocate enough time to review external code contributions properly. For that reason, pull requests are temporarily disabled.

If you discover a bug or have an idea for an improvement, please [open an issue](https://github.com/LBV2012-26/glsld/issues) so it can be discussed first.

If you need custom behavior or project-specific functionality, you are very welcome to [fork this repository](https://github.com/LBV2012-26/glsld/fork) and adapt it to your own requirements.

## Licensing

This repository contains separately licensed components:

- The glsld language server is licensed under the GNU General Public License version 3 only (`GPL-3.0-only`).
- The Visual Studio Code extension in `VSCodeExtension` is licensed under the MIT License.

See [LICENSE.md](LICENSE.md) for the precise component boundaries and links to the complete license texts.
