#include "zdvidlabelslice.h"

#include <QColor>

#include "zarray.h"
#include "dvid/zdvidreader.h"
#include "zobject3dfactory.h"
#include "flyem/zflyembodymerger.h"

ZDvidLabelSlice::ZDvidLabelSlice()
{
  init(512, 512);
}

ZDvidLabelSlice::ZDvidLabelSlice(int maxWidth, int maxHeight)
{
  init(maxWidth, maxHeight);
}

void ZDvidLabelSlice::init(int maxWidth, int maxHeight)
{
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_DVID_LABEL_SLICE;
  m_objColorSheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
  m_hitLabel = -1;
  m_bodyMerger = NULL;

  m_maxWidth = maxWidth;
  m_maxHeight = maxHeight;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidLabelSlice)

void ZDvidLabelSlice::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (isVisible()) {
    for (ZObject3dScanArray::const_iterator iter = m_objArray.begin();
         iter != m_objArray.end(); ++iter) {
      ZObject3dScan &obj = const_cast<ZObject3dScan&>(*iter);
      //if (m_selectedSet.count(obj.getLabel()) > 0) {
      if (m_selectedSet.count(getMappedLabel(obj)) > 0) {
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

      if (labelArray != NULL) {
        ZObject3dFactory::MakeObject3dScanArray(*labelArray, yStep, &m_objArray);

        m_objArray.translate(viewPort.left(), viewPort.top(),
                             viewParam.getZ());
        /*
        if (m_bodyMerger != NULL) {
          updateLabel(*m_bodyMerger);
        }
        */
        assignColorMap();

        delete labelArray;
      }
    }
  }
}

void ZDvidLabelSlice::update(int z)
{
  ZStackViewParam viewParam = m_currentViewParam;
  viewParam.setZ(z);

  update(viewParam);
}

void ZDvidLabelSlice::update(const ZStackViewParam &viewParam)
{
  ZStackViewParam newViewParam = viewParam;
  int area = viewParam.getViewPort().width() * viewParam.getViewPort().height();
//  const int maxWidth = 512;
//  const int maxHeight = 512;
  if (area > m_maxWidth * m_maxHeight) {
    newViewParam.resize(m_maxWidth, m_maxHeight);
  }

  if (!m_currentViewParam.contains(newViewParam)) {
    forceUpdate(newViewParam);

    m_currentViewParam = newViewParam;
  }
}

QColor ZDvidLabelSlice::getColor(
    uint64_t label, NeuTube::EBodyLabelType labelType) const
{
  return getColor((int64_t) label, labelType);
}

QColor ZDvidLabelSlice::getColor(
    int64_t label, NeuTube::EBodyLabelType labelType) const
{
  QColor color = m_objColorSheme.getColor(
        abs((int) getMappedLabel((uint64_t) label, labelType)));
  color.setAlpha(64);

  return color;
}

void ZDvidLabelSlice::assignColorMap()
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    //obj.setColor(getColor(obj.getLabel()));
    obj.setColor(getColor(obj.getLabel(), NeuTube::BODY_LABEL_ORIGINAL));
  }
}

bool ZDvidLabelSlice::hit(double x, double y, double z)
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    if (obj.hit(x, y, z)) {
      //m_hitLabel = obj.getLabel();
      m_hitLabel = getMappedLabel(obj);
      return true;
    }
  }

  return false;
}

void ZDvidLabelSlice::selectHit(bool appending)
{
  if (m_hitLabel > 0) {
    if (!appending) {
      clearSelection();
    }

//    addSelection(m_hitLabel);
    m_selectedSet.insert(m_hitLabel);
  }
}

void ZDvidLabelSlice::setSelection(std::set<uint64_t> &selected,
                                   NeuTube::EBodyLabelType labelType)
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_MAPPED:
    m_selectedSet = selected;
    break;
  case NeuTube::BODY_LABEL_ORIGINAL:
    if (m_bodyMerger != NULL) {
      m_selectedSet = m_bodyMerger->getFinalLabel(selected);
    } else {
      m_selectedSet = selected;
    }
    break;
  }
}

void ZDvidLabelSlice::addSelection(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  m_selectedSet.insert(getMappedLabel(bodyId, labelType));
}

void ZDvidLabelSlice::xorSelection(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  bodyId = getMappedLabel(bodyId, labelType);

  if (m_selectedSet.count(bodyId) > 0) {
    m_selectedSet.erase(bodyId);
  } else {
    m_selectedSet.insert(bodyId);
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

/*
void ZDvidLabelSlice::updateLabel(const ZFlyEmBodyMerger &merger)
{
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.setLabel(merger.getFinalLabel(obj.getLabel()));
  }
}
*/

void ZDvidLabelSlice::updateLabelColor()
{
  /*
  if (m_bodyMerger != NULL) {
    updateLabel(*m_bodyMerger);
  }
  */
  assignColorMap();
}

void ZDvidLabelSlice::setBodyMerger(ZFlyEmBodyMerger *bodyMerger)
{
  m_bodyMerger = bodyMerger;
}

void ZDvidLabelSlice::setMaxSize(int maxWidth, int maxHeight)
{
  if (m_maxWidth != maxWidth || m_maxHeight != maxHeight) {
    m_maxWidth = maxWidth;
    m_maxHeight = maxHeight;
    m_currentViewParam.resize(m_maxWidth, m_maxHeight);
    m_objArray.clear();
    update();
  }
}

uint64_t ZDvidLabelSlice::getMappedLabel(const ZObject3dScan &obj) const
{
  return getMappedLabel(obj.getLabel(), NeuTube::BODY_LABEL_ORIGINAL);
}

uint64_t ZDvidLabelSlice::getMappedLabel(
    uint64_t label, NeuTube::EBodyLabelType labelType) const
{
  if (labelType == NeuTube::BODY_LABEL_ORIGINAL) {
    if (m_bodyMerger != NULL) {
      return m_bodyMerger->getFinalLabel(label);
    }
  }

  return label;
}

