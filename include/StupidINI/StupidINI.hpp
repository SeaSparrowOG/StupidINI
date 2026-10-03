#pragma once

#include "detail/ParseUtils.hpp"

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
    
    /// @brief A representation of an INI file. Use `GetSetting` to access its members.
    class INI
    {
    public:
        friend std::expected<INI, ParseError> load_ini(std::string_view);

        /// @brief Returns the raw value of of setting `setting` in section `section`.
        /// @param setting The name of the setting.
        /// @param section The name of the section `setting` is in.
        /// @return The value of `setting`, otherwise the reason it didn't work.
        [[nodiscard]]
        std::expected<std::string, FailureReason> GetSetting(const std::string_view setting,
                                                                const std::string_view section) const noexcept 
        {
            const auto& foundSection = _settings.find(section);
            if (foundSection == _settings.end()) {
                return std::unexpected(FailureReason::SectionNotFound);
            }

            const auto& map = foundSection->second;
            const auto& foundSetting = map.find(setting);
            if (foundSetting == map.end()) {
                return std::unexpected(FailureReason::SettingNotFound);
            }
            return foundSetting->second;
        }
        
        /// @brief Returns the value of setting `setting` in section `section` cast as T.
        /// @tparam T The type to cast T as. Must be either integral or decimal.
        /// @param setting The name of the setting.
        /// @param section The section the setting is found in.
        /// @return On success, the value of `setting` as `T`. Otherwise, the reason it didn't work.
        template <typename T> requires is_setting<T>
        [[nodiscard]]
        std::expected<T, FailureReason> GetSetting(const std::string_view setting,
                                                                const std::string_view section) const noexcept 
        {
            const auto found = GetSetting(setting, section);
            if (!found) {
                return std::unexpected(found.error());
            }

            const auto& val = found.value();
            if constexpr (is_text<T>) {
                return static_cast<T>(val);
            }

            if constexpr (is_bool<T>) {
                std::string str;
                str.resize(val.size());

                std::transform(val.begin(), val.end(), str.begin(),
                    [](unsigned char chr) {
                        return static_cast<char>(std::tolower(chr));
                    });

                if (std::ranges::contains(truthy, str)) {
                    return true;
                }
                if (std::ranges::contains(falsy, str)) {
                    return false;
                }
                return std::unexpected(FailureReason::NotABoolean);
            }
            return As<T>(val);
        }

        /// @brief Returns the raw value of a setting either not under a section or under the `[]` section.
        /// @param setting The name of the setting.
        /// @return The value of `setting`, otherwise the reason it didn't work.
        [[nodiscard]]
        std::expected<std::string, FailureReason> GetSetting(const std::string_view setting) const noexcept 
        {
            return GetSetting(setting, "");
        }
        
        /// @brief Returns the value of a setting either not under a section or under the `[]` section cast as T.
        /// @tparam T The type to cast T as. Must be either integral or decimal.
        /// @param setting The name of the setting.
        /// @return On success, the value of `setting` as `T`. Otherwise, the reason it didn't work.
        template <typename T> requires is_setting<T>
        [[nodiscard]]
        std::expected<T, FailureReason> GetSetting(const std::string_view setting) const noexcept 
        {
            return GetSetting<T>(setting, "");
        }

    private:
        INI() = default;
        Settings _settings;

        inline static constexpr const std::array<std::string_view, 2> truthy = {
            "1", "true"
        };
        inline static constexpr const std::array<std::string_view, 2> falsy = {
            "0", "false"
        };

        template <typename T> requires is_numerical<T>
        static std::expected<T, FailureReason> As(const std::string_view str) noexcept
        {
            T out;
            auto [res, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
            if (ec == std::errc::invalid_argument) {
                return std::unexpected(FailureReason::NotANumber);
            }
            else if (ec == std::errc::result_out_of_range) {
                return std::unexpected(FailureReason::NumberOutOfRange);
            }
            else if (ptr != str.data() + str.size()) {
                return std::unexpected(FailureReason::NotANumber);
            }
            return out;
        }
    };

    /// @brief Use this function to construct an INI object. Any ini retrieved is valid. I recommend marking it "const".
    /// @param path The path to the ini file. It can be relative, absolute, or network.
    /// @return An INI object if it is successful, an error otherwise. Use StupidINI::to_string to print the error.
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

        auto createdData = detail::load_ini_data(createdStream.value());
        if (!createdData) {
            return std::unexpected(createdData.error());
        }

        auto& data = createdData.value();
        INI ini;
        ini._settings = std::move(data);

        return ini;
    }
}