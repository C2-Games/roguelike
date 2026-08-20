#include "entities/enemy_catalog.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace
{

// "tier_1" -> 1, "tier_2" -> 2, ...
int parseTierKey(const std::string& key)
{
  const std::string prefix = "tier_";
  if (key.rfind(prefix, 0) != 0)
  {
    throw std::runtime_error("attribute key '" + key +
                             "' does not match the expected 'tier_<N>' shape");
  }
  return std::stoi(key.substr(prefix.size()));
}

EnemyTierAttributes parseTierAttributes(char symbol,
                                        const nlohmann::json& attrs)
{
  const auto& fovArr = attrs.at("fov");
  return EnemyTierAttributes{
      symbol,
      attrs.at("health").get<int>(),
      attrs.at("damage").at("amount").get<int>(),
      ellipseFOV(fovArr.at(0).get<int>(), fovArr.at(1).get<int>()),
      attrs.at("chase").get<int>(),
      attrs.at("speed").get<int>(),
  };
}

}  // namespace

EnemyCatalog::EnemyCatalog(const std::filesystem::path& dir)
{
  if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
  {
    throw std::runtime_error("enemy catalog directory not found: " +
                             dir.string());
  }

  bool foundAny = false;
  for (const auto& entry : std::filesystem::directory_iterator(dir))
  {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
    {
      continue;
    }
    foundAny = true;
    loadFile(entry.path());
  }

  if (!foundAny)
  {
    throw std::runtime_error("no enemy *.json files found in: " + dir.string());
  }
}

void EnemyCatalog::loadFile(const std::filesystem::path& path)
{
  try
  {
    std::ifstream file(path);
    if (!file)
    {
      throw std::runtime_error("could not open file");
    }

    nlohmann::json enemyJson;
    file >> enemyJson;

    const std::string name = enemyJson.at("name").get<std::string>();
    const char symbol =
        enemyJson.at("symbol").at(0).at(0).get<std::string>().at(0);

    std::map<int, EnemyTierAttributes> tiers;
    for (const auto& [tierKey, attrs] : enemyJson.at("attributes").items())
    {
      tiers.emplace(parseTierKey(tierKey), parseTierAttributes(symbol, attrs));
    }

    catalog_[name] = std::move(tiers);
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error("malformed enemy definition in " + path.string() +
                             ": " + e.what());
  }
}

const EnemyTierAttributes* EnemyCatalog::find(const std::string& name,
                                              int tier) const
{
  auto namedEntry = catalog_.find(name);
  if (namedEntry == catalog_.end())
  {
    return nullptr;
  }

  const std::map<int, EnemyTierAttributes>& tiers = namedEntry->second;
  auto tierEntry = tiers.find(tier);
  if (tierEntry == tiers.end())
  {
    return nullptr;
  }

  return &tierEntry->second;
}
