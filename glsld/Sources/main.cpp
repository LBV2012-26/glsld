#include "stdafx.h"

#include <cstddef>
#include <chrono>
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
        std::ifstream shader_file("Tests/ScopeTest.glsl", std::ios::ate | std::ios::binary);
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

        auto lexer_start = std::chrono::high_resolution_clock::now();
        Parser parser(shader_source, symbols, 0, nullptr);
        auto lexer_end = std::chrono::high_resolution_clock::now();
        auto lexer_duration = lexer_end - lexer_start;

        auto parse_start = std::chrono::high_resolution_clock::now();
        std::unique_ptr<TranslationUnitNode> root = nullptr;
        root = parser.Parse();
        auto parse_end = std::chrono::high_resolution_clock::now();
        auto parse_duration = parse_end - parse_start;

        SymbolLinker linker(symbols, bindings, 0, nullptr);
        auto link_start = std::chrono::high_resolution_clock::now();
        linker.Traverse(root.get());
        auto link_end = std::chrono::high_resolution_clock::now();
        auto link_duration = link_end - link_start;

        TypeResolver collector(symbols, bindings, 0, nullptr);
        auto collect_start = std::chrono::high_resolution_clock::now();
        collector.Traverse(root.get());
        auto collect_end = std::chrono::high_resolution_clock::now();
        auto collect_duration = collect_end - collect_start;

        OverloadResolver resolver(symbols, bindings, 0, nullptr);
        auto resolve_start = std::chrono::high_resolution_clock::now();
        resolver.Traverse(root.get());
        auto resolve_end = std::chrono::high_resolution_clock::now();
        auto resolve_duration = resolve_end - resolve_start;

        AstDumper dumper(0, nullptr);
        dumper.Traverse(root.get());

        symbols.Dump();

        std::println("Lexer time: {}ms, Parse time: {}ms, SymbolLink time: {}ms, TypeResolve time: {}ms, OverloadResolve time: {}ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(lexer_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(parse_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(link_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(collect_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(resolve_duration).count());
    }
}
