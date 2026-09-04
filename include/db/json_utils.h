#ifndef DB_JSON_UTILS_H
#define DB_JSON_UTILS_H

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace db
{

/**
 * @brief Open a json file and parse it, raising a descriptive error if
 * either step fails.
 *
 * @param path Path to the json file to read.
 * @return The parsed json document.
 */
nlohmann::json readJsonFile(const std::filesystem::path& path);

/**
 * @brief Convert a "tier_<N>" key into its numeric tier.
 *
 * @param key The object key to parse, e.g. "tier_1".
 * @return The numeric tier extracted from the key.
 */
int parseTierKey(const std::string& key);

/**
 * @brief Raise a descriptive error if a directory does not exist.
 *
 * @param dir The directory to check.
 * @param label Description of the directory used in the error message,
 * e.g. "levels".
 */
void requireDirectory(const std::filesystem::path& dir,
                      const std::string& label);

}  // namespace db

#endif
