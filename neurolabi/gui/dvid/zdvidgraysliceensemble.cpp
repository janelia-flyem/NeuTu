#include "zdvidgraysliceensemble.h"

#include "zdvidgrayslice.h"
#include "zdvidtarget.h"
#include "zstackobjectsourcefactory.h"

ZDvidGraySliceEnsemble::ZDvidGraySliceEnsemble()
{
  setTarget(ZStackObject::ETarget::TILE_CANVAS);
  m_type = GetType();
}

ZDvidGraySliceEnsemble::~ZDvidGraySliceEnsemble()
{

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

void ZDvidGraySliceEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  m_sliceList.clear();
  std::vector<ZDvidTarget> targetList = target.getGrayScaleTargetList();
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
