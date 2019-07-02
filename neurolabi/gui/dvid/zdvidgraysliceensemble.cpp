#include "zdvidgraysliceensemble.h"

#include "zdvidgrayslice.h"
#include "zdvidtarget.h"
#include "zstackobjectsourcefactory.h"

ZDvidGraySliceEnsemble::ZDvidGraySliceEnsemble()
{

}

ZDvidGraySliceEnsemble::~ZDvidGraySliceEnsemble()
{

}

std::shared_ptr<ZDvidGraySlice> ZDvidGraySliceEnsemble::getActiveSlice() const
{
  if (m_activeIndex < m_grayList.size()) {
    return m_grayList[m_activeIndex];
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
  m_grayList.clear();
  std::vector<ZDvidTarget> targetList = target.getGrayScaleTargetList();
  for (const ZDvidTarget &target : targetList) {
    if (target.isValid()) {
      std::shared_ptr<ZDvidGraySlice> grayslice =
          std::shared_ptr<ZDvidGraySlice>(new ZDvidGraySlice);
      grayslice->setSliceAxis(getSliceAxis());
      grayslice->setDvidTarget(target);
      std::string source =
          ZStackObjectSourceFactory::MakeDvidGraySliceSource(getSliceAxis());
      if (!m_grayList.empty()) {
        source += "#" + std::to_string(m_grayList.size());
      }
      grayslice->setSource(source);

      m_grayList.push_back(grayslice);
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
