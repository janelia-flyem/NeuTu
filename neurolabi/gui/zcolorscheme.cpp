#include "zcolorscheme.h"
#include "zrandomgenerator.h"
#include "tz_color.h"

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

void ZColorScheme::buildRandomColorTable(int n)
{
  Rgb_Color color;
  ZRandomGenerator generator;
  std::vector<int> labelArray = generator.randperm(n);
  m_colorTable.clear();
  for (size_t i = 0; i < labelArray.size(); ++i) {
    Set_Color_Discrete(&color, labelArray[i] - 1);
    m_colorTable.append(QColor(color.r, color.g, color.b));
  }
}
