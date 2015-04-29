#ifndef FLYEMSPLITCONTROLFORM_H
#define FLYEMSPLITCONTROLFORM_H

#include <QWidget>

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

private:
  void setupWidgetBehavior();

private:
  Ui::FlyEmSplitControlForm *ui;
};

#endif // FLYEMSPLITCONTROLFORM_H
