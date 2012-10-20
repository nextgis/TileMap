// Andrew Naplavkov

#include <algorithm>
#include <cmath>
#include <QDir>
#include <QBuffer>
#include <QByteArray>

#include <qgsmaplayer.h>
#include <qgsmaprenderer.h>

#include "thread.h"

static const QString sRoot = QObject::tr("Mapnik");

Thread::Thread
  ( QObject* parent
  , QgsMapCanvas* ifaceCanvas
  , const QgsRectangle& rect
  , int minZ
  , int maxZ
  , int pixelsPerSide
  , const QFileInfo& fileInfo
  )
  : QThread(parent)
  , mRect(rect)
  , mMinZ(minZ)
  , mMaxZ(maxZ)
  , mPixmap(pixelsPerSide, pixelsPerSide)
  , mFileInfo(fileInfo)
  , mZip(fileInfo.isDir()? QString(): fileInfo.absoluteFilePath())
  , mStop(false)
  , mTotal(0)
  , mCounter(0)
{
  QList<QgsMapLayer*> layers = ifaceCanvas->layers();
  QList<QgsMapCanvasLayer> canvasLayers;
  for (int i(0); i < layers.size(); ++i)
    canvasLayers.push_back(QgsMapCanvasLayer(layers[i]));
  mCanvas.setLayerSet(canvasLayers);

  QgsCoordinateReferenceSystem latlon;
  latlon.createFromProj4("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"); // http://spatialreference.org/ref/epsg/4326/
  QgsCoordinateReferenceSystem mercator;
  mercator.createFromProj4("+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"); // http://spatialreference.org/ref/epsg/3395/

  mTransform.setSourceCrs(latlon);
  mTransform.setDestCRS(mercator);

  QgsMapRenderer* renderer(mCanvas.mapRenderer());
  renderer->setOutputSize(mPixmap.size(), mPixmap.logicalDpiX());
  renderer->setDestinationCrs(mercator);
  renderer->setProjectionsEnabled(true);
}

void Thread::estimate(const Tile& tile)
{
  if (mStop || !mRect.intersects(tile.toRect()))
    return;

  if (mMinZ <= tile.z && tile.z <= mMaxZ)
    ++mTotal;

  if (mTime.elapsed() > 1000)
  {
    emit signalProcess(QString(tr("Counting tiles: %1")).arg(mTotal), 0);
    mTime.restart();
  }

  // in-depth recursion
  if (tile.z < mMaxZ)
    for (int x(2 * tile.x); x <= (2 * tile.x + 1); ++x)
      for (int y(2 * tile.y); y <= (2 * tile.y + 1); ++y)
      {
        Tile subTile(x, y, tile.z + 1);
        estimate(subTile);
      }
}

void Thread::render(const Tile& tile)
{
  const QgsRectangle rect(tile.toRect());
  if (mStop || !mRect.intersects(rect))
    return;

  if (mMinZ <= tile.z && tile.z <= mMaxZ)
  {
    mCanvas.mapRenderer()->setExtent( mTransform.transform( rect ) );
    mCanvas.refresh();
    mPixmap.fill();
    QPainter painter;
    painter.begin(&mPixmap);
    mCanvas.mapRenderer()->render( &painter );
    painter.end();

    const QString path(QString("%1/%2/%3").arg(sRoot).arg(tile.z).arg(tile.x));
    if (mFileInfo.isDir())
    {
      QDir().mkpath(QString("%1/%2").arg(mFileInfo.absoluteFilePath()).arg(path));
      mPixmap.save(QString("%1/%2/%3.png").arg(mFileInfo.absoluteFilePath()).arg(path).arg(tile.y), "PNG");
    }
    else
    {
      QByteArray bytes;
      QBuffer buffer(&bytes);
      buffer.open(QIODevice::WriteOnly);
      mPixmap.save(&buffer, "PNG");
      mZip.addDirectory(path);
      mZip.addFile(QString("%1/%2.png").arg(path).arg(tile.y), bytes);
    }
  }

  ++mCounter;
  if (mTime.elapsed() > 1000)
  {
    emit signalProcess(QString(tr("Tiles: %1 of %2")).arg(mCounter).arg(mTotal), (mCounter * 100) / mTotal);
    mTime.restart();
  }

  // in-depth recursion
  if (tile.z < mMaxZ)
    for (int x(2 * tile.x); x <= (2 * tile.x + 1); ++x)
      for (int y(2 * tile.y); y <= (2 * tile.y + 1); ++y)
      {
        Tile subTile(x, y, tile.z + 1);
        render(subTile);
      }
}

bool removeDir(const QString& dirName)
{
  bool result(true);
  QDir dir(dirName);

  if (dir.exists(dirName))
  {
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
      if (info.isDir())
        result = removeDir(info.absoluteFilePath());
      else
        result = QFile::remove(info.absoluteFilePath());

      if (!result)
        return result;
    }
    result = dir.rmdir(dirName);
  }

  return result;
}

void Thread::run()
{
  mTime.start();
  if (!mFileInfo.isDir() || removeDir(QString("%1/%2").arg(mFileInfo.absoluteFilePath()).arg(sRoot)))
  {
    mCounter = 0;
    mTotal = 0;
    estimate(Tile());
    render(Tile());
    emit signalProcess(tr("Done"), 100);
  }
  emit signalFinish();
}
