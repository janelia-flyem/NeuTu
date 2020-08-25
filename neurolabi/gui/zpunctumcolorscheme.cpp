#include "zpunctumcolorscheme.h"

ZPunctumColorScheme::ZPunctumColorScheme()
{
}

void ZPunctumColorScheme::setColorScheme(EColorScheme scheme)
{
  switch (scheme) {
  case PUNCTUM_TYPE_COLOR:
    buildPunctumTypeColorTable();
    break;
  default:
    break;
  }
}

void ZPunctumColorScheme::buildPunctumTypeColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(GetIntCode(Qt::white));
  m_colorTable.push_back(GetIntCode(Qt::yellow));
  m_colorTable.push_back(GetIntCode(Qt::black));
  m_colorTable.push_back(GetIntCode(Qt::red));
  m_colorTable.push_back(GetIntCode(Qt::cyan));
  m_colorTable.push_back(GetIntCode(Qt::green));
  m_colorTable.push_back(GetIntCode(Qt::gray));
}
