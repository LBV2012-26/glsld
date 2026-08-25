#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace glsld {
    struct StoredLocation {
        std::string   uri;
        std::uint32_t line{};
        std::uint32_t column{};
    };

    struct StoredContribution {
        StoredLocation definition;
        StoredLocation reference;
    };

    struct IndexedFileStamp {
        std::string  uri;
        std::size_t  size{};
        std::int64_t write_time{};
    };

    struct DiskIndexRecord {
        std::string                     owner_uri;
        std::vector<std::string>        dependencies;
        std::vector<IndexedFileStamp>   stamps;
        std::vector<StoredContribution> contributions;
    };

    struct DiskIndexSnapshot {
        std::uint32_t                schema_version{};
        std::string                  cache_key;
        std::vector<DiskIndexRecord> records;
    };

    namespace IndexCache {
        static constexpr std::uint32_t kSchemaVersion = 1;

        struct StagedRecord {
            std::filesystem::path temporary;
            std::filesystem::path target;
        };

        std::string RecordFilename(std::string_view owner_uri);

        std::optional<StagedRecord> StageRecord(
            const std::filesystem::path& cache_path,
            std::string_view cache_key,
            const DiskIndexRecord& record,
            std::uint64_t revision);

        bool PublishRecord(const StagedRecord& staged);
        void DiscardRecord(const StagedRecord& staged);

        using RecordVisitor = std::function<bool(const DiskIndexRecord&)>;
        using KeepPredicate = std::function<bool(std::string_view)>;

        std::optional<DiskIndexRecord> LoadRecord(
            const std::filesystem::path& cache_path,
            std::string_view cache_key,
            std::string_view owner_uri);

        bool VisitRecords(
            const std::filesystem::path& cache_path,
            std::string_view cache_key,
            const RecordVisitor& visitor);

        std::vector<std::string> PruneRecords(
            const std::filesystem::path& cache_path,
            std::string_view cache_key,
            const KeepPredicate& keep);

        void RemoveRecord(
            const std::filesystem::path& cache_path,
            std::string_view owner_uri);

        std::optional<IndexedFileStamp> CaptureStamp(std::string_view uri);
        bool IsLatest(const DiskIndexRecord& record);
    }
}
