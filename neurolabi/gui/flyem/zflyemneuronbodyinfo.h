#ifndef ZFLYEMNEURONBODYINFO_H
#define ZFLYEMNEURONBODYINFO_H

#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"

class ZObject3dScan;
class ZJsonObject;

class ZFlyEmNeuronBodyInfo
{
public:
  ZFlyEmNeuronBodyInfo();

  inline void setBodySize(size_t bodySize) {
    m_bodySize = bodySize;
  }

  inline int getBodySize() const {
    return m_bodySize;
  }

  inline const ZIntCuboid& getBoundBox() const {
    return m_boundBox;
  }

  void setBoundBox(int x0, int y0, int z0, int x1, int y1, int z1);
  void setBoundBox(const ZIntCuboid &box);

  ZJsonObject toJsonObject() const;
  void loadJsonObject(const ZJsonObject &obj);

private:
  size_t m_bodySize;
  ZIntCuboid m_boundBox;
};

#endif // ZFLYEMNEURONBODYINFO_H
