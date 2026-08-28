// tile_glyph.h
#ifndef PRELOAD_UTILS_TILE_GLYPH_H
#define PRELOAD_UTILS_TILE_GLYPH_H

#include "objects/tiles/tile_type.h"

namespace preload
{

/**
 * @brief Get the glyph to render for a tile type when its source cell carries
 * no box-drawing art of its own.
 *
 * @param type The tile type to look up.
 * @return The wide-char glyph for that type.
 */
wchar_t defaultGlyph(TileType type);

}  // namespace preload

#endif
