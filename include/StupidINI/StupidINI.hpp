#pragma once

#include "detail/ParseUtils.hpp"

namespace StupidINI
{    
    struct Style
    {
    };

    /// @brief A representation of an INI file. Use `GetSetting` to access its members.
    class INI
    {
    public:
        friend std::expected<INI, ParseError> load_ini(std::string_view path) noexcept;

        /// @brief Default constructor. Cannot be actually used to load existing INI files, use `load_ini` for that purpose.
        INI() = default;

        /// @brief This constructor allows you to specify a style. When saving, the resulting INI will be saved in this format.
        /// @param style 
        INI(const Style& style) :
            _style(style)
        {}

        template <typename T> requires is_setting<T>
        [[nodiscard]]
        std::expected<T, FetchError> GetValue(const std::string& setting, const std::string& section) const noexcept
        {
#ifndef STUPIDINI_NO_CACHE
            static StringMap<T> cache{};
            std::string_view compoundName = setting + "|" + section;
            const auto cached = cache.find(compoundName);
            if (cached != cache.end()) {
                return cached->second;
            }
#endif
            const auto foundSection = _data.find(section);
            if (foundSection == _data.end()) {
                return std::unexpected(FetchError::SectionNotFound);
            }

            const auto foundSetting = std::ranges::find(foundSection->second, setting);
            if (foundSetting == foundSection->second.end()) {
                return std::unexpected(FetchError::SectionNotFound);
            }
            auto res = As<T>(*foundSetting);
            if (!res) {
                return std::unexpected(res.error());
            }
#ifndef STUPIDINI_NO_CACHE
            cache.emplace(compoundName, res.value());
#endif
            return res.value();
        }

        /// @brief Sets a specific setting in a specific section to this value. If the setting OR section don't exist, they are created.
        /// @tparam T For convenience. Must be either a decimal, integral, boolean, or text.
        /// @param setting The setting. If it doesn't exist in the given section, it is created. If it already exists, it is overwritten.
        /// @param section The section. If it doesn't exist, it is created.
        /// @param value The value to insert. 
        template <typename T> requires is_setting<T>
        void SetValue(const std::string& setting, const std::string& section, const T& value) {
            static_assert(always_false<T>, "SetValue is not implemented yet.");
        }

        /// @brief Saves this INI file. If there is an already existing INI file, it is COMPLETELY overwritten.
        /// @return `std::nullopt` in case the file was saved normally, otherwise a `SaveError` indicating why the save failed.
        template <typename T>
        [[nodiscard]]
        std::optional<SaveError> Save() const noexcept {
            static_assert(always_false<T>, "Save is not implemented yet.");
            return std::nullopt;
        }

        void SetStyle(const Style& style) noexcept {
            _style = style;
        }
    private:
        INI(const std::filesystem::path& path) :
            _path(path)
        {}

        template <typename T> requires is_numerical<T>
        [[nodiscard]]
        static inline std::expected<T, FetchError> As(const std::string_view str) noexcept
        {
            T out;
            auto [res, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
            if (ec == std::errc::invalid_argument) {
                return std::unexpected(FetchError::NotANumber);
            }
            else if (ec == std::errc::result_out_of_range) {
                return std::unexpected(FetchError::NumberOutOfRange);
            }
            else if (res != str.data() + str.size()) {
                return std::unexpected(FetchError::NotANumber);
            }
            return out;
        }

        Style                 _style{};
        INI_DATA              _data{};
        std::filesystem::path _path = "";
    };

    /// @brief Use this function to construct an INI object. Any ini retrieved is valid. I recommend marking it "const".
    /// @param path The path to the ini file. It can be relative, absolute, or network.
    /// @return An INI object if it is successful, an error otherwise. Use StupidINI::to_string to print the error.
    [[nodiscard]]
    inline std::expected<INI, ParseError> load_ini(std::string_view path) noexcept
    {
        const auto createdPath = detail::create_path(path);
        if (!createdPath) {
            return std::unexpected(createdPath.error());
        }

        auto createdStream = detail::open_stream(createdPath.value());
        if (!createdStream) {
            return std::unexpected(createdStream.error());
        }

        auto parsedData = detail::create_data(createdStream.value());
        if (!parsedData) {
            return std::unexpected(parsedData.error());
        }

        INI ini(createdPath.value());
        ini._data = std::move(parsedData.value());
        return ini;
    }
}