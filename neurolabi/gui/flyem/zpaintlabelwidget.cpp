#include "zpaintlabelwidget.h"
#include "zclickablelabel.h"
#include "zlabelcolortable.h"
#include "widgets/znumericparameter.h"
#include "widgets/widgets_def.h"

#include <QHBoxLayout>
#include <QGroupBox>

const int ZPaintLabelWidget::m_maxLabel = 9;

ZPaintLabelWidget::ZPaintLabelWidget(QWidget *parent) :
  QWidget(parent)
{
  init(m_maxLabel);
}

void ZPaintLabelWidget::setTitle(const QString &title)
{
  m_groupBox->setTitle(title);
}

void ZPaintLabelWidget::init(int maxLabel)
{
  m_groupBox = new QGroupBox(this);
//  groupBox->setStyleSheet("margin-top: 1px;");
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 2, 5, 2);
  ZLabelColorTable colorTable;
  for (int label = 1; label <= maxLabel; ++label) {
    ZClickableColorLabel *labelWidget =
        makeColorWidget(colorTable.getColor(label), label);
    layout->addWidget(labelWidget);
  }
  m_groupBox->setLayout(layout);
  setStyleSheet(neutu::GROUP_BOX_STYLE);
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

QSize ZPaintLabelWidget::minimumSizeHint() const
{
  return QWidget::minimumSizeHint() + QSize(0, 40);
}
