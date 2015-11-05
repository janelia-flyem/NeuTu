#ifndef ZINTCUBOIDOBJ_H
#define ZINTCUBOIDOBJ_H

#include "zstackobject.h"
#include "zintcuboid.h"

class ZIntCuboidObj : public ZStackObject
{
public:
  ZIntCuboidObj();

public:
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option) const;
  virtual const std::string& className() const;

  bool isSliceVisible(int z) const;
  bool isOnSlice(int z) const;

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

  bool hit(double x, double y);
  bool hit(double x, double y, double z);

  inline const ZIntCuboid& getCuboid() const { return m_cuboid; }

  void join(const ZIntCuboid &cuboid);


public:
  ZIntCuboid m_cuboid;
};

#endif // ZINTCUBOIDOBJ_H
