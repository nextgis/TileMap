// Andrew Naplavkov

#ifndef TILE_H
#define TILE_H

#include <qgspoint.h>
#include <qgsrectangle.h>

struct Tile {
  int x, y, z;
  Tile() : x(0), y(0), z(0)  {}
  Tile(int x_, int y_, int z_) : x(x_), y(y_), z(z_)  {}
  QgsPoint toPoint() const;
  QgsRectangle toRect() const;
};

#endif
