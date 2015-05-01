#ifndef FLYEMSPLITCONTROLFORM_H
#define FLYEMSPLITCONTROLFORM_H

#include <QWidget>
#include "zflyembodysplitproject.h"

namespace Ui {
class FlyEmSplitControlForm;
}

class FlyEmSplitControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmSplitControlForm(QWidget *parent = 0);
  ~FlyEmSplitControlForm();

signals:
  void exitingSplit();
  void quickViewTriggered();

private slots:
  void slotTest();
  void setSplit(int64_t bodyId);

private:
  void setupWidgetBehavior();

private:
  Ui::FlyEmSplitControlForm *ui;
  ZFlyEmBodySplitProject m_project;
};

#endif // FLYEMSPLITCONTROLFORM_H
