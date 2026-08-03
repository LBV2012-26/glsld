# Licensing

This repository contains two separately licensed components. The license that applies to a file is determined by the component to which that file belongs.

## glsld server

The glsld language server, including its native source code and server resources outside `VSCodeExtension`, is licensed under the **GNU General Public License version 3 only** (`GPL-3.0-only`).

The complete license text is available in [LICENSE-GPL-3.0.txt](LICENSE-GPLv3.0.txt).

## Visual Studio Code extension

The Visual Studio Code extension contained in `VSCodeExtension` is licensed under the **MIT License** unless a file explicitly states otherwise.

The complete license text is available in [VSCodeExtension/LICENSE](VSCodeExtension/LICENSE).

The extension package bundles the glsld server executable for convenience. Bundling the server does not change the license of either component: the extension code remains MIT-licensed, while the bundled glsld server remains licensed under GPL-3.0-only. A copy of the server license is included in the extension package as `GLSLD-SERVER-LICENSE.txt`.

## Third-party software

Third-party dependencies and incorporated materials remain subject to their respective licenses and copyright notices.
