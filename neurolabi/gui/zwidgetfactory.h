#ifndef ZWIDGETFACTORY_H
#define ZWIDGETFACTORY_H

#include <QWidget>

#include "zlabelededitwidget.h"
#include "zbuttonbox.h"

class QDialog;
class QFrame;

class ZWidgetFactory
{
public:
  ZWidgetFactory();

  enum ESpacerOption {
    SPACER_RIGHT, SPACER_NONE
  };

  static ZLabeledEditWidget* makeLabledEditWidget(
      const QString &label, ESpacerOption spacerOption, QWidget *parentWidget);
  static ZButtonBox* makeButtonBox(ZButtonBox::TRole role, QWidget *parent);
  static ZButtonBox* makeButtonBox(ZButtonBox::TRole role, QDialog *parent);
  static QFrame* MakeHorizontalLine(QWidget *parentWidget);

  static QSpacerItem *makeHSpacerItem();

};

#endif // ZWIDGETFACTORY_H
