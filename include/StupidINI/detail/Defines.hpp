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

#include <fmt/format.h>

namespace StupidINI
{
    template <typename T>
    concept is_bool =
        std::is_same_v<T, bool>;

    template <typename T>
    concept is_numerical = 
        (std::is_integral_v<T> || std::is_floating_point_v<T>) &&
        !is_bool<T>;
    
    template <typename T>
    concept is_text =
        std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

    template <typename T>
    concept is_setting =
        is_bool<T> || is_text<T> || is_numerical<T>;
        
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

    template <typename T>
    using StringMap = std::unordered_map<std::string, T, StringHash, StringEqual>;
    using Section = StringMap<std::vector<std::string>>;

    inline static constexpr const char assignChr = '=';
    inline static constexpr const char commentChr = ';';
    inline static constexpr const char sectionEnd = ']';
    inline static constexpr const char sectionStart = '[';

    inline static constexpr const std::array<std::string_view, 2u> truthy = {
        "true", "1"
    };

    inline static constexpr const std::array<std::string_view, 2u> falsy = {
        "false", "0"
    };

    inline static constexpr const size_t max_truthy_falsy_size = 5u;

    [[nodiscard]]
    inline static bool is_lower_in_array(const std::array<std::string_view, 2u>& arr, const std::string& str)
    {
        if (str.size() > max_truthy_falsy_size) {
            return false;
        }
        std::string lower{str};
        std::ranges::for_each(lower, [](char chr) {
            std::tolower(chr);
        });
        return std::ranges::contains(arr, lower);
    }

    [[nodiscard]]
    inline static bool is_truthy(const std::string& str) {
        return is_lower_in_array(truthy, str);
    }

    [[nodiscard]]
    inline static bool is_falsy(const std::string& str) {
        return is_lower_in_array(falsy, str);
    }

    enum class ParseError
    {
        MissingFile,     // Given file couldn't be found
        InvalidPath,     // Path passed in is invalid 

        NotAFile,        // Path points to something that isn't a file (directory)
        PermissionError, // Given file was found, but couldn't be opened
        FilesystemError  // std::filesystem couldn't open the file
    };

    [[nodiscard]] inline std::string to_string(ParseError err) noexcept
    {
        switch (err) {
        case ParseError::MissingFile: return "MissingFile";
        case ParseError::InvalidPath: return "InvalidPath";
        case ParseError::NotAFile: return "NotAFile";
        case ParseError::PermissionError: return "PermissionError";
        case ParseError::FilesystemError: return "FilesystemError";
        default: std::unreachable();
        }
    }

    std::ostream& operator<<(std::ostream& os, ParseError error) {
        return os << to_string(error);
    }
    
    enum class FetchError
    {
        SectionNotFound,   // Provided section is not in the ini
        SettingNotFound,   // Provided setting is not in the ini, in the given section
        NotANumber,        // Provided setting was not a number
        NumberOutOfRangev, // Provided setting was a number, but was out of bounds for the requested type
        NotABoolean        // Provided setting was not a boolean
    };

    [[nodiscard]] inline std::string to_string(FetchError err) noexcept
    {
        switch (err) {
        case FetchError::SectionNotFound: return "SectionNotFound";
        case FetchError::SettingNotFound: return "SettingNotFound";
        case FetchError::NotANumber: return "NotANumber";
        case FetchError::NumberOutOfRangev: return "NumberOutOfRangev";
        case FetchError::NotABoolean: return "NotABoolean";
        default: std::unreachable();
        }
    }

    std::ostream& operator<<(std::ostream& os, FetchError error) {
        return os << to_string(error);
    }

    enum class SaveError
    {

    };

    [[nodiscard]] inline std::string to_string(SaveError err) noexcept
    {
        switch (err) {
        default: std::unreachable();
        }
    }

    std::ostream& operator<<(std::ostream& os, SaveError error) {
        return os << to_string(error);
    }
}

template<>
struct fmt::formatter<StupidINI::FetchError> : fmt::formatter<std::string_view> {
    auto format(StupidINI::FetchError error, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(
            StupidINI::to_string(error), ctx
        );
    }
};

template<>
struct fmt::formatter<StupidINI::ParseError> : fmt::formatter<std::string_view> {
    auto format(StupidINI::ParseError error, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(
            StupidINI::to_string(error), ctx
        );
    }
};

template<>
struct fmt::formatter<StupidINI::SaveError> : fmt::formatter<std::string_view> {
    auto format(StupidINI::SaveError error, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(
            StupidINI::to_string(error), ctx
        );
    }
};