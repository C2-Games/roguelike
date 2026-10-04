#ifndef ENTITY_SYMBOL_H
#define ENTITY_SYMBOL_H

#include <vector>

#include "objects/coordinate.h"

using EntitySymbol = std::vector<std::vector<wchar_t>>;

enum class Orientation
{
  Horizontal,
  Vertical
};

/**
 * @brief Get the width of a symbol grid.
 *
 * @param symbol Symbol grid to measure.
 * @return Number of columns in the grid's first row, or 0 if empty.
 */
int symbolWidth(const EntitySymbol& symbol);

/**
 * @brief Get the height of a symbol grid.
 *
 * @param symbol Symbol grid to measure.
 * @return Number of rows in the grid.
 */
int symbolHeight(const EntitySymbol& symbol);

/**
 * @brief Check whether a symbol grid is a single cell.
 *
 * @param symbol Symbol grid to check.
 * @return True if the grid is exactly 1x1.
 */
bool isSingleCell(const EntitySymbol& symbol);

/**
 * @brief Pad ragged rows to a rectangle.
 *
 * @param symbol Symbol grid whose rows may differ in length.
 * @return A copy of `symbol` with every row padded on the right with
 * L'\0' up to the widest row's length.
 */
EntitySymbol padToRectangle(const EntitySymbol& symbol);

/**
 * @brief Rotate a symbol grid 90 degrees clockwise.
 *
 * @param symbol Canonical (Horizontal-authored) symbol grid to rotate.
 * @return A new grid with `symbol` rotated 90 degrees clockwise; width and
 * height are swapped.
 */
EntitySymbol rotateSymbol90(const EntitySymbol& symbol);

/**
 * @brief Derive the display/footprint grid for a given orientation.
 *
 * @param canonical Always-Horizontal-authored symbol grid, possibly ragged;
 * never mutated.
 * @param orientation Orientation to derive the grid for.
 * @return `canonical` rectangularized (via padToRectangle) and left
 * unchanged for Orientation::Horizontal, or rotated 90 degrees clockwise for
 * Orientation::Vertical.
 */
EntitySymbol orientedSymbol(const EntitySymbol& canonical,
                            Orientation orientation);

/**
 * @brief Get the offset from a grid's top-left corner to its anchor cell.
 *
 * @param oriented Oriented symbol grid to measure.
 * @return The offset to the footprint's center cell, floored top-left on
 * even dimensions.
 */
Coordinate centerOffset(const EntitySymbol& oriented);

/**
 * @brief Derive a grid's top-left origin from its anchor position.
 *
 * @param anchor Absolute position of the footprint's anchor cell.
 * @param oriented Oriented symbol grid the anchor belongs to.
 * @return The absolute position of the grid's top-left corner.
 */
Coordinate originFromAnchor(Coordinate anchor, const EntitySymbol& oriented);

/**
 * @brief Derive a grid's anchor position from its top-left origin.
 *
 * @param origin Absolute position of the grid's top-left corner.
 * @param oriented Oriented symbol grid the origin belongs to.
 * @return The absolute position of the footprint's anchor cell.
 */
Coordinate anchorFromOrigin(Coordinate origin, const EntitySymbol& oriented);

/**
 * @brief List every tile a footprint occupies.
 *
 * @param origin Absolute position of the grid's top-left corner.
 * @param oriented Oriented symbol grid to read.
 * @return Absolute coordinates of every non-null cell in `oriented`.
 */
std::vector<Coordinate> footprintTiles(Coordinate origin,
                                       const EntitySymbol& oriented);

/**
 * @brief Check whether a footprint occupies a given tile.
 *
 * @param origin Absolute position of the grid's top-left corner.
 * @param oriented Oriented symbol grid to check.
 * @param target Tile to test.
 * @return True if `target` maps to a non-null cell in `oriented`.
 */
bool occupies(Coordinate origin, const EntitySymbol& oriented,
              Coordinate target);

/**
 * @brief Check whether a tile is adjacent to a footprint without being part
 * of it.
 *
 * @param origin Absolute position of the grid's top-left corner.
 * @param oriented Oriented symbol grid to check against.
 * @param target Tile to test.
 * @return True if `target` is not itself part of the footprint, and at
 * least one of its cardinal neighbors is.
 */
bool isAdjacentToFootprint(Coordinate origin, const EntitySymbol& oriented,
                           Coordinate target);

#endif
