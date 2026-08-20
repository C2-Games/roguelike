#ifndef FRAME_STATE_H
#define FRAME_STATE_H

class Player;
struct Room;

// live world data for the frame currently being updated
struct FrameState
{
  const Player& player;
  const Room& currentRoom;
};

#endif
