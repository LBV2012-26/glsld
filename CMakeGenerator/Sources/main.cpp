#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ============================================================================
// Data structures
// ============================================================================
struct ClCompileSettings {
    std::vector<std::string> include_dirs;
    std::vector<std::string> defines;
    std::string pch_file;
    std::string additional_options;
    bool rtti_enabled{};
};

struct LinkSettings {
    std::vector<std::string> dependencies;
};

struct SourceFile {
    std::filesystem::path relative_path;
    bool is_pch_source{};
};

using VcpkgManifest = std::vector<std::string>;

struct DependencyInfo {
    std::string cmake_package;
    std::string cmake_target;
    bool        needs_find{};
};

struct ProjectPaths {
    std::filesystem::path solution_dir;
    std::filesystem::path project_dir;
    std::filesystem::path vcxproj_path;
};

// ============================================================================s
// Helpers
// ============================================================================
std::string ReadFile(const std::filesystem::path& filename) {
    std::ifstream stream(filename, std::ios::binary);

    auto size = std::filesystem::file_size(filename);
    std::string content;
    content.resize_and_overwrite(size, [&](char* buffer, std::size_t) -> std::size_t {
        stream.read(buffer, size);
        return static_cast<std::size_t>(stream.gcount());
    });

    return content;
}

void WriteFile(const std::filesystem::path& filename, std::string_view content) {
    std::ofstream stream(filename, std::ios::binary);
    if (!stream) {
        std::println(stderr, "ERROR: Cannot write {}",  filename.generic_string());
        std::exit(EXIT_FAILURE);
    }

    stream.write(content.data(), content.size());
}

std::string Trim(std::string_view text) {
    auto IsSpace = [](unsigned char ch) -> bool {
        return std::isspace(ch);
    };

    return text
        | std::views::drop_while(IsSpace)
        | std::views::reverse
        | std::views::drop_while(IsSpace)
        | std::views::reverse
        | std::ranges::to<std::string>();
}

std::string ReplaceAll(std::string_view text, std::string_view from, std::string_view to) {
    if (from.empty()) {
        return std::string(text);
    }

    return text | std::views::split(from) | std::views::join_with(to) | std::ranges::to<std::string>();
}

std::string ExpandMacros(std::string_view s, const std::filesystem::path& project_dir, const std::filesystem::path& solution_dir) {
    std::string result = ReplaceAll(s, "$(ProjectDir)", (project_dir / "").generic_string());
    result = ReplaceAll(result, "$(SolutionDir)", (solution_dir / "").generic_string());

    if (auto pos = result.find("%("); pos != std::string::npos) {
        return Trim(result.substr(0, pos));
    }
    return result;
}

std::vector<std::string> Split(std::string_view text, char delimiter) {
    return text
        | std::views::split(delimiter)
        | std::ranges::to<std::vector<std::string>>();
}

std::optional<ProjectPaths> LocateProjectStructure() {
    auto current = std::filesystem::current_path();

    while (true) {
        bool has_sln = std::filesystem::exists(current / "glsld.slnx");

        if (has_sln) {
            std::filesystem::path glsld_dir = current / "glsld";
            std::filesystem::path vcxproj = glsld_dir / "glsld.vcxproj";

            if (std::filesystem::exists(vcxproj)) {
                return ProjectPaths{
                    .solution_dir = current,
                    .project_dir  = glsld_dir,
                    .vcxproj_path = vcxproj
                };
            }
        }

        auto parent = current.parent_path();
        if (parent == current) {
            break;
        }

        current = std::move(parent);
    }

    return std::nullopt;
}

// ============================================================================
// Step 1: Discover .cpp files
// ============================================================================
std::vector<SourceFile> DiscoverSources(const std::filesystem::path& sources_dir) {
    std::vector<SourceFile> result;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(sources_dir)) {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() != ".cpp")
            continue;

        SourceFile file;
        file.relative_path = std::filesystem::relative(entry.path(), sources_dir);
        if (file.relative_path.filename() == "pch.cpp") {
            file.is_pch_source = true;
        }
        result.push_back(std::move(file));
    }

    std::ranges::sort(result, [](const auto& lhs, const auto& rhs) -> bool {
        auto is_lhs_pch = lhs.is_pch_source ? 0 : 1;
        auto is_rhs_pch = lhs.is_pch_source ? 0 : 1;
        if (is_lhs_pch != is_rhs_pch) {
            return is_lhs_pch < is_rhs_pch;
        }

        return lhs.relative_path.generic_string() < rhs.relative_path.generic_string();
    });

    return result;
}

// ============================================================================
// Step 2: Parse vcxproj
// ============================================================================

std::string ExtractXmlElement(std::string_view xml, std::string_view tag) {
    auto open_tag  = std::format("<{}>", tag);
    auto close_tag = std::format("</{}>", tag);

    auto start = xml.find(open_tag);
    if (start == std::string::npos) {
        return {};
    }

    start += open_tag.size();

    auto end = xml.find(close_tag, start);
    if (end == std::string::npos) {
        return {};
    }

    return std::string(xml.substr(start, end - start));
}

std::string FindReleaseX64Block(std::string_view xml) {
    std::string_view open_tag_prefix = "<ItemDefinitionGroup";
    std::string_view marker = R"(Condition="'$(Configuration)|$(Platform)'=='Release|x64'")";

    auto pos = 0uz;
    while ((pos = xml.find(open_tag_prefix, pos)) != std::string_view::npos) {
        auto tag_end = xml.find('>', pos);
        if (tag_end == std::string_view::npos) {
            break;
        }

        std::string_view tag_content = xml.substr(pos, tag_end - pos);
        if (tag_content.find(marker) != std::string_view::npos) {
            auto begin = tag_end + 1;
            auto end   = xml.find("</ItemDefinitionGroup>", begin);
            if (end == std::string_view::npos) {
                std::println(stderr, "ERROR: Malformed ItemDefinitionGroup closing tag");
                std::exit(EXIT_FAILURE);
            }

            return std::string(xml.substr(begin, end - begin));
        }

        pos = tag_end + 1;
    }

    std::println(stderr, "ERROR: Could not find Release|x64 ItemDefinitionGroup in vcxproj");
    std::exit(EXIT_FAILURE);
}

ClCompileSettings ParseClCompile(
    std::string_view xml,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& solution_dir)
{
    ClCompileSettings settings;

    auto        cl_block = ExtractXmlElement(xml, "ClCompile");
    const auto& haystack = cl_block.empty() ? xml : cl_block;

    auto include_raw = ExtractXmlElement(haystack, "AdditionalIncludeDirectories");
    if (!include_raw.empty()) {
        auto expanded = ExpandMacros(include_raw, project_dir, solution_dir);
        for (const auto& item : Split(expanded, ';')) {
            auto trimed_item = Trim(item);
            if (!trimed_item.empty()) {
                try {
                    auto absolute = std::filesystem::absolute(std::move(trimed_item));
                    auto relative = std::filesystem::relative(absolute, solution_dir);
                    settings.include_dirs.push_back(relative.generic_string());
                } catch (...) {
                    settings.include_dirs.push_back(std::move(trimed_item));
                }
            }
        }
    }

    auto definition_raw = ExtractXmlElement(haystack, "PreprocessorDefinitions");
    if (!definition_raw.empty()) {
        definition_raw = ExpandMacros(definition_raw, project_dir, solution_dir);
        for (const auto& item : Split(definition_raw, ';')) {
            auto trimed_item = Trim(item);

            if (trimed_item == "WIN32" || trimed_item == "_WIN32") {
                continue;
            }

            if (!trimed_item.empty()) {
                settings.defines.push_back(std::move(trimed_item));
            }
        }
    }

    settings.pch_file           = Trim(ExtractXmlElement(haystack, "PrecompiledHeaderFile"));
    settings.rtti_enabled       = Trim(ExtractXmlElement(haystack, "RuntimeTypeInfo")) == "true";
    settings.additional_options = Trim(ExtractXmlElement(haystack, "AdditionalOptions"));

    return settings;
}

LinkSettings ParseLink(std::string_view xml) {
    LinkSettings settings;

    auto link_block = ExtractXmlElement(xml, "Link");
    const auto& haystack = link_block.empty() ? xml : link_block;

    auto dependencies_raw = ExtractXmlElement(haystack, "AdditionalDependencies");
    if (!dependencies_raw.empty()) {
        dependencies_raw = ReplaceAll(dependencies_raw, "%(AdditionalDependencies)", "");
        for (const auto& item : Split(dependencies_raw, ';')) {
            auto trimed_item = Trim(item);
            if (!trimed_item.empty()) {
                settings.dependencies.push_back(trimed_item);
            }
        }
    }

    return settings;
}

// ============================================================================
// Step 3: Parse vcpkg.json
// ============================================================================
// Extract JSON string values from a JSON array block (e.g. ["foo", "bar"]).
// Returns each unquoted string between double quotes.
std::vector<std::string> ExtractJsonStrings(std::string_view text) {
    std::vector<std::string> result;
    auto pos = 0uz;

    while (true) {
        auto open_quote = text.find('"', pos);
        if (open_quote == std::string_view::npos) {
            break;
        }

        auto close_quote = text.find('"', open_quote + 1);
        if (close_quote == std::string_view::npos) {
            break;
        }

        result.emplace_back(text.substr(open_quote + 1, close_quote - open_quote - 1));
        pos = close_quote + 1;
    }

    return result;
}

VcpkgManifest ParseVcpkgJson(const std::filesystem::path& filename) {
    VcpkgManifest manifest;
    auto content = ReadFile(filename);

    auto start = content.find("\"dependencies\"");
    if (start == std::string::npos) {
        return manifest;
    }

    auto bracket_open  = content.find('[', start);
    auto bracket_close = content.find(']', bracket_open);
    if (bracket_open == std::string::npos || bracket_close == std::string::npos) {
        return manifest;
    }

    auto deps_block = content.substr(bracket_open + 1, bracket_close - bracket_open - 1);
    manifest = ExtractJsonStrings(deps_block);

    return manifest;
}

// ============================================================================
// Step 4: Dependency mapping
// ============================================================================
static const std::unordered_map<std::string, DependencyInfo> kDependencyMap = {
    { "magic-enum",       { "magic_enum",       "magic_enum::magic_enum",           true  } },
    { "nlohmann-json",    { "nlohmann_json",    "nlohmann_json::nlohmann_json",     true  } },
    { "rapidhash",        { "",                 "",                                 false } },
    { "spdlog",           { "spdlog",           "spdlog::spdlog",                   true  } },
    { "unordered-dense",  { "unordered_dense",  "unordered_dense::unordered_dense", true  } },
};

// ============================================================================
// Step 5: Generate CMakeLists.txt
// ============================================================================
std::string GenerateCMakeLists(
    const std::vector<SourceFile>& sources,
    const ClCompileSettings& compile,
    const VcpkgManifest& manifest,
    std::string_view pch_header)
{
    std::string result;

    result += "# Auto-generated by CMakeGenerator. DO NOT edit manually.\n";

    result += "cmake_minimum_required(VERSION 3.21)\n";
    result += "project(glsld\n";
    result += "    VERSION 1.0.0\n";
    result += "    DESCRIPTION \"GLSL Language Server\"\n";
    result += "    LANGUAGES CXX\n";
    result += ")\n\n";

    // Global settings
    result += "# Global compiler settings\n";
    result += "set(CMAKE_CXX_STANDARD 23)\n";
    result += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    result += "set(CMAKE_CXX_EXTENSIONS OFF)\n";
    result += "set(CMAKE_POSITION_INDEPENDENT_CODE ON)\n";
    result += "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
    if (compile.rtti_enabled) {
        result += "\n# RTTI is required\n";
        result += "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -frtti\")\n";
    }
    result += "\n";

    // vcpkg manifest dependencies
    result += "# ============================================================================\n";
    result += "# Dependencies — from vcpkg manifest (vcpkg.json)\n";
    result += "# Build: cmake -B build -S glsld \\\n";
    result += "#     -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake\n";
    result += "# ============================================================================\n";

    std::unordered_set<std::string> find_packages;
    std::vector<std::string> link_targets;
    for (const auto& dependency : manifest) {
        auto it = kDependencyMap.find(dependency);
        if (it == kDependencyMap.end()) {
            std::println(stderr, "WARNING: Unknown vcpkg dependency '{}', skipping", dependency);
            continue;
        }

        const auto& info = it->second;
        if (info.needs_find && !info.cmake_package.empty())
            find_packages.insert(info.cmake_package);
        if (!info.cmake_target.empty())
            link_targets.push_back(info.cmake_target);
    }

    for (const auto& package : find_packages) {
        result += std::format("find_package({} CONFIG REQUIRED)\n", package);
    }

    if (std::ranges::find(manifest, "rapidhash") != manifest.end()) {
        result += "# rapidhash is header-only (no CMake config) — include path from vcpkg toolchain\n";
    }

    result += "\n";

    // mimalloc
    result += "# ============================================================================\n";
    result += "# mimalloc (git submodule — static library)\n";
    result += "# ============================================================================\n";
    result += "set(MI_BUILD_SHARED OFF CACHE BOOL \"\" FORCE)\n";
    result += "set(MI_BUILD_STATIC ON  CACHE BOOL \"\" FORCE)\n";
    result += "set(MI_BUILD_OBJECT OFF CACHE BOOL \"\" FORCE)\n";
    result += "set(MI_BUILD_TESTS  OFF CACHE BOOL \"\" FORCE)\n";
    result += "set(MI_OVERRIDE     OFF CACHE BOOL \"\" FORCE)\n";
    result += "set(MI_USE_CXX      ON  CACHE BOOL \"\" FORCE)\n";
    result += "add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mimalloc/mimalloc mimalloc_build)\n\n";

    // Source files
    result += "# ============================================================================\n";
    result += "# Source files\n";
    result += "# ============================================================================\n";
    result += "set(GLSLD_SOURCES\n";
    for (const auto& source : sources) {
        result += std::format("    Sources/{}\n", source.relative_path.generic_string());
    }
    result += ")\n\n";

    // Executable
    result += "# ============================================================================\n";
    result += "# glsld executable\n";
    result += "# ============================================================================\n";
    result += "add_executable(glsld ${GLSLD_SOURCES})\n\n";

    if (!pch_header.empty()) {
        result += std::format("target_precompile_headers(glsld PRIVATE Sources/{})\n\n", pch_header);
    }

    // Include directories
    result += "target_include_directories(glsld PRIVATE\n";
    for (const auto& dir : compile.include_dirs) {
        result += std::format("    ${{CMAKE_CURRENT_SOURCE_DIR}}/{}\n", dir);
    }
    result += ")\n\n";

    // Link
    result += "target_link_libraries(glsld PRIVATE\n";
    result += "    mimalloc-static\n";
    for (const auto& target : link_targets) {
        result += std::format("    {}\n", target);
    }
    result += ")\n\n";

    // Compiler options
    result += "# ============================================================================\n";
    result += "# Compiler-specific options\n";
    result += "# ============================================================================\n";

    result += "target_compile_definitions(glsld PRIVATE\n";
    for (const auto& def : compile.defines) {
        result += std::format("    {}\n", def);
    }
    result += ")\n\n";

    result += "if(MSVC)\n";
    result += "    # Windows / MSVC\n";
    result += "    target_compile_options(glsld PRIVATE\n";
    result += "        /W4 /Zc:__cplusplus /utf-8\n";
    result += "    )\n";
    result += "    target_compile_definitions(glsld PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)\n";
    result += "    target_link_options(glsld PRIVATE /SUBSYSTEM:CONSOLE)\n";
    result += "else()\n";
    result += "    # Linux / GCC & Clang\n";
    result += "    target_compile_options(glsld PRIVATE\n";
    result += "        -Wall -Wextra -Wpedantic -Wno-gnu-line-marker\n";
    result += "    )\n";
    result += "    find_package(Threads REQUIRED)\n";
    result += "    target_link_libraries(glsld PRIVATE Threads::Threads)\n";
    result += "endif()\n";

    return result;
}

// ============================================================================
// Main
// ============================================================================
int main() {
    auto paths = LocateProjectStructure();
    if (!paths.has_value()) {
        std::println(stderr, "ERROR: Could not find project structure");
        return EXIT_FAILURE;
    }

    auto repo_root    = paths->solution_dir;
    auto glsld_dir    = repo_root / "glsld";
    auto sources_dir  = glsld_dir / "Sources";
    auto vcxproj_path = glsld_dir / "glsld.vcxproj";
    auto vcpkg_path   = glsld_dir / "vcpkg.json";
    auto cmake_out    = repo_root / "CMakeLists.txt";

    std::println("Repo root:    {}", repo_root.generic_string());
    std::println("glsld dir:    {}", glsld_dir.generic_string());
    std::println("Sources dir:  {}", sources_dir.generic_string());
    std::println("vcxproj:      {}", vcxproj_path.generic_string());
    std::println("vcpkg.json:   {}", vcpkg_path.generic_string());
    std::println("Output:       {}", cmake_out.generic_string());
    std::println("");

    // [1/4] Discover .cpp files
    std::println("[1/4] Discovering .cpp files...");
    auto sources = DiscoverSources(sources_dir);
    std::println("  Found {} source files", sources.size());
    for (const auto& source : sources) {
        std::println("    {}{}", source.relative_path.generic_string(), source.is_pch_source ? " (PCH)" : "");
    }

    // [2/4] Parse vcxproj
    std::println("[2/4] Parsing vcxproj...");
    auto vcxproj_xml      = ReadFile(vcxproj_path);
    auto release_block    = FindReleaseX64Block(vcxproj_xml);
    auto compile_settings = ParseClCompile(release_block, glsld_dir, repo_root);
    auto link_settings    = ParseLink(release_block);

    std::println("  Include dirs: {}", compile_settings.include_dirs.size());
    for (const auto& dir : compile_settings.include_dirs) {
        std::println("    {}", dir);
    }
    std::println("  Defines: {}", compile_settings.defines.size());
    for (const auto& def : compile_settings.defines) {
        std::println("    {}", def);
    }

    std::println("  PCH: {}", compile_settings.pch_file);
    std::println("  RTTI: {}", compile_settings.rtti_enabled);
    std::println("  Link deps: {}", link_settings.dependencies.size());
    for (const auto& dependency : link_settings.dependencies) {
        std::println("    {}", dependency);
    }

    // [3/4] Parse vcpkg.json
    std::println("[3/4] Parsing vcpkg.json...");
    auto manifest = ParseVcpkgJson(vcpkg_path);
    std::println("  Dependencies: {}", manifest.size());
    for (const auto& dependency : manifest) {
        auto it = kDependencyMap.find(dependency);
        auto status = (it != kDependencyMap.end()) ? "OK" : "UNKNOWN";
        std::println("    {} [{}]", dependency, status);
    }

    // [4/4] Generate CMakeLists.txt
    std::println("[4/4] Generating CMakeLists.txt...");
    auto cmake = GenerateCMakeLists(sources, compile_settings, manifest, compile_settings.pch_file);
    WriteFile(cmake_out, cmake);
    std::println("  Written {} bytes to {}", cmake.size(), cmake_out.generic_string());

    std::println("\nDone.");
    return 0;
}
