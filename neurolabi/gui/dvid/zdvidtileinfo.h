#ifndef ZDVIDTILEINFO_H
#define ZDVIDTILEINFO_H

#include <vector>
#include "zintpoint.h"

class ZJsonObject;
class QRect;

class ZDvidTileInfo
{
public:
  ZDvidTileInfo();

  typedef std::pair<int, int> TIndex;

  void load(const ZJsonObject &json);
  void clear();
  void print() const;

  inline int getWidth() const { return m_tileSize.getX(); }
  inline int getHeight() const { return m_tileSize.getY(); }

  int getWidth(int level) const;
  int getHeight(int level) const;

  static int getLevelScale() { return m_levelScale; }

  bool isValid() const;

  std::vector<TIndex> getCoverIndex(int resLevel, const QRect &rect) const;

  inline int getMaxLevel() const { return m_maxLevel; }

private:
  int m_maxLevel;
  ZIntPoint m_tileSize;

  const static int m_levelScale;
};

#endif // ZDVIDTILEINFO_H
