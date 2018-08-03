#ifndef ZINTCUBOIDOBJ_H
#define ZINTCUBOIDOBJ_H

#include "zstackobject.h"
#include "zintcuboid.h"

class ZIntCuboidObj : public ZStackObject
{
public:
  ZIntCuboidObj();

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option,
                       neutube::EAxis sliceAxis) const;
  virtual const std::string& className() const;

  bool isSliceVisible(int z, neutube::EAxis sliceAxis) const;
  bool isOnSlice(int z, neutube::EAxis sliceAxis) const;

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_INT_CUBOID;
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

  bool hit(double x, double y, neutube::EAxis axis);
  bool hit(double x, double y, double z);

  inline const ZIntCuboid& getCuboid() const { return m_cuboid; }

  void setCuboid(const ZIntCuboid &box) {
    m_cuboid = box;
  }

  void boundBox(ZIntCuboid *box) const;

  void join(const ZIntCuboid &cuboid);


public:
  ZIntCuboid m_cuboid;
};

#endif // ZINTCUBOIDOBJ_H
