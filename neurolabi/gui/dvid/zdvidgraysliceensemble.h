#ifndef ZDVIDGRAYSLICEENSEMBLE_H
#define ZDVIDGRAYSLICEENSEMBLE_H

#include <memory>
#include <vector>

#include "zstackobject.h"

class ZDvidGraySlice;
class ZDvidTarget;
class ZStackViewParam;
class ZTask;
class ZStackDoc;

class ZDvidGraySliceEnsemble : public ZStackObject
{
public:
  ZDvidGraySliceEnsemble();
  ~ZDvidGraySliceEnsemble();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_GRAY_SLICE_ENSEMBLE;
  }

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;

  void setDvidTarget(const ZDvidTarget &target);

  bool update(const ZStackViewParam &viewParam);
  ZTask* makeFutureTask(ZStackDoc *doc);

  std::shared_ptr<ZDvidGraySlice> getActiveSlice() const;
  std::shared_ptr<ZDvidGraySlice> getSlice(const std::string &source) const;

private:
  size_t m_activeIndex = 0;
  std::vector<std::shared_ptr<ZDvidGraySlice>> m_sliceList;
};

#endif // ZDVIDGRAYSLICEENSEMBLE_H
