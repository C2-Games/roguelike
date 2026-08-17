#ifndef ENEMY_CATALOG_H
#define ENEMY_CATALOG_H

#include <filesystem>
#include <map>
#include <string>

#include "entities/fov.h"

/** @brief Resolved per-tier stats for one enemy type. */
struct EnemyTierAttributes {
  char symbol;
  int health;
  int attackDamage;
  FOV fov;
  int chaseMemoryDuration;
  int speed;
};

/**
 * @brief Loads every enemy JSON file in a directory into an in-memory lookup
 * of type name -> tier -> resolved attributes.
 */
class EnemyCatalog {
 public:
  /**
   * @brief Load and parse every enemy definition in `dir`.
   *
   * @param dir Directory containing enemy *.json files (e.g. assets/enemies).
   * @throws std::runtime_error if the directory is missing/empty or a file
   *         fails to parse.
   */
  explicit EnemyCatalog(const std::filesystem::path& dir);

  /**
   * @brief Look up a type/tier combination.
   *
   * @param type Enemy type name (e.g. "goblin").
   * @param tier Tier number (e.g. 1 for "tier_1").
   * @return Pointer to the resolved attributes, or nullptr if not found.
   */
  const EnemyTierAttributes* find(const std::string& type, int tier) const;

 private:
  std::map<std::string, std::map<int, EnemyTierAttributes>> types_;

  void loadFile(const std::filesystem::path& path);
};

#endif
