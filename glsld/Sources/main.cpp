#include "stdafx.h"

#include <cstddef>
#include <cstdio>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <utility>
#include <vector>

#include <mimalloc.h>
#include <fcntl.h>
#include "Windows.h"

#include "Analyzer/Ast/AstDumper.hpp"
#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Config.hpp"
#include "Base/Logger.hpp"
#include "Server/Server.hpp"
#include "Utils/Utils.hpp"

namespace {
    void InitLargetPage() {
#ifdef _WIN64
        HANDLE token = nullptr;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            TOKEN_PRIVILEGES token_privileges{};
            LUID luid{};

            if (LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
                token_privileges.PrivilegeCount           = 1;
                token_privileges.Privileges[0].Luid       = luid;
                token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(token, FALSE, &token_privileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
            }

            CloseHandle(token);
        }

        constexpr auto kMemorySize = 256uz * 1024 * 1024;
        LPVOID memory = nullptr;
        int try_alloc = 0;

        do {
            ++try_alloc;
            memory = VirtualAlloc(nullptr, kMemorySize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
        } while (memory == nullptr);

        mi_manage_os_memory(memory, kMemorySize, true, true, true, -1);
#endif
    }
}

extern "C" {
    void mainCRTStartup();

    void Main() {
        // InitLargetPage();
        mainCRTStartup();
    }
}

int main() {
#ifdef _WIN64
    std::ignore = _setmode(_fileno(stdin), _O_BINARY);
    std::ignore = _setmode(_fileno(stdout), _O_BINARY);
#endif

    using namespace glsld;

    int result = MessageBox(nullptr, L"Run LSP", L"GLSL Analyzer", MB_OKCANCEL);
    if (result == IDOK) {
        Config::LoadFromFile(utils::GetFilePath("Win64/glsld.yml"));
        LoggerManager::GetInstance().Initialize();

        GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "GLSL Analyzer started.");

        LspServer server;
        server.Run();
    } else {
        auto filename = "Tests/BlackHole.glsl";
        std::ifstream shader_file(filename, std::ios::ate | std::ios::binary);
        if (!shader_file.is_open()) {
            std::println(stderr, "Failed to open test GLSL source.");
            return EXIT_FAILURE;
        }

        auto size = std::filesystem::file_size(filename);
        std::vector<char> source_buffer(size);

        shader_file.seekg(0);
        shader_file.read(source_buffer.data(), size);

        std::string_view shader_source(source_buffer);

        ThreadPool thread_pool;
        SourceTable source_table;
        IncludeLoader loader(source_table, thread_pool);
        Document document;

        std::array include_dirs{
            std::filesystem::path("Z:/Source/Repos/glsld/glsld/Tests")
        };

        const auto* source_file = source_table.InternByUri("file:///Z:/Source/Repos/glsld/glsld/Tests/Debugger.glsl");

        auto parse_start = std::chrono::high_resolution_clock::now();
        Parser parser(source_table, source_file, shader_source, loader, include_dirs, 0, nullptr, document);
        auto parse_end = std::chrono::high_resolution_clock::now();
        auto parse_duration = parse_end - parse_start;

        auto link_start = std::chrono::high_resolution_clock::now();
        SymbolLinker linker(document, 0, nullptr);
        auto link_end = std::chrono::high_resolution_clock::now();
        auto link_duration = link_end - link_start;

        auto resolve_start = std::chrono::high_resolution_clock::now();
        TypeResolver resolver(document, 0, nullptr);
        auto resolve_end = std::chrono::high_resolution_clock::now();
        auto resolve_duration = resolve_end - resolve_start;

        auto bind_start = std::chrono::high_resolution_clock::now();
        MacroBinder binder(document, 0, nullptr);
        auto bind_end = std::chrono::high_resolution_clock::now();
        auto bind_duration = bind_end - bind_start;

        //AstDumper dumper(0, nullptr);
        //dumper.Traverse(document.ast.get());

        //document.symbols.Dump();

        std::println("Parse time: {}ms, SymbolLink time: {}ms, TypeResolve time: {}ms, BindMacro time: {}ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(parse_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(link_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(resolve_duration).count(),
                     std::chrono::duration_cast<std::chrono::milliseconds>(bind_duration).count());
        std::println("Total time: {}ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(parse_duration + link_duration + resolve_duration + bind_duration).count());
    }
}
