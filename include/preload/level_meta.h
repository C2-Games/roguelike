#ifndef LEVEL_META_H
#define LEVEL_META_H

#include <string>

// level-wide metadata, parsed from level.json.
struct LevelMeta
{
  int id = 0;
  std::string name;
  std::string description;
  int roomCount = 0;
  int startRoomID = 0;
  int bossRoomID = 0;
};

#endif
