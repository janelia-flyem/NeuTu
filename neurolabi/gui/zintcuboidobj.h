#ifndef ZINTCUBOIDOBJ_H
#define ZINTCUBOIDOBJ_H

#include "zstackobject.h"
#include "geometry/zintcuboid.h"

class ZIntCuboidObj : public ZStackObject
{
public:
  ZIntCuboidObj();

public:
  bool display(QPainter *painter, const DisplayConfig &config) const override {
    return false;
  }
  /*
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       neutu::EAxis sliceAxis) const override;
                       */
//  virtual const std::string& className() const;

  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const override;
  bool isOnSlice(int z, neutu::EAxis sliceAxis) const;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::INT_CUBOID;
  }

  void clear();

  bool isValid() const;

  const ZIntPoint& getFirstCorner() const;
  const ZIntPoint& getLastCorner() const;

  void setFirstCorner(const ZIntPoint &firstCorner);
  void setLastCorner(const ZIntPoint &lastCorner);

  void setFirstCorner(int x, int y, int z);
  void setLastCorner(int x, int y, int z);

  int getWidth() const;
  int getHeight() const;
  int getDepth() const;

  bool hit(double x, double y, neutu::EAxis axis) override;
  bool hit(double x, double y, double z) override;

  inline const ZIntCuboid& getCuboid() const { return m_cuboid; }

  void setCuboid(const ZIntCuboid &box) {
    m_cuboid = box;
  }

  void boundBox(ZIntCuboid *box) const override;
  ZCuboid getBoundBox() const override;

  void join(const ZIntCuboid &cuboid);

public:
  //For display settings
  void setGridInterval(int intv);

public:
  ZIntCuboid m_cuboid;
  int m_gridIntv = 32;
};

#endif // ZINTCUBOIDOBJ_H
