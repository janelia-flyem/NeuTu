#include "flyembodysource.h"

#include "zflyembodymanager.h"

FlyEmBodySource::FlyEmBodySource()
{

}

FlyEmBodySource::~FlyEmBodySource()
{

}

ZObject3dScan* FlyEmBodySource::getCoarseSparsevol(uint64_t /*bodyId*/) const
{
  return nullptr;
}

int FlyEmBodySource::getCoarseSparsevolScale() const
{
  return 0;
}

ZIntPoint FlyEmBodySource::getBlockSize() const
{
  int s = zgeom::GetZoomScale(getCoarseSparsevolScale());

  return {s, s, s};
}

neutu::EBodyLabelType FlyEmBodySource::GetBodyType(uint64_t bodyId)
{
  neutu::EBodyLabelType bodyType = neutu::EBodyLabelType::BODY;
  if (ZFlyEmBodyManager::EncodingSupervoxel(bodyId)) {
    bodyType = neutu::EBodyLabelType::SUPERVOXEL;
  }

  return bodyType;
}

uint64_t FlyEmBodySource::DecodeBodyId(uint64_t bodyId)
{
  return ZFlyEmBodyManager::Decode(bodyId);
}

ZObject3dScan* FlyEmBodySource::PostProcess(ZObject3dScan *body, int dsLevel)
{
  if (body) {
    if (body->isEmpty()) {
      delete body;
      return nullptr;
    } else if (dsLevel > 0) {
      int scale = zgeom::GetZoomScale(dsLevel);
      body->setDsIntv(scale - 1);
    }
  }

  return body;
}

ZObject3dScan* FlyEmBodySource::getSparsevol(
      uint64_t bodyId, int dsLevel, const ZIntCuboid &range) const
{
  ZObject3dScan *body = getSparsevol(bodyId, dsLevel);
  if (body && !range.isEmpty()) {
    // Need to downsample range
    ZIntCuboid adjustedRange = range;
    if (dsLevel > 0) {
      adjustedRange.scaleDown(zgeom::GetZoomScale(dsLevel));
    }
    ZObject3dScan *cropped = body->subobject(adjustedRange, nullptr, nullptr);
    delete body;
    body = cropped;
  }

  return body;
}

ZObject3dScan* FlyEmBodySource::getCoarseSparsevol(
      uint64_t bodyId, const ZIntCuboid &range) const
{
  ZObject3dScan *body = getCoarseSparsevol(bodyId);
  if (body && !range.isEmpty()) {
    // Need to downsample range
    ZIntCuboid adjustedRange = range;
    adjustedRange.scaleDown(zgeom::GetZoomScale(getCoarseSparsevolScale()));
    ZObject3dScan *cropped = body->subobject(adjustedRange, nullptr, nullptr);
    delete body;
    body = cropped;
  }

  return body;
}

ZIntCuboid FlyEmBodySource::getBoundBox(uint64_t bodyId) const
{
  ZObject3dScan *body = getSparsevol(bodyId, 0);
  if (body) {
    return body->getIntBoundBox();
  }

  return ZIntCuboid::Empty();
}

ZIntCuboid FlyEmBodySource::getBoundBox(uint64_t bodyId, int dsLevel) const
{
  ZIntCuboid box = getBoundBox(bodyId);
  if (dsLevel > 0) {
    int scale = zgeom::GetZoomScale(dsLevel);
    box.downScale(scale);
  }

  return box;
}
