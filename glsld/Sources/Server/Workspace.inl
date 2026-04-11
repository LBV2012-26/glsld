#include "Workspace.hpp"

#include <utility>

namespace glsld {
    inline void Workspace::AddIncludeDirectory(std::filesystem::path directory) {
        include_dirs_.push_back(std::move(directory));
    }

    inline void Workspace::RemoveIncludeDirectory(const std::filesystem::path& directory) {
        std::erase_if(include_dirs_, [&directory](const auto& include_dir) {
            return include_dir == directory;
        });
    }

    inline void Workspace::set_include_dirs(std::vector<std::filesystem::path> include_dirs) {
        include_dirs_ = std::move(include_dirs);
    }

    inline std::span<const std::filesystem::path> Workspace::include_dirs() const {
        return include_dirs_;
    }
}
