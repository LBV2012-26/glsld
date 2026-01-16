#include "stdafx.h"

#include <cstddef>
#include <fstream>
#include <string_view>
#include <utility>
#include <vector>

#include <fcntl.h>
#include "Windows.h"

#include "Analyzer/Parser.hpp"
#include "Base/Config.hpp"
#include "Base/Logger.hpp"
#include "Server/Server.hpp"
#include "Utils/Utils.hpp"

int main() {
    std::ignore = _setmode(_fileno(stdin), _O_BINARY);
    std::ignore = _setmode(_fileno(stdout), _O_BINARY);

    using namespace glsld;

    //MessageBox(nullptr, L"Debug Breakpoint", L"GLSL Analyzer", MB_OK);

    Config::LoadFromFile(utils::GetFilePath("Win64/config.yml"));
    LoggerManager::GetInstance().Initialize();

    VALKY_LOG_INFO(VALKY_LOG_ROOT(), "GLSL Analyzer started.");

    LspServer server;
    //server.Run();

    std::ifstream shader_file("Assets/Test.glsl", std::ios::ate | std::ios::binary);
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
    Parser parser(shader_source, symbols);

    auto root = parser.Parse();
    DumpAst(root.get());

    symbols.Dump();
}
