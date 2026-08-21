#include "io/output/layers/map_layer.h"

#include <ncurses.h>

#include "io/output/colors.h"
#include "io/output/render_state.h"
#include "io/output/window_position.h"
#include "world/map/room.h"

MapLayer::MapLayer(int h, int w, int y, int x) : RenderStack(h, w, y, x) {}

void MapLayer::drawMap(const RenderState& state)
{
  for (int x = 0; x < Room::WIDTH; ++x)
  {
    for (int y = 0; y < Room::HEIGHT; ++y)
    {
      const TileView& tile = state.tiles[x][y];

      // 3-state fog of war:
      //   visible   -> normal render (terminal default colours).
      //   explored  -> light grey glyph on grey background for non-blank
      //                tile symbols (walls, doors, floor).
      //                Result: the whole explored area reads as a uniform
      //                grey shade with walls/doors/floor picked out on top.
      //   unseen    -> solid dark grey block so the room shape stays
      //                hidden until first sighting.
      //
      // Hook: door tiles could branch here on tile.getType() == Door and
      // OR in colorAttr(ColorPair::DoorDefault) once the pair is defined.
      if (tile.visibility == TileVisibility::Visible)
      {
        mvwaddch(win_, y, x, tile.symbol);
      }
      else if (tile.visibility == TileVisibility::Explored)
      {
        mvwaddch(win_, y, x, tile.symbol | colorAttr(ColorPair::FogExplored));
      }
      else
      {
        // Fog block: ncurses overlay() drops blanks, so we must draw a
        // non-blank glyph. Because the pair has fg == bg, whatever glyph
        // we pick is invisible against its own background and only the
        // solid grey cell shows through. '.' is arbitrary.
        mvwaddch(win_, y, x, '.' | colorAttr(ColorPair::FogUnexplored));
      }
    };
  };
};

void MapLayer::doRender(const RenderState& state)
{
  werase(win_);  // need to erase each frame.

  // boarder around map.
  box(win_, 0, 0);

  // draw map.
  this->drawMap(state);
};

void MapLayer::onResize(int termHeight, int termWidth)
{
  WindowPosition g = centerWindow(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
