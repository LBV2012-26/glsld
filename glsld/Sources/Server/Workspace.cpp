#include "pch.hpp"
#include "Workspace.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <future>
#include <ranges>
#include <system_error>
#include <utility>

#include "Analyzer/Passes/MacroBinder.hpp"
#include "Analyzer/Passes/SymbolLinker.hpp"
#include "Analyzer/Passes/TypeResolver.hpp"
#include "Analyzer/Syntax/Lexer.hpp"
#include "Analyzer/Syntax/MetadataManager.hpp"
#include "Analyzer/Syntax/Parser.hpp"
#include "Base/Arena.hpp"
#include "Base/Logger.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    void Workspace::UpdateDocument(
        std::string_view uri,
        std::string_view source,
        int version_replica,
        VersionPointer version_pointer)
    {
        auto document = std::make_shared<Document>();
        document->source  = source;
        document->version = version_replica;

        const auto* source_file = source_table_.InternByUri(uri);

        const auto process_start = std::chrono::high_resolution_clock::now();;
        ProcessSource(*document, source_file, document->source, version_replica, version_pointer);
        const auto process_end = std::chrono::high_resolution_clock::now();

        GLSLD_LOG(info, "Processed document {} in {} ms (replica: {}, current: {})",
                  uri, std::chrono::duration_cast<std::chrono::milliseconds>(process_end - process_start).count(),
                  version_replica, version_pointer != nullptr ? version_pointer->load() : 0);

        if (document->ast == nullptr) {
            return;
        }

        UpdateDependencies(uri, document);

        const auto index_update_start = std::chrono::high_resolution_clock::now();
        {
            std::lock_guard lock(index_mutex_);

            global_index_.IndexDocument(uri, *document);
            type_member_index_.RemoveDocument(uri);
            type_member_index_.IndexDocument(uri, document->symbols);
        }
        const auto index_update_end = std::chrono::high_resolution_clock::now();

        GLSLD_LOG(info, "Indexed document (with lock) {} in {} ms",
                  uri, std::chrono::duration_cast<std::chrono::milliseconds>(index_update_end - index_update_start).count());

        const auto reconcile_start = std::chrono::high_resolution_clock::now();
        {
            std::lock_guard lock(document_mutex_);
            documents_.insert_or_assign(uri, std::move(document));
        }
        const auto reconcile_end = std::chrono::high_resolution_clock::now();

        GLSLD_LOG(info, "Reconciled document (with lock) {} in {} ms",
                  uri, std::chrono::duration_cast<std::chrono::milliseconds>(reconcile_end - reconcile_start).count());
    }

    std::shared_ptr<Document> Workspace::GetDocumentSnapshot(std::string_view uri) const {
        std::shared_lock lock(document_mutex_);

        auto it = documents_.find(uri);
        if (it != documents_.end()) {
            return it->second;
        }

        return nullptr;
    }

    std::vector<std::string> Workspace::GetAffectedDocuments(std::string_view changed_uri) const {
        std::vector<std::string> results;
        auto it = reverse_dependencies_.find(changed_uri);
        if (it != reverse_dependencies_.end()) {
            results.assign(it->second.begin(), it->second.end());
        }

        return results;
    }

    void Workspace::ApplyVariants(std::optional<ActiveVariant> shared, StringHeteroHashMap<ActiveVariant> unique) {
        std::lock_guard lock(variant_mutex_);
        shared_variant_  = std::move(shared);
        active_variants_ = std::move(unique);
    }

    void Workspace::ChangeVariant(VariantType type, ActiveVariant variant, std::string_view uri) {
        if (type == VariantType::kShared) {
            {
                std::lock_guard lock(variant_mutex_);
                shared_variant_ = std::move(variant);
            }

            {
                std::lock_guard lock(background_index_mutex_);
                disk_index_records_.clear();
                background_cache_dirty_ = true;
            }

            for (const auto& filename : DiscoverIndexCandidates()) {
                ScheduleDiskIndex(filename);
            }

            return;
        }

        const auto filename       = Utils::UriToPath(uri);
        const auto normalized_uri = Utils::PathToUri(filename);

        {
            std::lock_guard lock(variant_mutex_);
            active_variants_[normalized_uri] = std::move(variant);
        }

        {
            std::lock_guard lock(background_index_mutex_);
            disk_index_records_.erase(normalized_uri);
            background_cache_dirty_ = true;
        }

        ScheduleDiskIndex(filename);
    }

    void Workspace::RemoveVariant(VariantType type, std::string_view uri) {
        if (type == VariantType::kShared) {
            {
                std::lock_guard lock(variant_mutex_);
                shared_variant_.reset();
            }

            {
                std::lock_guard lock(background_index_mutex_);
                disk_index_records_.clear();
                background_cache_dirty_ = true;
            }

            for (const auto& filename : DiscoverIndexCandidates()) {
                ScheduleDiskIndex(filename);
            }

            return;
        }

        const auto filename       = Utils::UriToPath(uri);
        const auto normalized_uri = Utils::PathToUri(filename);

        {
            std::lock_guard lock(variant_mutex_);
            active_variants_.erase(normalized_uri);
        }

        {
            std::lock_guard lock(background_index_mutex_);
            disk_index_records_.erase(normalized_uri);
            background_cache_dirty_ = true;
        }

        ScheduleDiskIndex(filename);
    }

    void Workspace::StartBackgroundIndex(
        std::vector<std::filesystem::path> roots,
        std::filesystem::path cache_path,
        std::string cache_key)
    {
        StopBackgroundIndex();

        {
            std::lock_guard lock(background_index_mutex_);

            index_roots_      = std::move(roots);
            index_cache_path_ = std::move(cache_path);
            index_cache_key_  = std::move(cache_key);
        }

        background_index_thread_ = std::jthread([this](std::stop_token stop_token) -> void {
            BackgroundIndexLoop(stop_token);
        });
    }

    void Workspace::StopBackgroundIndex() {
        if (!background_index_thread_.joinable()) {
            return;
        }

        background_index_thread_.request_stop();
        background_index_condition_.notify_all();
        background_index_thread_.join();
    }

    void Workspace::MarkDocumentOpen(std::string_view uri) {
        std::lock_guard lock(background_index_mutex_);

        open_document_uris_.emplace(uri);
        ++index_revisions_[uri];
    }

    void Workspace::CloseDocument(std::string_view uri) {
        {
            std::lock_guard lock(index_mutex_);
            type_member_index_.RemoveDocument(uri);
        }

        {
            std::lock_guard lock(document_mutex_);
            documents_.erase(uri);
        }

        {
            std::lock_guard lock(background_index_mutex_);
            open_document_uris_.erase(uri);
            ++index_revisions_[uri];
        }

        ScheduleDiskIndexByUri(uri);
    }

    void Workspace::ScheduleDiskIndex(const std::filesystem::path& filename) {
        auto normalized = Utils::NormalizePath(filename);
        auto uri        = Utils::PathToUri(normalized);

        {
            std::lock_guard lock(background_index_mutex_);

            if (open_document_uris_.contains(uri)) {
                return;
            }

            ++index_revisions_[uri];

            if (queued_disk_uris_.emplace(uri).second) {
                index_task_queue_.push(DiskIndexTask{
                    .uri      = std::move(uri),
                    .filename = std::move(normalized)
                });
            }
        }

        background_index_condition_.notify_one();
    }

    void Workspace::ProcessSource(
        Document& document,
        const SourceFile* source_file,
        std::string_view source,
        int version_replica,
        VersionPointer version_pointer)
    {
        Lexer lexer(source_file, source, include_loader_, include_dirs_);
        auto raw_tokens = lexer.Tokenize(version_replica, version_pointer);
        if (raw_tokens.empty()) {
            return;
        }

        std::string shader_stage;
        auto config_it = shader_configs_.find(source_file->uri());
        if (config_it != shader_configs_.end()) {
            shader_stage = config_it->second.shader_stage.value_or("");
        } else {
            auto extension = std::filesystem::path(source_file->filename()).extension().generic_string();
            if (!extension.empty() && extension.starts_with('.')) {
                shader_stage = extension.substr(1);
            }
        }

        MetadataManager::GetInstance().AttachBuiltinMetadata(document, shader_stage, raw_tokens, include_dirs_);

        {
            std::shared_lock lock(variant_mutex_);
            auto variant_it = active_variants_.find(source_file->uri());
            if (variant_it != active_variants_.end()) {
                for (const auto& macro : variant_it->second.macros) {
                    InjectVariantMacro(document, source_file, macro);
                }
            }

            if (shared_variant_.has_value()) {
                for (const auto& macro : shared_variant_->macros) {
                    InjectVariantMacro(document, source_file, macro);
                }
            }
        }

        Parser parser(document, source_table_, source_file, std::move(raw_tokens), include_loader_, include_dirs_, version_replica, version_pointer);
        if (document.ast == nullptr) { // 如果版本更改，会返回 nullptr
            if (version_pointer != nullptr) {
                GLSLD_LOG(debug, "Version changed during document update (replica: {}, current: {}), cancelling parse.",
                version_replica, version_pointer->load());
            }

            return;
        }

        auto Cancelled = [version_replica, &version_pointer]() -> bool {
            if (version_pointer != nullptr && version_replica != version_pointer->load()) {
                GLSLD_LOG(debug, "Version changed during document update (replica: {}, current: {}), cancelling update.",
                          version_replica, version_pointer->load());
                return true;
            }

            return false;
        };

        document.FinalizeInjectedMacros(source_file);

        if (Cancelled()) return;
        SymbolLinker linker(document, version_replica, version_pointer);

        if (Cancelled()) return;
        TypeResolver resolver(document, version_replica, version_pointer);

        if (Cancelled()) return;
        MacroBinder binder(document, version_replica, version_pointer);
    }

    void Workspace::InjectVariantMacro(Document& document, const SourceFile* source_file, const ActiveMacro& macro) {
        auto name        = document.StoreTokenText(macro.name);
        auto replacement = document.StoreTokenText(macro.replacement);

        Lexer lexer(source_file, replacement, include_loader_, include_dirs_);
        auto tokens = lexer.Tokenize();

        if (!tokens.empty() && tokens.back().type == TokenType::kEndOfFile) {
            tokens.pop_back();
        }

        const SourceLocation location(source_file, 0, 0);
        for (auto& token : tokens) {
            token.location = location;
        }

        document.InjectMacro(MacroDefinition{
            .is_function = false,
            .original_token = Token{
                .text     = name,
                .location = location,
                .type     = TokenType::kIdentifier
            },
            .replacement_list = std::move(tokens)
        });
    }

    void Workspace::UnregisterDependencies(std::string_view uri) {
        auto forward_it = forward_dependencies_.find(uri);
        if (forward_it == forward_dependencies_.end()) {
            return;
        }

        for (const auto& dependency : forward_it->second) {
            auto reverse_it = reverse_dependencies_.find(dependency);
            if (reverse_it == reverse_dependencies_.end()) {
                continue;
            }

            reverse_it->second.erase(uri);
            if (reverse_it->second.empty()) {
                reverse_dependencies_.erase(reverse_it);
            }
        }
    }

    void Workspace::UpdateDependencies(std::string_view uri, std::span<const std::string> dependencies) {
        std::lock_guard lock(dependency_mutex_);

        UnregisterDependencies(uri);

        forward_dependencies_[std::string(uri)] = std::ranges::to<std::vector<std::string>>(dependencies);
        for (const auto& dependency : dependencies) {
            reverse_dependencies_[dependency].emplace(uri);
        }
    }

    void Workspace::UpdateDependencies(std::string_view uri, std::shared_ptr<const Document> document) {
        UpdateDependencies(uri, document->dependencies);
    }

    void Workspace::RemoveDependencies(std::string_view uri) {
        std::lock_guard lock(dependency_mutex_);

        UnregisterDependencies(uri);
        forward_dependencies_.erase(uri);
    }

    void Workspace::BackgroundIndexLoop(std::stop_token stop_token) {
        using namespace std::chrono_literals;

        LoadBackgroundCache();
        ReconcileWorkspace();

        auto next_reconcile = std::chrono::steady_clock::now() + 5s;

        while (!stop_token.stop_requested()) {
            std::vector<std::future<void>> futures;
            {
                std::unique_lock lock(background_index_mutex_);
                background_index_condition_.wait_until(lock, stop_token, next_reconcile, [this]() -> bool {
                    return !index_task_queue_.empty();
                });

                if (stop_token.stop_requested()) {
                    break;
                }

                while (!index_task_queue_.empty()) {
                    auto index_task = std::move(index_task_queue_.front());
                    index_task_queue_.pop();
                    queued_disk_uris_.erase(index_task.uri);

                    const auto revision = index_revisions_[index_task.uri];
                    futures.push_back(background_index_pool_.Submit([this, index_task = std::move(index_task), revision]() mutable -> void {
                        try {
                            ProcessDiskIndexTask(index_task.uri, index_task.filename, revision);
                        } catch (const std::exception& e) {
                            GLSLD_LOG(warn, "Background index task failed for {}: {}",
                                      index_task.filename.generic_string(), e.what());
                        } catch (...) {
                            GLSLD_LOG(warn, "Background index task failed for {} with an unknown exception.",
                                      index_task.filename.generic_string());
                        }
                    }));
                }
            }

            for (auto& future : futures) {
                future.get();
            }

            if (stop_token.stop_requested()) {
                break;
            }

            const auto now = std::chrono::steady_clock::now();
            if (now >= next_reconcile) {
                ReconcileWorkspace();
                next_reconcile = now + 5s;
                continue;
            }

            bool has_pending_tasks = false;
            {
                std::lock_guard lock(background_index_mutex_);
                has_pending_tasks = !index_task_queue_.empty();
            }

            if (!has_pending_tasks) {
                FlushBackgroundCache();
            }
        }

        FlushBackgroundCache();
    }

    namespace {
        SourceLocation RestoreLocation(Workspace* workspace, const StoredLocation& location) {
            return SourceLocation(workspace->InternSource(location.uri), location.line, location.column);
        }
    }

    void Workspace::LoadBackgroundCache() {
        auto snapshot = IndexCache::Load(index_cache_path_, index_cache_key_);

        if (!snapshot.has_value()) {
            return;
        }

        for (auto& record : snapshot->records) {
            if (!IndexCache::IsLatest(record)) {
                continue;
            }

            std::vector<Contribution> contributions;
            contributions.reserve(record.contributions.size());

            for (const auto& stored : record.contributions) {
                contributions.push_back(Contribution{
                    .definition = RestoreLocation(this, stored.definition),
                    .reference  = RestoreLocation(this, stored.reference)
                });
            }

            {
                std::lock_guard lock(background_index_mutex_);

                if (open_document_uris_.contains(record.owner_uri)) {
                    continue;
                }

                global_index_.RestoreDocument(record.owner_uri, std::move(contributions));
                UpdateDependencies(record.owner_uri, record.dependencies);
                disk_index_records_.insert_or_assign(record.owner_uri, std::move(record));
            }
        }
    }

    void Workspace::ReconcileWorkspace() {
        const auto candidates = DiscoverIndexCandidates();

        StringHeteroHashSet candidate_uris;
        candidate_uris.reserve(candidates.size());

        for (const auto& filename : candidates) {
            const auto uri = Utils::PathToUri(filename);
            candidate_uris.emplace(uri);

            bool need_index = true;

            {
                std::lock_guard lock(background_index_mutex_);
                auto it = disk_index_records_.find(uri);
                if (it != disk_index_records_.end() && IndexCache::IsLatest(it->second)) {
                    need_index = false;
                }
            }

            if (need_index) {
                ScheduleDiskIndex(filename);
            }
        }

        std::vector<std::string> removed;

        {
            std::lock_guard lock(background_index_mutex_);
            for (const auto& [uri, _] : disk_index_records_) {
                if (!candidate_uris.contains(uri) && !open_document_uris_.contains(uri)) {
                    removed.push_back(uri);
                }
            }

            for (const auto& uri : removed) {
                ++index_revisions_[uri];
                disk_index_records_.erase(uri);
            }

            if (!removed.empty()) {
                background_cache_dirty_ = true;
            }
        }

        for (const auto& uri : removed) {
            global_index_.RemoveDocument(uri);
            RemoveDependencies(uri);
        }
    }

    namespace {
        StoredLocation StoreLocation(const SourceLocation& location) {
            return StoredLocation{
                .uri    = std::string(location.uri()),
                .line   = location.line(),
                .column = location.column()
            };
        }
    }

    void Workspace::ProcessDiskIndexTask(
        std::string_view uri,
        const std::filesystem::path& filename,
        std::uint64_t revision)
    {
        auto source = LoadSource(filename);
        if (!source.has_value()) {
            GLSLD_LOG(warn, "Failed to load source file for disk index: {}", source.error());
            return;
        }

        auto document = std::make_shared<Document>();
        document->source  = std::move(*source);
        document->version = 0;

        const auto* source_file = source_table_.InternByUri(uri);

        ProcessSource(*document, source_file, document->source, 0, nullptr);
        if (document->ast == nullptr) {
            return;
        }

        auto contributions = GlobalIndex::CollectContributions(*document);

        DiskIndexRecord record{
            .owner_uri    = std::string(uri),
            .dependencies = document->dependencies,
        };
        record.contributions.reserve(contributions.size());

        for (const auto& contribution : contributions) {
            record.contributions.push_back(StoredContribution{
                .definition = StoreLocation(contribution.definition),
                .reference  = StoreLocation(contribution.reference)
            });
        }

        std::vector<std::string> stamped_uris;
        stamped_uris.reserve(record.dependencies.size() + 1);
        stamped_uris.push_back(record.owner_uri);
        stamped_uris.append_range(record.dependencies);

        std::ranges::sort(stamped_uris);
        auto [first, last] = std::ranges::unique(stamped_uris);
        stamped_uris.erase(first, last);

        for (const auto& stamped_uri : stamped_uris) {
            auto stamp = IndexCache::CaptureStamp(stamped_uri);
            if (!stamp.has_value()) {
                record.stamps.clear();
                break;
            }

            record.stamps.push_back(std::move(*stamp));
        }

        {
            std::lock_guard lock(background_index_mutex_);

            auto it = index_revisions_.find(uri);
            if (it == index_revisions_.end() ||
                it->second != revision ||
                open_document_uris_.contains(uri))
            {
                GLSLD_LOG(warn, "Revision mismatch for disk index of {} (expected: {}, current: {}), skipping.",
                          uri, revision, it != index_revisions_.end() ? it->second : 0);
                return;
            }

            global_index_.RestoreDocument(uri, std::move(contributions));
            UpdateDependencies(uri, record.dependencies);

            if (!record.stamps.empty()) {
                disk_index_records_.insert_or_assign(uri, std::move(record));
                background_cache_dirty_ = true;
            }
        }
    }

    void Workspace::FlushBackgroundCache() {
        DiskIndexSnapshot snapshot;

        {
            std::lock_guard lock(background_index_mutex_);

            if (!background_cache_dirty_) {
                return;
            }

            snapshot.schema_version = IndexCache::kSchemaVersion;
            snapshot.cache_key      = index_cache_key_;
            snapshot.records.reserve(disk_index_records_.size());

            for (const auto& [_, record] : disk_index_records_) {
                snapshot.records.push_back(record);
            }

            background_cache_dirty_ = false;
        }

        if (IndexCache::Save(index_cache_path_, snapshot)) {
            return;
        }

        std::lock_guard lock(background_index_mutex_);
        background_cache_dirty_ = true;
    }

    std::vector<std::filesystem::path> Workspace::DiscoverIndexCandidates() const {
        std::vector<std::filesystem::path> result;

        for (const auto& root : index_roots_) {
            std::error_code ec;
            std::filesystem::recursive_directory_iterator it(
                root, std::filesystem::directory_options::skip_permission_denied, ec);

            const std::filesystem::recursive_directory_iterator end;

            while (it != end) {
                if (ec) {
                    ec.clear();
                    it.increment(ec);
                    continue;
                }

                const auto& entry = *it;
                if (entry.is_directory(ec)) {
                    auto name = entry.path().filename().generic_string();

                    if (name == ".git" || name == ".glsld" || name == ".vs" ||
                        name == "node_modules" || name == "vcpkg_installed")
                    {
                        it.disable_recursion_pending();
                    }
                } else if (entry.is_regular_file(ec) && IsIndexCandidate(entry.path())) {
                    result.push_back(Utils::NormalizePath(entry.path()));
                }

                ec.clear();
                it.increment(ec);
            }
        }

        return result;
    }

    bool Workspace::IsIndexCandidate(const std::filesystem::path& filename) const {
        static constexpr std::array<std::string_view, 16> kExtensions{
            ".glsl", ".vert", ".frag", ".geom", ".comp", ".tesc", ".tese", ".mesh",
            ".task", ".rgen", ".rchit", ".rahit", ".rmiss", ".rint", ".rcall"
        };

        const auto extension = filename.extension().generic_string();
        return std::ranges::contains(kExtensions, extension);
    }
}
