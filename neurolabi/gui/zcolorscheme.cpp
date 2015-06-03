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

void ZColorScheme::buildConvRandomColorTable(int n)
{
  ZRandomGenerator generator;
  const int maxInt = 65534;
  for (int i = 1; i < n; ++i) {
    int r = generator.rndint(maxInt)%255;
    int g = generator.rndint(maxInt)%255;
    int b = generator.rndint(maxInt)%255;
    int temp1 = std::min(r,g);
    temp1 = std::min(temp1,b);
    int temp2 = std::max(r,g);
    temp2 = std::max(temp2,b);
    int diff = 100 - (temp2 - temp1);
    if (diff > 0) {
      int dec = std::min(diff, temp1);
      diff -= dec;
      if ((r < b) && (r < g)) {
        r -= dec;
      } else if ((b < r) && (b < g)) {
        b -= dec;
      } else {
        g -= dec;
      }
    }
    if (diff > 0) {
      if ((r > b) && (r > g)) {
        r += diff;
      } else if ((b > r) && (b > g)) {
        b += diff;
      } else {
        g += diff;
      }
    }
    m_colorTable.append(QColor(r, g, b));
  }
}
