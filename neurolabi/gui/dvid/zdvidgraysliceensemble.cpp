#include "zdvidgraysliceensemble.h"

#include "zdvidgrayslice.h"
#include "zdvidtarget.h"
#include "zstackobjectsourcefactory.h"
#include "zdvidenv.h"
#include "zcontrastprotocol.h"

ZDvidGraySliceEnsemble::ZDvidGraySliceEnsemble()
{
  setTarget(ZStackObject::ETarget::TILE_CANVAS);
  m_type = GetType();
}

ZDvidGraySliceEnsemble::~ZDvidGraySliceEnsemble()
{

}

bool ZDvidGraySliceEnsemble::activateNext()
{
  if (m_sliceList.size() > 1) {
    std::shared_ptr<ZDvidGraySlice> currentSlice =
        ZDvidGraySliceEnsemble::getActiveSlice();

    ++m_activeIndex;
    if (m_activeIndex >= m_sliceList.size()) {
      m_activeIndex = 0;
    }

    std::shared_ptr<ZDvidGraySlice> newSlice =
        ZDvidGraySliceEnsemble::getActiveSlice();

    newSlice->update(currentSlice->getViewParam());

    return true;
  }

  return false;
}

std::shared_ptr<ZDvidGraySlice> ZDvidGraySliceEnsemble::getActiveSlice() const
{
  if (m_activeIndex < m_sliceList.size()) {
    return m_sliceList[m_activeIndex];
  }

  return std::shared_ptr<ZDvidGraySlice>();
}

std::shared_ptr<ZDvidGraySlice> ZDvidGraySliceEnsemble::getSlice(
    const std::string &source) const
{
  for (auto slice : m_sliceList) {
    if (slice->getSource() == source) {
      return slice;
    }
  }

  return std::shared_ptr<ZDvidGraySlice>();
}

void ZDvidGraySliceEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option, neutu::EAxis sliceAxis) const
{
  std::shared_ptr<ZDvidGraySlice> grayslice = getActiveSlice();
  if (grayslice) {
    grayslice->display(painter, slice, option, sliceAxis);
  }
}

void ZDvidGraySliceEnsemble::prepare(const ZDvidTarget &target)
{
  m_sliceList.clear();
  std::vector<ZDvidTarget> targetList = target.getGrayScaleTargetList();
  prepare(targetList);
}

void ZDvidGraySliceEnsemble::prepare(const std::vector<ZDvidTarget> &targetList)
{
  for (const ZDvidTarget &target : targetList) {
    if (target.isValid()) {
      std::shared_ptr<ZDvidGraySlice> grayslice =
          std::shared_ptr<ZDvidGraySlice>(new ZDvidGraySlice);
      grayslice->setSliceAxis(getSliceAxis());
      grayslice->setDvidTarget(target);
      std::string source =
          ZStackObjectSourceFactory::MakeDvidGraySliceSource(getSliceAxis());
      if (!m_sliceList.empty()) {
        source += "#" + std::to_string(m_sliceList.size());
      }
      grayslice->setSource(source);

      m_sliceList.push_back(grayslice);
    }
  }
}

void ZDvidGraySliceEnsemble::prepare(const ZDvidEnv &env)
{
  prepare(env.getTargetList(ZDvidEnv::ERole::GRAYSCALE));
}


bool ZDvidGraySliceEnsemble::update(const ZStackViewParam &viewParam)
{
  std::shared_ptr<ZDvidGraySlice> grayslice = getActiveSlice();
  if (grayslice) {
    return grayslice->update(viewParam);
  }

  return false;
}

ZTask* ZDvidGraySliceEnsemble::makeFutureTask(ZStackDoc *doc)
{
  std::shared_ptr<ZDvidGraySlice> grayslice = getActiveSlice();
  if (grayslice) {
    return grayslice->makeFutureTask(doc);
  }

  return nullptr;
}

void ZDvidGraySliceEnsemble::updateContrast(bool contrast)
{
  m_usingContrastProtocol = contrast;
  std::shared_ptr<ZDvidGraySlice> grayslice = getActiveSlice();
  if (grayslice) {
    grayslice->updateContrast(contrast);
  }
}

void ZDvidGraySliceEnsemble::updateContrast(
    const ZJsonObject &protocolJson, bool hc)
{
  m_usingContrastProtocol = hc;
  if (!m_sliceList.empty()) {
    std::shared_ptr<ZDvidGraySlice> grayslice = m_sliceList.front();
    ZContrastProtocol protocal;
    protocal.load(protocolJson);
    grayslice->setContrastProtocol(protocal);
    grayslice->updateContrast(hc);
  }
}

void ZDvidGraySliceEnsemble::setCenterCut(int width, int height)
{
  for (auto slice : m_sliceList) {
    slice->setCenterCut(width, height);
  }
}
