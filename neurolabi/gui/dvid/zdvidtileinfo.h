#ifndef ZDVIDTILEINFO_H
#define ZDVIDTILEINFO_H

#include "zintpoint.h"

class ZJsonObject;

class ZDvidTileInfo
{
public:
  ZDvidTileInfo();

  void load(const ZJsonObject &json);
  void clear();
  void print() const;

  inline int getWidth() const { return m_tileSize.getX(); }
  inline int getHeight() const { return m_tileSize.getY(); }

private:
  int m_maxLevel;
  ZIntPoint m_tileSize;

  const static int m_levelScale;
};

#endif // ZDVIDTILEINFO_H
