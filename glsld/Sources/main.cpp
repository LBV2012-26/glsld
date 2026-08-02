#include "pch.hpp"

#include <fcntl.h>
#include <Windows.h>

#include "Base/Logger.hpp"
#include "Server/LspServer.hpp"

int main() {
#ifdef _WIN64
    std::ignore = _setmode(_fileno(stdin), _O_BINARY);
    std::ignore = _setmode(_fileno(stdout), _O_BINARY);
#endif

    glsld::Logger::GetInstance();
    GLSLD_LOG(info, "glsld started.");
    glsld::LspServer server;
    server.Run();
}
