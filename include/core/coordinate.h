#ifndef COORDINATE_H
#define COORDINATE_H

struct Coordinate {
  int x, y;

  explicit Coordinate(int x = 0, int y = 0) : x(x), y(y) {}

  Coordinate operator+(const Coordinate& other) const {
    return Coordinate(x + other.x, y + other.y);
  }

  Coordinate operator-(const Coordinate& other) const {
    return Coordinate(x - other.x, y - other.y);
  }

  bool operator==(const Coordinate& other) const {
    return x == other.x && y == other.y;
  }

  bool operator<(const Coordinate& other) const {
    if (y != other.y) return y < other.y;
    return x < other.x;
  }
};

#endif
