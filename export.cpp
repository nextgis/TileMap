// Andrew Naplavkov

#include <qgisinterface.h>

#include "plugin.h"

static const QString sName = QObject::tr( "Tile map plugin" );
static const QString sDescription = QObject::tr( "A plugin to export tile map" );
static const QString sCategory = QObject::tr( "Raster" );
static const QString sIcon = ":/plugins/tile_map/tile_map.svg";
static const QString sVersion = QObject::tr( "0.1" );
static const QgisPlugin::PLUGINTYPE sType = QgisPlugin::UI;

QGISEXTERN QgisPlugin* classFactory(QgisInterface* theQgisInterface)
{
  return new Plugin(theQgisInterface, sName, sDescription, sCategory, sVersion, sType);
}

QGISEXTERN QString name()
{
  return sName;
}

QGISEXTERN QString description()
{
  return sDescription;
}

QGISEXTERN QString category()
{
  return sCategory;
}

QGISEXTERN QString version()
{
  return sVersion;
}

QGISEXTERN int type()
{
  return sType;
}

QGISEXTERN QString icon()
{
  return sIcon;
}

QGISEXTERN void unload(QgisPlugin* thePlugin)
{
  delete thePlugin;
}
