#ifndef ZNEU3SLICEVIEWDIALOG_H
#define ZNEU3SLICEVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class ZNeu3SliceViewDialog;
}

#include "neu3window.h"

class ZNeu3SliceViewDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZNeu3SliceViewDialog(QWidget *parent = 0);
  ~ZNeu3SliceViewDialog();

  Neu3Window::EBrowseMode getBrowseMode() const;

private:
  Ui::ZNeu3SliceViewDialog *ui;
};

#endif // ZNEU3SLICEVIEWDIALOG_H
