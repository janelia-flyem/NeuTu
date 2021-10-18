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
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  init(m_maxLabel);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

}

void ZPaintLabelWidget::setTitle(const QString &title)
{
  m_groupBox->setTitle(title);
}

void ZPaintLabelWidget::init(int maxLabel)
{
  m_groupBox = new QGroupBox(this);
  m_groupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  layout()->addWidget(m_groupBox);
//  groupBox->setStyleSheet("margin-top: 1px;");
  QHBoxLayout *layout = new QHBoxLayout;
  m_groupBox->setLayout(layout);
  layout->setContentsMargins(5, 2, 5, 2);
  ZLabelColorTable colorTable;
  for (int label = 1; label <= maxLabel; ++label) {
    ZClickableColorLabel *labelWidget =
        makeColorWidget(colorTable.getColor(label), label);
    layout->addWidget(labelWidget);
  }
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
  labelWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  labelWidget->setWidth(33);
  labelWidget->setHeight(16);
  labelWidget->setClickable(false);

  return labelWidget;
}

QSize ZPaintLabelWidget::minimumSizeHint() const
{
  return QWidget::minimumSizeHint()/* + QSize(0, 40)*/;
}
