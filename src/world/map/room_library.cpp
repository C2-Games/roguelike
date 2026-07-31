#include "world/map/room_library.h"

#include <algorithm>
#include <stdexcept>

#include "core/paths.h"

namespace fs = std::filesystem;

void RoomLibrary::scan(const fs::path& dir) {
  paths_.clear();

  if (!fs::exists(dir) || !fs::is_directory(dir)) {
    throw std::runtime_error("Room library directory does not exist: " +
                             dir.string());
  }

  for (const auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != paths::kRoomFileExt) continue;
    paths_.push_back(entry.path());
  }

  // Sort so scan order is deterministic across filesystems.
  std::sort(paths_.begin(), paths_.end());

  if (paths_.empty()) {
    throw std::runtime_error("No *.txt room files found in " + dir.string());
  }
}

const fs::path& RoomLibrary::pickRandom(std::mt19937& rng) const {
  // Caller is required to check empty() first.
  std::uniform_int_distribution<std::size_t> dist(0, paths_.size() - 1);
  return paths_[dist(rng)];
}
