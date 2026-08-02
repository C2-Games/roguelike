#ifndef ROOM_LIBRARY_H
#define ROOM_LIBRARY_H

#include <filesystem>
#include <random>
#include <vector>

class RoomLibrary {
 public:
  /**
   * @brief Scan the given directory for room files.
   *
   * @param dir Directory to scan (e.g. "assets/rooms").
   * @throws std::runtime_error if the directory does not exist or contains
   *         no room files.
   */
  void scan(const std::filesystem::path& dir);

  /** @brief True when no room templates have been discovered. */
  bool empty() const { return paths_.empty(); }

  /** @brief Number of discovered room templates. */
  std::size_t size() const { return paths_.size(); }

  /**
   * @brief Pick a random template path using the supplied RNG.
   *
   * @param rng Random engine used for the uniform pick.
   * @return Reference to a path in the internal store.
   */
  const std::filesystem::path& pickRandom(std::mt19937& rng) const;

 private:
  std::vector<std::filesystem::path> paths_;
};

#endif
