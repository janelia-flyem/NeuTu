#ifndef TILEMANAGERDIALOG_H
#define TILEMANAGERDIALOG_H

#include <QDialog>

class ZTileManager;

class ZQtBarProgressReporter;

namespace Ui {
class TileManagerDialog;
}

class TileManagerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit TileManagerDialog(QWidget *parent = 0);
  ~TileManagerDialog();

  void setTileManager(ZTileManager *manager);

private:
  Ui::TileManagerDialog *ui;
  ZQtBarProgressReporter *m_progressReporter;
};

#endif // TILEMANAGERDIALOG_H
