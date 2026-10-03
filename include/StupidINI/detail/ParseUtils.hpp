#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace StupidINI
{
    struct StringHash {
        using is_transparent = void;

        size_t operator()(std::string_view sv) const noexcept {
            return std::hash<std::string_view>{}(sv);
        }

        size_t operator()(const std::string& s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
    };

    struct StringEqual {
        using is_transparent = void;

        bool operator()(std::string_view a, std::string_view b) const noexcept {
            return a == b;
        }
    };
    
    template <typename T, typename U>
    using Map = std::unordered_map<T, U, StringHash, StringEqual>;

    using Section = Map<std::string, std::string>;
    using Settings = Map<std::string, Section>;

    enum class FailureReason
    {
        SectionNotFound,
        SettingNotFound,
        NumberOutOfRange,
        NotABoolean,
        NotANumber
    };

    [[nodiscard]] inline std::string to_string(FailureReason err) noexcept
    {
        switch (err) {
            case FailureReason::SectionNotFound: return "SectionNotFound";
            case FailureReason::SettingNotFound: return "SettingNotFound";
            case FailureReason::NumberOutOfRange: return "NumberOutOfRange";
            default: std::unreachable();
        }
    }

    enum class ParseError
    {
        None,

        MissingFile,
        PermissionError,
        IOError,
        NotAFile,

        EmptyPath,
        InvalidPath,
        FilesystemError,

        MalformedSection,
        MalformedKey
    };

    [[nodiscard]] inline std::string to_string(ParseError err) noexcept
    {
        switch (err) {
        default: std::unreachable();
        }
    }

    namespace detail
    {
        [[nodiscard]]
        inline std::expected<std::ifstream, ParseError> open_stream(const std::filesystem::path& path)
        {

            std::ifstream stream{path};
            if (stream.is_open()) {
                return stream;
            }

            std::error_code ec;
            const auto status = std::filesystem::status(path, ec);

            if (ec) {
                if (ec == std::errc::no_such_file_or_directory) {
                    return std::unexpected(ParseError::MissingFile);
                }

                if (ec == std::errc::permission_denied) {
                    return std::unexpected(ParseError::PermissionError);
                }

                return std::unexpected(ParseError::IOError); // fallback, this shouldn't happen.
            }

            if (!std::filesystem::is_regular_file(status)) {
                return std::unexpected(ParseError::NotAFile);
            }
            return std::unexpected(ParseError::IOError);
        }

        enum class LineType
        {
            Empty,
            Section,
            KeyVal
        };

        struct LINE_DATA
        {
            size_t size = 0u;
            size_t charStart = 0u;

            LineType type = LineType::Empty;
        };

        [[nodiscard]]
        std::expected<LINE_DATA, ParseError> get_line_data(const std::string& line) {
            inline static constexpr const unsigned char comment_chr = ';';
            inline static constexpr const unsigned char section_start_chr = '[';
            inline static constexpr const unsigned char assign_chr = '=';

            LINE_DATA data;
            unsigned char chr;
            data.size = line.size();
            for (size_t i = 0u; i < data.size; ++i) {
                chr = line[i];
                if (std::isspace(chr)) {
                    continue;
                }

                if (chr == assign_chr) {
                    return std::unexpected(ParseError::MalformedKey);
                }

                if (chr == section_start_chr) {
                    data.type = LineType::Section;
                }
                else {
                    data.type = LineType::KeyVal;
                }
                return data;
            }
            return data;
        }

        [[nodiscard]]
        inline std::expected<std::string, ParseError> get_section(const std::string& line, const size_t start)
        {
            inline static constexpr const unsigned char section_end_chr = ']';
            auto where = line.find(section_end_chr, start);
            if (where == std::string::npos) {
                return std::unexpected(ParseError::MalformedSection);
            }
            return line.substr(start + 1u, where);
        }

        struct KeyValPair
        {
            std::string key{};
            std::string val{};
        };

        [[nodiscard]]
        inline std::expected<KeyValPair, ParseError> get_key_val_pair(const std::string& line, const size_t start)
        {

        }

        [[nodiscard]]
        inline std::expected<Settings, ParseError> load_ini_data(std::ifstream& stream)
        {
            Settings iniData;
            std::string line;
            std::string section{};

            ParseError lineError = ParseError::MalformedKey;
            std::expected<LINE_DATA, ParseError> lineData;
            while (std::getline(stream, line)) {
                
                lineData = get_line_data(line);

                if (!lineData.has_value()) {
                    switch (lineData.error()) {
                    case ParseError::MalformedKey:
                        break;
                    default: return std::unexpected(lineData.error());
                    }
                    continue;
                }
                
                auto& data = lineData.value();
                if (data.type == LineType::Section) {
                    const auto recoveredSection = get_section(line, data.charStart);
                    if (!recoveredSection.has_value()) {
                        return std::unexpected(recoveredSection.error());
                    }
                    section = recoveredSection.value();
                }
                else if (data.type == LineType::KeyVal) {
                    const auto keyval = get_key_val_pair(line, data.charStart);
                    if (!keyval.has_value()) {
                        return std::unexpected(keyval.error());
                    }
                    auto& sectionSettings = iniData[section];
                    sectionSettings[keyval.value().key] = keyval.value().val;
                }
                
            }
            return iniData;
        }

        [[nodiscard]]
        inline std::expected<std::filesystem::path, ParseError> create_path(std::string_view path)
        {
            try {
                std::filesystem::path file_path{std::string{path}};

                std::error_code ec;
                file_path = std::filesystem::weakly_canonical(file_path, ec);

                if (!ec) {
                    return file_path;
                }
                else if (ec == std::errc::no_such_file_or_directory) {
                    return std::unexpected(ParseError::MissingFile);
                }
                else if (ec == std::errc::permission_denied) {
                    return std::unexpected(ParseError::PermissionError);
                }
                else if (ec == std::errc::invalid_argument) {
                    return std::unexpected(ParseError::InvalidPath);
                }
                return std::unexpected(ParseError::IOError);
            }
            catch (std::filesystem::filesystem_error& e) {
                return std::unexpected(ParseError::FilesystemError);
            }
        }
    }
}