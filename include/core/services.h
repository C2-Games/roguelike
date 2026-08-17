#ifndef SERVICES_H
#define SERVICES_H

#include <random>

struct GameServices {
  std::mt19937 rng;

  /**
   * @brief Seed the RNG.
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
