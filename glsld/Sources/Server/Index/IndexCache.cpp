#include "stdafx.h"
#include "IndexCache.hpp"

#include <fstream>
#include <system_error>

#include <nlohmann/json.hpp>
#include "Base/FileSystem/Source.hpp"
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
                .line   = json.at("line").get<std::uint64_t>(),
                .column = json.at("column").get<std::uint64_t>()
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
            DiskIndexRecord record;

            record.owner_uri    = json.at("owner").get<std::string>();
            record.dependencies = json.at("dependencies").get<std::vector<std::string>>();

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
    }

    std::optional<DiskIndexSnapshot> Load(const std::filesystem::path& filename, std::string_view expected_cache_key) {
        auto binary = LoadBinary(filename);
        if (!binary.has_value()) {
            GLSLD_LOG_WARN(GLSLD_LOG_ROOT(), "Failed to load global index cache {}: {}",
                           filename.generic_string(), binary.error());
            return std::nullopt;
        }

        auto json = nlohmann::json::from_msgpack(*binary);

        DiskIndexSnapshot snapshot{
            .schema_version = json.at("schemaVersion").get<std::uint32_t>(),
            .cache_key      = json.at("cacheKey").get<std::string>()
        };

        if (snapshot.schema_version != kSchemaVersion || snapshot.cache_key != expected_cache_key) {
            return std::nullopt;
        }

        for (const auto& record_json : json.at("records")) {
            snapshot.records.push_back(DeserializeRecord(record_json));
        }

        return snapshot;
    }

    bool Save(const std::filesystem::path& filename, const DiskIndexSnapshot& snapshot) {
        try {
            nlohmann::json json{
                { "schemaVersion", snapshot.schema_version },
                { "cacheKey",      snapshot.cache_key },
                { "records",       nlohmann::json::array() }
            };

            for (const auto& record : snapshot.records) {
                json["records"].push_back(SerializeRecord(record));
            }

            auto bytes = nlohmann::json::to_msgpack(json);

            std::error_code ec;
            std::filesystem::create_directories(filename.parent_path(), ec);

            auto temporary = filename;
            temporary += ".temp";

            std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
            if (!stream.is_open()) {
                return false;
            }

            stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            stream.flush();
            stream.close();

            if (!stream) {
                return false;
            }

            ec.clear();
            std::filesystem::remove(filename, ec);
            if (ec) {
                std::filesystem::remove(temporary, ec);
                return false;
            }

            ec.clear();
            std::filesystem::rename(temporary, filename, ec);
            if (ec) {
                std::filesystem::remove(temporary, ec);
                return false;
            }

            return true;
        } catch (const std::exception& e) {
            GLSLD_LOG_WARN(GLSLD_LOG_ROOT(), "Failed to save global index cache {}: {}",
                           filename.generic_string(), e.what());
            return false;
        }
    }

    std::optional<IndexedFileStamp> CaptureStamp(std::string_view uri) {
        auto filename = utils::UriToPath(uri);
        std::error_code ec;

        auto size = std::filesystem::file_size(filename, ec);
        if (ec) {
            return std::nullopt;
        }

        auto write_time = std::filesystem::last_write_time(filename, ec);
        if (ec) {
            return std::nullopt;
        }

        return IndexedFileStamp{
            .uri        = std::string(uri),
            .size       = size,
            .write_time = static_cast<std::int64_t>(write_time.time_since_epoch().count())
        };
    }

    bool IsFresh(const DiskIndexRecord& record) {
        if (record.stamps.empty()) {
            return false;
        }

        for (const auto& expected : record.stamps) {
            auto actual = CaptureStamp(expected.uri);
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
