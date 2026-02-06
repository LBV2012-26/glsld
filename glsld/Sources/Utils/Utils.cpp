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

    bool IsPositionInToken(const Token& token, SourceLocation position) {
        if (token.location.line != position.line) {
            return false;
        }

        std::size_t start_col = token.location.column;
        std::size_t end_col   = start_col + token.text.length();

        // [start_col, end_col]
        return position.column >= start_col && position.column <= end_col;
    }
}
