#include "objects/entities/entity_symbol.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>

#include "objects/direction.h"

int symbolWidth(const EntitySymbol& symbol)
{
  return symbol.empty() ? 0 : static_cast<int>(symbol[0].size());
}

int symbolHeight(const EntitySymbol& symbol)
{
  return static_cast<int>(symbol.size());
}

bool isSingleCell(const EntitySymbol& symbol)
{
  return symbolWidth(symbol) == 1 && symbolHeight(symbol) == 1;
}

EntitySymbol padToRectangle(const EntitySymbol& symbol)
{
  const std::size_t maxWidth =
      std::accumulate(symbol.begin(), symbol.end(), std::size_t{0},
                      [](std::size_t width, const auto& row) {
                        return std::max(width, row.size());
                      });

  EntitySymbol padded = symbol;
  for (auto& row : padded)
  {
    row.resize(maxWidth, L'\0');
  }
  return padded;
}

EntitySymbol rotateSymbol90(const EntitySymbol& symbol)
{
  const int rows = symbolHeight(symbol);
  const int cols = symbolWidth(symbol);

  EntitySymbol rotated(cols, std::vector<wchar_t>(rows));
  for (int col = 0; col < cols; ++col)
  {
    for (int row = 0; row < rows; ++row)
    {
      rotated[col][row] = symbol[rows - 1 - row][col];
    }
  }
  return rotated;
}

EntitySymbol orientedSymbol(const EntitySymbol& canonical,
                            Orientation orientation)
{
  // rows may be ragged straight out of a room/enemy loader -- rectangularize
  // before rotating or indexing so every other footprint helper can assume a
  // uniform row width.
  const EntitySymbol rectangular = padToRectangle(canonical);
  if (orientation == Orientation::Vertical)
  {
    return rotateSymbol90(rectangular);
  }
  return rectangular;
}

Coordinate centerOffset(const EntitySymbol& oriented)
{
  return Coordinate((symbolWidth(oriented) - 1) / 2,
                    (symbolHeight(oriented) - 1) / 2);
}

Coordinate originFromAnchor(Coordinate anchor, const EntitySymbol& oriented)
{
  return anchor - centerOffset(oriented);
}

Coordinate anchorFromOrigin(Coordinate origin, const EntitySymbol& oriented)
{
  return origin + centerOffset(oriented);
}

std::vector<Coordinate> footprintTiles(Coordinate origin,
                                       const EntitySymbol& oriented)
{
  std::vector<Coordinate> tiles;
  for (int row = 0; row < symbolHeight(oriented); ++row)
  {
    for (int col = 0; col < symbolWidth(oriented); ++col)
    {
      if (oriented[row][col] != L'\0')
      {
        tiles.push_back(origin + Coordinate(col, row));
      }
    }
  }
  return tiles;
}

bool occupies(Coordinate origin, const EntitySymbol& oriented,
              Coordinate target)
{
  const Coordinate relative = target - origin;
  if (relative.x < 0 || relative.y < 0 || relative.x >= symbolWidth(oriented) ||
      relative.y >= symbolHeight(oriented))
  {
    return false;
  }
  return oriented[relative.y][relative.x] != L'\0';
}

bool isAdjacentToFootprint(Coordinate origin, const EntitySymbol& oriented,
                           Coordinate target)
{
  if (occupies(origin, oriented, target))
  {
    return false;
  }

  return std::any_of(
      ALL_DIRECTIONS.begin(), ALL_DIRECTIONS.end(), [&](Direction direction) {
        return occupies(origin, oriented, target + toOffset(direction));
      });
}

bool isWithinRangeOfFootprint(Coordinate origin, const EntitySymbol& oriented,
                              Coordinate target, int range)
{
  const std::vector<Coordinate> tiles = footprintTiles(origin, oriented);
  return std::any_of(tiles.begin(), tiles.end(), [&](Coordinate tile) {
    return std::max(std::abs(tile.x - target.x), std::abs(tile.y - target.y)) <=
           range;
  });
}
