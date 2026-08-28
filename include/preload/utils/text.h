// text.h
#ifndef PRELOAD_UTILS_TEXT_H
#define PRELOAD_UTILS_TEXT_H

#include <filesystem>
#include <string>
#include <vector>

namespace preload
{

/**
 * @brief Strip leading and trailing whitespace from a string.
 *
 * @param s The string to trim.
 * @return The string with surrounding whitespace removed.
 */
std::string trim(std::string s);

/**
 * @brief Decode a UTF-8 byte string into its Unicode codepoints.
 *
 * @param line The raw UTF-8 bytes of one row.
 * @param path Source file path, used only in error messages.
 * @param row Zero-based row index, used only in error messages.
 * @return The decoded codepoints, in order.
 */
std::vector<char32_t> decodeUtf8(const std::string& line,
                                 const std::filesystem::path& path, int row);

}  // namespace preload

#endif
