// Andrew Naplavkov

#ifndef DIALOG_H
#define DIALOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QString>

#include "thread.h"

namespace Ui  { class Dialog; }

class QgsMapCanvas;

class Dialog : public QDialog {
  Q_OBJECT
  Ui::Dialog *ui;
  QString mName;
  QgsMapCanvas* mCanvas;
  Thread* mThread;

private slots:
  void on_closePushButton_clicked();
  void on_exportPushButton_clicked();
  void on_finish();
  void on_layerExtentRadioButton_toggled(bool checked);
  void on_maximumZoomSpinBox_valueChanged(int z);
  void on_minimumZoomSpinBox_valueChanged(int z);
  void on_process(QString msg, int percent);
  void on_stopBushButton_clicked();
  void on_tileDirectoryPushButton_clicked();
  void on_tileDirectoryRadioButton_toggled(bool checked);
  void on_zippedTilesPushButton_clicked();
  void on_zippedTilesRadioButton_toggled(bool checked);

signals:
  void signalStop();

public:
  explicit Dialog
    ( const QString& sName
    , QgsMapCanvas* ifaceCanvas
    );
  virtual ~Dialog();
};

#endif
