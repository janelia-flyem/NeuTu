#ifndef ZDVIDTILE_H
#define ZDVIDTILE_H

#include <QImage>
#include "zstackobject.h"
#include "dvid/zdvidresolution.h"
#include "zdvidtarget.h"
#include "zintpoint.h"
#include "dvid/zdvidtileinfo.h"

class ZPainter;
class ZStack;

class ZDvidTile : public ZStackObject
{
public:
  ZDvidTile();
  ~ZDvidTile();

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option) const;
  void clear();

  void update(int z);
//  void update(int x, int y, int z, int width, int height);

  void setTileIndex(int ix, int iy);

  void loadDvidPng(const QByteArray &buffer);
  void setResolutionLevel(int level);
//  void setTileOffset(int x, int y, int z);

  virtual const std::string& className() const;

  void printInfo() const;

  void setDvidTarget(const ZDvidTarget &target);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int getX() const;
  int getY() const;
  int getZ() const;

private:
  QImage m_image;
  int m_ix;
  int m_iy;
  int m_z;
  mutable int m_latestZ;
  ZDvidResolution m_res;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;
};

#endif // ZDVIDTILE_H
