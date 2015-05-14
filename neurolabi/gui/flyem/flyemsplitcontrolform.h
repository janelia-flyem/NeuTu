#ifndef FLYEMSPLITCONTROLFORM_H
#define FLYEMSPLITCONTROLFORM_H

#include <QWidget>
#include "zflyembodysplitproject.h"

class QMenu;

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
  void splitQuickViewTriggered();
  void bodyViewTriggered();
  void splitViewTriggered();
  void changingSplit(uint64_t);
  void savingSeed();
  void committingResult();

private slots:
  void slotTest();
  void setSplit(uint64_t bodyId);
  void changeSplit();
  void commitResult();

private:
  void setupWidgetBehavior();

private:
  Ui::FlyEmSplitControlForm *ui;
  ZFlyEmBodySplitProject m_project;
  QMenu *m_mainMenu;
};

#endif // FLYEMSPLITCONTROLFORM_H
