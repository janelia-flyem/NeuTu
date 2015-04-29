#include "zdvidlabelslice.h"

#include <QColor>

#include "zarray.h"
#include "dvid/zdvidreader.h"
#include "zobject3dfactory.h"
#include "flyem/zflyembodymerger.h"

ZDvidLabelSlice::ZDvidLabelSlice()
{
  setTarget(ZStackObject::OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_DVID_LABEL_SLICE;
  m_objColorSheme.setColorScheme(ZColorScheme::RANDOM_COLOR);
  m_hitLabel = -1;
  m_bodyMerger = NULL;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidLabelSlice)

void ZDvidLabelSlice::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (isVisible()) {
    for (ZObject3dScanArray::const_iterator iter = m_objArray.begin();
         iter != m_objArray.end(); ++iter) {
      ZObject3dScan &obj = const_cast<ZObject3dScan&>(*iter);
      if (m_selectedSet.count((int64_t) obj.getLabel()) > 0) {
        obj.setSelected(true);
      } else {
        obj.setSelected(false);
      }
      obj.display(painter, slice, option);
    }
  }
}

void ZDvidLabelSlice::update()
{
  if (m_objArray.empty()) {
    forceUpdate(m_currentViewParam);
  }
}

void ZDvidLabelSlice::forceUpdate(const ZStackViewParam &viewParam)
{
  m_objArray.clear();
  if (isVisible()) {
    int yStep = 1;

    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      QRect viewPort = viewParam.getViewPort();

      ZArray *labelArray = reader.readLabels64(
            getDvidTarget().getLabelBlockName(),
            viewPort.left(), viewPort.top(), viewParam.getZ(),
            viewPort.width(), viewPort.height(), 1);

      ZObject3dFactory::MakeObject3dScanArray(*labelArray, yStep, &m_objArray);

      m_objArray.translate(viewPort.left(), viewPort.top(),
                           viewParam.getZ());
      if (m_bodyMerger != NULL) {
        updateLabel(*m_bodyMerger);
      }
      assignColorMap();

      delete labelArray;
    }
  }
}

void ZDvidLabelSlice::update(const ZStackViewParam &viewParam)
{
  ZStackViewParam newViewParam = viewParam;
  int area = viewParam.getViewPort().width() * viewParam.getViewPort().height();
  const int maxWidth = 512;
  const int maxHeight = 512;
  if (area > maxWidth * maxHeight) {
    newViewParam.resize(maxWidth, maxHeight);
  }

  if (!m_currentViewParam.contains(newViewParam)) {
    forceUpdate(newViewParam);

    m_currentViewParam = newViewParam;
  }
}

QColor ZDvidLabelSlice::getColor(uint64_t label) const
{
  return getColor((int64_t) label);
}

QColor ZDvidLabelSlice::getColor(int64_t label) const
{
  QColor color = m_objColorSheme.getColor(abs((int) label));
  color.setAlpha(64);

  return color;
}

void ZDvidLabelSlice::assignColorMap()
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.setColor(getColor(obj.getLabel()));
  }
}

bool ZDvidLabelSlice::hit(double x, double y, double z)
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    if (obj.hit(x, y, z)) {
      m_hitLabel = obj.getLabel();
      return true;
    }
  }

  return false;
}

void ZDvidLabelSlice::selectHit(bool appending)
{
  if (m_hitLabel >= 0) {
    if (!appending) {
      clearSelection();
    }
    m_selectedSet.insert(m_hitLabel);
  }
}

void ZDvidLabelSlice::deselectAll()
{
  m_selectedSet.clear();
}

void ZDvidLabelSlice::toggleHitSelection(bool appending)
{
  bool hasSelected = false;
  if (m_selectedSet.count(m_hitLabel) > 0) {
    hasSelected = true;
    m_selectedSet.erase(m_hitLabel);
  }

  if (!appending) {
    clearSelection();
  }

  if (!hasSelected) {
    selectHit(true);
  }
}

void ZDvidLabelSlice::clearSelection()
{
  m_selectedSet.clear();
}

void ZDvidLabelSlice::updateLabel(const ZFlyEmBodyMerger &merger)
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.setLabel(merger.getFinalLabel(obj.getLabel()));
  }
}

void ZDvidLabelSlice::updateLabelColor()
{
  if (m_bodyMerger != NULL) {
    updateLabel(*m_bodyMerger);
  }
  assignColorMap();
}

void ZDvidLabelSlice::setBodyMerger(ZFlyEmBodyMerger *bodyMerger)
{
  m_bodyMerger = bodyMerger;
}
