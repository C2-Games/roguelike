#ifndef ROOM_DIMENSIONS_H
#define ROOM_DIMENSIONS_H

// fixed grid size shared by every Room. Kept in its own data-only header so
// io/ can read the dimensions without depending on the Room class itself
// (rule 6: io/ may only reach into objects/ data, never a class).
inline constexpr int ROOM_WIDTH = 175;
inline constexpr int ROOM_HEIGHT = 50;

#endif
