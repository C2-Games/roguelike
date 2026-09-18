#ifndef SERVICES_H
#define SERVICES_H

#include <random>

struct GameServices
{
  std::mt19937 rng;          // spawn/level generation.
  std::mt19937 movementRng;  // enemy movement (tiebreak shuffle, wander pick).
  std::mt19937 combatRng;    // crit-tier Bernoulli trials.

  /**
   * @brief Seed all three RNG streams.
   *
   * @param seed Initial seed. movementRng and combatRng are offset so their
   * streams don't start identical to rng's or each other's.
   */
  explicit GameServices(std::mt19937::result_type seed)
      : rng(seed), movementRng(seed + 1), combatRng(seed + 2)
  {}

  GameServices(const GameServices&) = delete;
  GameServices& operator=(const GameServices&) = delete;
  GameServices(GameServices&&) = default;
  GameServices& operator=(GameServices&&) = default;
};

#endif
