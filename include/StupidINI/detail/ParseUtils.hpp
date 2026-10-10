#pragma once

#include "Defines.hpp"

namespace StupidINI
{
    namespace detail
    {
        [[nodiscard]]
        inline std::expected<std::ifstream, ParseError> open_stream(const std::filesystem::path& path) noexcept
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

                return std::unexpected(ParseError::FilesystemError); // fallback, this shouldn't happen.
            }

            if (!std::filesystem::is_regular_file(status)) {
                return std::unexpected(ParseError::NotAFile);
            }
            return std::unexpected(ParseError::FilesystemError); // stream closed... but no error code?
        }

        [[nodiscard]]
        inline std::expected<std::filesystem::path, ParseError> create_path(std::string_view path) noexcept
        {
            static constexpr std::string_view expected_extension = ".ini";
            static constexpr size_t expected_extension_size = expected_extension.size();

            const auto pathSize = path.size();
            if (pathSize < expected_extension_size) {
                return std::unexpected(ParseError::InvalidPath);
            }

            const auto ext = path.substr(pathSize - expected_extension_size, std::string::npos);
            const bool is_ini = ext.size() == expected_extension_size &&
                std::equal(ext.begin(), ext.end(), expected_extension,
                    [](unsigned char a, unsigned char b) {
                        return std::tolower(a) == b;
                });

            if (!is_ini) {
                return std::unexpected(ParseError::InvalidPath);
            }

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
                return std::unexpected(ParseError::FilesystemError);
            }
            catch (std::filesystem::filesystem_error& e) {
                return std::unexpected(ParseError::FilesystemError);
            }
        }
    }
}