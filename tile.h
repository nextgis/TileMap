// Andrew Naplavkov

#ifndef TILE_H
#define TILE_H

#include <qgspoint.h>
#include <qgsrectangle.h>

struct Tile {
  int x, y, z, maxZ;
  Tile() : x(0), y(0), z(0), maxZ(18)  {}
  Tile(int x_, int y_, int z_, int maxZ_) : x(x_), y(y_), z(z_), maxZ(maxZ_)  {}
  QgsPoint toPoint() const;
  QgsRectangle toRect() const;
  Tile zoomToRect(const QgsRectangle& rect) const;
};

#endif
