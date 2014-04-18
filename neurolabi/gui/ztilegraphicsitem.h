#ifndef ZTILEGRAPHICSITEM_H
#define ZTILEGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include "ztileinfo.h"
#include "zjsonobject.h"

class ZTileGraphicsItem : public QGraphicsRectItem
{
public:
  inline const ZTileInfo& getTileInfo() const {
    return m_tileInfo;
  }

  bool loadJsonObject(const ZJsonObject &obj);

private:
  ZTileInfo m_tileInfo;
};

#endif // ZTILEGRAPHICSITEM_H
