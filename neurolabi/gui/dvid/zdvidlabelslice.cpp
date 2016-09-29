#include "zdvidlabelslice.h"

#include <QColor>
#include <QRect>
#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

#include "tz_math.h"
#include "zarray.h"
#include "dvid/zdvidreader.h"
#include "zobject3dfactory.h"
#include "flyem/zflyembodymerger.h"
#include "zimage.h"
#include "zpainter.h"
#include "neutubeconfig.h"
#include "zpixmap.h"

ZDvidLabelSlice::ZDvidLabelSlice()
{
  init(512, 512);
}

ZDvidLabelSlice::ZDvidLabelSlice(int maxWidth, int maxHeight)
{
  init(maxWidth, maxHeight);
}

ZDvidLabelSlice::~ZDvidLabelSlice()
{
  delete m_paintBuffer;
  delete m_labelArray;
  delete m_mappedLabelArray;
}

void ZDvidLabelSlice::init(int maxWidth, int maxHeight  , NeuTube::EAxis sliceAxis)
{
  setTarget(ZStackObject::TARGET_DYNAMIC_OBJECT_CANVAS);
  m_type = GetType();
  m_objColorSheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
  m_hitLabel = 0;
  m_bodyMerger = NULL;
  setZOrder(0);

  m_maxWidth = maxWidth;
  m_maxHeight = maxHeight;

  m_paintBuffer = NULL;
//  m_paintBuffer = new ZImage(m_maxWidth, m_maxHeight, QImage::Format_ARGB32);
  m_labelArray = NULL;
  m_mappedLabelArray = NULL;

  m_selectionFrozen = false;
  m_isFullView = false;
  m_sliceAxis = sliceAxis;
//  m_zoom = 0;

//  m_objCache.setMaxCost();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidLabelSlice)

class ZDvidLabelSlicePaintTask {
public:
  ZDvidLabelSlicePaintTask(ZDvidLabelSlice *labelSlice)
  {
    m_labelSlice = labelSlice;
  }

  void setLabelSlice(ZDvidLabelSlice *slice) {
    m_labelSlice = slice;
  }

  void addObject(ZObject3dScan *obj) {
    m_objArray.append(obj);
  }

  static void ExecuteTask(ZDvidLabelSlicePaintTask &task) {
    for (QList<ZObject3dScan*>::iterator iter = task.m_objArray.begin();
         iter != task.m_objArray.end(); ++iter) {
      ZObject3dScan *obj = *iter;

      if (task.m_labelSlice->getSelectedOriginal().count(obj->getLabel()) > 0) {
        obj->setSelected(true);
      } else {
        obj->setSelected(false);
      }

      //      obj.display(painter, slice, option);

      if (!obj->isSelected()) {
        task.m_labelSlice->getPaintBuffer()->setData(*(obj));
      } else {
        task.m_labelSlice->getPaintBuffer()->setData(
              *(obj), QColor(255, 255, 255, 164));
      }
    }
  }

private:
  ZDvidLabelSlice *m_labelSlice;
  QList<ZObject3dScan*> m_objArray;
};

#define ZDVIDLABELSLICE_MT 1

void ZDvidLabelSlice::setSliceAxis(NeuTube::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
}

void ZDvidLabelSlice::display(
    ZPainter &painter, int /*slice*/, EDisplayStyle /*option*/,
    NeuTube::EAxis sliceAxis) const
{
  if (m_sliceAxis != sliceAxis) {
    return;
  }

#ifdef _DEBUG_
  QElapsedTimer timer;
  timer.start();
#endif

  if (isVisible()) {
    if (m_paintBuffer != NULL) {
      if (m_paintBuffer->isVisible()) {
        ZPixmap pixmap;
        pixmap.convertFromImage(*m_paintBuffer, Qt::ColorOnly);
        pixmap.setTransform(m_paintBuffer->getTransform());

        pixmap.matchProj();
        painter.drawPixmap(pixmap);
        painter.setPainted(true);
      }
    }
#if 0
    if (m_currentViewParam.getViewPort().width() > m_paintBuffer->width() ||
        m_currentViewParam.getViewPort().height() > m_paintBuffer->height()) {
      for (ZObject3dScanArray::const_iterator iter = m_objArray.begin();
           iter != m_objArray.end(); ++iter) {
        ZObject3dScan &obj = const_cast<ZObject3dScan&>(*iter);

        if (m_selectedOriginal.count(obj.getLabel()) > 0) {
          obj.setSelected(true);
        } else {
          obj.setSelected(false);
        }

        obj.display(painter, slice, option, sliceAxis);
      }
    } else {
      m_paintBuffer->clear();
      m_paintBuffer->setOffset(-m_currentViewParam.getViewPort().x(),
                               -m_currentViewParam.getViewPort().y());

#if defined(ZDVIDLABELSLICE_MT)
      QList<ZDvidLabelSlicePaintTask> taskList;
      const int taskCount = 4;
      for (int i = 0; i < taskCount; ++i) {
        taskList.append(ZDvidLabelSlicePaintTask(
                          const_cast<ZDvidLabelSlice*>(this)));
      }
#endif

      int count = 0;
      for (ZObject3dScanArray::const_iterator iter = m_objArray.begin();
           iter != m_objArray.end(); ++iter, ++count) {
        ZObject3dScan &obj = const_cast<ZObject3dScan&>(*iter);
#if defined(ZDVIDLABELSLICE_MT)
        taskList[count % taskCount].addObject(&obj);
#else
        //if (m_selectedSet.count(obj.getLabel()) > 0) {
        if (m_selectedOriginal.count(obj.getLabel()) > 0) {
          obj.setSelected(true);
        } else {
          obj.setSelected(false);
        }

        //      obj.display(painter, slice, option);

        if (!obj.isSelected()) {
          m_paintBuffer->setData(obj);
        } else {
          m_paintBuffer->setData(obj, QColor(255, 255, 255, 164));
        }
#endif
      }

#if defined(ZDVIDLABELSLICE_MT)
      QtConcurrent::blockingMap(taskList, &ZDvidLabelSlicePaintTask::ExecuteTask);
#endif

      //    painter.save();
      //    painter.setOpacity(0.5);

      painter.drawImage(m_currentViewParam.getViewPort().x(),
                        m_currentViewParam.getViewPort().y(),
                        *m_paintBuffer);

    }

    ZOUT(LTRACE(), 5) << "Body buffer painting time: " << timer.elapsed();
//    painter.restore();

#ifdef _DEBUG_2
//      m_paintBuffer->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
#endif
  }
}

void ZDvidLabelSlice::update()
{
  if (m_objArray.empty()) {
    forceUpdate(m_currentViewParam);
  }
}

void ZDvidLabelSlice::forceUpdate()
{
  forceUpdate(m_currentViewParam);
}

void ZDvidLabelSlice::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
#ifdef _DEBUG_2
  m_dvidTarget.set("emdata1.int.janelia.org", "e8c1", 8600);
  m_dvidTarget.setLabelBlockName("labels3");
#endif
  m_reader.open(m_dvidTarget);
}

int64_t ZDvidLabelSlice::getReadingTime() const
{
  return m_reader.getReadingTime();
}
/*
int ZDvidLabelSlice::getZoom() const
{
  return std::min(m_zoom, getDvidTarget().getMaxLabelZoom());
}
*/

int ZDvidLabelSlice::getZoomLevel(const ZStackViewParam &viewParam) const
{
  double zoomRatio = viewParam.getZoomRatio();
  if (zoomRatio == 0.0) {
    return 0;
  }

  int zoom = iround(std::log(1.0 / zoomRatio) / std::log(2.0) );

  if (zoom < 0) {
    zoom = 0;
  }

  int scale = pow(2, zoom);
  if (viewParam.getViewPort().width() * viewParam.getViewPort().height() /
      scale / scale > 512 * 512) {
    zoom += 1;
  }

  if (zoom > getDvidTarget().getMaxLabelZoom()) {
    zoom = getDvidTarget().getMaxLabelZoom();
  }

  return zoom;
}

void ZDvidLabelSlice::updateRgbTable()
{
  const QVector<QColor>& colorTable = getColorScheme().getColorTable();
  m_rgbTable.resize(colorTable.size());
  for (int i = 0; i < colorTable.size(); ++i) {
    const QColor &color = colorTable[i];
    m_rgbTable[i] = (64 << 24) + (color.red() << 16) + (color.green() << 8) +
        (color.blue());
  }
}

void ZDvidLabelSlice::paintBufferUnsync()
{
  if (m_labelArray != NULL && m_paintBuffer != NULL) {
    if ((int) m_labelArray->getElementNumber() ==
        m_paintBuffer->width() * m_paintBuffer->height()) {
      updateRgbTable();
      remapId();

      uint64_t *labelArray = NULL;

      if (m_paintBuffer->isVisible()) {
        if (m_selectedOriginal.empty() && getLabelMap().empty()) {
          labelArray = m_labelArray->getDataPointer<uint64_t>();
        } else {
          labelArray = m_mappedLabelArray->getDataPointer<uint64_t>();
        }
      }

      if (labelArray != NULL) {
        if (getSliceAxis() == NeuTube::X_AXIS) {
          m_paintBuffer->drawLabelFieldTranspose(
                labelArray, m_rgbTable, 0, 0xA4FFFFFF);
        } else {
          m_paintBuffer->drawLabelField(labelArray, m_rgbTable, 0, 0xA4FFFFFF);
        }
      }
    }
  }
}

void ZDvidLabelSlice::paintBuffer()
{
  QMutexLocker locker(&m_updateMutex);
  paintBufferUnsync();
}

void ZDvidLabelSlice::forceUpdate(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  QMutexLocker locker(&m_updateMutex);

  m_objArray.clear();
  if (isVisible()) {
    int zoom = getZoomLevel(viewParam);
    int zoomRatio = pow(2, zoom);



//    int yStep = 1;

    //    ZDvidReader reader;
    //    if (reader.open(getDvidTarget())) {
    QRect viewPort = viewParam.getViewPort();

    if (NeutubeConfig::GetVerboseLevel() >= 1) {
      std::cout << "Deleting label array:" << m_labelArray << std::endl;
    }

    ZIntCuboid box;
    box.setFirstCorner(viewPort.left(), viewPort.top(), viewParam.getZ());
    box.setSize(viewPort.width(), viewPort.height(), 1);

    int width = box.getWidth() / zoomRatio;
    int height = box.getHeight() / zoomRatio;
    int depth = box.getDepth();
    int x0 = box.getFirstCorner().getX() / zoomRatio;
    int y0 = box.getFirstCorner().getY() / zoomRatio;
    int z0 = box.getFirstCorner().getZ();

    ZGeometry::shiftSliceAxisInverse(x0, y0, z0, getSliceAxis());
    ZGeometry::shiftSliceAxisInverse(width, height, depth, getSliceAxis());


    box.shiftSliceAxisInverse(m_sliceAxis);

    delete m_labelArray;
    m_labelArray = NULL;
    delete m_mappedLabelArray;
    m_mappedLabelArray = NULL;
    /*
      m_labelArray = m_reader.readLabels64(
            getDvidTarget().getLabelBlockName(),
            viewPort.left(), viewPort.top(), viewParam.getZ(),
            viewPort.width(), viewPort.height(), 1);
            */
    QString cacheKey = (box.getFirstCorner().toString() + " " +
        box.getLastCorner().toString()).c_str();

    if (m_objCache.contains(cacheKey)) {
      m_labelArray = m_objCache.take(cacheKey);
    } else {
      if (box.getDepth() == 1) {
#if defined(_ENABLE_LOWTIS_)
        m_labelArray = m_reader.readLabels64Lowtis(
              box.getFirstCorner().getX(), box.getFirstCorner().getY(),
              box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(),
              zoom);
#else
        m_labelArray = m_reader.readLabels64(
              box.getFirstCorner().getX(), box.getFirstCorner().getY(),
              box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(), 1,
              zoom);
#endif
      } else {
        m_labelArray = m_reader.readLabels64Raw(
              x0, y0, z0, width, height, depth, zoom);
//        m_labelArray = m_reader.readLabels64(box, zoom);
      }
    }

    if (m_labelArray != NULL) {

      ZGeometry::shiftSliceAxis(width, height, depth, getSliceAxis());
      ZGeometry::shiftSliceAxis(x0, y0, z0, getSliceAxis());

      delete m_paintBuffer;
      m_paintBuffer = new ZImage(width, height, QImage::Format_ARGB32);
      paintBufferUnsync();
      ZStTransform transform;
      transform.setScale(1.0 / zoomRatio, 1.0 / zoomRatio);
      transform.setOffset(-x0, -y0);;
//      transform.setOffset(
//            -(double) box.getFirstCorner().getX() / zoomRatio,
//            -(double) box.getFirstCorner().getY() / zoomRatio);
      m_paintBuffer->setTransform(transform);

//      ZObject3dFactory::MakeObject3dScanArray(
//            *m_labelArray, yStep, &m_objArray, true);
      /*
      updateRgbTable();
      remapId(m_labelArray);

      delete m_paintBuffer;
      m_paintBuffer = new ZImage(
            m_labelArray->dim(0), m_labelArray->dim(1),
            QImage::Format_ARGB32);
      m_paintBuffer->drawLabelField(m_labelArray->getDataPointer<uint64_t>(),
                                    m_rgbTable, 0, 0x40FFFFFF);
      ZStTransform transform;
      int zoomRatio = pow(2, zoom);
      transform.setScale(1.0 / zoomRatio, 1.0 / zoomRatio);
      transform.setOffset(
            -(double) box.getFirstCorner().getX() / zoomRatio,
            -(double) box.getFirstCorner().getY() / zoomRatio);
      m_paintBuffer->setTransform(transform);
      */

#if 0
      ZObject3dFactory::MakeObject3dScanArray(
            *m_labelArray, m_sliceAxis, true, &m_objArray);

      if (zoom > 0) {
        int intv = pow(2, zoom) - 1;
        m_objArray.upsample(intv, intv, intv);
      }
      m_objArray.translate(box.getFirstCorner().getX(),
                           box.getFirstCorner().getY(),
                           box.getFirstCorner().getZ());
      //caching
#if 0
      if (!m_objCache.contains(cacheKey)) {
        if (box.getWidth() * box.getHeight() >= 10000) {
          m_objCache.insert(cacheKey, m_labelArray);
          m_labelArray = NULL;
        }
      }
#endif
      /*
        if (m_bodyMerger != NULL) {
          updateLabel(*m_bodyMerger);
        }
        */
      assignColorMap();
#endif

      //        delete labelArray;
    }
  }
  //  }
}

void ZDvidLabelSlice::update(int z)
{
  ZStackViewParam viewParam = m_currentViewParam;
  viewParam.setZ(z);

  update(viewParam);

  m_isFullView = false;
}

void ZDvidLabelSlice::updateFullView(const ZStackViewParam &viewParam)
{
  forceUpdate(viewParam);
  m_isFullView = true;
  m_currentViewParam = viewParam;
}

bool ZDvidLabelSlice::update(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return false;
  }

  if (viewParam.getViewPort().isEmpty()) {
    return false;
  }

  bool updated = false;
  if (!m_isFullView || (viewParam.getZ() != m_currentViewParam.getZ())) {
    ZStackViewParam newViewParam = viewParam;

    if (getDvidTarget().getMaxLabelZoom() < 3) {
      int width = viewParam.getViewPort().width();
      int height = viewParam.getViewPort().height();
      int area = width * height;
      //  const int maxWidth = 512;
      //  const int maxHeight = 512;
      if (area > m_maxWidth * m_maxHeight) {
        if (width > m_maxWidth) {
          width = m_maxWidth;
        }
        if (height > m_maxHeight) {
          height = m_maxHeight;
        }
        newViewParam.resize(width, height);
      }
    }


    if (!m_currentViewParam.contains(newViewParam) ||
        getZoomLevel(viewParam) != getZoomLevel(m_currentViewParam)) {
      forceUpdate(newViewParam);
      updated = true;

      m_currentViewParam = newViewParam;
    }
    m_isFullView = false;
  }

  return updated;
}

QColor ZDvidLabelSlice::getColor(
    uint64_t label, NeuTube::EBodyLabelType labelType) const
{
  QColor color;
  if (hasCustomColorMap()) {
    color = getCustomColor(label);
    if (color.alpha() != 0) {
      color.setAlpha(64);
    }
  } else {
    color = m_objColorSheme.getColor(
          getMappedLabel(label, labelType));
    color.setAlpha(64);
  }

  return color;
}

QColor ZDvidLabelSlice::getColor(
    int64_t label, NeuTube::EBodyLabelType labelType) const
{
  return getColor((uint64_t) label, labelType);
}

void ZDvidLabelSlice::setCustomColorMap(
    const ZSharedPointer<ZFlyEmBodyColorScheme> &colorMap)
{
  m_customColorScheme = colorMap;
  assignColorMap();
}

bool ZDvidLabelSlice::hasCustomColorMap() const
{
  return m_customColorScheme.get() != NULL;
}

void ZDvidLabelSlice::removeCustomColorMap()
{
  m_customColorScheme.reset();
  assignColorMap();
}

void ZDvidLabelSlice::assignColorMap()
{
  updateRgbTable();
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    //obj.setColor(getColor(obj.getLabel()));
    obj.setColor(getColor(obj.getLabel(), NeuTube::BODY_LABEL_ORIGINAL));
  }
}

void ZDvidLabelSlice::remapId()
{
//  QMutexLocker locker(&m_updateMutex);
  if (m_labelArray != NULL && m_mappedLabelArray == NULL) {
    m_mappedLabelArray = new ZArray(m_labelArray->valueType(),
                                    m_labelArray->ndims(),
                                    m_labelArray->dims());
  }
  remapId(m_mappedLabelArray);
}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v,
    std::set<uint64_t> &selected)
{
  if (hasVisualEffect(NeuTube::Display::LabelField::VE_HIGHLIGHT_SELECTED)) {
    for (size_t i = 0; i < v; ++i) {
      if (selected.count(originalArray[i]) > 0) {
        array[i] = originalArray[i];
      } else {
        array[i] = 0;
      }
    }
  } else {
    for (size_t i = 0; i < v; ++i) {
      if (selected.count(originalArray[i]) > 0) {
        array[i] = FlyEM::LABEL_ID_SELECTION;
      } else {
        array[i] = originalArray[i];
      }
    }
  }
}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v,
    const ZFlyEmBodyMerger::TLabelMap &bodyMap)
{
  if (hasVisualEffect(NeuTube::Display::LabelField::VE_HIGHLIGHT_SELECTED)) {
    m_paintBuffer->setVisible(false);
  } else {
    for (size_t i = 0; i < v; ++i) {
      if (bodyMap.count(originalArray[i]) > 0) {
        array[i] = bodyMap[originalArray[i]];
      } else {
        array[i] = originalArray[i];
      }
    }
  }
}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v,
    std::set<uint64_t> &selected, const ZFlyEmBodyMerger::TLabelMap &bodyMap)
{
  std::set<uint64_t> selectedSet = selected;
//  std::set<uint64_t> mappedSelected;
//  for (std::set<uint64_t>::const_iterator iter = selected.begin();
//       iter != selected.end(); ++iter) {
//    mappedSelected.insert(bodyMap[*iter]);
//  }
//  selectedSet.insert(mappedSelected.begin(), mappedSelected.end());

//  for (ZFlyEmBodyMerger::TLabelMap::const_iterator iter = bodyMap.begin();
//       iter != bodyMap.end(); ++iter) {
//    if (mappedSelected.count(iter.value()) > 0) {
//      selectedSet.insert(iter.key());
//    }
//  }


//  for (std::set<uint64_t>::const_iterator iter = m_selectedOriginal.begin();
//       iter != selected.end(); ++iter) {
//    if (bodyMap.count(*iter) > 0) {
//      selectedSet.insert(bodyMap[*iter]);
//    }
//  }

  if (hasVisualEffect(NeuTube::Display::LabelField::VE_HIGHLIGHT_SELECTED)) {
    for (size_t i = 0; i < v; ++i) {
      if (selectedSet.count(originalArray[i]) > 0) {
        if (bodyMap.count(originalArray[i]) > 0) {
          array[i] = bodyMap[originalArray[i]];
        } else {
          array[i] = originalArray[i];
        }
      } else {
        array[i] = 0;
      }
    }
  } else {
    for (size_t i = 0; i < v; ++i) {
      if (selectedSet.count(originalArray[i]) > 0) {
        array[i] = FlyEM::LABEL_ID_SELECTION;
      } else if (bodyMap.count(originalArray[i]) > 0) {
        array[i] = bodyMap[originalArray[i]];
      } else {
        array[i] = originalArray[i];
      }
    }
  }
}

void ZDvidLabelSlice::remapId(ZArray *label)
{
  if (m_paintBuffer != NULL) {
    m_paintBuffer->setVisible(true);
  }

  if (m_labelArray != NULL && label != NULL) {
    ZFlyEmBodyMerger::TLabelMap bodyMap = getLabelMap();
    if (!bodyMap.empty() || !m_selectedOriginal.empty()) {
      uint64_t *array = label->getDataPointer<uint64_t>();
      const uint64_t *originalArray = m_labelArray->getDataPointer<uint64_t>();
      size_t v = label->getElementNumber();
      if (bodyMap.empty()) {
        remapId(array, originalArray, v, m_selectedOriginal);
      } else if (m_selectedOriginal.empty()) {
        remapId(array, originalArray, v, bodyMap);
      } else {
        remapId(array, originalArray, v, m_selectedOriginal, bodyMap);
      }
    } else {
      if (hasVisualEffect(NeuTube::Display::LabelField::VE_HIGHLIGHT_SELECTED)) {
        if (m_paintBuffer != NULL) {
          m_paintBuffer->setVisible(true);
        }
      }
    }
  }
}

bool ZDvidLabelSlice::hit(double x, double y, double z)
{
  m_hitLabel = 0;

  if (!m_objArray.empty()) {
    for (ZObject3dScanArray::iterator iter = m_objArray.begin();
         iter != m_objArray.end(); ++iter) {
      ZObject3dScan &obj = *iter;
      if (obj.hit(x, y, z)) {
        //m_hitLabel = obj.getLabel();
        m_hitLabel = getMappedLabel(obj);
        return true;
      }
    }
  } else {
    int nx = iround(x);
    int ny = iround(y);
    int nz = iround(z);

    if (m_currentViewParam.contains(nx, ny, nz)) {
//      ZDvidReader reader;
      if (m_reader.isReady()) {
        m_hitLabel = getMappedLabel(m_reader.readBodyIdAt(nx, ny, nz),
                                    NeuTube::BODY_LABEL_ORIGINAL);
      }

      return m_hitLabel > 0;
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

    addSelection(m_hitLabel, NeuTube::BODY_LABEL_MAPPED);
//    addSelection(m_hitLabel);
//    m_selectedOriginal.insert(m_hitLabel);
  }
}

ZFlyEmBodyMerger::TLabelMap ZDvidLabelSlice::getLabelMap() const
{
  ZFlyEmBodyMerger::TLabelMap labelMap;
  if (m_bodyMerger != NULL) {
    labelMap = m_bodyMerger->getFinalMap();
  }

  return labelMap;
}

void ZDvidLabelSlice::setSelection(const std::set<uint64_t> &selected,
                                   NeuTube::EBodyLabelType labelType)
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    m_selectedOriginal = selected;
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(selected.begin(), selected.end());
      m_selectedOriginal.insert(
            selectedOriginal.begin(), selectedOriginal.end());
    } else {
      m_selectedOriginal = selected;
    }
    break;
  }
}

void ZDvidLabelSlice::addSelection(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    m_selectedOriginal.insert(bodyId);
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(bodyId);
      m_selectedOriginal.insert(
            selectedOriginal.begin(), selectedOriginal.end());
    } else {
      m_selectedOriginal.insert(bodyId);
    }
    break;
  }

//  m_selectedSet.insert(getMappedLabel(bodyId, labelType));
}

void ZDvidLabelSlice::xorSelection(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    if (m_selectedOriginal.count(bodyId) > 0) {
      m_selectedOriginal.erase(bodyId);
    } else {
      m_selectedOriginal.insert(bodyId);
    }
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(bodyId);
      xorSelectionGroup(selectedOriginal.begin(), selectedOriginal.end(),
                        NeuTube::BODY_LABEL_ORIGINAL);
    } else {
      xorSelection(bodyId, NeuTube::BODY_LABEL_ORIGINAL);
    }
  }
}

void ZDvidLabelSlice::deselectAll()
{
  m_selectedOriginal.clear();
}

bool ZDvidLabelSlice::isBodySelected(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType) const
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    return m_selectedOriginal.count(bodyId) > 0;
  case NeuTube::BODY_LABEL_MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(bodyId);
      for (QSet<uint64_t>::const_iterator iter = selectedOriginal.begin();
           iter != selectedOriginal.end(); ++iter) {
        if (m_selectedOriginal.count(*iter) > 0) {
          return true;
        }
      }
    } else {
      return m_selectedOriginal.count(bodyId) > 0;
    }
    break;
  }

  return false;
}

void ZDvidLabelSlice::toggleHitSelection(bool appending)
{
  bool hasSelected = isBodySelected(m_hitLabel, NeuTube::BODY_LABEL_MAPPED);
  xorSelection(m_hitLabel, NeuTube::BODY_LABEL_MAPPED);

  if (!appending) {
    clearSelection();

    if (!hasSelected) {
      addSelection(m_hitLabel, NeuTube::BODY_LABEL_MAPPED);
    }
  }

//  paintBuffer();
  //xorSelection(m_hitLabel, NeuTube::BODY_LABEL_MAPPED);

  /*
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
  */
}

void ZDvidLabelSlice::clearSelection()
{
  m_selectedOriginal.clear();
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
    delete m_paintBuffer;
    m_paintBuffer = new ZImage(m_maxWidth, m_maxHeight, QImage::Format_ARGB32);

    update();
  }
}

std::set<uint64_t> ZDvidLabelSlice::getOriginalLabelSet(
    uint64_t mappedLabel) const
{
  std::set<uint64_t> labelSet;
  if (m_bodyMerger == NULL) {
    labelSet.insert(mappedLabel);
  } else {
    const QSet<uint64_t> &sourceSet = m_bodyMerger->getOriginalLabelSet(mappedLabel);
    labelSet.insert(sourceSet.begin(), sourceSet.end());
  }

  return labelSet;
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

const ZStackViewParam& ZDvidLabelSlice::getViewParam() const
{
  return m_currentViewParam;
}

std::set<uint64_t> ZDvidLabelSlice::getSelected(
    NeuTube::EBodyLabelType labelType) const
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    return getSelectedOriginal();
  case NeuTube::BODY_LABEL_MAPPED:
    if (m_bodyMerger != NULL) {
      return m_bodyMerger->getFinalLabel(getSelectedOriginal());
    } else {
      return getSelectedOriginal();
    }
    break;
  }

  return std::set<uint64_t>();
}

void ZDvidLabelSlice::mapSelection()
{
  m_selectedOriginal = getSelected(NeuTube::BODY_LABEL_MAPPED);
}

void ZDvidLabelSlice::recordSelection()
{
  m_prevSelectedOriginal = m_selectedOriginal;
}

void ZDvidLabelSlice::processSelection()
{
  m_selector.reset(m_selectedOriginal, m_prevSelectedOriginal);

#if 0
  for (std::set<uint64_t>::const_iterator iter = m_selectedOriginal.begin();
       iter != m_selectedOriginal.end(); ++iter) {
    if (m_prevSelectedOriginal.count(*iter) == 0) {
      m_selector.selectObject(*iter);
    }
  }

  for (std::set<uint64_t>::const_iterator iter = m_prevSelectedOriginal.begin();
       iter != m_prevSelectedOriginal.end(); ++iter) {
    if (m_selectedOriginal.count(*iter) == 0) {
      m_selector.deselectObject(*iter);
    }
  }
#endif
}

QColor ZDvidLabelSlice::getCustomColor(uint64_t label) const
{
  QColor color(0, 0, 0, 0);

  if (m_customColorScheme.get() != NULL) {
    color = m_customColorScheme->getBodyColor(label);
  }

  return color;
}

void ZDvidLabelSlice::clearCache()
{
  m_objCache.clear();
}

bool ZDvidLabelSlice::refreshReaderBuffer()
{
  return m_reader.refreshLabelBuffer();
}
