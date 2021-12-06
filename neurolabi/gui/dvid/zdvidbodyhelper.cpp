#include "zdvidbodyhelper.h"

#include <QElapsedTimer>

#include "logging/zqslog.h"
#include "logging/zlog.h"
//#include "zdvidreader.h"
#include "zobject3dscan.h"
#include "misc/miscutility.h"
#include "logging/zlog.h"
#include "flyem/flyembodysource.h"

ZDvidBodyHelper::ZDvidBodyHelper(std::shared_ptr<FlyEmBodySource> source) :
  m_bodySource(source)
{
}

/*
const ZDvidReader *ZDvidBodyHelper::getDvidReader() const
{
  return m_bodySource->getReader();
}
*/

void ZDvidBodyHelper::setCanonizing(bool on)
{
  m_canonizing = on;
}

/*
void ZDvidBodyHelper::setLabelType(neutu::EBodyLabelType type)
{
  m_labelType = type;
}
*/

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

ZObject3dScan* ZDvidBodyHelper::readBody(uint64_t bodyId)
{
  ZObject3dScan *out = nullptr;

  if (!m_coarseVol) {
    out = m_bodySource->getSparsevol(bodyId, m_zoom, m_range);
    /*
    out = getDvidReader()->readBody(
          bodyId, m_labelType, m_zoom, m_range, m_canonizing, result);
    if (out) {
      out->setDsIntv(zgeom::GetZoomScale(m_zoom) - 1);
    }
    */
  } else {
    out = m_bodySource->getCoarseSparsevol(bodyId, m_range);
//    out = getDvidReader()->readCoarseBody(bodyId, m_labelType, m_range, result);
    if (out && m_canonizing) {
      out->canonize();
    }
  }

  return out;
}

namespace {
int adjust_min_corner(int x, int s)
{
  if (x % s >= s / 2) {
    x = (x / s) * s + s / 2;
  } else {
    x = (x / s) * s - s / 2;
  }

  return x;
}

int adjust_max_corner(int x, int s)
{
  if (x % s <= s / 2) {
    x = (x / s) * s + s / 2;
  } else {
    x = (x / s + 1) * s + s / 2;
  }

  return x;
}
}

ZIntCuboid ZDvidBodyHelper::getAdjustedRange() const
{
  ZIntCuboid range = m_range;
  if (!m_range.isEmpty() && (m_coarseVol || (m_lowresZoom < m_zoom))) { //hybrid
//    ZDvidInfo dvidInfo = getDvidReader()->readLabelInfo();
//    ZIntPoint scale = dvidInfo.getBlockSize();
    ZIntPoint scale = m_bodySource->getBlockSize();

    int s = zgeom::GetZoomScale(m_zoom);
    range.scaleDown(s);
    range.scaleUp(s);

    range.set(
          ZIntPoint(adjust_min_corner(range.getMinCorner().getX(), scale.getX()),
                    adjust_min_corner(range.getMinCorner().getY(), scale.getY()),
                    adjust_min_corner(range.getMinCorner().getZ(), scale.getZ())),
          ZIntPoint(adjust_max_corner(range.getMaxCorner().getX(), scale.getX()),
                    adjust_max_corner(range.getMaxCorner().getY(), scale.getY()),
                    adjust_max_corner(range.getMaxCorner().getZ(), scale.getZ())));
  }

  return range;
}

std::vector<std::shared_ptr<ZObject3dScan> >
ZDvidBodyHelper::readHybridBody(uint64_t bodyId)
{
  std::vector<std::shared_ptr<ZObject3dScan>> result;

  ZIntCuboid range = getAdjustedRange();

  ZDvidBodyHelper highResHelper = fork();
  highResHelper.setCoarse(false);
  highResHelper.setRange(range);
  highResHelper.setZoom(m_zoom);

  QElapsedTimer timer;
  timer.start();
  ZObject3dScan *highResObj = highResHelper.readBody(bodyId);
  KLog(neutu::TOPIC_NULL) << ZLog::Profile()
         << ZLog::Diagnostic(
              "High res reading time for " +
              std::to_string(bodyId) + "@" + std::to_string(m_zoom))
         << ZLog::Duration(timer.elapsed());
//  LINFO() << "High res reading time:" << timer.elapsed() << "ms";

  if (highResObj != NULL) {
    if (!highResObj->isEmpty()) {
      result.emplace_back(highResObj);
    } else {
      delete highResObj;
      ZWARN(neutu::TOPIC_NULL) << "Failed to read highres object";
    }
  }

  if (!range.isEmpty()) { //load low res parts
    ZDvidBodyHelper lowResHelper = fork();
    lowResHelper.setRange(ZIntCuboid());
    ZObject3dScan *lowResObj = nullptr;

    ZIntPoint scale;
    if (m_coarseVol) {
      lowResObj = lowResHelper.readBody(bodyId);
//      ZDvidInfo dvidInfo = getDvidReader()->readLabelInfo();
//      scale = dvidInfo.getBlockSize();
      scale = m_bodySource->getBlockSize();
    } else {
      if (m_lowresZoom > m_zoom) {
        lowResHelper.setZoom(m_lowresZoom);
        timer.restart();
        lowResObj = lowResHelper.readBody(bodyId);
        KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description(
                  QString("Low res reading time: %1 ms").
                  arg(timer.elapsed()).toStdString());
//        LINFO() << "Low res reading time:" << timer.elapsed() << "ms";
        int s = zgeom::GetZoomScale(m_lowresZoom);
        scale = ZIntPoint(s, s, s);
      }
    }

    if (lowResObj != nullptr) {
//      lowResObj->remove(range);
      ZIntCuboid box = range;
      box.scaleDown(scale);
      box.expand(-1, -1, -1);
      timer.restart();
      lowResObj->remove(box);
      KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description(
                QString("Low res cropping time: %1 ms").
                arg(timer.elapsed()).toStdString());
//      LINFO() << "Low res cropping time:" << timer.elapsed() << "ms";
      lowResObj->setDsIntv(scale - 1);

      result.emplace_back(lowResObj);
    }
  }

  return result;
}

ZDvidBodyHelper ZDvidBodyHelper::fork() const
{
  return *this;
}
