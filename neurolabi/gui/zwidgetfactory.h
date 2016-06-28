#ifndef ZWIDGETFACTORY_H
#define ZWIDGETFACTORY_H

#include <QWidget>

#include "zlabelededitwidget.h"
#include "zbuttonbox.h"

class QDialog;
class QFrame;
class ZClickableColorLabel;
class ZColorLabel;

class ZWidgetFactory
{
public:
  ZWidgetFactory();

  enum ESpacerOption {
    SPACER_RIGHT, SPACER_NONE
  };

  static ZLabeledEditWidget* MakeLabledEditWidget(
      const QString &label, ESpacerOption spacerOption, QWidget *parentWidget);
  static ZButtonBox* makeButtonBox(ZButtonBox::TRole role, QWidget *parent);
  static ZButtonBox* makeButtonBox(ZButtonBox::TRole role, QDialog *parent);
  static QFrame* MakeHorizontalLine(QWidget *parentWidget);
  static ZClickableColorLabel* MakeClickableColorLabel(
      const QColor &color, const QString &text, int width, bool clickable,
      QWidget *parent);

  static ZColorLabel* MakeColorLabel(
      const QColor &color, const QString &text, int width, bool clickable,
      QWidget *parent);

  static QSpacerItem *MakeHSpacerItem();

};

#endif // ZWIDGETFACTORY_H
