#include "zcolorlabel.h"

#include <QPalette>

ZColorLabel::ZColorLabel(QWidget *parent) :
  QLabel(parent),
  m_width(50),
  m_height(33),
  m_clickable(true)
{
  setAutoFillBackground(true);
  setAlignment(Qt::AlignCenter);
}

void ZColorLabel::setColor(const QColor &col)
{
  QPalette pal;
  pal.setColor(QPalette::Background, col);
  setPalette(pal);
}

QSize ZColorLabel::minimumSizeHint() const
{
  return QSize(m_width, m_height);
}
