#include "pch.hpp"
#include "IndexCache.hpp"

#include <exception>
#include <format>
#include <fstream>
#include <ios>
#include <system_error>

#include <nlohmann/json.hpp>

#include "Base/FileSystem/Source.hpp"
#include "Base/Hash.hpp"
#include "Base/Logger.hpp"
#include "Utils/Utils.hpp"

namespace glsld::IndexCache {
    namespace {
        nlohmann::json SerializeLocation(const StoredLocation& location) {
            return {
                { "uri",    location.uri },
                { "line",   location.line },
                { "column", location.column }
            };
        }

        StoredLocation DeserializeLocation(const nlohmann::json& json) {
            return {
                .uri    = json.at("uri").get<std::string>(),
                .line   = json.at("line").get<std::uint32_t>(),
                .column = json.at("column").get<std::uint32_t>()
            };
        }

        nlohmann::json SerializeRecord(const DiskIndexRecord& record) {
            nlohmann::json json;

            json["owner"]         = record.owner_uri;
            json["dependencies"]  = record.dependencies;
            json["stamps"]        = nlohmann::json::array();
            json["contributions"] = nlohmann::json::array();

            for (const auto& stamp : record.stamps) {
                json["stamps"].push_back({
                    { "uri",        stamp.uri },
                    { "size",       stamp.size },
                    { "writeTime",  stamp.write_time }
                });
            }

            for (const auto& contribution : record.contributions) {
                json["contributions"].push_back({
                    {
                        "definition",
                        SerializeLocation(contribution.definition)
                    },
                    {
                        "reference",
                        SerializeLocation(contribution.reference)
                    }
                });
            }

            return json;
        }

        DiskIndexRecord DeserializeRecord(const nlohmann::json& json) {
            DiskIndexRecord record{
                .owner_uri    = json.at("owner").get<std::string>(),
                .dependencies = json.at("dependencies").get<std::vector<std::string>>()
            };

            for (const auto& stamp_json : json.at("stamps")) {
                record.stamps.push_back(IndexedFileStamp{
                    .uri        = stamp_json.at("uri").get<std::string>(),
                    .size       = stamp_json.at("size").get<std::uint64_t>(),
                    .write_time = stamp_json.at("writeTime").get<std::int64_t>()
                });
            }

            for (const auto& contribution_json : json.at("contributions")) {
                record.contributions.push_back(StoredContribution{
                    .definition = DeserializeLocation(contribution_json.at("definition")),
                    .reference  = DeserializeLocation(contribution_json.at("reference"))
                });
            }

            return record;
        }

        std::filesystem::path RecordDirectory(const std::filesystem::path& cache_path) {
            auto result = cache_path;
            result += "Indexes";
            return result;
        }

        std::filesystem::path RecordPath(
            const std::filesystem::path& cache_path,
            std::string_view owner_uri)
        {
            const auto hash = rapidhashMicro(owner_uri.data(), owner_uri.size());
            return RecordDirectory(cache_path) / std::format("{:016x}.idx", hash);
        }
    }

    std::optional<StagedRecord> StageRecord(
        const std::filesystem::path& cache_path,
        std::string_view cache_key,
        const DiskIndexRecord& record,
        std::uint64_t revision)
    {
        try {
            auto target = RecordPath(cache_path, record.owner_uri);

            std::error_code ec;
            std::filesystem::create_directories(target.parent_path(), ec);
            if (ec) {
                return std::nullopt;
            }

            nlohmann::json json{
                { "schemaVersion", kSchemaVersion },
                { "cacheKey",      cache_key },
                { "record",        SerializeRecord(record) }
            };

            const auto bytes = nlohmann::json::to_msgpack(json);

            auto temporary = target;
            temporary += std::format(".tmp.{}", revision);

            std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
            if (!stream.is_open()) {
                return std::nullopt;
            }

            stream.write(reinterpret_cast<const char*>(bytes.data()),
                         static_cast<std::streamsize>(bytes.size()));
            stream.flush();
            stream.close();

            if (!stream) {
                std::filesystem::remove(temporary, ec);
                return std::nullopt;
            }

            return StagedRecord{
                .temporary = std::move(temporary),
                .target    = std::move(target)
            };
        } catch (const std::exception& e) {
            GLSLD_LOG(warn, "Failed to stage index record for {}: {}",
                      record.owner_uri, e.what());
            return std::nullopt;
        }
    }

    bool PublishRecord(const StagedRecord& staged) {
        std::error_code ec;
        std::filesystem::rename(staged.temporary, staged.target, ec);
        return !ec;
    }

    void DiscardRecord(const StagedRecord& staged) {
        std::filesystem::remove(staged.temporary);
    }

    namespace {
        std::optional<DiskIndexRecord> LoadRecordFile(
            const std::filesystem::path& filename,
            std::string_view expected_cache_key)
        {
            const auto binary = Utils::LoadBinary(filename);
            if (!binary.has_value()) {
                return std::nullopt;
            }

            try {
                const auto json = nlohmann::json::from_msgpack(*binary);

                if (json.at("schemaVersion").get<std::uint32_t>() != kSchemaVersion ||
                    json.at("cacheKey").get<std::string_view>() != expected_cache_key) {
                    return std::nullopt;
                }

                return DeserializeRecord(json.at("record"));
            } catch (const std::exception& e) {
                GLSLD_LOG(warn, "Failed to load index record from {}: {}",
                          filename.generic_string(), e.what());
                return std::nullopt;
            }
        }
    }

    std::optional<DiskIndexRecord> LoadRecord(
        const std::filesystem::path& cache_path,
        std::string_view cache_key,
        std::string_view owner_uri)
    {
        auto record = LoadRecordFile(RecordPath(cache_path, owner_uri), cache_key);
        if (!record.has_value() || record->owner_uri != owner_uri) {
            return std::nullopt;
        }

        return record;
    }

    bool VisitRecords(
        const std::filesystem::path& cache_path,
        std::string_view cache_key,
        const RecordVisitor& visitor)
    {
        std::error_code ec;
        const auto directory = RecordDirectory(cache_path);

        for (std::filesystem::directory_iterator it(directory, ec), end;
             !ec && it != end; it.increment(ec))
        {
            if (!it->is_regular_file(ec) || it->path().extension() != ".idx") {
                continue;
            }

            auto record = LoadRecordFile(it->path(), cache_key);
            if (!record.has_value()) {
                continue;
            }

            if (!visitor(*record)) {
                return false;
            }
        }

        return !ec;
    }

    std::vector<std::string> PruneRecords(
        const std::filesystem::path& cache_path,
        std::string_view cache_key,
        const KeepPredicate& keep)
    {
        std::error_code ec;
        const auto directory = RecordDirectory(cache_path);

        std::vector<std::string> removed;
        for (std::filesystem::directory_iterator it(directory, ec), end;
             !ec && it != end; it.increment(ec))
        {
            if (!it->is_regular_file(ec) || it->path().extension() != ".idx") {
                continue;
            }

            auto record = LoadRecordFile(it->path(), cache_key);
            if (!record.has_value() || !keep(record->owner_uri)) {
                std::error_code remove_ec;
                std::filesystem::remove(it->path(), remove_ec);

                if (!remove_ec && record.has_value()) {
                    removed.push_back(std::move(record->owner_uri));
                }
            }
        }

        return removed;
    }

    void RemoveRecord(
        const std::filesystem::path& cache_path,
        std::string_view owner_uri)
    {
        std::filesystem::remove(RecordPath(cache_path, owner_uri));
    }

    std::optional<IndexedFileStamp> CaptureStamp(std::string_view uri) {
        const auto filename = Utils::UriToPath(uri);
        std::error_code ec;

        const auto size = std::filesystem::file_size(filename, ec);
        if (ec) {
            return std::nullopt;
        }

        const auto write_time = std::filesystem::last_write_time(filename, ec);
        if (ec) {
            return std::nullopt;
        }

        return IndexedFileStamp{
            .uri        = std::string(uri),
            .size       = size,
            .write_time = static_cast<std::int64_t>(write_time.time_since_epoch().count())
        };
    }

    bool IsLatest(const DiskIndexRecord& record) {
        if (record.stamps.empty()) {
            return false;
        }

        for (const auto& expected : record.stamps) {
            const auto actual = CaptureStamp(expected.uri);
            if (!actual.has_value()) {
                return false;
            }

            if (actual->size != expected.size || actual->write_time != expected.write_time) {
                return false;
            }
        }

        return true;
    }
}
