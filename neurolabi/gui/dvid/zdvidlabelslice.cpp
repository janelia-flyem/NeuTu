#include "zdvidlabelslice.h"

#include <QColor>
#include <QRect>
#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

#include "common/math.h"
#include "zutils.h"
#include "zarray.h"
#include "dvid/zdvidreader.h"
#include "zobject3dfactory.h"
#include "flyem/zflyembodymerger.h"
#include "flyem/zflyemrandombodycolorscheme.h"
#include "zimage.h"
#include "zpainter.h"
#include "neutubeconfig.h"
#include "zpixmap.h"
#include "zstring.h"
#include "geometry/zintcuboid.h"
#include "zarbsliceviewparam.h"
#include "zstackviewparam.h"
#include "zdviddataslicehelper.h"
#include "misc/miscutility.h"
#include "zdviddataslicetask.h"
#include "zdviddataslicetaskfactory.h"
#include "flyem/zflyemgeneralbodycolorscheme.h"
#include "flyem/zflyemcompositebodycolorscheme.h"
#include "flyem/zflyembodyidcolorscheme.h"

//#include "flyem/zdvidlabelslicehighrestask.h"

/* Implementation details:
 *
 * A DVID label slice is defined as a rectangle slice provided by a certain
 * DVID label data instance, which should contain uint64 data. The geometry of
 * the slice is managed by m_helper, a ZDvidDataSliceHelper object.
 *
 * Data update: forceUpdate() takes the viewing parameters specified in m_helper
 *   and then converts the parameters into a dvid reading call. The slice data
 *   is stored in m_labelArray, which might be used to generate a set of sparse
 *   objects stored in m_objArray.
 *
 * Display update:
 */

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

void ZDvidLabelSlice::init(int maxWidth, int maxHeight  , neutu::EAxis sliceAxis)
{
  setTarget(ZStackObject::ETarget::DYNAMIC_OBJECT_CANVAS);
  m_type = GetType();
  m_defaultColorSheme = std::shared_ptr<ZFlyEmRandomBodyColorScheme>(
        new ZFlyEmRandomBodyColorScheme);
//  m_defaultColorSheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
  m_hitLabel = 0;
  m_bodyMerger = NULL;
  setZOrder(0);

  m_helper = std::make_unique<ZDvidDataSliceHelper>(ZDvidData::ERole::SEGMENTATION);
  getHelper()->setMaxSize(maxWidth, maxHeight);
//  m_maxWidth = maxWidth;
//  m_maxHeight = maxHeight;

  m_paintBuffer = NULL;
//  m_paintBuffer = new ZImage(m_maxWidth, m_maxHeight, QImage::Format_ARGB32);
  m_labelArray = NULL;
  m_mappedLabelArray = NULL;

  m_selectionFrozen = false;
//  m_isFullView = false;
  m_sliceAxis = sliceAxis;
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidLabelSlice)


#define ZDVIDLABELSLICE_MT 1

void ZDvidLabelSlice::setSliceAxis(neutu::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
}

void ZDvidLabelSlice::updatePixmap(ZPixmap *pixmap) const
{
  pixmap->convertFromImage(*m_paintBuffer, Qt::ColorOnly);
  pixmap->setTransform(m_paintBuffer->getTransform());
  pixmap->matchProj();

#ifdef _DEBUG_2
  std::cout << "label slice pixmap offset: "
            << pixmap->getTransform().getTx() << " "
            << pixmap->getTransform().getTy() << std::endl;

  std::cout << "label slice pixmap scale: "
            << pixmap->getTransform().getSx() << " "
            << pixmap->getTransform().getSy() << std::endl;
#endif
}

void ZDvidLabelSlice::display(
    ZPainter &painter, int /*slice*/, EDisplayStyle /*option*/,
    neutu::EAxis sliceAxis) const
{
  if (m_sliceAxis != sliceAxis) {
    return;
  }

#ifdef _DEBUG_2
  QElapsedTimer timer;
  timer.start();
#endif

  if (isVisible()) {
    if (hasValidPaintBuffer()) {
      if (m_paintBuffer->isVisible()) {
        ZPixmap pixmap;
        updatePixmap(&pixmap);

#ifdef _DEBUG_2
        pixmap.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
//        painter.save();
//        painter.setOpacity(getColor().alphaF());
        painter.drawPixmap(pixmap);
        painter.setPainted(true);
//        painter.restore();
      }
    }
  }
}

void ZDvidLabelSlice::setCenterCut(int width, int height)
{
  getHelper()->setCenterCut(width, height);
}

#if 0
void ZDvidLabelSlice::update()
{
  if (m_objArray.empty()) {
    update(true);
//    update(m_currentViewParam, true);
  }
}
#endif

void ZDvidLabelSlice::setUpdatePolicy(neutu::EDataSliceUpdatePolicy policy)
{
  getHelper()->setUpdatePolicy(policy);
}

bool ZDvidLabelSlice::containedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool usingCenterCut) const
{
  return getHelper()->actualContainedIn(
        viewParam, zoom, centerCutX, centerCutY, usingCenterCut);
}

ZTask* ZDvidLabelSlice::makeFutureTask(ZStackDoc *doc)
{
  ZDvidDataSliceTask *task = nullptr;
//  ZDvidLabelSliceHighresTask *task = NULL;
  if (m_taskFactory) {
    const int maxSize = 1024*1024;
    if (getHelper()->needHighResUpdate()
        && getHelper()->getViewDataSize() < maxSize) {
      //    task = new ZDvidLabelSliceHighresTask;
      task = m_taskFactory->makeTask();
      if (task) {
        ZStackViewParam viewParam = getHelper()->getViewParam();
        viewParam.openViewPort();
        task->setViewParam(viewParam);
        task->setZoom(getHelper()->getZoom());
        task->setCenterCut(
              getHelper()->getCenterCutWidth(), getHelper()->getCenterCutHeight());
        task->setDelay(50);
        task->setDoc(doc);
        task->setSupervoxel(getDvidTarget().isSupervoxelView());
        task->setName(this->getSource().c_str());
      }
    }
  }

  return task;
}

#if 0
void ZDvidLabelSlice::forceUpdate(bool ignoringHidden)
{
  forceUpdate(m_currentDataRect, m_currentZ, ignoringHidden);
}
#endif

void ZDvidLabelSlice::setDvidTarget(const ZDvidTarget &target)
{
//  m_dvidTarget = target;
#ifdef _DEBUG_2
  m_dvidTarget.set("emdata1.int.janelia.org", "e8c1", 8600);
  m_dvidTarget.setLabelBlockName("labels3");
#endif
//  m_reader.open(target);
  getHelper()->setDvidTarget(target);
  getHelper()->setMaxZoom(target.getMaxLabelZoom());
  getHelper()->inferUpdatePolicy(getSliceAxis());
}

bool ZDvidLabelSlice::isSupervoxel() const
{
  return getDvidTarget().isSupervoxelView();
}

int64_t ZDvidLabelSlice::getReadingTime() const
{
  return getHelper()->getDvidReader().getReadingTime();
}

void ZDvidLabelSlice::allowBlinking(bool on)
{
  if (on) {
    setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy::HIDDEN);
  } else {
    setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy::LOWRES);
  }
}

void ZDvidLabelSlice::setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy policy)
{
  getHelper()->setPreferredUpdatePolicy(policy);
  getHelper()->inferUpdatePolicy(getSliceAxis());
}

void ZDvidLabelSlice::setTaskFactory(
    std::unique_ptr<ZDvidDataSliceTaskFactory> &&factory)
{
  m_taskFactory = std::move(factory);
}

/*
int ZDvidLabelSlice::getZoom() const
{
  return std::min(m_zoom, getDvidTarget().getMaxLabelZoom());
}
*/

#if 0
int ZDvidLabelSlice::getZoomLevel(const ZStackViewParam &viewParam) const
{
  double zoomRatio = viewParam.getZoomRatio();
  if (zoomRatio == 0.0) {
    return 0;
  }

  int zoom = 0;

  if (getDvidTarget().usingMulitresBodylabel()) {
    zoom = iround(std::log(1.0 / zoomRatio) / std::log(2.0) );

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
  }

  return zoom;
}
#endif

void ZDvidLabelSlice::updateRgbTable()
{
  if (m_rgbTable.isEmpty()) {
    ZFlyEmBodyColorScheme *colorScheme = getColorScheme().get();
    if (m_customColorScheme) {
      colorScheme = m_customColorScheme.get();
    }

    //  const QVector<QColor>& colorTable = colorScheme->getColorTable();
    //  m_rgbTable.resize(colorTable.size());
    m_rgbTable.resize(colorScheme->getColorNumber() - 1);
    for (int i = 0; i < m_rgbTable.size(); ++i) {
      //    const QColor &color = colorTable[i];
      QColor color = colorScheme->getBodyColorFromIndex(i + 1);
      m_rgbTable[i] = (164 << 24) + (color.red() << 16) + (color.green() << 8) +
          (color.blue());
    }
  }
}

void ZDvidLabelSlice::paintBufferUnsync()
{
  if (m_labelArray != NULL && m_paintBuffer != NULL) {
    if ((int) m_labelArray->getElementNumber() ==
        m_paintBuffer->width() * m_paintBuffer->height()) {
      updateColorField();
      if (getSliceAxis() == neutu::EAxis::X) {
        m_paintBuffer->drawColorFieldTranspose(m_colorField.data());
      } else {
        m_paintBuffer->drawColorField(m_colorField.data());
      }

#if 0
      updateRgbTable();

      // TODO: Consider a way to use m_customColorScheme without remapId(), because remapId()
      // takes around 3 ms on a Macbook Pro.  That may not sound like much but it is about
      // 20% of the budget per frame for 60 frames/sec.

      remapId();

      uint64_t *labelArray = NULL;

      if (m_paintBuffer->isVisible()) {
        if (m_selectedOriginal.empty() && getLabelMap().empty() &&
            !m_customColorScheme) {
          labelArray = m_labelArray->getDataPointer<uint64_t>();
        } else {
          labelArray = m_mappedLabelArray->getDataPointer<uint64_t>();
        }
      }

      if (labelArray != NULL) {
        if (getSliceAxis() == neutu::EAxis::X) {
          m_paintBuffer->drawLabelFieldTranspose(
                labelArray, m_rgbTable, 0, 0xFFFFFFFF);
        } else {
          m_paintBuffer->drawLabelField(labelArray, m_rgbTable, 0, 0xFFFFFFFF);
        }
      }
#endif
    }
  }
}

void ZDvidLabelSlice::paintBuffer()
{
  QMutexLocker locker(&m_updateMutex);
  paintBufferUnsync();
}

void ZDvidLabelSlice::clearLabelData()
{
  delete m_labelArray;
  m_labelArray = NULL;
  delete m_mappedLabelArray;
  m_mappedLabelArray = NULL;
}

void ZDvidLabelSlice::forceUpdate(bool ignoringHidden)
{
  forceUpdate(getHelper()->getViewParam(), ignoringHidden);
}

bool ZDvidLabelSlice::hasValidPaintBuffer() const
{
  bool valid = false;

  if (m_paintBuffer != NULL && m_labelArray != NULL) {
    int width = m_labelArray->getDim(0);
    int height = m_labelArray->getDim(1);
    int depth = m_labelArray->getDim(2);
    zgeom::shiftSliceAxis(width, height, depth, getSliceAxis());
    if (m_paintBuffer->width() == width &&
        m_paintBuffer->height() == height) {
      valid = true;
    }
  }

  return valid;
}

int ZDvidLabelSlice::getFirstZoom(const ZStackViewParam &viewParam) const
{
  int zoom = viewParam.getZoomLevel();

  if (zoom > getHelper()->getMaxZoom()) {
    zoom = getHelper()->getMaxZoom();
  }

  switch (getHelper()->getUpdatePolicy()) {
  case neutu::EDataSliceUpdatePolicy::LOWRES:
    if (zoom < getHelper()->getMaxZoom() &&
        ZDvidDataSliceHelper::GetViewDataSize(viewParam, zoom) > 256 * 256) {
      zoom += 1;
    }
    break;
  case neutu::EDataSliceUpdatePolicy::LOWEST_RES:
    zoom = getHelper()->getMaxZoom();
    break;
  default:
    break;
  }

  return zoom;
}

void ZDvidLabelSlice::forceUpdate(
    const ZStackViewParam &viewParam, bool ignoringHidden)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  if ((!ignoringHidden) || isVisible()) {
    getHelper()->setZoom(viewParam.getZoomLevel());

    if (getHelper()->getUpdatePolicy() == neutu::EDataSliceUpdatePolicy::HIDDEN) {
      clearLabelData();
    } else {
      int zoom = getFirstZoom(viewParam);
      if (m_sliceAxis == neutu::EAxis::ARB) {
        forceUpdate(viewParam.getSliceViewParam(), zoom);
      } else {
        QRect viewPort = viewParam.getViewPort();
        forceUpdate(viewPort, viewParam.getZ(), zoom);
      }
    }
  }

  getHelper()->setViewParam(viewParam);
  if (getHelper()->getUpdatePolicy() == neutu::EDataSliceUpdatePolicy::HIDDEN) {
    getHelper()->closeViewPort();
  }
  updatePaintBuffer();
}

void ZDvidLabelSlice::forceUpdate(const QRect &viewPort, int z, int zoom)
{
  if (getSliceAxis() != neutu::EAxis::ARB) {
    clearLabelData();
    if (!viewPort.isEmpty()) {
      ZIntCuboid box = ZDvidDataSliceHelper::GetBoundBox(viewPort, z);
      if (getSliceAxis() == neutu::EAxis::Z) {
        m_labelArray = getHelper()->getDvidReader().readLabels64Lowtis(
              box.getMinCorner().getX(), box.getMinCorner().getY(),
              box.getMinCorner().getZ(), box.getWidth(), box.getHeight(),
              zoom, getHelper()->getCenterCutWidth(),
              getHelper()->getCenterCutHeight(), getHelper()->usingCenterCut());
        getHelper()->setActualQuality(
              zoom, getHelper()->getCenterCutWidth(),
              getHelper()->getCenterCutHeight(), getHelper()->usingCenterCut());
      } else {
        int zoomRatio = pow(2, zoom);
        int width = box.getWidth() / zoomRatio;
        int height = box.getHeight() / zoomRatio;
        int depth = box.getDepth();
        int x0 = box.getMinCorner().getX() / zoomRatio;
        int y0 = box.getMinCorner().getY() / zoomRatio;
        int z0 = box.getMinCorner().getZ();

        zgeom::shiftSliceAxisInverse(x0, y0, z0, getSliceAxis());
        zgeom::shiftSliceAxisInverse(width, height, depth, getSliceAxis());

        m_labelArray = getHelper()->getDvidReader().readLabels64Raw(
              x0, y0, z0, width, height, depth, zoom);
        getHelper()->setActualQuality(zoom, 0, 0, false);
      }
    }
  }
}

void ZDvidLabelSlice::forceUpdate(
    const ZArbSliceViewParam &viewParam, int zoom)
{
  if (m_sliceAxis == neutu::EAxis::ARB) {
    clearLabelData();
    if (viewParam.isValid()) {
      m_labelArray = getHelper()->getDvidReader().readLabels64Lowtis(
            viewParam.getCenter(), viewParam.getPlaneV1(), viewParam.getPlaneV2(),
            viewParam.getWidth(), viewParam.getHeight(),
            zoom, getHelper()->getCenterCutWidth(),
            getHelper()->getCenterCutHeight(), getHelper()->usingCenterCut());
      getHelper()->setActualQuality(
            zoom, getHelper()->getCenterCutWidth(),
            getHelper()->getCenterCutHeight(), getHelper()->usingCenterCut());
    }
  }
}

bool ZDvidLabelSlice::isPaintBufferAllocNeeded(int width, int height) const
{
  if (m_paintBuffer == NULL) {
    return true;
  }

  return m_paintBuffer->width() !=  width || m_paintBuffer->height() != height;
}

void ZDvidLabelSlice::updatePaintBuffer()
{
  if (m_labelArray != NULL) {
    int width = m_labelArray->getDim(0);
    int height = m_labelArray->getDim(1);
    if (getSliceAxis() == neutu::EAxis::X || getSliceAxis() == neutu::EAxis::Y) {
      int depth = m_labelArray->getDim(2);
      zgeom::shiftSliceAxisInverse(width, height, depth, getSliceAxis());
    }

#ifdef _DEBUG_2
      std::cout << "Max label: " << m_labelArray->getMax<uint64_t>() << std::endl;
#endif

    if (isPaintBufferAllocNeeded(width, height)) {
      delete m_paintBuffer;
      m_paintBuffer = new ZImage(width, height, QImage::Format_ARGB32);
    }

    paintBufferUnsync();
    setTransform(m_paintBuffer);
  }
}

void ZDvidLabelSlice::setTransform(ZImage *image) const
{
  ZStTransform transform;
  double scale = 1.0 / getHelper()->getActualScale();
  transform.setScale(scale, scale);
  transform.setOffset(-getHelper()->getX() * scale, -getHelper()->getY() * scale);
  image->setTransform(transform);
}

#if 0
void ZDvidLabelSlice::update(int z)
{
//  ZStackViewParam viewParam = m_currentViewParam;
//  viewParam.setZ(z);

  if (getHelper()->getZ() != z) {
    ZStackViewParam viewParam = getHelper()->getViewParam();
    viewParam.moveSlice(z - viewParam.getZ());
    forceUpdate(viewParam, true);
  }
//  update(getHelper()->getViewPort(), getHelper()->getZoom(), z);

//  m_isFullView = false;
}
#endif

const ZDvidTarget& ZDvidLabelSlice::getDvidTarget() const
{
  return getHelper()->getDvidReader().getDvidTarget();
}

void ZDvidLabelSlice::updateFullView(const ZStackViewParam &viewParam)
{
//  m_isFullView = true;
  getHelper()->setUnlimitedSize();
  update(viewParam);
}

/*void ZDvidLabelSlice::disableFullView()
{
  m_isFullView = false;
}
*/

QRect ZDvidLabelSlice::getDataRect(const ZStackViewParam &viewParam) const
{
  QRect viewPort = viewParam.getViewPort();

  if (!getDvidTarget().usingMulitresBodylabel() && getHelper()->hasMaxSize()) {
    int width = viewPort.width();
    int height = viewPort.height();
    int area = width * height;
    if (area > getHelper()->getMaxArea()) {
      if (width > getHelper()->getMaxWidth()) {
        width = getHelper()->getMaxWidth();
      }
      if (height > getHelper()->getMaxWidth()) {
        height = getHelper()->getMaxWidth();
      }
      QPoint oldCenter = viewPort.center();
      viewPort.setSize(QSize(width, height));
      viewPort.moveCenter(oldCenter);
    }
  }

  return viewPort;
}

int ZDvidLabelSlice::getCurrentZ() const
{
  return getHelper()->getZ();
}

bool ZDvidLabelSlice::consume(
    ZArray *array, const ZStackViewParam &viewParam, int zoom,
    int centerCutX, int centerCutY, bool usingCenterCut)
{
  bool succ = false;
  if (array != NULL) {
    if (containedIn(viewParam, zoom, centerCutX, centerCutY, usingCenterCut)) {
//      getHelper()->setZoom(zoom);
      getHelper()->setViewParam(viewParam);
      getHelper()->setActualQuality(zoom, centerCutX, centerCutY, usingCenterCut);
//      getHelper()->setCenterCut(centerCutX, centerCutY);
      clearLabelData();
      m_labelArray = array;
      updatePaintBuffer();
      succ = true;
    } else {
      delete array;
    }
  }
  return succ;
}

bool ZDvidLabelSlice::update(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return false;
  }

  bool updated = false;

  if (viewParam.getViewPort().isEmpty()) {
    if (m_labelArray != NULL) {
      clearLabelData();
      updated = true;
    }
    getHelper()->setViewParam(viewParam);
  } else {
    ZStackViewParam newViewParam = getHelper()->getValidViewParam(viewParam);
    if (getHelper()->hasNewView(newViewParam)) {
      forceUpdate(newViewParam, true);
      updated = true;
    }
  }

  return updated;

  /*
  QRect dataRect = getDataRect(viewParam);

  return update(dataRect, getZoomLevel(viewParam), viewParam.getZ());
  */
}

std::shared_ptr<ZFlyEmBodyColorScheme>
ZDvidLabelSlice::getBaseColorScheme() const
{
  std::shared_ptr<ZFlyEmBodyColorScheme> baseScheme =
      m_customColorScheme ? m_customColorScheme : m_defaultColorSheme;

  if (baseScheme == m_defaultColorSheme) {
    if (m_individualColorScheme) {
      auto newBaseScheme = std::shared_ptr<ZFlyEmCompositeBodyColorScheme>(
            new ZFlyEmCompositeBodyColorScheme);
      newBaseScheme->appendScheme(
            std::dynamic_pointer_cast<ZFlyEmBodyColorScheme>(
              m_individualColorScheme));
      newBaseScheme->appendScheme(baseScheme);
      baseScheme =
          std::dynamic_pointer_cast<ZFlyEmBodyColorScheme>(newBaseScheme);
    }
  }

  return baseScheme;
}

QColor ZDvidLabelSlice::getLabelColor(
    uint64_t label, neutu::ELabelSource labelType) const
{
  return getBaseColorScheme()->getBodyColor(getMappedLabel(label, labelType));
  /*
  QColor color;
  if (hasCustomColorMap()) {
    color = getCustomColor(getMappedLabel(label, labelType));
    if (color.alpha() != 0) {
      color.setAlpha(164);
    }
  } else {
    color = m_defaultColorSheme->getBodyColor(
          getMappedLabel(label, labelType));
    color.setAlpha(164);
  }

  return color;
  */
}

QColor ZDvidLabelSlice::getLabelColor(
    int64_t label, neutu::ELabelSource labelType) const
{
  return getLabelColor((uint64_t) label, labelType);
}

bool ZDvidLabelSlice::setLabelColor(uint64_t label, const QColor &color)
{
  if (!m_individualColorScheme) {
    m_individualColorScheme = std::shared_ptr<ZFlyEmBodyIdColorScheme>(
          new ZFlyEmBodyIdColorScheme);
  }

  return m_individualColorScheme->setColor(label, color);
}

bool ZDvidLabelSlice::setLabelColor(uint64_t label, const QString &colorCode)
{
  if (colorCode.isEmpty()) {
    return removeLabelColor(label);
  } else {
    return setLabelColor(label, QColor(colorCode));
  }
}

bool ZDvidLabelSlice::removeLabelColor(uint64_t label)
{
  if (m_individualColorScheme &&
      m_individualColorScheme->hasOwnColor(label)) {
    return m_individualColorScheme->removeBody(label);
  }

  return false;
}

void ZDvidLabelSlice::setCustomColorMap(
    const ZSharedPointer<ZFlyEmBodyColorScheme> &colorMap)
{
  m_customColorScheme = colorMap;
#ifdef _DEBUG_
  if (m_customColorScheme->getColorNumber() < 65535) {
    std::cout << "debug here" << m_customColorScheme->getColorNumber() << std::endl;
  }
#endif
  m_rgbTable.clear();
  assignColorMap();
}

bool ZDvidLabelSlice::hasCustomColorMap() const
{
  return m_customColorScheme.get() != NULL;
}

void ZDvidLabelSlice::removeCustomColorMap()
{
  if (m_customColorScheme) {
    m_customColorScheme.reset();
    m_rgbTable.clear();
    assignColorMap();
  }
}

void ZDvidLabelSlice::assignColorMap()
{
  updateRgbTable();
  for (ZObject3dScanArray::iterator iter = m_objArray.begin();
       iter != m_objArray.end(); ++iter) {
    ZObject3dScan &obj = **iter;
    //obj.setColor(getColor(obj.getLabel()));
    obj.setColor(getLabelColor(obj.getLabel(), neutu::ELabelSource::ORIGINAL));
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

namespace {

void remap_id(uint64_t *dstArray, const uint64_t *srcArray,
    size_t nvoxel, std::function<void(uint64_t*, const uint64_t*)> m)
{
  if (nvoxel > 0) {
    m(dstArray, srcArray);
    for (size_t i = 1; i < nvoxel; ++i) {
      if (srcArray[i] == srcArray[i - 1]) {
        dstArray[i] = dstArray[i - 1];
      } else {
        m(dstArray + i, srcArray + i);
//        dstArray[i] = scheme->getBodyColorIndex(srcArray[i]);
      }
    }
  }
}

void remap_id(
    ZFlyEmBodyColorScheme *scheme, uint64_t *dstArray, const uint64_t *srcArray,
    size_t nvoxel)
{
  remap_id(dstArray, srcArray, nvoxel, [&](uint64_t *dst, const uint64_t *src) {
    *dst = scheme->getBodyColorIndex(*src);
  });
  /*
  if (nvoxel > 0) {
    dstArray[0] = scheme->getBodyColorIndex(srcArray[0]);
    for (size_t i = 1; i < nvoxel; ++i) {
      if (srcArray[i] == srcArray[i - 1]) {
        dstArray[i] = dstArray[i - 1];
      } else {
        dstArray[i] = scheme->getBodyColorIndex(srcArray[i]);
      }
    }
  }
  */
}

void remap_id(
    ZFlyEmBodyColorScheme *scheme, uint64_t *dstArray, const uint64_t *srcArray,
    size_t nvoxel, const std::set<uint64_t> &selectedSet)
{
  remap_id(dstArray, srcArray, nvoxel, [&](uint64_t *dst, const uint64_t *src) {
    if (selectedSet.count(*src) > 0) {
      *dst = neutu::LABEL_ID_SELECTION;
    } else {
      *dst = scheme->getBodyColorIndex(*src);
    }
  });

  /*
  if (nvoxel > 0) {
    dstArray[0] = scheme->getBodyColorIndex(srcArray[0]);
    for (size_t i = 1; i < nvoxel; ++i) {
      if (srcArray[i] == srcArray[i - 1]) {
        dstArray[i] = dstArray[i - 1];
      } else {
        if (selectedSet.count(srcArray[i]) > 0) {
          dstArray[i] = neutu::LABEL_ID_SELECTION;
        } else {
          dstArray[i] = scheme->getBodyColorIndex(srcArray[i]);
        }
      }
    }
  }
  */
}

void remap_id(
    ZFlyEmBodyColorScheme *scheme, uint64_t *dstArray, const uint64_t *srcArray,
    size_t nvoxel, const ZFlyEmBodyMerger::TLabelMap &bodyMap)
{
  auto voxel_map = [&](uint64_t *dst, const uint64_t *src) {
    if (bodyMap.contains(*src)) {
      *dst = scheme->getBodyColorIndex(bodyMap[*src]);
    } else {
      *dst = scheme->getBodyColorIndex(*src);
    }
  };
  remap_id(dstArray, srcArray, nvoxel, voxel_map);


  /*
  if (nvoxel > 0) {
    voxel_map(dstArray, srcArray);

    for (size_t i = 1; i < nvoxel; ++i) {
      if (srcArray[i] == srcArray[i - 1]) {
        dstArray[i] = dstArray[i - 1];
      } else {
        voxel_map(dstArray + i, srcArray + i);
      }
    }
  }
  */
}

void remap_id(
    ZFlyEmBodyColorScheme *scheme, uint64_t *dstArray, const uint64_t *srcArray,
    size_t nvoxel, const std::set<uint64_t> &selectedSet,
    const ZFlyEmBodyMerger::TLabelMap &bodyMap, bool highlighted)
{
  if (highlighted) {
    auto voxel_map = [&](uint64_t *dst, const uint64_t *src) {
      if (selectedSet.count(*src) > 0) {
        if (bodyMap.count(*src) > 0) {
          *dst = scheme->getBodyColorIndex(bodyMap[*src]);
        } else {
          *dst = scheme->getBodyColorIndex(*src);
        }
      } else {
        *dst = 0;
      }
    };
    remap_id(dstArray, srcArray, nvoxel, voxel_map);
  } else {
    auto voxel_map = [&](uint64_t *dst, const uint64_t *src) {
      if (selectedSet.count(*src) > 0) {
        if (bodyMap.count(*src) > 0) {
          *dst = scheme->getBodyColorIndex(bodyMap[*src]);
        } else {
          *dst = neutu::LABEL_ID_SELECTION;
        }
      } else { //not selected
        if (bodyMap.count(*src) > 0) {
          *dst = scheme->getBodyColorIndex(bodyMap[*src]);
        } else {
          *dst = scheme->getBodyColorIndex(*src);
        }
      }
    };
    remap_id(dstArray, srcArray, nvoxel, voxel_map);
  }
}

}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v)
{
  if (m_customColorScheme.get() != NULL) {

//    QHash<uint64_t, int> idMap;

//    idMap = m_customColorScheme->getColorIndexMap();

    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
      for (size_t i = 0; i < v; ++i) {
        array[i] = 0;
      }
    } else {
      remap_id(m_customColorScheme.get(), array, originalArray, v);
#if 0
      for (size_t i = 0; i < v; ++i) {
        array[i] = m_customColorScheme->getBodyColorIndex(originalArray[i]);
        /*
        array[i] = originalArray[i];
        if (idMap.contains(array[i])) {
          array[i] = idMap[array[i]];
        } else {
          array[i] = 0;
        }
        */
      }
#endif
    }
  }
}


void ZDvidLabelSlice::remapId(uint64_t *array, const uint64_t *originalArray, uint64_t v,
    std::set<uint64_t> &selected)
{
  if (m_customColorScheme.get() != NULL) {

//    QHash<uint64_t, int> idMap;

//    idMap = m_customColorScheme->getColorIndexMap();

    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
      for (size_t i = 0; i < v; ++i) {
        if (selected.count(originalArray[i]) > 0) {
          array[i] = m_customColorScheme->getBodyColorIndex(originalArray[i]);
          /*
          array[i] = originalArray[i];
          if (idMap.contains(array[i])) {
            array[i] = idMap[array[i]];
          } else {
            array[i] = 0;
          }
          */
        } else {
          array[i] = 0;
        }
      }
    } else {
      remap_id(m_customColorScheme.get(), array, originalArray, v, selected);
#if 0
      for (size_t i = 0; i < v; ++i) {
        if (selected.count(originalArray[i]) > 0) {
          array[i] = neutu::LABEL_ID_SELECTION;
        } else {
          array[i] = m_customColorScheme->getBodyColorIndex(originalArray[i]);
//          array[i] = originalArray[i];
          /*
          if (idMap.contains(array[i])) {
            array[i] = idMap[array[i]];
          } else {
            array[i] = 0;
          }
          */
        }
      }
#endif
    }
  } else {
    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
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
          array[i] = neutu::LABEL_ID_SELECTION;
        } else {
          array[i] = originalArray[i];
        }
      }
    }
  }
}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v,
    const ZFlyEmBodyMerger::TLabelMap &bodyMap)
{

  if (m_customColorScheme.get() != NULL) {
//    QHash<uint64_t, int> idMap;

//    idMap = m_customColorScheme->getColorIndexMap();

    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
      m_paintBuffer->setVisible(false);
    } else {
      remap_id(m_customColorScheme.get(), array, originalArray, v, bodyMap);
#if 0
      for (size_t i = 0; i < v; ++i) {
        if (bodyMap.contains(originalArray[i])) {
          array[i] = bodyMap[originalArray[i]];
        } else {
          array[i] = originalArray[i];
        }
        array[i] = m_customColorScheme->getBodyColorIndex(array[i]);
        /*
        if (idMap.contains(array[i])) {
          array[i] = idMap[array[i]];
        } else {
          array[i] = 0;
        }
        */
      }
#endif
    }
  } else {
    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
      m_paintBuffer->setVisible(false);
    } else {
      for (size_t i = 0; i < v; ++i) {
        if (bodyMap.contains(originalArray[i])) {
          array[i] = bodyMap[originalArray[i]];
        } else {
          array[i] = originalArray[i];
        }
      }
    }
  }
}

void ZDvidLabelSlice::remapId(
    uint64_t *array, const uint64_t *originalArray, uint64_t v,
    std::set<uint64_t> &selected,
    const ZFlyEmBodyMerger::TLabelMap &bodyMap)
{
  if (m_customColorScheme.get() != NULL) {
//    QHash<uint64_t, int> idMap;


//    idMap = m_customColorScheme->getColorIndexMap();

    auto selectedSet = selected;

    remap_id(
          m_customColorScheme.get(), array, originalArray, v,
          selectedSet,  bodyMap,
          hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED));
#if 0
    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
      for (size_t i = 0; i < v; ++i) {
        if (selectedSet.count(originalArray[i]) > 0) {
          if (bodyMap.count(originalArray[i]) > 0) {
            array[i] = bodyMap[originalArray[i]];
          } else {
            array[i] = originalArray[i];
          }
          array[i] = m_customColorScheme->getBodyColorIndex(array[i]);
          /*
          if (idMap.contains(array[i])) {
            array[i] = idMap[array[i]];
          } else {
            array[i] = 0;
          }
          */
        } else {
          array[i] = 0;
        }
      }
    } else {
      for (size_t i = 0; i < v; ++i) {
        if (selectedSet.count(originalArray[i]) > 0) {
          array[i] = neutu::LABEL_ID_SELECTION;
        } else if (bodyMap.count(originalArray[i]) > 0) {
          /*
          array[i] = bodyMap[originalArray[i]];
          if (idMap.contains(array[i])) {
            array[i] = idMap[array[i]];
          } else {
            array[i] = 0;
          }*/
          array[i] = m_customColorScheme->getBodyColorIndex(
                bodyMap[originalArray[i]]);
        } else {
          /*
          array[i] = originalArray[i];
          if (idMap.contains(array[i])) {
            array[i] = idMap[array[i]];
          } else {
            array[i] = 0;
          }
          */
          array[i] = m_customColorScheme->getBodyColorIndex(originalArray[i]);
        }
      }
    }
#endif
  } else {
    auto selectedSet = selected;

    if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
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
          array[i] = neutu::LABEL_ID_SELECTION;
        } else if (bodyMap.count(originalArray[i]) > 0) {
          array[i] = bodyMap[originalArray[i]];
        } else {
          array[i] = originalArray[i];
        }
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
    uint64_t *array = label->getDataPointer<uint64_t>();
    const uint64_t *originalArray = m_labelArray->getDataPointer<uint64_t>();
    size_t v = label->getElementNumber();
    if (!bodyMap.empty() || !m_selectedOriginal.empty()) {
      if (bodyMap.empty()) {
        remapId(array, originalArray, v, m_selectedOriginal);
      } else if (m_selectedOriginal.empty()) {
        remapId(array, originalArray, v, bodyMap);
      } else {
        remapId(array, originalArray, v, m_selectedOriginal, bodyMap);
      }
    } else {
      if (m_customColorScheme.get() != NULL) {
        remapId(array, originalArray, v);
      }
      if (hasVisualEffect(neutu::display::LabelField::VE_HIGHLIGHT_SELECTED)) {
        if (m_paintBuffer != NULL) {
          m_paintBuffer->setVisible(false);
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
      ZObject3dScan &obj = **iter;
      if (obj.hit(x, y, z)) {
        //m_hitLabel = obj.getLabel();
        m_hitLabel = getMappedLabel(obj);
        return true;
      }
    }
  } else {
    int nx = neutu::iround(x);
    int ny = neutu::iround(y);
    int nz = neutu::iround(z);

//    ZGeometry::shiftSliceAxisInverse(nx, ny, nz, m_sliceAxis);

//    if (getHelper()->getViewPort().contains(nx, ny) &&
//        nz == getHelper()->getZ()) {
//      ZDvidReader reader;
    bool withinRange = true;
    if (getSliceAxis() != neutu::EAxis::ARB) {
      zgeom::shiftSliceAxisInverse(nx, ny, nz, m_sliceAxis);
      if (!getHelper()->getViewPort().contains(nx, ny) ||
              nz != getHelper()->getZ()) {
        withinRange = false;
      }
      zgeom::shiftSliceAxis(nx, ny, nz, m_sliceAxis);
    }

    if (withinRange) {
      if (getHelper()->getDvidReader().isReady()) {
//        ZGeometry::shiftSliceAxis(nx, ny, nz, m_sliceAxis);
        m_hitLabel = getMappedLabel(
              getHelper()->getDvidReader().readBodyIdAt(nx, ny, nz),
              neutu::ELabelSource::ORIGINAL);
      }

      return m_hitLabel > 0;
    }
  }

  return false;
}

void ZDvidLabelSlice::updateColorField()
{
  if (m_labelArray == nullptr) {
    m_colorField.clear();
    return;
  }

  if (m_labelArray->isEmpty()) {
    m_colorField.clear();
    return;
  }

  if (m_colorField.size() != m_labelArray->getElementNumber()) {
    m_colorField.resize(m_labelArray->getElementNumber());
  }

  ZFlyEmGeneralBodyColorScheme scheme;

  std::shared_ptr<ZFlyEmBodyColorScheme> baseScheme = getBaseColorScheme();

  ZFlyEmBodyMerger::TLabelMap bodyMap = getLabelMap();
  bool highlight = hasVisualEffect(
        neutu::display::LabelField::VE_HIGHLIGHT_SELECTED);
  if (highlight) {
    if (m_selectedOriginal.empty()) {
      scheme._getBodyColorCode = [&](uint64_t /*bodyId*/) {
        return 0u;
      };
    } else if (bodyMap.isEmpty()) {
      scheme._hasExplicitColor = [&](uint64_t bodyId) {
        return m_selectedOriginal.count(bodyId) > 0;
      };

      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        return baseScheme->getBodyColorCode(bodyId);
      };
    } else {
      scheme._hasExplicitColor = [](uint64_t) {
        return true;
      };

      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        if (m_selectedOriginal.count(bodyId) > 0) {
          bodyId = bodyMap.value(bodyId, bodyId);
          return baseScheme->getBodyColorCode(bodyId);
        }

        return 0u;
      };
    }
  } else {
    scheme._hasExplicitColor = [](uint64_t) {
      return true;
    };

    if (bodyMap.isEmpty() && m_selectedOriginal.empty()) {
      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        return baseScheme->getBodyColorCode(bodyId);
      };
    } else if (bodyMap.isEmpty()) {
      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        if (m_selectedOriginal.count(bodyId) > 0) {
          return 0xFFFFFFFFu;
        }
        return baseScheme->getBodyColorCode(bodyId);
      };
    } else if (m_selectedOriginal.empty()) {
      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        bodyId = bodyMap.value(bodyId, bodyId);
        return baseScheme->getBodyColorCode(bodyId);
      };
    } else {
      scheme._getBodyColorCode = [&](uint64_t bodyId) {
        if (m_selectedOriginal.count(bodyId) > 0) {
          return 0xFFFFFFFFu;
        } else {
          bodyId = bodyMap.value(bodyId, bodyId);
        }
        return baseScheme->getBodyColorCode(bodyId);
      };
    }
  }
  scheme.mapColor(
        m_labelArray->getDataPointer<uint64_t>(),
        m_colorField.data(), m_colorField.size());
}

std::set<uint64_t> ZDvidLabelSlice::getHitLabelSet() const
{
  std::set<uint64_t> labelSet;
  if (m_hitLabel > 0) {
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(m_hitLabel);
      labelSet.insert(
            selectedOriginal.begin(), selectedOriginal.end());
    } else {
      labelSet.insert(m_hitLabel);
    }
  }

  return labelSet;
}

uint64_t ZDvidLabelSlice::getHitLabel() const
{
  return m_hitLabel;
}

void ZDvidLabelSlice::selectHit(bool appending)
{
  if (m_hitLabel > 0) {
    if (!appending) {
      clearSelection();
    }

    addSelection(m_hitLabel, neutu::ELabelSource::MAPPED);
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
                                   neutu::ELabelSource labelType)
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    m_selectedOriginal = selected;
    break;
  case neutu::ELabelSource::MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(selected.begin(), selected.end());
      m_selectedOriginal.clear();
      m_selectedOriginal.insert(
            selectedOriginal.begin(), selectedOriginal.end());
    } else {
      m_selectedOriginal = selected;
    }
    break;
  }
}

void ZDvidLabelSlice::addSelection(
    uint64_t bodyId, neutu::ELabelSource labelType)
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    m_selectedOriginal.insert(bodyId);
    break;
  case neutu::ELabelSource::MAPPED:
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

void ZDvidLabelSlice::removeSelection(
    uint64_t bodyId, neutu::ELabelSource labelType)
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    m_selectedOriginal.erase(bodyId);
    break;
  case neutu::ELabelSource::MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(bodyId);
      foreach (uint64_t bodyId, selectedOriginal) {
        m_selectedOriginal.erase(bodyId);
      }
    } else {
      m_selectedOriginal.erase(bodyId);
    }
    break;
  }
}

void ZDvidLabelSlice::xorSelection(
    uint64_t bodyId, neutu::ELabelSource labelType)
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    if (m_selectedOriginal.count(bodyId) > 0) {
      m_selectedOriginal.erase(bodyId);
    } else {
      m_selectedOriginal.insert(bodyId);
    }
    break;
  case neutu::ELabelSource::MAPPED:
    if (m_bodyMerger != NULL) {
      QSet<uint64_t> selectedOriginal =
          m_bodyMerger->getOriginalLabelSet(bodyId);
      xorSelectionGroup(selectedOriginal.begin(), selectedOriginal.end(),
                        neutu::ELabelSource::ORIGINAL);
    } else {
      xorSelection(bodyId, neutu::ELabelSource::ORIGINAL);
    }
  }
}

void ZDvidLabelSlice::deselectAll()
{
  m_selectedOriginal.clear();
}

bool ZDvidLabelSlice::isBodySelected(
    uint64_t bodyId, neutu::ELabelSource labelType) const
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    return m_selectedOriginal.count(bodyId) > 0;
  case neutu::ELabelSource::MAPPED:
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
  bool hasSelected = isBodySelected(m_hitLabel, neutu::ELabelSource::MAPPED);
  xorSelection(m_hitLabel, neutu::ELabelSource::MAPPED);

  if (!appending) {
    clearSelection();

    if (!hasSelected) {
      addSelection(m_hitLabel, neutu::ELabelSource::MAPPED);
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

void ZDvidLabelSlice::setMaxSize(
    const ZStackViewParam &viewParam, int maxWidth, int maxHeight)
{
  if (!getHelper()->hasMaxSize(maxWidth, maxHeight)) {
    getHelper()->setMaxSize(maxWidth, maxHeight);
    getHelper()->invalidateViewParam();
//    m_maxWidth = maxWidth;
//    m_maxHeight = maxHeight;
    m_objArray.clear();
    delete m_paintBuffer;
    m_paintBuffer = new ZImage(maxWidth, maxHeight, QImage::Format_ARGB32);

//    m_currentDataRect.setSize(QSize(0, 0));

    update(viewParam);
  }
}

bool ZDvidLabelSlice::resetSelectedLabelColor()
{
  bool changed = false;

  auto bodySet = getSelected(neutu::ELabelSource::MAPPED);
  for (auto body : bodySet) {
    if (removeLabelColor(body)) {
      changed = true;
    }
  }

  return changed;
}

bool ZDvidLabelSlice::setSelectedLabelColor(const QColor &color)
{
  bool changed = false;

  auto bodySet = getSelected(neutu::ELabelSource::MAPPED);
  for (auto body : bodySet) {
    if (setLabelColor(body, color)) {
      changed = true;
    }
  }

  return changed;
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
  return getMappedLabel(obj.getLabel(), neutu::ELabelSource::ORIGINAL);
}

uint64_t ZDvidLabelSlice::getMappedLabel(
    uint64_t label, neutu::ELabelSource labelType) const
{
  if (labelType == neutu::ELabelSource::ORIGINAL) {
    if (m_bodyMerger != NULL) {
      return m_bodyMerger->getFinalLabel(label);
    }
  }

  return label;
}
/*
const ZStackViewParam& ZDvidLabelSlice::getViewParam() const
{
  return m_currentViewParam;
}
*/
std::set<uint64_t> ZDvidLabelSlice::getSelected(
    neutu::ELabelSource labelType) const
{
  switch (labelType) {
  case neutu::ELabelSource::ORIGINAL:
    return getSelectedOriginal();
  case neutu::ELabelSource::MAPPED:
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
  m_selectedOriginal = getSelected(neutu::ELabelSource::MAPPED);
}

void ZDvidLabelSlice::startSelection()
{
  m_prevSelectedOriginal = m_selectedOriginal;
}

void ZDvidLabelSlice::endSelection()
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

/*
void ZDvidLabelSlice::clearCache()
{
  m_objCache.clear();
}
*/

bool ZDvidLabelSlice::refreshReaderBuffer()
{
  return getHelper()->getDvidReader().refreshLabelBuffer();
}
