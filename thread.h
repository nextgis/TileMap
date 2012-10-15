// Andrew Naplavkov

#ifndef THREAD_H
#define THREAD_H

#include <QFileInfo>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QTime>
#include <QVector>

#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>

#include "qzipwriter_p.h"
#include "tile.h"

class Thread : public QThread {
  Q_OBJECT

  QString mName;
  QgsMapCanvas mCanvas;
  QVector<Tile> mTiles;

  QFileInfo mFileInfo;
  QZipWriter mZip;
  QPixmap mPixmap;
  QgsCoordinateTransform mTransform;
  bool mStop;
  qint64 mTotal;
  qint64 mCounter;
  QTime mTime;

  void render(const Tile& tile);

protected:
  void run();

signals:
  void signalProcess(QString msg, int percent);
  void signalFinish();

public slots:
  void on_stop()  { mStop = true; }

public:
  Thread
    ( QObject* parent
    , QgsMapCanvas* ifaceCanvas
    , const QVector<Tile>& tiles
    , int pixelsPerSide
    , const QFileInfo& fileInfo
    );
};

#endif
