#include "pch.hpp"

#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <memory>
#include <print>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _WIN64
#include <fcntl.h>
#include <Windows.h>
#endif

#include "Analyzer/Ast/AstDumper.hpp"
#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Document.hpp"
#include "Analyzer/Syntax/Lexer.hpp"
#include "Analyzer/Syntax/MetadataManager.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/FileSystem/Source.hpp"
#include "Base/Arena.hpp"
#include "Base/Logger.hpp"
#include "Server/LspServer.hpp"
#include "Utils/Utils.hpp"

namespace {
    int Benchmark(glsld::ArenaPool& arena_pool, const std::filesystem::path& filename, bool dump_ast = false) {
        using namespace glsld;

        const auto normalized = Utils::GetFilePath(filename.generic_string());
        auto shader_source = *glsld::Utils::LoadSource(normalized);

        ThreadPool thread_pool;
        SourceTable source_table;
        IncludeLoader loader(source_table, thread_pool);
        Document document;
        document.arena = arena_pool.Acquire();;

        std::vector includes{
            std::filesystem::path("Z:/Source/Repos/glsld/glsld/Tests")
        };

        auto include_dirs = std::make_shared<std::vector<std::filesystem::path>>(std::move(includes));

        const auto* source_file = source_table.InternByUri(Utils::PathToUri(filename));

        Lexer lexer(source_file, shader_source, loader, include_dirs);
        auto lexer_start    = std::chrono::high_resolution_clock::now();
        auto raw_tokens     = lexer.Tokenize();
        auto lexer_end      = std::chrono::high_resolution_clock::now();
        auto lexer_duration = lexer_end - lexer_start;

        auto attach_start    = std::chrono::high_resolution_clock::now();
        MetadataManager::GetInstance().AttachBuiltinMetadata(document, normalized, raw_tokens, include_dirs);
        auto attach_end      = std::chrono::high_resolution_clock::now();
        auto attach_duration = attach_end - attach_start;

        auto parse_start    = std::chrono::high_resolution_clock::now();
        Parser parser(document, source_table, source_file, std::move(raw_tokens), loader, include_dirs, 0, nullptr);
        auto parse_end      = std::chrono::high_resolution_clock::now();
        auto parse_duration = parse_end - parse_start;

        auto link_start    = std::chrono::high_resolution_clock::now();
        SymbolLinker linker(document, 0, nullptr);
        auto link_end      = std::chrono::high_resolution_clock::now();
        auto link_duration = link_end - link_start;

        auto resolve_start    = std::chrono::high_resolution_clock::now();
        TypeResolver resolver(document, 0, nullptr);
        auto resolve_end      = std::chrono::high_resolution_clock::now();
        auto resolve_duration = resolve_end - resolve_start;

        auto bind_start    = std::chrono::high_resolution_clock::now();
        MacroBinder binder(document, 0, nullptr);
        auto bind_end      = std::chrono::high_resolution_clock::now();
        auto bind_duration = bind_end - bind_start;

        if (dump_ast) {
            AstDumper dumper(0, nullptr);
            dumper.Traverse(document.ast);
            document.symbols.Dump();
        }

        std::println("Benchmark completed for file: {}", filename.generic_string());
        std::println("Lex time: {}ms, Metadata attach time: {}ms, Parse time: {}ms, SymbolLink time: {}ms, TypeResolve time: {}ms, BindMacro time: {}ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(lexer_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(attach_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(parse_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(link_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(resolve_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(bind_duration).count());

        auto total_duration = lexer_duration + attach_duration + parse_duration + link_duration + resolve_duration + bind_duration;
        std::println("Total time: {}ms", std::chrono::duration_cast<std::chrono::milliseconds>(total_duration).count());

        return EXIT_SUCCESS;
    }
}

// #define BENCHMARK

int main() {
#ifdef _WIN64
    (void)_setmode(_fileno(stdin), _O_BINARY);
    (void)_setmode(_fileno(stdout), _O_BINARY);
#endif

    glsld::Logger::GetInstance();

#ifdef BENCHMARK
    glsld::ArenaPool arena_pool;

    std::println("-----------------------------------------------------------------------");
    Benchmark(arena_pool, "Tests/Benchmark/BlackHole.glsl.bak");
    Benchmark(arena_pool, "Tests/Benchmark/BlackHoleHeavy.glsl.bak");
    Benchmark(arena_pool, "Tests/OverloadTest.glsl");
    Benchmark(arena_pool, "Tests/HoverTest.glsl");
    Benchmark(arena_pool, "Tests/ExtensionTest.glsl");
    Benchmark(arena_pool, "Tests/MacroTest.glsl");
    std::println("-----------------------------------------------------------------------");

    return 0;
#endif

    glsld::LspServer server;
    server.Run();

    return EXIT_SUCCESS;
}
