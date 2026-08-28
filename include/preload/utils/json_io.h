// json_io.h
#ifndef PRELOAD_UTILS_JSON_IO_H
#define PRELOAD_UTILS_JSON_IO_H

#include <filesystem>
#include <nlohmann/json.hpp>

namespace preload
{

/**
 * @brief Read and parse a JSON file from disk.
 *
 * @param path Path to the JSON file.
 * @return The parsed JSON document.
 */
nlohmann::json readJson(const std::filesystem::path& path);

}  // namespace preload

#endif
