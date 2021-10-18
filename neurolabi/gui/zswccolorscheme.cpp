#include "zswccolorscheme.h"
#include <QDebug>
#include <QColor>

ZSwcColorScheme::ZSwcColorScheme()
{
}



void ZSwcColorScheme::buildVaa3dColorTable()
{
  m_colorTable.resize(21);
  m_colorTable[0] = GetIntCode(255, 255, 255);
  m_colorTable[1] = GetIntCode(20, 20, 20);
  m_colorTable[2] = GetIntCode(200, 20, 0); //red
  m_colorTable[3] = GetIntCode(0, 20, 200); //blue
  m_colorTable[4] = GetIntCode(200, 0, 200); //purple
  m_colorTable[5] = GetIntCode(0, 200, 200); //cyan
  m_colorTable[6] = GetIntCode(220, 200, 0); //yellow
  m_colorTable[7] = GetIntCode(0, 200, 20); //green
  m_colorTable[8] = GetIntCode(188, 94, 37); //coffee
  m_colorTable[9] = GetIntCode(180, 200, 120); //asparagus
  m_colorTable[10] = GetIntCode(250, 100, 120); //salmon
  m_colorTable[11] = GetIntCode(120, 200, 200); //ice
  m_colorTable[12] = GetIntCode(100, 120, 200); //orchid
  m_colorTable[13] = GetIntCode(255, 128, 168);
  m_colorTable[14] = GetIntCode(128, 255, 168);
  m_colorTable[16] = GetIntCode(128, 168, 255);
  m_colorTable[17] = GetIntCode(128, 255, 168);
  m_colorTable[18] = GetIntCode(255, 168, 128);
  m_colorTable[19] = GetIntCode(168, 128, 255);
  m_colorTable[20] = GetIntCode(0xcc, 0xcc, 0xcc);
}

void ZSwcColorScheme::buildBiocytinColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(GetIntCode(200, 20, 0)); //red
  m_colorTable.push_back(GetIntCode(0, 20, 200)); //blue
  m_colorTable.push_back(GetIntCode(200, 0, 200)); //purple
  m_colorTable.push_back(GetIntCode(0, 200, 200)); //cyan
  m_colorTable.push_back(GetIntCode(220, 200, 0)); //yellow
  m_colorTable.push_back(GetIntCode(0, 200, 20)); //green
  m_colorTable.push_back(GetIntCode(188, 94, 37)); //coffee
  //m_colorTable.push_back(QColor(180, 200, 120)); //asparagus
  //m_colorTable.push_back(QColor(250, 100, 120)); //salmon
  //m_colorTable.push_back(QColor(120, 200, 200)); //ice
  //m_colorTable.push_back(QColor(100, 120, 200)); //orchid
}

void ZSwcColorScheme::setColorScheme(EColorScheme scheme)
{
  if (scheme != m_colorScheme) {
    m_colorScheme = scheme;
    switch (scheme) {
    case VAA3D_TYPE_COLOR:
      buildVaa3dColorTable();
      break;
    case BIOCYTIN_TYPE_COLOR:
      buildBiocytinColorTable();
      break;
    case UNIQUE_COLOR:
      buildUniqueColorTable();
      break;
    case JIN_TYPE_COLOR:
      buildJinTypeColorTable();
      break;
    case GMU_TYPE_COLOR:
      buildGmuTypeColorTable();
      break;
    default:
      ZColorScheme::setColorScheme(scheme);
      break;
    }
  }
}

void ZSwcColorScheme::buildJinTypeColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(GetIntCode(Qt::white));
  m_colorTable.push_back(GetIntCode(Qt::blue));
  m_colorTable.push_back(GetIntCode(QColor(200, 153, 0)));
  m_colorTable.push_back(GetIntCode(Qt::green));
  m_colorTable.push_back(GetIntCode(Qt::red));
  m_colorTable.push_back(GetIntCode(Qt::gray));
}

void ZSwcColorScheme::buildGmuTypeColorTable()
{
  buildVaa3dColorTable();
  m_colorTable.resize(275);

  for (int i = 21; i < (int) m_colorTable.size(); ++i) {
    int baseType = i / 10 % 18 + 2;
    QColor color = m_colorTable[baseType];
    int h = color.hue();
    int s = color.saturation();
    int v = i % 10 * 28;
    color.setHsv(h, s, v);
    m_colorTable[i] = color.rgba();
  }
}
