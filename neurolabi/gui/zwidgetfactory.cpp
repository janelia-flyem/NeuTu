#include "zwidgetfactory.h"
#include <QDialog>

ZWidgetFactory::ZWidgetFactory()
{
}

ZLabeledEditWidget* ZWidgetFactory::makeLabledEditWidget(
    const QString &label, ESpacerOption spacerOption, QWidget *parentWidget)
{
  ZLabeledEditWidget *widget = new ZLabeledEditWidget(parentWidget);
  widget->setLabel(label);
  if (spacerOption == SPACER_RIGHT) {
    widget->addSpacer();
  }

  return widget;
}

ZButtonBox* ZWidgetFactory::makeButtonBox(
    ZButtonBox::TRole role, QWidget *parent)
{
  ZButtonBox *buttonBox = new ZButtonBox(parent);
  buttonBox->activate(role);

  return buttonBox;
}

ZButtonBox* ZWidgetFactory::makeButtonBox(ZButtonBox::TRole role, QDialog *parent)
{
  ZButtonBox *buttonBox = makeButtonBox(role, dynamic_cast<QWidget*>(parent));

  parent->connect(buttonBox, SIGNAL(clickedYes()), parent, SLOT(accept()));
  parent->connect(buttonBox, SIGNAL(clickedNo()), parent, SLOT(reject()));

  return buttonBox;
}

QSpacerItem *ZWidgetFactory::makeHSpacerItem()
{
  return new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
}

