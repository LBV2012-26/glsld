#include "Workspace.hpp"

#include <utility>
#include "Utils/Utils.hpp"

namespace glsld {
    inline const SourceFile* Workspace::InternSource(std::string_view uri) {
        return source_table_.InternByUri(uri);
    }

    inline const SourceFile* Workspace::GetSource(std::string_view uri) const {
        return source_table_.GetByUri(uri);
    }

    inline void Workspace::InvalidateInclude(std::string_view uri) {
        const auto filename = GetSource(uri)->filename();
        include_loader_.Invalidate(filename);
    }

    inline void Workspace::AddIncludeDirectory(std::filesystem::path directory) {
        include_dirs_->push_back(std::move(directory));
    }

    inline void Workspace::RemoveIncludeDirectory(const std::filesystem::path& directory) {
        std::erase_if(*include_dirs_, [&directory](const auto& include_dir) {
            return include_dir == directory;
        });
    }

    inline void Workspace::AddExtraShaderConfig(std::string_view key, ExtraShaderConfig config) {
        shader_configs_[key] = std::move(config);
    }

    inline void Workspace::RemoveExtraShaderConfig(std::string_view key) {
        shader_configs_.erase(key);
    }

    inline void Workspace::ScheduleDiskIndexByUri(std::string_view uri) {
        ScheduleDiskIndex(Utils::UriToPath(uri));
    }

    inline void Workspace::set_include_dirs(IncludeDirectoryHandle include_dirs) {
        include_dirs_ = std::move(include_dirs);
    }

    inline IncludeDirectoryHandle Workspace::include_dirs() const {
        return include_dirs_;
    }

    inline const StringHeteroHashMap<ExtraShaderConfig>& Workspace::shader_configs() const {
        return shader_configs_;
    }

    inline const TypeMemberIndex& Workspace::type_member_index() const {
        return type_member_index_;
    }
}
