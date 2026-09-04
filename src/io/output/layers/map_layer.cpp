#include "io/output/layers/map_layer.h"

#include <ncurses.h>

#include "io/output/render_state.h"
#include "io/output/window_position.h"
#include "objects/room/room_dimensions.h"

MapLayer::MapLayer(int h, int w, int y, int x) : RenderStack(h, w, y, x) {}

void MapLayer::drawMap(const MapLayerPacket& data)
{
  for (int x = 0; x < ROOM_WIDTH; ++x)
  {
    for (int y = 0; y < ROOM_HEIGHT; ++y)
    {
      const TileView& tile = data.tiles[x][y];

      // 3-state fog of war:
      //   visible   -> normal render (terminal default colours).
      //   explored  -> light grey glyph on grey background for non-blank
      //                tile symbols (walls, doors, floor).
      //                result: the whole explored area reads as a uniform
      //                grey shade with walls/doors/floor picked out on top.
      //   unseen    -> solid dark grey block so the room shape stays
      //                hidden until first sighting.
      // to tint doors here (e.g. colorAttr(ColorPair::DoorDefault)) or set
      // the locked door apart, TileView would need to carry the tile type
      // or a flag; today it carries only the glyph, and an open door is
      // recognised by its symbol being blank.
      if (tile.visibility == TileVisibility::Visible)
      {
        addWideGlyph(y, x, tile.symbol, ColorPair::Default);
      }
      else if (tile.visibility == TileVisibility::Explored)
      {
        // a blank open-door cell would be dropped by ncurses overlay(), so
        // substitute a non-blank filler to keep the fogged doorway cell. an
        // open door is the only blank-symbol tile that reaches this branch:
        // Tile::toggleReveal never marks a Void tile explored.
        const wchar_t glyph = tile.symbol == L' ' ? L'.' : tile.symbol;
        addWideGlyph(y, x, glyph, ColorPair::FogExplored);
      }
      else
      {
        // fog block: ncurses overlay() drops blanks, so we must draw a
        // non-blank glyph. because the pair has fg == bg, whatever glyph
        // we pick is invisible against its own background and only the
        // solid grey cell shows through. '.' is arbitrary.
        addWideGlyph(y, x, L'.', ColorPair::FogUnexplored);
      }
    };
  };
};

void MapLayer::doRender(const MapLayerPacket& data)
{
  werase(win_);  // need to erase each frame.

  // boarder around map.
  box(win_, 0, 0);

  // draw map.
  this->drawMap(data);
};

void MapLayer::onResize(int termHeight, int termWidth)
{
  WindowPosition g = centerWindow(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
