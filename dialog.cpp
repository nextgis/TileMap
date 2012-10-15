// Andrew Naplavkov

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaprenderer.h>
#include <qgsrectangle.h>

#include "dialog.h"
#include "tile.h"
#include "ui_dialog.h"

static const QString sErrorSuffix = QObject::tr(" error");
static const QString sExampleZip = QObject::tr("example.zip");

Dialog::Dialog
  ( const QString& sName
  , QgsMapCanvas* mCanvas
  )
  : ui(new Ui::Dialog)
  , mName(sName)
  , mCanvas(mCanvas)
  , mThread(0)
{
  ui->setupUi(this);
  setWindowTitle(mName);
  QgsMapLayer* currentLayer(mCanvas->currentLayer());
  for (int i(0); i < mCanvas->layerCount(); ++i)
  {
    QgsMapLayer* layer(mCanvas->layer(i));
    ui->layersComboBox->addItem(layer->name());
    if (layer == currentLayer)
      ui->layersComboBox->setCurrentIndex(i);
  }

  QSettings settings;
  if (settings.value(QString("%1/%2").arg(mName).arg(ui->tileDirectoryRadioButton->text()), false).toBool())
    ui->tileDirectoryRadioButton->setChecked(true);
  QString target = settings.value(QString("%1/%2").arg(mName).arg(ui->targetGroupBox->title()), QString()).toString();
  QFileInfo file
    = target.isEmpty()
    ? QFileInfo(QDir::currentPath(), sExampleZip)
    : ui->zippedTilesRadioButton->isChecked()
    ? QFileInfo(target)
    : QFileInfo(target, sExampleZip)
    ;
  ui->zippedTilesLineEdit->setText(file.absoluteFilePath());
  ui->tileDirectoryLineEdit->setText(file.absolutePath());
  if (settings.value(QString("%1/%2").arg(mName).arg(ui->fullExtentRadioButton->text()), false).toBool())
    ui->fullExtentRadioButton->setChecked(true);
  else
  {
    const QString name(settings.value(QString("%1/%2").arg(mName).arg(ui->layerExtentRadioButton->text()), QString()).toString());
    if (!name.isEmpty())
    {
      ui->layerExtentRadioButton->setChecked(true);
      const int idx = ui->layersComboBox->findText(name);
      if (idx >= 0)
        ui->layersComboBox->setCurrentIndex(idx);
    }
  }
  ui->maximumZoomSpinBox->setValue(settings.value(QString("%1/%2").arg(mName).arg(ui->maximumZoomLabel->text()), 18).toInt());
  ui->minimumZoomSpinBox->setValue(settings.value(QString("%1/%2").arg(mName).arg(ui->minimumZoomLabel->text()), 0).toInt());
  ui->pixelsPerSideSpinBox->setValue(settings.value(QString("%1/%2").arg(mName).arg(ui->pixelsPerSideLabel->text()), 256).toInt());
}

Dialog::~Dialog()
{
  emit signalStop();
  on_finish();
  delete ui;
}

void Dialog::on_minimumZoomSpinBox_valueChanged(int z)
{
  ui->maximumZoomSpinBox->setMinimum(z);
}

void Dialog::on_maximumZoomSpinBox_valueChanged(int z)
{
  ui->minimumZoomSpinBox->setMaximum(z);
}

void Dialog::on_layerExtentRadioButton_toggled(bool checked)
{
  ui->layersComboBox->setEnabled(checked);
}

void Dialog::on_exportPushButton_clicked()
{
  QFileInfo fileInfo(ui->zippedTilesRadioButton->isChecked()? ui->zippedTilesLineEdit->text(): ui->tileDirectoryLineEdit->text());

  QgsRectangle extent;
  if (ui->currentExtentRadioButton->isChecked())
    extent = mCanvas->extent();
  else if (ui->fullExtentRadioButton->isChecked())
    extent = mCanvas->fullExtent();
  else
  {
    const int i = ui->layersComboBox->currentIndex();
    if (i < 0)
    {
      QMessageBox::critical(this, mName, ui->layerExtentRadioButton->text() + sErrorSuffix);
      return;
    }
    QgsMapLayer* layer(mCanvas->layer(i));
    extent = mCanvas->mapRenderer()->layerExtentToOutputExtent(layer, layer->extent());
  }
  QgsCoordinateReferenceSystem latlon;
  latlon.createFromProj4("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"); // http://spatialreference.org/ref/epsg/4326/
  extent = QgsCoordinateTransform(mCanvas->mapRenderer()->destinationCrs(), latlon).transform(extent);
  const double ArctanSinhPi = -85.051128779806592377; // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#X_and_Y
  extent = extent.intersect(&QgsRectangle(-180, -ArctanSinhPi, 180, ArctanSinhPi));

  QVector<Tile> tiles;
  const int count = int(std::pow(2., double(ui->minimumZoomSpinBox->value())));
  for (int x(0); x < count; ++x)
    for (int y(0); y < count; ++y)
    {
      Tile tile(x, y, ui->minimumZoomSpinBox->value(), ui->maximumZoomSpinBox->value());
      if (tile.toRect().intersects(extent))
        tiles.push_back(tile);
    }
  if (tiles.empty())
  {
    QMessageBox::critical(this, mName, ui->targetGroupBox->title() + sErrorSuffix);
    return;
  }
  if (tiles.size() == 1)
  {
    tiles[0] = tiles[0].zoomToRect(extent);
    if (tiles[0].maxZ < tiles[0].z)
    {
      QMessageBox::critical(this, mName, ui->maximumZoomLabel->text() + sErrorSuffix);
      return;
    }
  }

  ui->exportPushButton->setEnabled(false);
  ui->stopBushButton->setEnabled(true);
  ui->progressBar->setValue(0);
  ui->progressBar->setFormat("%p%");
  ui->progressBar->setEnabled(true);

  QSettings settings;
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->tileDirectoryRadioButton->text()), ui->tileDirectoryRadioButton->isChecked());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->targetGroupBox->title()), ui->zippedTilesRadioButton->isChecked()? ui->zippedTilesLineEdit->text(): ui->tileDirectoryLineEdit->text());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->fullExtentRadioButton->text()), ui->fullExtentRadioButton->isChecked());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->layerExtentRadioButton->text()), ui->layerExtentRadioButton->isChecked()? ui->layersComboBox->currentText(): QString());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->maximumZoomLabel->text()), ui->maximumZoomSpinBox->value());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->minimumZoomLabel->text()), ui->minimumZoomSpinBox->value());
  settings.setValue(QString("%1/%2").arg(mName).arg(ui->pixelsPerSideLabel->text()), ui->pixelsPerSideSpinBox->value());

  mThread = new Thread(this, mCanvas, tiles, ui->pixelsPerSideSpinBox->value(), fileInfo);
  connect(mThread, SIGNAL(signalProcess(QString, int)), this, SLOT(on_process(QString, int)));
  connect(mThread, SIGNAL(signalFinish()), this, SLOT(on_finish()));
  connect(this, SIGNAL(signalStop()), mThread, SLOT(on_stop()));
  mThread->start();
}

void Dialog::on_closePushButton_clicked()
{
  reject();
}

void Dialog::on_stopBushButton_clicked()
{
  emit signalStop();
}

void Dialog::on_process(QString msg, int percent)
{
  ui->progressBar->setFormat(msg);
  ui->progressBar->setValue(percent);
}

void Dialog::on_finish()
{
  if (mThread)
  {
    mThread->wait();
    delete mThread;
    mThread = 0;
  }

  ui->exportPushButton->setEnabled(true);
  ui->stopBushButton->setEnabled(false);
  ui->progressBar->setEnabled(false);
}

void Dialog::on_zippedTilesRadioButton_toggled(bool checked)
{
  ui->zippedTilesLineEdit->setEnabled(checked);
  ui->zippedTilesPushButton->setEnabled(checked);
}

void Dialog::on_tileDirectoryRadioButton_toggled(bool checked)
{
  ui->tileDirectoryLineEdit->setEnabled(checked);
  ui->tileDirectoryPushButton->setEnabled(checked);
}

void Dialog::on_zippedTilesPushButton_clicked()
{
  QFileInfo fileInfo(ui->zippedTilesLineEdit->text());
  QString dir(fileInfo.isFile()? fileInfo.absolutePath(): QDir::currentPath());
  QString target = QFileDialog::getSaveFileName
      ( parentWidget()
      , ui->zippedTilesRadioButton->text()
      , dir
      , ui->zippedTilesRadioButton->text() + QString(" (*.zip)")
      );
  if (!target.isEmpty())
    ui->zippedTilesLineEdit->setText(target);
}

void Dialog::on_tileDirectoryPushButton_clicked()
{
  QFileInfo fileInfo(ui->tileDirectoryLineEdit->text());
  QString dir(fileInfo.isDir()? fileInfo.absoluteFilePath(): QDir::currentPath());
  QString target = QFileDialog::getExistingDirectory
      ( parentWidget()
      , ui->tileDirectoryLineEdit->text()
      , dir
      );
  if (!target.isEmpty())
    ui->tileDirectoryLineEdit->setText(target);
}
