#include "ztilegraphicsitem.h"

bool ZTileGraphicsItem::loadJsonObject(const ZJsonObject &obj)
{
  if (m_tileInfo.loadJsonObject(obj)) {
    setRect(m_tileInfo.getOffset().x(), m_tileInfo.getOffset().y(),
            m_tileInfo.getWidth(), m_tileInfo.getHeight());
    setToolTip(m_tileInfo.getSource().c_str());

    return true;
  }

  return false;
}
