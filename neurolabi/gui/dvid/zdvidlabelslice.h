#ifndef ZDVIDLABELSLICE_H
#define ZDVIDLABELSLICE_H

#include <QCache>
#include <QMutex>

#include "common/zsharedpointer.h"
#include "zuncopyable.h"
#include "neutube.h"
#include "zstackobject.h"
#include "zdvidtarget.h"
#include "zobject3dscan.h"
#include "zobject3dscanarray.h"
#include "zobjectcolorscheme.h"
#include "zimage.h"
#include "zselector.h"

#include "flyem/zflyembodycolorscheme.h"
#include "flyem/zflyembodymerger.h"

class QColor;
class ZArray;
class ZPixmap;
class ZDvidDataSliceHelper;
class ZStackViewParam;
class ZArbSliceViewParam;
class ZTask;
class ZStackDoc;
class ZDvidDataSliceTaskFactory;

class ZDvidLabelSlice : public ZStackObject, ZUncopyable
{
public:
  ZDvidLabelSlice();
  ZDvidLabelSlice(int maxWidth, int maxHeight);
  ~ZDvidLabelSlice();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_LABEL_SLICE;
  }

  void setMaxSize(const ZStackViewParam &viewParam, int maxWidth, int maxHeight);

  bool update(const ZStackViewParam &viewParam);
//  bool update(const QRect &dataRect, int zoom, int z);
//  void update(int z);
//  void update();

  void setUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);

  void updateFullView(const ZStackViewParam &viewParam);
//  void disableFullView();

  void setSliceAxis(neutu::EAxis sliceAxis);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

//  const std::string& className() const;

  void setDvidTarget(const ZDvidTarget &target);

  bool hit(double x, double y, double z);

  void selectHit(bool appending = false);
//  void selectLabel(uint64_t bodyId, bool appending = false);

  void deselectAll();
  void toggleHitSelection(bool appending = false);
  void clearSelection();

  bool isSelectionFrozen() const { return m_selectionFrozen; }
  void freezeSelection(bool on) { m_selectionFrozen = on; }

  bool isSupervoxel() const;

  void setSelection(
      const std::set<uint64_t> &selected, neutu::ELabelSource labelType);
  void addSelection(uint64_t bodyId, neutu::ELabelSource labelType);
  void xorSelection(uint64_t bodyId, neutu::ELabelSource labelType);
  void removeSelection(uint64_t bodyId, neutu::ELabelSource labelType);

  template <typename InputIterator>
  void addSelection(const InputIterator &begin, const InputIterator &end,
                    neutu::ELabelSource labelType);

  template <typename InputIterator>
  void setSelection(const InputIterator &begin, const InputIterator &end,
                    neutu::ELabelSource labelType);


  template <typename InputIterator>
  void xorSelection(const InputIterator &begin, const InputIterator &end,
                    neutu::ELabelSource labelType);

  template <typename InputIterator>
  void xorSelectionGroup(const InputIterator &begin, const InputIterator &end,
                         neutu::ELabelSource labelType);

  inline const std::set<uint64_t>& getSelectedOriginal() const {
    return m_selectedOriginal;
  }

  std::set<uint64_t> getSelected(neutu::ELabelSource labelType) const;

  bool isBodySelected(uint64_t bodyId, neutu::ELabelSource labelType) const;

  void setBodyMerger(ZFlyEmBodyMerger *bodyMerger);
  void updateLabelColor();

  const ZObjectColorScheme& getColorScheme() const {
    return m_objColorSheme;
  }

  QColor getLabelColor(uint64_t label, neutu::ELabelSource labelType) const;
  QColor getLabelColor(int64_t label, neutu::ELabelSource labelType) const;

  uint64_t getMappedLabel(const ZObject3dScan &obj) const;
  uint64_t getMappedLabel(uint64_t label) const;
  uint64_t getMappedLabel(
      uint64_t label, neutu::ELabelSource labelType) const;

  std::set<uint64_t> getOriginalLabelSet(uint64_t mappedLabel) const;

  uint64_t getHitLabel() const;
  std::set<uint64_t> getHitLabelSet() const;

//  const ZStackViewParam& getViewParam() const;
  int getCurrentZ() const;
  QRect getDataRect() const;

  void mapSelection();

  void forceUpdate(bool ignoringHidden);

  void setCenterCut(int width, int height);

  //Selection events
  void recordSelection();
  void processSelection();

  const ZSelector<uint64_t>& getSelector() const {
    return m_selector;
  }

  ZSelector<uint64_t>& getSelector() {
    return m_selector;
  }

  void setCustomColorMap(const ZSharedPointer<ZFlyEmBodyColorScheme> &colorMap);
  void removeCustomColorMap();
  bool hasCustomColorMap() const;
  void assignColorMap();

  ZImage* getPaintBuffer() {
    return m_paintBuffer;
  }

  int64_t getReadingTime() const;

  bool refreshReaderBuffer();

  void paintBuffer();

  QRect getDataRect(const ZStackViewParam &viewParam) const;

  bool consume(ZArray *array, const ZStackViewParam &viewParam,
               int zoom, int centerCutX, int centerCutY, bool usingCenterCut);
  bool containedIn(const ZStackViewParam &viewParam, int zoom,
                   int centerCutX, int centerCutY, bool usingCenterCut) const;
  ZTask* makeFutureTask(ZStackDoc *doc);
  void setTaskFactory(std::unique_ptr<ZDvidDataSliceTaskFactory> &&factory);

  void allowBlinking(bool on);

private:
  const ZDvidTarget& getDvidTarget() const;// { return m_dvidTarget; }

  void forceUpdate(
      const ZStackViewParam &viewParam, bool ignoringHidden);
  void forceUpdate(const QRect &viewPort, int z, int zoom);
  void forceUpdate(const ZArbSliceViewParam &viewParam, int zoom);
//  void forceUpdate(bool ignoringHidden);
  //void updateLabel(const ZFlyEmBodyMerger &merger);
  void init(int maxWidth, int maxHeight,
            neutu::EAxis sliceAxis = neutu::EAxis::Z);
  QColor getCustomColor(uint64_t label) const;

  void paintBufferUnsync();
  void remapId(ZArray *label);
  void remapId();

  void remapId(uint64_t *array, const uint64_t *originalArray, uint64_t v);
  void remapId(uint64_t *array, const uint64_t *originalArray, uint64_t v,
               std::set<uint64_t> &selected);
  void remapId(uint64_t *array, const uint64_t *originalArray, uint64_t v,
               const ZFlyEmBodyMerger::TLabelMap &bodyMap);
  void remapId(uint64_t *array, const uint64_t *originalArray, uint64_t v,
               std::set<uint64_t> &selected,
               const ZFlyEmBodyMerger::TLabelMap &bodyMap);

  void updateRgbTable();

  ZFlyEmBodyMerger::TLabelMap getLabelMap() const;
  void clearLabelData();

  void updatePixmap(ZPixmap *pixmap) const;
  void updatePaintBuffer();
  void setTransform(ZImage *image) const;

  const ZDvidDataSliceHelper* getHelper() const {
    return m_helper.get();
  }
  ZDvidDataSliceHelper* getHelper() {
    return m_helper.get();
  }

  void setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);

  bool isPaintBufferAllocNeeded(int width, int height) const;

  int getFirstZoom(const ZStackViewParam &viewParam) const;

  bool hasValidPaintBuffer() const;

private:
  ZObject3dScanArray m_objArray;

  ZObjectColorScheme m_objColorSheme;
  ZSharedPointer<ZFlyEmBodyColorScheme> m_customColorScheme;

  QVector<int> m_rgbTable;

  uint64_t m_hitLabel; //Mapped label
  std::set<uint64_t> m_selectedOriginal;
  ZFlyEmBodyMerger *m_bodyMerger;
  ZImage *m_paintBuffer;

  ZArray *m_labelArray;
  ZArray *m_mappedLabelArray;
  QMutex m_updateMutex;

  std::set<uint64_t> m_prevSelectedOriginal;
  ZSelector<uint64_t> m_selector; //original labels


  std::unique_ptr<ZDvidDataSliceHelper> m_helper;
  std::unique_ptr<ZDvidDataSliceTaskFactory> m_taskFactory;

  bool m_selectionFrozen;
//  bool m_multiResUpdate = true;
};

template <typename InputIterator>
void ZDvidLabelSlice::xorSelection(
    const InputIterator &begin, const InputIterator &end,
    neutu::ELabelSource labelType)
{
  std::set<uint64_t> labelSet;

  for (InputIterator iter = begin; iter != end; ++iter) {
    labelSet.insert(getMappedLabel(*iter, labelType));
  }

  for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
       iter != labelSet.end(); ++iter) {
    xorSelection(*iter, neutu::ELabelSource::MAPPED);
  }
}

template <typename InputIterator>
void ZDvidLabelSlice::addSelection(
    const InputIterator &begin, const InputIterator &end,
    neutu::ELabelSource labelType)
{
  std::set<uint64_t> labelSet;

  for (InputIterator iter = begin; iter != end; ++iter) {
    labelSet.insert(getMappedLabel(*iter, labelType));
  }

  for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
       iter != labelSet.end(); ++iter) {
    addSelection(*iter, neutu::ELabelSource::MAPPED);
  }
}

template <typename InputIterator>
void ZDvidLabelSlice::setSelection(
    const InputIterator &begin, const InputIterator &end,
    neutu::ELabelSource labelType)
{
  clearSelection();
  addSelection(begin, end, labelType);
  paintBuffer();
}

template <typename InputIterator>
void ZDvidLabelSlice::xorSelectionGroup(
    const InputIterator &begin, const InputIterator &end,
    neutu::ELabelSource labelType)
{
  std::set<uint64_t> labelSet; //original label set

  switch (labelType) {
  case neutu::ELabelSource::MAPPED:
    for (InputIterator iter = begin; iter != end; ++iter) {
//      uint64_t label = getMappedLabel(*iter, labelType);
      std::set<uint64_t> sourceLabel = getOriginalLabelSet(*iter);
      labelSet.insert(sourceLabel.begin(), sourceLabel.end());
    }
    break;
  case neutu::ELabelSource::ORIGINAL:
    for (InputIterator iter = begin; iter != end; ++iter) {
      uint64_t label = *iter;
      labelSet.insert(label);
    }
    break;
  }

  bool selecting = false;
  for (std::set<uint64_t>::const_iterator iter = labelSet.begin();
       iter != labelSet.end(); ++iter) {
    uint64_t label = *iter;
    if (m_selectedOriginal.count(label) == 0) { //any body has not been selected
      selecting = true;
      break;
    }
  }

  if (selecting) {
    for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
         iter != labelSet.end(); ++iter) {
      m_selectedOriginal.insert(*iter);
    }
  } else {
    for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
         iter != labelSet.end(); ++iter) {
      m_selectedOriginal.erase(*iter);
    }
  }
}


#endif // ZDVIDLABELSLICE_H
