#include "zpaintlabelwidget.h"
#include "zclickablelabel.h"
#include "zlabelcolortable.h"
#include "znumericparameter.h"

#include <QHBoxLayout>

const int ZPaintLabelWidget::m_maxLabel = 9;

ZPaintLabelWidget::ZPaintLabelWidget(QWidget *parent) :
  QWidget(parent)
{
  init(m_maxLabel);
}

void ZPaintLabelWidget::init(int maxLabel)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  ZLabelColorTable colorTable;
  for (int label = 1; label <= maxLabel; ++label) {
    ZClickableColorLabel *labelWidget =
        makeColorWidget(colorTable.getColor(label), label);
    layout->addWidget(labelWidget);
  }
}

ZClickableColorLabel* ZPaintLabelWidget::makeColorWidget(
    const QColor &color, int label)
{
  ZVec4Parameter *colorVec = new ZVec4Parameter(
        QString("%1").arg(label), glm::vec4(
          color.redF(), color.greenF(), color.blueF(), color.alphaF()),
        glm::vec4(0.f), glm::vec4(1.f), this);

  ZClickableColorLabel *labelWidget = new ZClickableColorLabel(colorVec);
  labelWidget->setWidth(33);
  labelWidget->setHeight(16);
  labelWidget->setClickable(false);

  return labelWidget;
}
