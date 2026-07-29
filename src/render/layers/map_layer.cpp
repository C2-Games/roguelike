#include "render/layers/map_layer.h"

#include <ncurses.h>

#include "core/colors.h"
#include "render/ui.h"
#include "world/map/room.h"

MapLayer::MapLayer(int h, int w, int y, int x, const RoomGraph& graph)
    : RenderStack(h, w, y, x), graph(graph) {}

void MapLayer::drawMap() {
  const Room& room = graph.getCurrentRoom();

  for (int x = 0; x < Room::WIDTH; ++x) {
    for (int y = 0; y < Room::HEIGHT; ++y) {
      // get tile reference & print it.
      const Tile& tile = room.tiles[x][y];
      const int tx = tile.getPosition().x;
      const int ty = tile.getPosition().y;

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
      if (room.isVisible(x, y)) {
        mvwaddch(win, ty, tx, tile.getSymbol());
      } else if (room.isExplored(x, y)) {
        const char sym = tile.getSymbol();
        mvwaddch(win, ty, tx, sym | colorAttr(ColorPair::FogExplored));
      } else {
        // Fog block: ncurses overlay() drops blanks, so we must draw a
        // non-blank glyph. Because the pair has fg == bg, whatever glyph
        // we pick is invisible against its own background and only the
        // solid grey cell shows through. '.' is arbitrary.
        mvwaddch(win, ty, tx, '.' | colorAttr(ColorPair::FogUnexplored));
      }
    };
  };
};

void MapLayer::doRender() {
  werase(win);  // need to erase each frame.

  // boarder around map.
  box(win, 0, 0);

  // draw map.
  this->drawMap();
};

void MapLayer::onResize(int termHeight, int termWidth) {
  UI g = computeUI(termHeight, termWidth);
  reshape(g.winHeight, g.winWidth, g.originY, g.originX);
};
