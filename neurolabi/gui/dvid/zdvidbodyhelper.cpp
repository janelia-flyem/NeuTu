#include "zdvidbodyhelper.h"

#include "zdvidreader.h"
#include "zobject3dscan.h"
#include "misc/miscutility.h"

ZDvidBodyHelper::ZDvidBodyHelper(const ZDvidReader *reader) : m_reader(reader)
{
}

const ZDvidReader *ZDvidBodyHelper::getDvidReader() const
{
  return m_reader;
}

void ZDvidBodyHelper::setCanonizing(bool on)
{
  m_canonizing = on;
}

void ZDvidBodyHelper::setLabelType(flyem::EBodyLabelType type)
{
  m_labelType = type;
}

void ZDvidBodyHelper::setRange(const ZIntCuboid &box)
{
  m_range = box;
}

void ZDvidBodyHelper::setCoarse(bool on)
{
  m_coarseVol = on;
}

void ZDvidBodyHelper::setZoom(int zoom)
{
  m_zoom = zoom;
//  m_coarseVol = false;
}

void ZDvidBodyHelper::setLowresZoom(int zoom)
{
  m_lowresZoom = zoom;
}

ZObject3dScan* ZDvidBodyHelper::readBody(uint64_t bodyId, ZObject3dScan *result)
{
  ZObject3dScan *out = nullptr;

  if (!m_coarseVol) {
    out = getDvidReader()->readBody(
          bodyId, m_labelType, m_zoom, m_range, m_canonizing, result);
    out->setDsIntv(misc::GetZoomScale(m_zoom) - 1);
  } else {
    out = getDvidReader()->readCoarseBody(bodyId, m_labelType, m_range, result);
    if (m_canonizing) {
      out->canonize();
    }
  }

  return out;
}

std::vector<ZObject3dScan*> ZDvidBodyHelper::readHybridBody(uint64_t bodyId)
{
  std::vector<ZObject3dScan*> result;

  ZDvidBodyHelper highResHelper = fork();
  highResHelper.setCoarse(false);
  ZObject3dScan *highResObj = highResHelper.readBody(bodyId);
  if (highResObj != NULL) {
    result.push_back(highResObj);
  }

  if (!m_range.isEmpty()) { //load low res parts
    ZDvidBodyHelper lowResHelper = fork();
    lowResHelper.setRange(ZIntCuboid());
    ZObject3dScan *lowResObj = nullptr;

    ZIntPoint scale;
    if (m_coarseVol) {
      lowResObj = lowResHelper.readBody(bodyId);
      ZDvidInfo dvidInfo = getDvidReader()->readLabelInfo();
      scale = dvidInfo.getBlockSize();
    } else {
      if (m_lowresZoom > m_zoom) {
        lowResHelper.setZoom(m_lowresZoom);
        lowResObj = lowResHelper.readBody(bodyId);
        int s = misc::GetZoomScale(m_lowresZoom);
        scale = ZIntPoint(s, s, s);
      }
    }

    if (lowResObj != nullptr) {
      lowResObj->remove(m_range);
      ZIntCuboid box = m_range;
      box.scaleDown(scale);
      box.expand(-2, -2, -2);
      lowResObj->remove(box);
      lowResObj->setDsIntv(scale - 1);

      result.push_back(lowResObj);
    }
  }

  return result;
}

ZDvidBodyHelper ZDvidBodyHelper::fork() const
{
  return *this;
}
