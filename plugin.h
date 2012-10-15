// Andrew Naplavkov

#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QString>

#include <qgisplugin.h>

class QAction;
class QgisInterface;

class Plugin: public QObject, public QgisPlugin {
  Q_OBJECT
  QgisInterface* mIface;
  QAction* mAction;

private slots:
  void run();

public:
  Plugin
    ( QgisInterface* theQgisInterface
    , const QString& sName
    , const QString& sDescription
    , const QString& sCategory
    , const QString& sVersion
    , const QgisPlugin::PLUGINTYPE sType
    );
  void initGui();
  void unload();
};

#endif
