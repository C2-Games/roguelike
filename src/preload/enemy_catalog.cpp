#include "preload/enemy_catalog.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "objects/fovs/ellipse_fov.h"
#include "preload/utils/text.h"

namespace
{

// splits a ";"-joined row string into an EntitySymbol grid, decoding each
// row's UTF-8 text into wide-char cells.
EntitySymbol decodeSymbol(const std::string& encoded, const std::string& name)
{
  const std::string source = "db:enemies.symbol[" + name + "]";
  EntitySymbol symbol;
  std::size_t rowStart = 0;
  int row = 0;
  while (rowStart <= encoded.size())
  {
    const std::size_t rowEnd = encoded.find(';', rowStart);
    const std::string rowText = encoded.substr(
        rowStart,
        rowEnd == std::string::npos ? std::string::npos : rowEnd - rowStart);

    const std::vector<char32_t> codepoints =
        preload::decodeUtf8(rowText, source, row);
    std::vector<wchar_t> rowCells(codepoints.size());
    std::transform(codepoints.begin(), codepoints.end(), rowCells.begin(),
                   [](char32_t cp) { return static_cast<wchar_t>(cp); });
    symbol.push_back(std::move(rowCells));
    ++row;

    if (rowEnd == std::string::npos)
    {
      break;
    }
    rowStart = rowEnd + 1;
  }
  return symbol;
}

}  // namespace

EnemyCatalog::EnemyCatalog(SQLite::Database& database)
{
  SQLite::Statement statement(
      database,
      "SELECT enemies.name, enemies.symbol, enemy_tiers.tier, "
      "enemy_tiers.health, enemy_tiers.damage_amount, "
      "enemy_tiers.damage_type, enemy_tiers.fov_x, enemy_tiers.fov_y, "
      "enemy_tiers.chase, enemy_tiers.speed FROM enemies JOIN enemy_tiers ON "
      "enemy_tiers.enemy_id = enemies.id");

  // the join repeats each enemy's name/symbol once per tier row; cache the
  // decoded symbol per name so it's parsed once regardless of tier count.
  std::map<std::string, EntitySymbol> symbolsByName;

  while (statement.executeStep())
  {
    const std::string name = statement.getColumn(0).getString();
    auto symbolEntry = symbolsByName.find(name);
    if (symbolEntry == symbolsByName.end())
    {
      symbolEntry =
          symbolsByName
              .emplace(name,
                       decodeSymbol(statement.getColumn(1).getString(), name))
              .first;
    }
    const EntitySymbol& symbol = symbolEntry->second;
    const int tier = statement.getColumn(2).getInt();
    const int health = statement.getColumn(3).getInt();
    const int damageAmount = statement.getColumn(4).getInt();
    const int fovX = statement.getColumn(6).getInt();
    const int fovY = statement.getColumn(7).getInt();
    const int chase = statement.getColumn(8).getInt();
    const int speed = statement.getColumn(9).getInt();

    catalog_[name][tier] = EnemyTierAttributes{
        symbol, health, damageAmount, std::make_unique<EllipseFOV>(fovX, fovY),
        chase,  speed,
    };
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
