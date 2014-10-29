#include "zcolorscheme.h"

ZColorScheme::ZColorScheme()
{
}

QColor ZColorScheme::getColor(int index) const
{
  QColor color;

  if (m_colorTable.isEmpty()) {
    color = QColor(0, 0, 0);
  } else {
    color = m_colorTable[index % m_colorTable.size()];
  }

  //qDebug() << color;

  return color;
}

void ZColorScheme::setColorScheme(EColorScheme /*scheme*/)
{

}
