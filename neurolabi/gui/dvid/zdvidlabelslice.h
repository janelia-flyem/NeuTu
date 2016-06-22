#ifndef ZDVIDLABELSLICE_H
#define ZDVIDLABELSLICE_H

#include "zstackobject.h"
#include "zdvidtarget.h"
#include "zobject3dscan.h"
#include "zobject3dscanarray.h"
#include "zstackviewparam.h"
#include "zobjectcolorscheme.h"
#include "neutube.h"
#include "zimage.h"
#include "zselector.h"
#include "dvid/zdvidreader.h"
#include "zsharedpointer.h"
#include "flyem/zflyembodycolorscheme.h"

class ZFlyEmBodyMerger;
class QColor;
class ZArray;

class ZDvidLabelSlice : public ZStackObject
{
public:
  ZDvidLabelSlice();
  ZDvidLabelSlice(int maxWidth, int maxHeight);
  ~ZDvidLabelSlice();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_LABEL_SLICE;
  }

  void setMaxSize(int maxWidth, int maxHeight);

  bool update(const ZStackViewParam &viewParam);
  void update(int z);
  void update();

  void updateFullView(const ZStackViewParam &viewParam);

  void setSliceAxis(NeuTube::EAxis sliceAxis);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  const std::string& className() const;

  void setDvidTarget(const ZDvidTarget &target);

  bool hit(double x, double y, double z);

  void selectHit(bool appending = false);
//  void selectLabel(uint64_t bodyId, bool appending = false);

  void deselectAll();
  void toggleHitSelection(bool appending = false);
  void clearSelection();

  bool isSelectionFrozen() const { return m_selectionFrozen; }
  void freezeSelection(bool on) { m_selectionFrozen = on; }


  void setSelection(
      const std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);
  void addSelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  void xorSelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);

  template <typename InputIterator>
  void addSelection(const InputIterator &begin, const InputIterator &end,
                    NeuTube::EBodyLabelType labelType);

  template <typename InputIterator>
  void setSelection(const InputIterator &begin, const InputIterator &end,
                    NeuTube::EBodyLabelType labelType);


  template <typename InputIterator>
  void xorSelection(const InputIterator &begin, const InputIterator &end,
                    NeuTube::EBodyLabelType labelType);

  template <typename InputIterator>
  void xorSelectionGroup(const InputIterator &begin, const InputIterator &end,
                         NeuTube::EBodyLabelType labelType);

  inline const std::set<uint64_t>& getSelectedOriginal() const {
    return m_selectedOriginal;
  }

  std::set<uint64_t> getSelected(NeuTube::EBodyLabelType labelType) const;

  bool isBodySelected(uint64_t bodyId, NeuTube::EBodyLabelType labelType) const;

  void setBodyMerger(ZFlyEmBodyMerger *bodyMerger);
  void updateLabelColor();

  const ZObjectColorScheme& getColorScheme() const {
    return m_objColorSheme;
  }

  QColor getColor(uint64_t label, NeuTube::EBodyLabelType labelType) const;
  QColor getColor(int64_t label, NeuTube::EBodyLabelType labelType) const;

  uint64_t getMappedLabel(const ZObject3dScan &obj) const;
  uint64_t getMappedLabel(uint64_t label) const;
  uint64_t getMappedLabel(
      uint64_t label, NeuTube::EBodyLabelType labelType) const;

  std::set<uint64_t> getOriginalLabelSet(uint64_t mappedLabel) const;

  uint64_t getHitLabel() const;

  const ZStackViewParam& getViewParam() const;

  void mapSelection();

  void forceUpdate();

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

private:
  inline const ZDvidTarget& getDvidTarget() const { return m_dvidTarget; }
  void forceUpdate(const ZStackViewParam &viewParam);
  //void updateLabel(const ZFlyEmBodyMerger &merger);
  void init(int maxWidth, int maxHeight,
            NeuTube::EAxis sliceAxis = NeuTube::Z_AXIS);
  QColor getCustomColor(uint64_t label) const;

private:
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZObject3dScanArray m_objArray;
  ZStackViewParam m_currentViewParam;
  ZObjectColorScheme m_objColorSheme;
  ZSharedPointer<ZFlyEmBodyColorScheme> m_customColorScheme;

  uint64_t m_hitLabel; //Mapped label
  std::set<uint64_t> m_selectedOriginal;
//  std::set<uint64_t> m_selectedSet; //Mapped label set
  ZFlyEmBodyMerger *m_bodyMerger;
  ZImage *m_paintBuffer;
  ZArray *m_labelArray;

  std::set<uint64_t> m_prevSelectedOriginal;
  ZSelector<uint64_t> m_selector; //original labels

  int m_maxWidth;
  int m_maxHeight;

  bool m_selectionFrozen;
  bool m_isFullView;
//  NeuTube::EAxis m_sliceAxis;
};

template <typename InputIterator>
void ZDvidLabelSlice::xorSelection(
    const InputIterator &begin, const InputIterator &end,
    NeuTube::EBodyLabelType labelType)
{
  std::set<uint64_t> labelSet;

  for (InputIterator iter = begin; iter != end; ++iter) {
    labelSet.insert(getMappedLabel(*iter, labelType));
  }

  for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
       iter != labelSet.end(); ++iter) {
    xorSelection(*iter, NeuTube::BODY_LABEL_MAPPED);
  }
}

template <typename InputIterator>
void ZDvidLabelSlice::addSelection(
    const InputIterator &begin, const InputIterator &end,
    NeuTube::EBodyLabelType labelType)
{
  std::set<uint64_t> labelSet;

  for (InputIterator iter = begin; iter != end; ++iter) {
    labelSet.insert(getMappedLabel(*iter, labelType));
  }

  for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
       iter != labelSet.end(); ++iter) {
    addSelection(*iter, NeuTube::BODY_LABEL_MAPPED);
  }
}

template <typename InputIterator>
void ZDvidLabelSlice::setSelection(
    const InputIterator &begin, const InputIterator &end,
    NeuTube::EBodyLabelType labelType)
{
  clearSelection();
  addSelection(begin, end, labelType);
}

template <typename InputIterator>
void ZDvidLabelSlice::xorSelectionGroup(
    const InputIterator &begin, const InputIterator &end,
    NeuTube::EBodyLabelType labelType)
{
  std::set<uint64_t> labelSet; //original label set

  switch (labelType) {
  case NeuTube::BODY_LABEL_MAPPED:
    for (InputIterator iter = begin; iter != end; ++iter) {
//      uint64_t label = getMappedLabel(*iter, labelType);
      std::set<uint64_t> sourceLabel = getOriginalLabelSet(*iter);
      labelSet.insert(sourceLabel.begin(), sourceLabel.end());
    }
    break;
  case NeuTube::BODY_LABEL_ORIGINAL:
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
