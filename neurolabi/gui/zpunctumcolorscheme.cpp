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
  m_colorTable.push_back(QColor(Qt::white));
  m_colorTable.push_back(QColor(Qt::yellow));
  m_colorTable.push_back(QColor(Qt::black));
  m_colorTable.push_back(QColor(Qt::red));
  m_colorTable.push_back(QColor(Qt::cyan));
  m_colorTable.push_back(QColor(Qt::green));
  m_colorTable.push_back(QColor(Qt::gray));
}
