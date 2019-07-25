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
class ZDvidEnv;
class ZJsonObject;

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

  void prepare(const ZDvidTarget &target);
  void prepare(const ZDvidEnv &env);
  void prepare(const std::vector<ZDvidTarget>& targetList);

  bool update(const ZStackViewParam &viewParam);
  ZTask* makeFutureTask(ZStackDoc *doc);

  std::shared_ptr<ZDvidGraySlice> getActiveSlice() const;
  std::shared_ptr<ZDvidGraySlice> getSlice(const std::string &source) const;

  void updateContrast(bool contrast);
  void updateContrast(const ZJsonObject &protocolJson, bool hc);
  void setCenterCut(int width, int height);

  /*!
   * \brief Activate next slice
   *
   * \return true iff the active slice is changed.
   */
  std::shared_ptr<ZDvidGraySlice> activateNext();

private:
  size_t m_activeIndex = 0;
  std::vector<std::shared_ptr<ZDvidGraySlice>> m_sliceList;
  bool m_usingContrastProtocol = false;
};

#endif // ZDVIDGRAYSLICEENSEMBLE_H
