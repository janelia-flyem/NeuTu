#ifndef FLYEMPOINTANNOTATIONENSEMBLE_H
#define FLYEMPOINTANNOTATIONENSEMBLE_H

#include <QElapsedTimer>
#include <neulib/core/stringbuilder.h>

#include "geometry/zaffinerect.h"
#include "data3d/displayconfig.h"
#include "logging/utilities.h"
#include "zstackobject.h"
#include "zselector.h"
#include "bigdata/zintpointannotationblockgrid.hpp"

class ZDvidTarget;

template<typename TItem, typename TChunk>
class FlyEmPointAnnotationEnsemble : public ZStackObject
{
public:
  FlyEmPointAnnotationEnsemble() {
    m_usingCosmeticPen = true;
  };

  bool display_inner(
      QPainter *painter, const DisplayConfig &config) const override;

  virtual bool display_selected(
      QPainter *painter, const DisplayConfig &config) const;

  void addItem(const TItem &item);
  void removeItem(const ZIntPoint &pos);
  void updatePartner(const ZIntPoint &pos);
  void updateItem(const ZIntPoint &pos);
  void moveItem(const ZIntPoint &from, const ZIntPoint &to);

  TItem getItem(const ZIntPoint &pos) const
  {
    return m_blockGrid->getItem(pos);
  }

  TItem getSingleSelectedItem() const;

  /*!
   * \brief Remove selected todos
   *
   * It returns the number of todos that are removed.
   */
  int removeSelected(std::function<void(const std::string &)> errorProcessor);

  bool hit(double x, double y, double z, int viewId) override;

  void processHit(ESelection s) override;
  void deselectSub() override;

  bool hasSelected() const;

  std::set<ZIntPoint> getSelectedPos() const;

protected:
  void selectAt(const ZIntPoint &pos);
  void deselectAt(const ZIntPoint &pos);
  void setSelectionAt(const ZIntPoint &pos, bool selecting);
  void processDeselected();

  template<typename TSource>
  void _setDvidTarget(const ZDvidTarget &target)
  {
    TSource *source = new TSource;
    source->setDvidTarget(target);
    m_blockGrid->setSource(std::shared_ptr<TSource>(source));
  }

protected:
  std::shared_ptr<ZIntPointAnnotationBlockGrid<TItem, TChunk>> m_blockGrid;
  TItem m_hitItem;
  ZSelector<ZIntPoint> m_selector;
};

template<typename T, typename TChunk>
bool FlyEmPointAnnotationEnsemble<T, TChunk>::display_selected(
    QPainter *painter, const DisplayConfig &config) const
{
  bool painted =  false;
  std::set<ZIntPoint> selected = getSelectedPos();
  for (const ZIntPoint &pos : selected) {
    auto item = getItem(pos);
    if (item.isValid()) {
      if (item.display(painter, config)) {
        painted = true;
      }
    }
  }

  return painted;
}

template<typename T, typename TChunk>
bool FlyEmPointAnnotationEnsemble<T, TChunk>::display_inner(
    QPainter *painter, const DisplayConfig &config) const
{
  bool painted = false;
  ZAffineRect rect;
  QElapsedTimer timer;
  timer.start();

  std::function<void(const T&)> displayFunc = [&](const T &item) {
    if (!item.isSelected()) { // Selected items are painted separately
      if (item.display(painter, config)) {
        painted = true;
      }
    }
  };

  auto selectedPos = getSelectedPos();
  if (hasVisualEffect(neutu::display::VE_GROUP_HIGHLIGHT)) {
    displayFunc = [&](const T &item) {
      if (!item.isSelected()) { // Selected items are painted separately
        if (item.isPrimaryPartner()) {
          if (item.display(painter, config)) {
            painted = true;
          }
        } else {
          if (item.hasPartnerIn(selectedPos)) {
            if (item.display(painter, config)) {
              painted = true;
            }
          } else {
            if (item.display(
                  painter, neutu::data3d::DisplayConfigBuilder(config).withStyle(
                    neutu::data3d::EDisplayStyle::SKELETON))) {
              painted = true;
            }
          }
        }
      }
    };
  }

  int count = m_blockGrid->forEachIntersectedBlockApprox(
        config.getCutRect(
          painter->device()->width(), painter->device()->height(),
          neutu::data3d::ESpace::CANVAS), [&](int i, int j, int k) {
    m_blockGrid->forEachItemInChunk(i, j, k, displayFunc);
  }, 1.5);

  neutu::LogProfileInfo(
        timer.elapsed(),
        neulib::StringBuilder("Display point annotation: ").
        append(count).append(" blocks"));

  bool selectedPainted = display_selected(painter, config);
  painted = painted || selectedPainted;

#ifdef _DEBUG_0
  std::cout << "FlyEmPointAnnotationEnsemble<T, TChunk>::display: " << painted << std::endl;
#endif
  return painted;
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::addItem(const T &item)
{
  T oldItem = m_blockGrid->getCachedItem(item.getPosition());
  m_blockGrid->addItem(item);

  if (oldItem.isValid()) {
    if (oldItem.isSelected() != item.isSelected()) {
      m_selector.removeObject(item.getPosition());
    }
  }

  if (item.isSelected()) {
    m_selector.setSelection(item.getPosition(), item.isSelected());
  }
}

template<typename T, typename TChunk>
bool FlyEmPointAnnotationEnsemble<T, TChunk>::hit(
    double x, double y, double z, int viewId)
{
  if (isVisible() && isHittable()) {
    m_hitItem = m_blockGrid->hitClosestCachedItem(
          x, y, z, m_blockGrid->getBlockSize().toPoint().length() * 0.5, viewId);

    return m_hitItem.isValid();
  }

  return false;
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::processDeselected()
{
  m_selector.forEachDeselected([&](const ZIntPoint &pt) {
    m_blockGrid->setSelectionForCached(pt, false);
  });
  m_selector.clearDeselected();
//  setSelected(m_selector.hasSelected());
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::setSelectionAt(
    const ZIntPoint &pos, bool selecting)
{
  m_selector.setSelection(pos, selecting);
  processDeselected();
  m_blockGrid->setSelectionForCached(pos, selecting);
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::selectAt(const ZIntPoint &pos)
{
  setSelectionAt(pos, true);
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::deselectAt(const ZIntPoint &pos)
{
  setSelectionAt(pos, false);
}

template<typename T, typename TChunk>
bool FlyEmPointAnnotationEnsemble<T, TChunk>::hasSelected() const
{
  return m_selector.hasSelected();
}

template<typename T, typename TChunk>
std::set<ZIntPoint> FlyEmPointAnnotationEnsemble<T, TChunk>::getSelectedPos() const
{
  return m_selector.getSelectedSet();
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::processHit(ESelection s)
{
  ZIntPoint pos = m_hitItem.getPosition();
  switch (s) {
  case ESelection::SELECT_SINGLE:
    if (m_hitItem.isValid()) {
      deselectSub();
      selectAt(pos);
    }
    break;
  case ESelection::SELECT_MULTIPLE:
    if (m_hitItem.isValid()) {
      selectAt(pos);
    }
    break;
  case ESelection::SELECT_TOGGLE:
    if (m_hitItem.isValid()) {
      setSelectionAt(pos, !m_selector.isInSelectedSet(pos));
    }
    break;
  case ESelection::SELECT_TOGGLE_SINGLE:
    if (m_hitItem.isValid()) {
      bool selected = m_selector.isInSelectedSet(pos);
      deselectSub();
      if (!selected) {
        setSelectionAt(pos, true);
      }
    }
    break;
  case ESelection::DESELECT:
    deselectSub();
    break;
  }

  m_hitItem.invalidate();
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::deselectSub()
{
  m_selector.deselectAll();
  processDeselected();
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::removeItem(const ZIntPoint &pos)
{
  m_blockGrid->removeItem(pos);
  m_selector.removeObject(pos);
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::updatePartner(const ZIntPoint &pos)
{
  m_blockGrid->syncPartnerToCache(pos);
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::updateItem(const ZIntPoint &pos)
{
  m_blockGrid->syncItemToCache(pos);
}

template<typename T, typename TChunk>
void FlyEmPointAnnotationEnsemble<T, TChunk>::moveItem(
    const ZIntPoint &from, const ZIntPoint &to)
{
  m_blockGrid->moveItem(from, to);
}

template<typename T, typename TChunk>
T FlyEmPointAnnotationEnsemble<T, TChunk>::getSingleSelectedItem() const
{
  if (m_selector.getSelectedSet().size() == 1) {
    return getItem(*(m_selector.getSelectedSet().begin()));
  }

  return T();
}

template<typename T, typename TChunk>
int FlyEmPointAnnotationEnsemble<T, TChunk>::removeSelected(
    std::function<void(const std::string&)> errorProcessor)
{
  std::vector<ZIntPoint> removed;
  m_selector.forEachSelected([&](const ZIntPoint &pos) {
    try {
      m_blockGrid->removeItem(pos);
      removed.push_back(pos);
    } catch (std::exception &e) {
      errorProcessor(e.what());
    }
  });
  for (const ZIntPoint &pos : removed) {
    m_selector.removeObject(pos);
  }

  return removed.size();
}

#endif // FLYEMPOINTANNOTATIONENSEMBLE_H
