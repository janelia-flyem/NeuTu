#include "zhwidget.h"

#include <QHBoxLayout>
#include <QSpacerItem>

#include "qt/gui/utilities.h"

ZHWidget::ZHWidget(int layoutCount, QWidget *parent) : ZGroupVisWidget(parent)
{
  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setMargin(0);
  mainLayout->setSpacing(0);
  setLayout(mainLayout);

  m_childLayout.resize(layoutCount);
  for (int i = 0; i < layoutCount; ++i) {
    m_childLayout[i] = new QHBoxLayout;
    m_childLayout[i]->setMargin(0);
    m_childLayout[i]->setSpacing(0);
    mainLayout->addLayout(m_childLayout[i]);
    if (i < layoutCount - 1) {
      mainLayout->addSpacerItem(new QSpacerItem(0, 1, QSizePolicy::Expanding));
    }
  }

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
}

void ZHWidget::addWidget(QWidget *widget, int layoutIndex)
{
  if (widget) {
    QHBoxLayout *container = getLayout(layoutIndex);
    if (container) {
      container->addWidget(widget);
      assumeVisible(widget, true);
    }
  }
}

void ZHWidget::addLayout(QLayout *layout, int layoutIndex)
{
  QHBoxLayout *container = getLayout(layoutIndex);
  if (container) {
    container->addLayout(layout);
    neutu::ForEachWidgetInLayout(layout, [this](QWidget *widget) {
      assumeVisible(widget, true);
    });
  }
}

void ZHWidget::removeWidget(QWidget *widget)
{
  if (widget) {
    /* Removing widget from layout doesn't work
    for (size_t i = 0; i < 3; ++i) {
      m_childLayout[i]->removeWidget(widget);
    }
    */
    widget->setParent(nullptr);
#ifdef _DEBUG_0
      neutu::PrintLayoutInfo(layout());
#endif
    assumeVisible(widget, false);
  }
}

QHBoxLayout *ZHWidget::getLayout(int layoutIndex) const
{
  if (layoutIndex >= 0 && layoutIndex < m_childLayout.size()) {
    return m_childLayout[layoutIndex];
  }

  return nullptr;
}
