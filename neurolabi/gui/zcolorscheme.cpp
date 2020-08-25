#include "zcolorscheme.h"

#include <iostream>

#include "zrandomgenerator.h"
#include "tz_color.h"
#include "zlabelcolortable.h"

ZColorScheme::ZColorScheme()
{
}

uint32_t ZColorScheme::getColorCode(int index) const
{
  uint32_t code = 0;
  if (!m_colorTable.isEmpty()) {
    code = m_colorTable[(index + m_startIndex) % m_colorTable.size()];
  }

  return code;
}

uint32_t ZColorScheme::getColorCode(uint64_t index) const
{
  uint32_t code = 0;

  int startIndex = m_startIndex;
  if (startIndex < 0) {
    startIndex = startIndex % m_colorTable.size();
  }

  if (!m_colorTable.isEmpty()) {
    code = m_colorTable[(index + startIndex) % (uint64_t) m_colorTable.size()];
  }

  return code;
}

QColor ZColorScheme::getColor(int index) const
{
  return GetColorFromCode(getColorCode(index));
}

QColor ZColorScheme::getColor(uint64_t index) const
{
  return GetColorFromCode(getColorCode(index));
}

void ZColorScheme::setStartIndex(int startIndex)
{
  m_startIndex = startIndex;
}

void ZColorScheme::setColorScheme(EColorScheme scheme)
{
  switch (scheme) {
  case RANDOM_COLOR:
    buildRandomColorTable(64);
    break;
  case CONV_RANDOM_COLOR:
    buildConvRandomColorTable(64);
    break;
  case PUNCTUM_TYPE_COLOR:
    buildPunctumColorTable();
    break;
  case UNIQUE_COLOR:
    buildUniqueColorTable();
    break;
  case LABEL_COLOR:
    buildLabelColorTable();
    break;
  default:
    break;
  }
}

void ZColorScheme::buildRandomColorTable(int n)
{
  Rgb_Color color;
  ZRandomGenerator generator;
  std::vector<int> labelArray = generator.randperm(n);
  m_colorTable.clear();
  for (size_t i = 0; i < labelArray.size(); ++i) {
    Set_Color_Discrete(&color, labelArray[i] - 1);
    m_colorTable.append(GetIntCode(color.r, color.g, color.b, 255));
  }
}

void ZColorScheme::buildLabelColorTable()
{
  ZLabelColorTable colorTable;
  m_colorTable.clear();

  for (int i = 0; i < colorTable.GetColorCount(); ++i) {
    m_colorTable.append(GetIntCode(getColor(i)));
  }

}

void ZColorScheme::buildConvRandomColorTable(int n, int seed)
{
//  m_colorTable.append(Qt::transparent);
  m_colorTable.clear();

  ZRandomGenerator generator(seed);
  const int maxInt = 20000;
  for (int i = 1; i <= n; ++i) {
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
    QColor color(r, g, b);
    color.setHsv(color.hue(), std::min(255, color.saturation() * 2),
                 color.value());
    m_colorTable.append(GetIntCode(color));
  }
}

void ZColorScheme::buildConvRandomColorTable(int n)
{
  buildConvRandomColorTable(n, 42);
}

void ZColorScheme::buildPunctumColorTable()
{
  m_colorTable.clear();

  m_colorTable.push_back(GetIntCode(255, 255, 255, 255));
  m_colorTable.push_back(GetIntCode(0, 255, 255, 255));
  m_colorTable.push_back(GetIntCode(255, 128, 0, 255));
  m_colorTable.push_back(GetIntCode(0, 0, 255, 255));
  m_colorTable.push_back(GetIntCode(255, 0, 255, 255));
  m_colorTable.push_back(GetIntCode(127, 0, 255, 255));
  m_colorTable.push_back(GetIntCode(0, 255, 0, 255));
  m_colorTable.push_back(GetIntCode(255, 255, 0, 255));
  m_colorTable.push_back(GetIntCode(128, 255, 255, 255));
  m_colorTable.push_back(GetIntCode(255, 128, 128, 255));
  m_colorTable.push_back(GetIntCode(128, 128, 255, 255));
  m_colorTable.push_back(GetIntCode(255, 128, 255, 255));
  m_colorTable.push_back(GetIntCode(127, 128, 255, 255));
  m_colorTable.push_back(GetIntCode(128, 255, 128, 255));
  m_colorTable.push_back(GetIntCode(255, 255, 128, 255));
}

void ZColorScheme::buildUniqueColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(GetIntCode(QColor(Qt::red)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::green)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::blue)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::cyan)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::magenta)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkRed)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkGreen)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkBlue)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkCyan)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkMagenta)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::darkYellow)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::white)));
  m_colorTable.push_back(GetIntCode(QColor(Qt::black)));
}

void ZColorScheme::printColorTable() const
{
  if (m_colorTable.isEmpty()) {
    std::cout << "Empty color table" << std::endl;
  } else {
    for (int i = 0; i < m_colorTable.size(); ++i) {
      const QColor &color = GetColorFromCode(m_colorTable[i]);
      std::cout << "i: (" << color.red() << ", " << color.green() << ", "
                << color.blue() << ", " << color.alpha() << ")" << std::endl;
    }
  }
}
