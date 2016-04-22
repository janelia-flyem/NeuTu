#include "zwidgetfactory.h"
#include <QDialog>
#include <QFrame>
#include "zclickablelabel.h"
#include "znumericparameter.h"
#include "widgets/zcolorlabel.h"

ZWidgetFactory::ZWidgetFactory()
{
}

ZLabeledEditWidget* ZWidgetFactory::MakeLabledEditWidget(
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

QSpacerItem *ZWidgetFactory::MakeHSpacerItem()
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

ZClickableColorLabel* ZWidgetFactory::MakeClickableColorLabel(
    const QColor &color, const QString &text, int width,
    bool clickable, QWidget *parent)
{
  ZVec4Parameter *colorVec = new ZVec4Parameter(
        QString("%1").arg(text), glm::vec4(
          color.redF(), color.greenF(), color.blueF(), color.alphaF()),
        glm::vec4(0.f), glm::vec4(1.f), parent);

  ZClickableColorLabel *labelWidget = new ZClickableColorLabel(colorVec);
  labelWidget->setWidth(width);
  labelWidget->setHeight(16);
  labelWidget->setClickable(clickable);

  return labelWidget;
}

ZColorLabel* ZWidgetFactory::MakeColorLabel(
    const QColor &color, const QString &text, int width, bool clickable,
    QWidget *parent)
{
  ZColorLabel *labelWidget = new ZColorLabel(parent);
  labelWidget->setColor(color);
  labelWidget->setText(text);
  labelWidget->setWidth(width);
  labelWidget->setHeight(16);
  labelWidget->setClickable(clickable);

  return labelWidget;
}
