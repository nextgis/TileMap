// Andrew Naplavkov

#ifndef THREAD_H
#define THREAD_H

#include <QFileInfo>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QTime>

#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>
#include <qgsrectangle.h>

#include "qzipwriter_p.h"
#include "tile.h"

class Thread : public QThread {
  Q_OBJECT

  QgsMapCanvas mCanvas;
  const QgsRectangle mRect;
  const int mMinZ;
  const int mMaxZ;
  QPixmap mPixmap;
  QFileInfo mFileInfo;
  QZipWriter mZip;

  QgsCoordinateTransform mTransform;
  bool mStop;
  qint64 mTotal;
  qint64 mCounter;
  QTime mTime;

  void estimate(const Tile& tile);
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
    , const QgsRectangle& rect
    , int minZ
    , int maxZ
    , int pixelsPerSide
    , const QFileInfo& fileInfo
    );
};

#endif
