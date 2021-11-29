#include "zh3widget.h"

#include <QHBoxLayout>
#include <QSpacerItem>

#include "qt/gui/utilities.h"

ZH3Widget::ZH3Widget(QWidget *parent) : ZHWidget(3, parent)
{
}

void ZH3Widget::addWidget(QWidget *widget, neutu::EH3Layout seg)
{
  ZHWidget::addWidget(widget, getLayoutIndex(seg));
}

void ZH3Widget::addLayout(QLayout *layout, neutu::EH3Layout seg)
{
  ZHWidget::addLayout(layout, getLayoutIndex(seg));
}

int ZH3Widget::getLayoutIndex(neutu::EH3Layout seg) const
{
  switch (seg) {
  case neutu::EH3Layout::LEFT:
    return 0;
  case neutu::EH3Layout::CENTER:
    return 1;
  case neutu::EH3Layout::RIGHT:
    return 2;
  }

  return -1;
}
