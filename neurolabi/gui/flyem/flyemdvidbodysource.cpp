#include "flyemdvidbodysource.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidglobal.h"

#include "flyem/zflyembodymanager.h"

FlyEmDvidBodySource::FlyEmDvidBodySource()
{

}

void FlyEmDvidBodySource::setDvidTarget(const ZDvidTarget &target)
{
  if (target.isValid() && target.hasSegmentation()) {
    m_writer = std::shared_ptr<ZDvidWriter>(new ZDvidWriter);
    if (m_writer->open(target)) {
      m_dataInfo = ZDvidGlobal::Memo::ReadSegmentationInfo(m_writer->getDvidTarget());
    } else {
      m_dataInfo.clear();
      m_writer.reset();
    }
  } else {
    m_dataInfo.clear();
    m_writer.reset();
  }
}

ZDvidReader* FlyEmDvidBodySource::getReader() const
{
  if (m_writer) {
    return &(m_writer->getDvidReader());
  }

  return nullptr;
}

std::mutex* FlyEmDvidBodySource::getIoMutex() const
{
  return &m_ioMutex;
}

ZObject3dScan* FlyEmDvidBodySource::getSparsevol(uint64_t bodyId, int dsLevel) const
{
  ZObject3dScan *body = nullptr;
   ZDvidReader *reader = getReader();
   if (reader) {
     const std::lock_guard<std::mutex> lock(m_ioMutex);
     neutu::EBodyLabelType bodyType = GetBodyType(bodyId);

     body = PostProcess(
           reader->readBody(
             DecodeBodyId(bodyId), bodyType, dsLevel, ZIntCuboid(), true, nullptr),
           dsLevel);
   }

   return body;
}

ZObject3dScan* FlyEmDvidBodySource::getSparsevol(
    uint64_t bodyId, int dsLevel, const ZIntCuboid &range) const
{
  ZObject3dScan *body = nullptr;
   ZDvidReader *reader = getReader();
   if (reader) {
     const std::lock_guard<std::mutex> lock(m_ioMutex);
     neutu::EBodyLabelType bodyType = GetBodyType(bodyId);

     body = PostProcess(
           reader->readBody(
             DecodeBodyId(bodyId), bodyType, dsLevel, range, true, nullptr),
           dsLevel);
   }

   return body;
}

ZObject3dScan* FlyEmDvidBodySource::getCoarseSparsevol(
    uint64_t bodyId, const ZIntCuboid &range) const
{
  ZObject3dScan *body = nullptr;
   ZDvidReader *reader = getReader();
   if (reader) {
     const std::lock_guard<std::mutex> lock(m_ioMutex);
     neutu::EBodyLabelType bodyType = GetBodyType(bodyId);

     body = PostProcess(
           reader->readCoarseBody(DecodeBodyId(bodyId), bodyType, range, nullptr),
           getCoarseSparsevolScale());
   }

   return body;
}

int FlyEmDvidBodySource::getCoarseSparsevolScale() const
{
  if (getReader()) {
    return zgeom::GetZoomLevel(m_dataInfo.getBlockSize().getX());
  }

  return 0;
}

ZObject3dScan* FlyEmDvidBodySource::getCoarseSparsevol(uint64_t bodyId) const
{
  ZObject3dScan *body = nullptr;
   ZDvidReader *reader = getReader();
   if (reader) {
     const std::lock_guard<std::mutex> lock(m_ioMutex);
     neutu::EBodyLabelType bodyType = GetBodyType(bodyId);

     body = PostProcess(
           reader->readCoarseBody(DecodeBodyId(bodyId), bodyType, nullptr),
           getCoarseSparsevolScale());
   }

   return body;
}

ZIntPoint FlyEmDvidBodySource::getBlockSize() const
{
  return m_dataInfo.getBlockSize();
}

ZIntCuboid FlyEmDvidBodySource::getBoundBox(uint64_t bodyId) const
{
  ZDvidReader *reader = getReader();
  if (reader) {
    const std::lock_guard<std::mutex> lock(m_ioMutex);
    neutu::EBodyLabelType bodyType = GetBodyType(bodyId);
    return reader->readBodyBoundBox(DecodeBodyId(bodyId), bodyType);
  }

  return ZIntCuboid::Empty();
}

ZIntCuboid FlyEmDvidBodySource::getBoundBox(uint64_t bodyId, int dsLevel) const
{
  return FlyEmBodySource::getBoundBox(bodyId, dsLevel);
}
