#ifndef ZDVIDLABELSLICE_H
#define ZDVIDLABELSLICE_H

#include "zstackobject.h"
#include "zdvidtarget.h"
#include "zobject3dscan.h"
#include "zobject3dscanarray.h"
#include "zstackviewparam.h"
#include "zobjectcolorscheme.h"
#include "neutube.h"

class ZFlyEmBodyMerger;
class QColor;

class ZDvidLabelSlice : public ZStackObject
{
public:
  ZDvidLabelSlice();
  ZDvidLabelSlice(int maxWidth, int maxHeight);

  void setMaxSize(int maxWidth, int maxHeight);

  void update(const ZStackViewParam &viewParam);
  void update(int z);
  void update();

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  const std::string& className() const;

  inline void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  bool hit(double x, double y, double z);

  void selectHit(bool appending = false);
  void deselectAll();
  void toggleHitSelection(bool appending = false);
  void clearSelection();


  void setSelection(
      std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);
  void addSelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  void xorSelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);

  template <typename InputIterator>
  void addSelection(const InputIterator &begin, const InputIterator &end,
                    NeuTube::EBodyLabelType labelType);


  template <typename InputIterator>
  void xorSelection(const InputIterator &begin, const InputIterator &end,
                    NeuTube::EBodyLabelType labelType);

  template <typename InputIterator>
  void xorSelectionGroup(const InputIterator &begin, const InputIterator &end,
                         NeuTube::EBodyLabelType labelType);


  inline const std::set<uint64_t>& getSelected() const {
    return m_selectedSet;
  }

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

  uint64_t getHitLabel() const;

private:
  inline const ZDvidTarget& getDvidTarget() const { return m_dvidTarget; }
  void assignColorMap();
  void forceUpdate(const ZStackViewParam &viewParam);
  //void updateLabel(const ZFlyEmBodyMerger &merger);
  void init(int maxWidth, int maxHeight);

private:
  ZDvidTarget m_dvidTarget;
  ZObject3dScanArray m_objArray;
  ZStackViewParam m_currentViewParam;
  ZObjectColorScheme m_objColorSheme;
  uint64_t m_hitLabel; //Mapped label
  std::set<uint64_t> m_selectedSet; //Mapped label set
  ZFlyEmBodyMerger *m_bodyMerger;

  int m_maxWidth;
  int m_maxHeight;
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
void ZDvidLabelSlice::xorSelectionGroup(
    const InputIterator &begin, const InputIterator &end,
    NeuTube::EBodyLabelType labelType)
{
  std::set<uint64_t> labelSet;

  bool selecting = false;
  for (InputIterator iter = begin; iter != end; ++iter) {
    uint64_t label = getMappedLabel(*iter, labelType);
    labelSet.insert(label);
    if (m_selectedSet.count(label) == 0) { //any body has not been selected
      selecting = true;
    }
  }

  if (selecting) {
    for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
         iter != labelSet.end(); ++iter) {
      m_selectedSet.insert(*iter);
    }
  } else {
    for (std::set<uint64_t>::const_iterator iter  = labelSet.begin();
         iter != labelSet.end(); ++iter) {
      m_selectedSet.erase(*iter);
    }
  }
}

#endif // ZDVIDLABELSLICE_H
