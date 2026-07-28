#ifndef SERVICES_H
#define SERVICES_H

#include <random>

/**
 * @brief Shared cross-cutting services threaded through the game.
 *
 * Currently exposes a single Mersenne Twister random engine used by room
 * selection and enemy spawn placement. Passing this by reference from Game
 * down to consumers replaces ambient std::rand() calls, so seeds are
 * explicit and runs can be reproduced by pinning the seed.
 *
 * Non-copyable to prevent accidental forking of the RNG stream.
 */
struct GameServices {
  std::mt19937 rng;

  /**
   * @brief Seed the RNG. Typical seed is `std::time(nullptr)` at startup.
   *
   * @param seed Initial RNG seed.
   */
  explicit GameServices(std::mt19937::result_type seed) : rng(seed) {}

  GameServices(const GameServices&) = delete;
  GameServices& operator=(const GameServices&) = delete;
  GameServices(GameServices&&) = default;
  GameServices& operator=(GameServices&&) = default;
};

#endif
