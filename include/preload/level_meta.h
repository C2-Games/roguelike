#ifndef LEVEL_META_H
#define LEVEL_META_H

#include <string>

// level-wide metadata, parsed from level.json.
struct LevelMeta
{
  int id;
  std::string name;
  std::string description;
  int roomCount;
  int startRoomID;
  int bossRoomID;
};

#endif
