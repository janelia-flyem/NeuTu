#include "zflyemneuronbodyinfo.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

ZFlyEmNeuronBodyInfo::ZFlyEmNeuronBodyInfo()
{
}

void ZFlyEmNeuronBodyInfo::setBoundBox(
    int x0, int y0, int z0, int x1, int y1, int z1)
{
  m_boundBox.set(x0, y0, z0, x1, y1, z1);
}
void ZFlyEmNeuronBodyInfo::setBoundBox(const ZIntCuboid &box)
{
  m_boundBox = box;
}

ZJsonObject ZFlyEmNeuronBodyInfo::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("volume", m_bodySize);

  ZJsonArray boundBoxOffsetJson;
  boundBoxOffsetJson.append(m_boundBox.getFirstCorner().getX());
  boundBoxOffsetJson.append(m_boundBox.getFirstCorner().getY());
  boundBoxOffsetJson.append(m_boundBox.getFirstCorner().getZ());
  obj.setEntry("bound_offset", boundBoxOffsetJson);

  ZJsonArray boundBoxSizeJson;
  boundBoxSizeJson.append(m_boundBox.getWidth());
  boundBoxSizeJson.append(m_boundBox.getHeight());
  boundBoxSizeJson.append(m_boundBox.getDepth());
  obj.setEntry("bound_size", boundBoxSizeJson);

  return obj;
}

void ZFlyEmNeuronBodyInfo::loadJsonObject(const ZJsonObject &obj)
{
  m_bodySize = 0;
  m_boundBox.reset();

  if (obj.hasKey("volume")) {
    m_bodySize = ZJsonParser::integerValue(obj["volume"]);
  }

  if (obj.hasKey("bound_offset")) {
    std::vector<int> array =
        ZJsonParser::integerArray(obj["bound_offset"]);
    if (array.size() == 3) {
      m_boundBox.setFirstCorner(array[0], array[1], array[2]);
    }
  }

  if (obj.hasKey("bound_size")) {
    std::vector<int> array =
        ZJsonParser::integerArray(obj["bound_size"]);
    if (array.size() == 3) {
      m_boundBox.setSize(array[0], array[1], array[2]);
    }
  }
}
