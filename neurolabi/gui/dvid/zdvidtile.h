#ifndef ZDVIDTILE_H
#define ZDVIDTILE_H

#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>

#include "zstackobject.h"
#include "dvid/zdvidresolution.h"
#include "zdvidtarget.h"
//#include "geometry/zintpoint.h"
#include "dvid/zdvidtileinfo.h"
#include "zpixmap.h"
#include "zjsonobject.h"

class ZPainter;
class ZStack;
//class ZStackView;
class ZRect2d;
class ZIntPoint;
class ZImage;

class ZDvidTile : public ZStackObject
{
public:
  ZDvidTile();
  ~ZDvidTile() override;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_TILE;
  }

public:
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
  void clear();

  void update(int z);
//  void update(int x, int y, int z, int width, int height);

  void setTileIndex(int ix, int iy);
  void setResolutionLevel(int level);

  void loadDvidSlice(const QByteArray &buffer, int z, bool highConstrast);
  void loadDvidSlice(const uchar *buf, int length, int z, bool highContrast);

//  void setTileOffset(int x, int y, int z);

//  virtual const std::string& className() const;

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

//  void attachView(ZStackView *view);

  ZCuboid getBoundBox() const override;
//  ZRect2d getBoundBox() const;
//  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

//  void setImageData(const uint8_t *data, int width, int height);

  void enhanceContrast(bool high, bool updatingPixmap);
  void setContrastProtocal(const ZJsonObject &obj);

  void updatePixmap();

private:
  void updateImageContrast();

private:
  ZImage *m_image = nullptr;
  ZPixmap m_pixmap;
  ZImage *m_originalBackup = nullptr;
  int m_ix = 0;
  int m_iy = 0;
  int m_z = 0;
  mutable int m_latestZ = 0;
  ZDvidResolution m_res;
  ZDvidTileInfo m_tilingInfo;
  ZDvidTarget m_dvidTarget;
  ZJsonObject m_contrastProtocal;

  QMutex m_pixmapMutex;

//  ZStackView *m_view;
};

#endif // ZDVIDTILE_H
