#ifndef RENDER_STATE_BUILDER_H
#define RENDER_STATE_BUILDER_H

#include <memory>
#include <vector>

#include "io/output/render_state.h"

class Level;
class Player;
class Projectile;

namespace render_state_builder
{

// snapshots the live game object graph into a stateless RenderState for the
// io/output/ render layers to consume.
RenderState build(const Player& player, const Level& level,
                  const std::vector<std::unique_ptr<Projectile>>& projectiles,
                  double fps, bool playerHitFlashActive);

}  // namespace render_state_builder

#endif
