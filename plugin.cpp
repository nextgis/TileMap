// Andrew Naplavkov

#include <QAction>
#include <QLocale>
#include <QCoreApplication>

#include <qgisinterface.h>
#include <qgsmaplayer.h>

#include "dialog.h"
#include "plugin.h"
#include "thread.h"

static const QString sAction = QObject::tr("Tile map");

Plugin::Plugin
  ( QgisInterface* theQgisInterface
  , const QString& sName
  , const QString& sDescription
  , const QString& sCategory
  , const QString& sVersion
  , const QgisPlugin::PLUGINTYPE sType
  )
  : QgisPlugin(sName, sDescription, sCategory, sVersion, sType)
  , mIface(theQgisInterface)
  , mAction(0)
{
  QSettings settings;
  QTranslator *translator = new QTranslator( this );
  //QString locale = QLocale::system().name();
  QString locale = settings.value( "locale/userLocale", QLocale::system().name() ).toString();

  QString loc = locale.split( "_" ).at( 0 );
  bool bTranslationLoaded = translator->load( ":/plugins/tile_map/i18n/tile_map_" + loc + ".qm" );
  //bool bTranslationLoaded = translator->load( ":/plugins/tile_map/i18n/tile_map_ru.qm" );
  if ( bTranslationLoaded )
  {
    QCoreApplication::installTranslator( translator );
  }
}

void Plugin::initGui()
{
  mAction = new QAction(QIcon(":/plugins/tile_map/tile_map.svg"), sAction, this);
  connect(mAction, SIGNAL(activated()), this, SLOT(run()));
  mIface->addRasterToolBarIcon(mAction);
  mIface->addPluginToRasterMenu(sAction, mAction);
}

void Plugin::unload()
{
  mIface->removeRasterToolBarIcon(mAction);
  mIface->removePluginRasterMenu(sAction, mAction);
  delete mAction;
}

void Plugin::run()
{
  Dialog dialog(name(), mIface->mapCanvas());
  dialog.exec();
}
