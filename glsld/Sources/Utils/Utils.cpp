#include "stdafx.h"
#include "Utils.hpp"

#include <cstddef>
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

        std::size_t start_column = token.location.column;
        std::size_t end_column   = start_column + token.text.length();

        // [start_column, end_column]
        return position.column >= start_column && position.column <= end_column;
    }

    bool IsPositionInFunctionName(const SymbolInfo* symbol, SourceLocation position) {
        if (symbol->location.line != position.line) {
            return false;
        }

        // __Impl_main(void) -> main
        std::string_view raw_name = symbol->name;
        if (raw_name.starts_with("__Decl_") || raw_name.starts_with("__Impl_")) {
            raw_name = raw_name.substr(7);
            auto paren = raw_name.find('(');
            if (paren != std::string_view::npos) {
                raw_name = raw_name.substr(0, paren);
            }
        }

        std::size_t start_column = symbol->location.column;
        std::size_t end_column   = start_column + raw_name.length();

        return position.column >= start_column && position.column <= end_column;
    }
}
