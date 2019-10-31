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

  inline size_t getBodySize() const {
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
  void clear();

private:
  size_t m_bodySize = 0;
  ZIntCuboid m_boundBox;
  std::vector<int64_t> m_connSize;
  int64_t m_mutationId = 0;
  int m_version = 0;
  const static int VERSION;
};

#endif // ZFLYEMNEURONBODYINFO_H
