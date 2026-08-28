#include "preload/utils/json_io.h"

#include <fstream>
#include <stdexcept>

namespace preload
{

nlohmann::json readJson(const std::filesystem::path& path)
{
  std::ifstream in(path);
  if (!in)
  {
    throw std::runtime_error("could not open file: " + path.string());
  }

  nlohmann::json j;
  in >> j;
  return j;
}

}  // namespace preload
