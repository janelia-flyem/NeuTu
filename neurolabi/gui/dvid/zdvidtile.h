#ifndef ZDVIDTILE_H
#define ZDVIDTILE_H

#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>

#include "zimage.h"
#include "zstackobject.h"
#include "dvid/zdvidresolution.h"
#include "zdvidtarget.h"
//#include "zintpoint.h"
#include "dvid/zdvidtileinfo.h"
#include "zpixmap.h"

class ZPainter;
class ZStack;
class ZStackView;
class ZRect2d;
class ZIntPoint;

class ZDvidTile : public ZStackObject
{
public:
  ZDvidTile();
  ~ZDvidTile();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_TILE;
  }

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;
  void clear();

  void update(int z);
//  void update(int x, int y, int z, int width, int height);

  void setTileIndex(int ix, int iy);
  void setResolutionLevel(int level);

  void loadDvidSlice(const QByteArray &buffer, int z, bool highConstrast);
  void loadDvidSlice(const uchar *buf, int length, int z, bool highContrast);

//  void setTileOffset(int x, int y, int z);

  virtual const std::string& className() const;

  void printInfo() const;

  void setDvidTarget(const ZDvidTarget &target,
                     const ZDvidTileInfo &tileInfo);
  void setTileInfo(const ZDvidTileInfo &tileInfo);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int getX() const;
  int getY() const;
  int getZ() const;

  inline int getIx() { return m_ix; }
  inline int getIy() { return m_iy; }

  void setZ(int z) { m_z = z; }

  int getWidth() const;
  int getHeight() const;

  void attachView(ZStackView *view);

  ZRect2d getBoundBox() const;
  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

//  void setImageData(const uint8_t *data, int width, int height);

  void enhanceContrast(bool high, bool updatingPixmap);

  void updatePixmap();

private:
  ZImage *m_image;
  ZPixmap m_pixmap;
  int m_ix;
  int m_iy;
  int m_z;
  mutable int m_latestZ;
  ZDvidResolution m_res;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;

  QMutex m_pixmapMutex;

  ZStackView *m_view;
};

#endif // ZDVIDTILE_H
