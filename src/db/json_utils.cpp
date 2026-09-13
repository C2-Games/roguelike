#include "db/json_utils.h"

#include <fstream>
#include <stdexcept>

namespace db
{

nlohmann::json readJsonFile(const std::filesystem::path& path)
{
  std::ifstream stream(path);
  if (!stream)
  {
    throw std::runtime_error("could not open file: " + path.string());
  }
  try
  {
    return nlohmann::json::parse(stream);
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed json at " + path.string() + ": " +
                             e.what());
  }
}

int parseTierKey(const std::string& key)
{
  const std::string prefix = "tier_";
  if (key.rfind(prefix, 0) != 0)
  {
    throw std::runtime_error("tier key '" + key +
                             "' does not match the expected 'tier_<N>' shape");
  }
  return std::stoi(key.substr(prefix.size()));
}

void requireDirectory(const std::filesystem::path& dir,
                      const std::string& label)
{
  if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
  {
    throw std::runtime_error(label + " directory not found: " + dir.string());
  }
}

}  // namespace db
