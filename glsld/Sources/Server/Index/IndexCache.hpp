#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace glsld {
    struct StoredLocation {
        std::string uri;
        std::size_t line{};
        std::size_t column{};
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

        std::optional<DiskIndexSnapshot> Load(const std::filesystem::path& filename, std::string_view expected_cache_key);
        bool Save(const std::filesystem::path& filename, const DiskIndexSnapshot& snapshot);
        std::optional<IndexedFileStamp> CaptureStamp(std::string_view uri);
        bool IsFresh(const DiskIndexRecord& record);
    }
}
