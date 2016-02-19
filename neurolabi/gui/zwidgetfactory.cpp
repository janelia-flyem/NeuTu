#include "zwidgetfactory.h"
#include <QDialog>
#include <QFrame>

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
  ZButtonBox *buttonBox = makeButtonBox(role, qobject_cast<QWidget*>(parent));

  bool ok =
      QObject::connect(buttonBox, SIGNAL(clickedYes()), parent, SLOT(accept()));
  Q_ASSERT(ok);

  ok = QObject::connect(buttonBox, SIGNAL(clickedNo()), parent, SLOT(reject()));
  Q_ASSERT(ok);

  return buttonBox;
}

QSpacerItem *ZWidgetFactory::makeHSpacerItem()
{
  return new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
}

QFrame* ZWidgetFactory::MakeHorizontalLine(QWidget *parentWidget)
{
  QFrame *frame = new QFrame(parentWidget);
  frame->setFrameShape(QFrame::HLine);
  frame->setFrameShadow(QFrame::Raised);
  frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  return frame;
}

