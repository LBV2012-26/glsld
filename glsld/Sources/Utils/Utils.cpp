#include "stdafx.h"
#include "Utils.hpp"

#include <array>
#include <filesystem>
#include <Windows.h>

namespace glsld::utils {
    std::string GetFilePath(std::string_view filename) {
        std::array<wchar_t, MAX_PATH> buffer{};
        GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

        auto work_directory = std::filesystem::path(buffer.data()).parent_path().parent_path();
        auto file_path = work_directory / std::filesystem::path(filename);

        return file_path.string();
    }

    void PrintIndent(int level) {
        for (int i = 0; i < level; ++i) {
            std::print("  "); // 2 spaces per indent level
        }
    }
}
