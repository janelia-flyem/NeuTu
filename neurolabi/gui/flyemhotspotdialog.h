#ifndef FLYEMHOTSPOTDIALOG_H
#define FLYEMHOTSPOTDIALOG_H

#include <QDialog>
#include "flyem/zhotspot.h"

namespace Ui {
class FlyEmHotSpotDialog;
}

class FlyEmHotSpotDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmHotSpotDialog(QWidget *parent = 0);
  ~FlyEmHotSpotDialog();

  enum EHotSpotType {
    FALSE_MERGE, FALSE_SPLIT
  };

  int getBodyId() const;
  EHotSpotType getType() const;

private:
  Ui::FlyEmHotSpotDialog *ui;
};

#endif // FLYEMHOTSPOTDIALOG_H
