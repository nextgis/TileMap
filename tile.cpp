// Andrew Naplavkov

#include <cmath>

#include "tile.h"

// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Tile_numbers_to_lon..2Flat.
QgsPoint Tile::toPoint() const
{
  const double Pi = 3.1415926535897932384;
  const double n(std::pow(2., double(z)));
  const double lon_deg = double(x) / n * 360.0 - 180.0;
  const double lat_rad = std::atan(std::sinh(Pi * (1. - 2. * double(y) / n)));
  const double lat_deg = lat_rad * 180.0 / Pi;
  return QgsPoint(lon_deg, lat_deg);
}

QgsRectangle Tile::toRect() const
{
  return QgsRectangle( this->toPoint(), Tile(x + 1, y + 1, z).toPoint() );
}
