#include "Workspace.hpp"

#include <utility>

namespace glsld {
    inline const SourceFile* Workspace::InternSource(std::string_view uri) {
        return source_table_.InternByUri(uri);
    }

    inline const SourceFile* Workspace::GetSource(std::string_view uri) const {
        return source_table_.GetByUri(uri);
    }

    inline void Workspace::InvalidateInclude(std::string_view uri) {
        auto filename = GetSource(uri)->filename();
        include_loader_.Invalidate(filename);
    }

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

    inline const StringHeteroHashMap<ExtraShaderConfig>& Workspace::shader_configs() const {
        return shader_configs_;
    }
}
