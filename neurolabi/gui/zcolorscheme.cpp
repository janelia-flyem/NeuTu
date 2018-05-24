#include "zcolorscheme.h"
#include "zrandomgenerator.h"
#include "tz_color.h"
#include "zlabelcolortable.h"

ZColorScheme::ZColorScheme()
{
}

QColor ZColorScheme::getColor(int index) const
{
  QColor color;

  if (m_colorTable.isEmpty()) {
    color = QColor(0, 0, 0);
  } else {
    color = m_colorTable[(index + m_startIndex) % m_colorTable.size()];
  }

  //qDebug() << color;

  return color;
}

QColor ZColorScheme::getColor(uint64_t index) const
{
  QColor color;

  int startIndex = m_startIndex;
  if (startIndex < 0) {
    startIndex = startIndex % m_colorTable.size();
  }

  if (m_colorTable.isEmpty()) {
    color = QColor(0, 0, 0);
  } else {
    color = m_colorTable[(index + startIndex) % (uint64_t) m_colorTable.size()];
  }

  return color;
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
    m_colorTable.append(QColor(color.r, color.g, color.b));
  }
}

void ZColorScheme::buildLabelColorTable()
{
  ZLabelColorTable colorTable;
  m_colorTable.clear();

  for (int i = 0; i < colorTable.GetColorCount(); ++i) {
    m_colorTable.append(colorTable.getColor(i));
  }

}

void ZColorScheme::buildConvRandomColorTable(int n)
{
  m_colorTable.append(Qt::black);

  ZRandomGenerator generator(42);
  const int maxInt = 20000;
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
    QColor color(r, g, b);
    color.setHsv(color.hue(), std::min(255, color.saturation() * 2),
                 color.value());
    m_colorTable.append(color);
  }
}

void ZColorScheme::buildPunctumColorTable()
{
  m_colorTable.clear();

  m_colorTable.push_back(QColor(255, 255, 255, 255));
  m_colorTable.push_back(QColor(0, 255, 255, 255));
  m_colorTable.push_back(QColor(255, 128, 0, 255));
  m_colorTable.push_back(QColor(0, 0, 255, 255));
  m_colorTable.push_back(QColor(255, 0, 255, 255));
  m_colorTable.push_back(QColor(127, 0, 255, 255));
  m_colorTable.push_back(QColor(0, 255, 0, 255));
  m_colorTable.push_back(QColor(255, 255, 0, 255));

  m_colorTable.push_back(QColor(128, 255, 255, 255));
  m_colorTable.push_back(QColor(255, 128, 128, 255));
  m_colorTable.push_back(QColor(128, 128, 255, 255));
  m_colorTable.push_back(QColor(255, 128, 255, 255));
  m_colorTable.push_back(QColor(127, 128, 255, 255));
  m_colorTable.push_back(QColor(128, 255, 128, 255));
  m_colorTable.push_back(QColor(255, 255, 128, 255));
}

void ZColorScheme::buildUniqueColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(QColor(Qt::red));
  m_colorTable.push_back(QColor(Qt::green));
  m_colorTable.push_back(QColor(Qt::blue));
  m_colorTable.push_back(QColor(Qt::cyan));
  m_colorTable.push_back(QColor(Qt::magenta));
  m_colorTable.push_back(QColor(Qt::darkRed));
  m_colorTable.push_back(QColor(Qt::darkGreen));
  m_colorTable.push_back(QColor(Qt::darkBlue));
  m_colorTable.push_back(QColor(Qt::darkCyan));
  m_colorTable.push_back(QColor(Qt::darkMagenta));
  m_colorTable.push_back(QColor(Qt::darkYellow));
  m_colorTable.push_back(QColor(Qt::white));
  m_colorTable.push_back(QColor(Qt::black));
}
