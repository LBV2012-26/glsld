#include "stdafx.h"

#include <cstddef>
#include <fstream>
#include <string_view>
#include <utility>
#include <vector>

#include <fcntl.h>
#include "Windows.h"

#include "Analyzer/Ast/AstDumper.hpp"
#include "Analyzer/Passes/OverloadResolver.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Config.hpp"
#include "Base/Logger.hpp"
#include "Server/Server.hpp"
#include "Utils/Utils.hpp"

int main() {
    std::ignore = _setmode(_fileno(stdin), _O_BINARY);
    std::ignore = _setmode(_fileno(stdout), _O_BINARY);

    using namespace glsld;

    int result = MessageBox(nullptr, L"Run LSP", L"GLSL Analyzer", MB_OKCANCEL);
    if (result == IDOK) {
        Config::LoadFromFile(utils::GetFilePath("Win64/config.yml"));
        LoggerManager::GetInstance().Initialize();

        VALKY_LOG_INFO(VALKY_LOG_ROOT(), "GLSL Analyzer started.");

        LspServer server;
        server.Run();
    } else {
        std::ifstream shader_file("Tests/HoverTest.glsl", std::ios::ate | std::ios::binary);
        if (!shader_file.is_open()) {
            std::cerr << "Failed to open test GLSL source." << std::endl;
            return EXIT_FAILURE;
        }

        auto size = shader_file.tellg();
        shader_file.seekg(0);

        std::vector<char> source_buffer(size);
        shader_file.read(source_buffer.data(), size);

        std::string_view shader_source(source_buffer);

        DocumentSymbols symbols;
        BindingMap bindings;
        Parser parser(shader_source, symbols);
        auto root = parser.Parse();

        SymbolLinker linker(symbols, bindings);
        linker.Traverse(root.get());

        TypeResolver collector(symbols, bindings);
        collector.Traverse(root.get());

        OverloadResolver resolver(symbols, bindings);
        resolver.Traverse(root.get());

        AstDumper dumper;
        dumper.Traverse(root.get());
        //DumpAst(root.get());

        symbols.Dump();
    }
}
