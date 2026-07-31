#ifndef ROOM_LIBRARY_H
#define ROOM_LIBRARY_H

#include <filesystem>
#include <random>
#include <vector>

/**
 * @brief Discovers and provides access to authored room template files.
 *
 * Scans a directory (typically assets/rooms/) for *.txt files at startup and
 * exposes them to RoomGraph for random selection during room generation.
 */
class RoomLibrary {
 public:
  /**
   * @brief Scan the given directory for room files.
   *
   * Only files ending in ".txt" are collected. The library does not open or
   * parse the files here; Room::loadFromFile handles that at generation time.
   *
   * @param dir Directory to scan (e.g. "assets/rooms").
   * @throws std::runtime_error if the directory does not exist or contains
   *         no room files.
   */
  void scan(const std::filesystem::path& dir);

  /**
   * @brief True when no room templates have been discovered.
   */
  bool empty() const { return paths_.empty(); }

  /**
   * @brief Number of discovered room templates.
   */
  std::size_t size() const { return paths_.size(); }

  /**
   * @brief Pick a random template path using the supplied RNG.
   *
   * Threading the engine in from the caller (typically GameServices::rng)
   * keeps room-selection reproducible when the seed is pinned. Callers must
   * ensure the library is non-empty.
   *
   * @param rng Random engine used for the uniform pick.
   * @return Reference to a path in the internal store.
   */
  const std::filesystem::path& pickRandom(std::mt19937& rng) const;

 private:
  std::vector<std::filesystem::path> paths_;
};

#endif
