#include "world/room_library.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace fs = std::filesystem;

void RoomLibrary::scan(const fs::path& dir) {
  paths_.clear();

  if (!fs::exists(dir) || !fs::is_directory(dir)) {
    throw std::runtime_error("Room library directory does not exist: " +
                             dir.string());
  }

  for (const auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    if (entry.path().extension() != ".txt") continue;
    paths_.push_back(entry.path());
  }

  // Sort so scan order is deterministic across filesystems.
  std::sort(paths_.begin(), paths_.end());

  if (paths_.empty()) {
    throw std::runtime_error("No *.txt room files found in " + dir.string());
  }
}

const fs::path& RoomLibrary::pickRandom() const {
  // Caller is required to check empty() first.
  std::size_t idx = static_cast<std::size_t>(std::rand()) % paths_.size();
  return paths_[idx];
}
