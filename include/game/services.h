#ifndef SERVICES_H
#define SERVICES_H

#include <random>

struct GameServices
{
  std::mt19937 rng;          // spawn/level generation.
  std::mt19937 movementRng;  // enemy movement (tiebreak shuffle, wander pick).

  /**
   * @brief Seed both RNG streams.
   *
   * @param seed Initial seed. movementRng is offset by one so its stream
   * doesn't start identical to rng's.
   */
  explicit GameServices(std::mt19937::result_type seed)
      : rng(seed), movementRng(seed + 1)
  {}

  GameServices(const GameServices&) = delete;
  GameServices& operator=(const GameServices&) = delete;
  GameServices(GameServices&&) = default;
  GameServices& operator=(GameServices&&) = default;
};

#endif
