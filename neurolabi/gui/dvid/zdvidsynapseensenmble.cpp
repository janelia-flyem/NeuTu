#include "zdvidsynapseensenmble.h"

#include <QRect>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QSet>

#include "common/math.h"
#include "zdvidurl.h"
#include "zpainter.h"
#include "zdvidwriter.h"
#include "mvc/zstackview.h"
#include "flyem/zflyemsynapsedatafetcher.h"
#include "geometry/zgeometry.h"

ZDvidSynapseEnsemble::SynapseSlice
ZDvidSynapseEnsemble::m_emptySlice(ZDvidSynapseEnsemble::EDataStatus::NONE);
ZDvidSynapse ZDvidSynapseEnsemble::m_emptySynapse;

ZDvidSynapseEnsemble::ZDvidSynapseEnsemble()
{
  init();
}

void ZDvidSynapseEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (m_reader.open(target)) {
    m_writer.open(target);
//    m_dvidInfo = m_reader.readGrayScaleInfo();

//    m_startY = m_dvidInfo.getStartCoordinates().getY();
  }
}

void ZDvidSynapseEnsemble::setDvidInfo(const ZDvidInfo &info)
{
  m_dvidInfo = info;
  m_startZ = m_dvidInfo.getStartCoordinates().getSliceCoord(m_sliceAxis);
}

void ZDvidSynapseEnsemble::init()
{
//  m_startZ = 0;
  m_type = EType::DVID_SYNAPE_ENSEMBLE;
//  m_view = NULL;
  m_maxPartialArea = 1024 * 1024;
  m_sliceAxis = neutu::EAxis::Z;
  addVisualEffect(neutu::display::VE_GROUP_HIGHLIGHT);
//  m_dataFetcher = NULL;
//  m_isReady = false;
}

ZIntCuboid ZDvidSynapseEnsemble::update(const ZIntCuboid &box)
{
  QMutexLocker locker(&m_dataMutex);

  return updateUnsync(box);
}

ZIntCuboid ZDvidSynapseEnsemble::updateUnsync(const ZIntCuboid &box)
{
  ZIntCuboid dataBox = box;
  if (!m_dataRange.isEmpty()) {
    dataBox.intersect(m_dataRange);
  }

  if (!dataBox.isEmpty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    QElapsedTimer timer;
    timer.start();
    ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getSynapseUrl(dataBox));
    LINFO() << "Synapse reading time: " << timer.elapsed();

    for (size_t i = 0; i < obj.size(); ++i) {
      ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (synapseJson.hasKey("Pos")) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(synapseJson, dvid::EAnnotationLoadMode::NO_PARTNER);
        addSynapseUnsync(synapse, EDataScope::LOCAL);
      }
    }
  }

  return dataBox;
}

void ZDvidSynapseEnsemble::update(int x, int y, int z)
{
  QMutexLocker locker(&m_dataMutex);

  updateUnsync(x, y, z);
}

void ZDvidSynapseEnsemble::update(const ZIntPoint &pt)
{
  update(pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidSynapseEnsemble::updateUnsync(int x, int y, int z)
{
  ZDvidSynapse synapse = m_reader.readSynapse(x, y, z);
  if (synapse.isValid()) {
    addSynapseUnsync(synapse, EDataScope::LOCAL);
  } else {
    removeSynapseUnsync(x, y, z, EDataScope::LOCAL);
  }
}

void ZDvidSynapseEnsemble::updateUnsync(const ZIntPoint &pt)
{
  updateUnsync(pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidSynapseEnsemble::attachView(ZStackView *view)
{
  m_view = view;
}

void ZDvidSynapseEnsemble::setDataFetcher(ZFlyEmSynapseDataFetcher *fetcher)
{
  m_dataFetcher = fetcher;
}

void ZDvidSynapseEnsemble::download(int z)
{
  QMutexLocker locker(&m_dataMutex);

  downloadUnsync(z);
}

void ZDvidSynapseEnsemble::unsyncedFetch(const ZIntCuboid &box)
{
  QVector<ZIntCuboid> region;
  region.append(box);
  m_dataFetcher->submit(region);
}

void ZDvidSynapseEnsemble::syncedFetch(
    const ZIntCuboid &box, int startZ, int endZ, bool isFull)
{
  ZIntCuboid dataBox = updateUnsync(box);
  for (int cz = startZ; cz <= endZ; ++cz) {
    SynapseSlice &slice = getSliceUnsync(cz, ADJUST_FULL);
    if (isFull && m_dataRange.isEmpty()) {
      slice.setStatus(EDataStatus::READY);
    } else {
      slice.setDataRect(
            QRect(dataBox.getMinCorner().getX(), dataBox.getMinCorner().getY(),
                  dataBox.getWidth(), box.getHeight()));
      slice.setStatus(EDataStatus::PARTIAL_READY);
    }
  }
}

void ZDvidSynapseEnsemble::downloadUnsync(int z)
{
  if (m_dvidTarget.getSynapseName().empty()) {
    return;
  }

  int currentArea = 0;
  if (m_view != NULL) {
    currentArea = m_view->getViewParameter().getArea();
  }

  int blockIndex = m_dvidInfo.getBlockIndexZ(z);
  ZIntCuboid blockBox =
      m_dvidInfo.getBlockBox(blockIndex, blockIndex, blockIndex);
  blockBox.shiftSliceAxis(m_sliceAxis);
  int startZ = blockBox.getMinCorner().getZ();
  int endZ = blockBox.getMaxCorner().getZ();

  if (currentArea > 0 && currentArea <= m_maxPartialArea) {
    QRect viewPort = m_view->getViewParameter().getViewPort();
    ZIntCuboid box(
          viewPort.left(), viewPort.top(), blockBox.getMinCorner().getZ(),
          viewPort.right(), viewPort.bottom(), blockBox.getMaxCorner().getZ());
    box.shiftSliceAxisInverse(m_sliceAxis);
    if (m_dataFetcher == NULL /*|| getSliceAxis() == neutube::Z_AXIS*/) {
      syncedFetch(box, startZ, endZ, false);
      /*
      updateUnsync(box);
      for (int cz = startZ; cz <= endZ; ++cz) {
        SynapseSlice &slice = getSliceUnsync(cz, ADJUST_FULL);
        slice.setDataRect(viewPort);
        slice.setStatus(STATUS_PARTIAL_READY);
      }
      */
    } else {
      unsyncedFetch(box);
    }
  } else if (m_fetchingFullAllowed) {
    ZIntPoint lastCorner = m_dvidInfo.getEndCoordinates();
    ZIntPoint firstCorner = m_dvidInfo.getStartCoordinates();

    firstCorner.shiftSliceAxis(m_sliceAxis);
    lastCorner.shiftSliceAxis(m_sliceAxis);

    int width = lastCorner.getX() - firstCorner.getX() + 1;
    int height = lastCorner.getY() - firstCorner.getY() + 1;
    ZIntCuboid box;

    box.setMinCorner(firstCorner.getX(), firstCorner.getY(),
                       blockBox.getMinCorner().getZ());
    box.setSize(width, height, blockBox.getDepth());

    box.shiftSliceAxisInverse(m_sliceAxis);

    if (m_dataFetcher == NULL) {
      syncedFetch(box, startZ, endZ, true);
      /*
      box = updateUnsync(box);

      for (int cz = startZ; cz <= endZ; ++cz) {
        SynapseSlice &slice = getSliceUnsync(cz, ADJUST_FULL);
        if (m_dataRange.isEmpty()) {
          slice.setStatus(STATUS_READY);
        } else {
          slice.setDataRect(
                QRect(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                      box.getWidth(), box.getHeight()));
          slice.setStatus(STATUS_PARTIAL_READY);
        }
      }
      */
    } else {
      unsyncedFetch(box);
    }
  }
}

void ZDvidSynapseEnsemble::downloadUnsync(const QVector<int> &zs)
{
  if (m_dvidTarget.getSynapseName().empty()) {
    return;
  }

  int currentArea = 0;
  if (m_view != NULL) {
    currentArea = m_view->getViewParameter().getArea();
  }

  QSet<int> blockZSet;

  for (QVector<int>::const_iterator iter = zs.begin(); iter != zs.end();
       ++iter) {
    int z = *iter;

    if (m_dataFetcher == NULL) {
      downloadUnsync(z);
    } else {
      if (currentArea > 0 && currentArea < m_maxPartialArea) {
        downloadUnsync(z);
      } else if (m_fetchingFullAllowed) {
        int blockIndex = m_dvidInfo.getBlockIndexZ(z);
        blockZSet.insert(blockIndex);
      }
    }
  }

  QVector<ZIntCuboid> region;

  for (QSet<int>::const_iterator iter = blockZSet.begin();
       iter != blockZSet.end(); ++iter) {
    int blockIndex = *iter;
    ZIntCuboid blockBox =
        m_dvidInfo.getBlockBox(blockIndex, blockIndex, blockIndex);
    blockBox.shiftSliceAxis(m_sliceAxis);

    ZIntPoint lastCorner = m_dvidInfo.getEndCoordinates();
    ZIntPoint firstCorner = m_dvidInfo.getStartCoordinates();

    firstCorner.shiftSliceAxis(m_sliceAxis);
    lastCorner.shiftSliceAxis(m_sliceAxis);

    int width = lastCorner.getX() - firstCorner.getX() + 1;
    int height = lastCorner.getY() - firstCorner.getY() + 1;
    ZIntCuboid box;

    box.setMinCorner(firstCorner.getX(), firstCorner.getY(),
                       blockBox.getMinCorner().getZ());
    box.setSize(width, height, blockBox.getDepth());

    box.shiftSliceAxisInverse(m_sliceAxis);
    region.append(box);
  }

  if (!region.isEmpty() && m_dataFetcher != NULL) {
    m_dataFetcher->submit(region);
    //      m_dataFetcher->addSynapse(this); //for testing
  }
}

void ZDvidSynapseEnsemble::setReadyUnsync(const ZIntCuboid &box)
{
  ZIntCuboid shiftedBox = box;
  shiftedBox.shiftSliceAxis(getSliceAxis());
  for (int cz = shiftedBox.getMinCorner().getZ();
       cz <= shiftedBox.getMaxCorner().getZ(); ++cz) {
    SynapseSlice &slice = getSliceUnsync(cz, ADJUST_FULL);
    slice.setDataRect(
          QRect(shiftedBox.getMinCorner().getX(),
                shiftedBox.getMinCorner().getY(),
                shiftedBox.getWidth(), shiftedBox.getHeight()));
    slice.setStatus(EDataStatus::PARTIAL_READY);
  }
}

void ZDvidSynapseEnsemble::setReady(const ZIntCuboid &box)
{
  QMutexLocker locker(&m_dataMutex);

  setReadyUnsync(box);
}

void ZDvidSynapseEnsemble::setReady(bool ready)
{
  m_isReady = ready;
}

bool ZDvidSynapseEnsemble::isReady() const
{
  return m_isReady;
}

void ZDvidSynapseEnsemble::downloadForLabelUnsync(uint64_t label)
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getSynapseUrl(label, false));

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZDvidSynapse synapse;
    synapse.loadJsonObject(synapseJson, dvid::EAnnotationLoadMode::PARTNER_LOCATION);
    if (synapse.isValid()) {
      addSynapse(synapse, EDataScope::LOCAL);
    }
#if 0
    if (synapseJson.hasKey("Pos")) {
      ZDvidSynapse synapse

      ZJsonArray posJson(synapseJson.value("Pos"));
      int x = ZJsonParser::integerValue(posJson.at(0));
      int y = ZJsonParser::integerValue(posJson.at(1));
      int z = ZJsonParser::integerValue(posJson.at(2));

      ZDvidSynapse& synapse = getSynapse(x, y, z);
      synapse.loadJsonObject(synapseJson);
    }
#endif
  }
}

ZDvidSynapseEnsemble::SynapseSlice& ZDvidSynapseEnsemble::getSliceUnsync(
    int z, EAdjustment adjust)
{
  if (adjust == ADJUST_NONE) {
    return getSliceUnsync(z);
  }

  int zIndex = z - m_startZ;
  if (m_synapseEnsemble.size() <= zIndex) {
    m_synapseEnsemble.resize(zIndex + 1);
  } else if (zIndex < 0) {
    if (adjust == ADJUST_FULL) {
      if (zIndex < 0) {
        QVector<SynapseSlice> se = m_synapseEnsemble;
        m_synapseEnsemble.clear();
        m_synapseEnsemble.resize(se.size() - zIndex);
        for (int i = 0; i < se.size(); ++i) {
          m_synapseEnsemble[i - zIndex] = se[i];
        }
        m_startZ = z;
        zIndex = 0;
      }
    }
  }


  return m_synapseEnsemble[zIndex];
}

const ZDvidSynapseEnsemble::SynapseSlice&
ZDvidSynapseEnsemble::getSliceUnsync(int z) const
{
  int zIndex = z - m_startZ;

  if (zIndex >= 0 && zIndex < m_synapseEnsemble.size()) {
    return m_synapseEnsemble[zIndex];
  }

  return m_emptySlice;
}

ZDvidSynapseEnsemble::SynapseSlice& ZDvidSynapseEnsemble::getSliceUnsync(int z)
{
  return const_cast<ZDvidSynapseEnsemble::SynapseSlice&>(
        static_cast<const ZDvidSynapseEnsemble&>(*this).getSliceUnsync(z));
}

ZDvidSynapseEnsemble::SynapseMap &ZDvidSynapseEnsemble::getSynapseMapUnsync(
    int y, int z, EAdjustment adjust)
{
  SynapseSlice &slice = getSliceUnsync(z, adjust);

  return slice.getMap(y, adjust);
}

const ZDvidSynapseEnsemble::SynapseMap&
ZDvidSynapseEnsemble::getSynapseMapUnsync(int y, int z) const
{
  const ZDvidSynapseEnsemble::SynapseSlice &slice = getSliceUnsync(z);

  return slice.getMap(y);
}

ZDvidSynapseEnsemble::SynapseMap &ZDvidSynapseEnsemble::getSynapseMapUnsync(
    int y, int z)
{
  return const_cast<ZDvidSynapseEnsemble::SynapseMap&>(
        static_cast<const ZDvidSynapseEnsemble&>(*this).getSynapseMapUnsync(y, z));
  /*
  int yIndex = y - m_startY;

  if (slice.size() <= yIndex) {
    slice.resize(yIndex + 1);
  }

  return slice[yIndex];
  */
}

int ZDvidSynapseEnsemble::getMinZUnsync() const
{
  return m_startZ;
}

int ZDvidSynapseEnsemble::getMaxZUnsync() const
{
  return m_startZ + m_synapseEnsemble.size() - 1;
}

bool ZDvidSynapseEnsemble::hasLocalSynapseUnsync(int x, int y, int z) const
{
  zgeom::shiftSliceAxis(x, y, z, m_sliceAxis);

  int zIndex = z - m_startZ;


  if (zIndex < 0 || zIndex >= m_synapseEnsemble.size()) {
    return false;
  }

  const SynapseSlice &slice = getSliceUnsync(z);

  return slice.contains(x, y);

//  return m_synapseEnsemble[zIndex][yIndex].contains(x);
}

bool ZDvidSynapseEnsemble::removeSynapseUnsync(
    const ZIntPoint &pt, EDataScope scope)
{
  return removeSynapseUnsync(pt.getX(), pt.getY(), pt.getZ(), scope);
}

bool ZDvidSynapseEnsemble::removeSynapse(const ZIntPoint &pt, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  return removeSynapseUnsync(pt, scope);
}

bool ZDvidSynapseEnsemble::removeSynapse(int x, int y, int z, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  return removeSynapseUnsync(x, y, z, scope);
}

bool ZDvidSynapseEnsemble::removeSynapseUnsync(int x, int y, int z, EDataScope scope)
{
  if (scope == ZDvidSynapseEnsemble::EDataScope::LOCAL) {
    if (hasLocalSynapseUnsync(x, y, z)) {
      int sx = x;
      int sy = y;
      int sz = z;
      zgeom::shiftSliceAxis(sx, sy, sz, m_sliceAxis);
      getSynapseMapUnsync(sy, sz).remove(sx);
      getSelector().deselectObject(ZIntPoint(x, y, z));

      return true;
    }
  } else {
    ZDvidWriter &writer = m_writer;
    if (writer.good()) {
      writer.deleteSynapse(x, y, z);
    }

    if (writer.isStatusOk()) {
      return removeSynapseUnsync(x, y, z, EDataScope::LOCAL);
    }
  }

  return false;
}


#if 0
void ZDvidSynapseEnsemble::commitSynapse(const ZIntPoint &pt)
{
  ZDvidSynapse &synapse = getSynapse(pt, DATA_LOCAL);
  if (!synapse.isValid()) {
    synapse.setPosition(pt);
    synapse.setKind(ZDvidSynapse::KIND_POST_SYN);
    synapse.setDefaultRadius();
    synapse.setDefaultColor();
    addSynapse(synapse);

    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
      writer.writeSynapse(synapse);
    }
  }
}
#endif

void ZDvidSynapseEnsemble::addSynapse(const ZDvidSynapse &synapse, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  addSynapseUnsync(synapse, scope);
}

void ZDvidSynapseEnsemble::addSynapseUnsync(
    const ZDvidSynapse &synapse, EDataScope scope)
{
  if (scope == EDataScope::LOCAL) {
    ZIntPoint center = synapse.getPosition();
    center.shiftSliceAxis(m_sliceAxis);
    SynapseMap &synapseMap =
        getSynapseMapUnsync(center.getY(), center.getZ(), ADJUST_FULL);

    ZDvidSynapse &targetSynapse = synapseMap[center.getX()];
    if (!targetSynapse.isSelected() && synapse.isSelected()) {
      getSelector().selectObject(synapse.getPosition());
    }

    bool isSelected = targetSynapse.isSelected() || synapse.isSelected();
    targetSynapse = synapse;
    targetSynapse.setDefaultRadius(getResolution());

    if (isSelected) {
      targetSynapse.setSelected(isSelected);
      updatePartner(targetSynapse);
    }
  } else {
    ZDvidWriter &writer = m_writer;
    if (writer.good()) {
      writer.writeSynapse(synapse);
      if (writer.isStatusOk()) {
        addSynapseUnsync(synapse, EDataScope::LOCAL);
      }
    }
  }
}

void ZDvidSynapseEnsemble::setUserName(
    int x, int y, int z, const std::string &userName, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  setUserNameUnsync(x, y, z, userName, scope);
}

void ZDvidSynapseEnsemble::setUserName(
    const ZIntPoint &pt, const std::string &userName, EDataScope scope)
{
  setUserName(pt.getX(), pt.getY(), pt.getZ(), userName, scope);
}

void ZDvidSynapseEnsemble::setUserNameUnsync(
    int x, int y, int z, const std::string &userName, EDataScope scope)
{
  if (scope == EDataScope::GLOBAL) {
    scope = EDataScope::SYNC;
  }
  ZDvidSynapse &synapse = getSynapseUnsync(x, y, z, scope);
  if (synapse.isValid()) {
    synapse.setUserName(userName);
    if (scope == EDataScope::GLOBAL || scope == EDataScope::SYNC) {
      ZDvidWriter &writer = m_writer;
      if (writer.good()) {
        writer.writeSynapse(synapse);
      }
    }
  }
}

void ZDvidSynapseEnsemble::setUserNameUnsync(
    const ZIntPoint &pt, const std::string &userName, EDataScope scope)
{
  setUserNameUnsync(pt.getX(), pt.getY(), pt.getZ(), userName, scope);
}

void ZDvidSynapseEnsemble::setConfidence(
    int x, int y, int z, const double c, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  setConfidenceUnsync(x, y, z, c, scope);
}

void ZDvidSynapseEnsemble::setConfidenceUnsync(
    int x, int y, int z, const double c, EDataScope scope)
{
    // copied mostly from setUserName()
    if (scope == EDataScope::GLOBAL) {
      scope = EDataScope::SYNC;
    }
    ZDvidSynapse &synapse = getSynapseUnsync(x, y, z, scope);
    if (synapse.isValid()) {
      synapse.setConfidence(c);
      if (scope == EDataScope::GLOBAL || scope == EDataScope::SYNC) {
        ZDvidWriter &writer = m_writer;
        if (writer.good()) {
          writer.writeSynapse(synapse);
        }
      }
    }
}

void ZDvidSynapseEnsemble::setConfidence(
    const ZIntPoint &pt, const double c, EDataScope scope) {
    setConfidence(pt.getX(), pt.getY(), pt.getZ(), c, scope);
}

void ZDvidSynapseEnsemble::setConfidenceUnsync(
    const ZIntPoint &pt, const double c, EDataScope scope) {
    setConfidenceUnsync(pt.getX(), pt.getY(), pt.getZ(), c, scope);
}

void ZDvidSynapseEnsemble::annotateSynapse(
    int x, int y, int z, const ZJsonObject &propJson, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);
  annotateSynapseUnsync(x, y, z, propJson, scope);
}

void ZDvidSynapseEnsemble::annotateSynapseUnsync(
    int x, int y, int z, const ZJsonObject &propJson, EDataScope scope)
{
  if (scope == EDataScope::GLOBAL) {
    scope = EDataScope::SYNC;
  }
  ZDvidSynapse &synapse = getSynapseUnsync(x, y, z, scope);
  if (synapse.isValid()) {
    synapse.updateProperty(propJson);
    if (scope == EDataScope::GLOBAL || scope == EDataScope::SYNC) {
      ZDvidWriter &writer = m_writer;
      if (writer.good()) {
        writer.writeSynapse(synapse);
      }
    }
  }
}

void ZDvidSynapseEnsemble::annotateSynapse(
    const ZIntPoint &pt, const ZJsonObject &propJson, EDataScope scope)
{
  annotateSynapse(pt.getX(), pt.getY(), pt.getZ(), propJson, scope);
}

void ZDvidSynapseEnsemble::annotateSynapseUnsync(
    const ZIntPoint &pt, const ZJsonObject &propJson, EDataScope scope)
{
  annotateSynapseUnsync(pt.getX(), pt.getY(), pt.getZ(), propJson, scope);
}


void ZDvidSynapseEnsemble::setRange(const ZIntCuboid &dataRange)
{
  m_dataRange = dataRange;
}

void ZDvidSynapseEnsemble::updateFromCacheUnsync(int z)
{
  if (m_sliceCache.contains(z)) {
    SynapseSlice &slice =
        const_cast<ZDvidSynapseEnsemble&>(*this).getSliceUnsync(z, ADJUST_FULL);
    if (m_sliceCache[z]->isReady()) {
      slice = *(m_sliceCache[z]);
    }
  }
}

void ZDvidSynapseEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    neutu::EAxis sliceAxis) const
{
  QMutexLocker locker(&m_dataMutex);

  if (sliceAxis != getSliceAxis()) {
    return;
  }

  if (slice >= 0) {
    const int sliceRange = 5;

    int currentBlockZ = m_dvidInfo.getStartBlockIndex().getZ() - 1;

    QRect rangeRect;
    if (!m_dataRange.isEmpty()) {
      ZIntCuboid range = m_dataRange;
      range.shiftSliceAxis(getSliceAxis());

      rangeRect.setTopLeft(QPoint(range.getMinCorner().getX(),
                                  range.getMinCorner().getY()));
      rangeRect.setSize(QSize(range.getWidth(), range.getHeight()));
    }

    if (!isReady()) {
      QVector<int> zs;
      for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
        int z = painter.getZ(slice + ds);
        if (z >= m_dvidInfo.getStartCoordinates().getZ() ||
            z <= m_dvidInfo.getEndCoordinates().getZ()) {
          SynapseSlice &synapseSlice =
              const_cast<ZDvidSynapseEnsemble&>(*this).getSliceUnsync(
                z, ADJUST_FULL);
          bool ready = synapseSlice.isReady();

          if (!ready && m_view != NULL) {
            ready = synapseSlice.isReady(
                  m_view->getViewPort(neutu::ECoordinateSystem::STACK), rangeRect);
          }
          if (!ready) {
            int blockZ = m_dvidInfo.getBlockIndexZ(z);
            if (blockZ != currentBlockZ) {
              currentBlockZ = blockZ;
              zs.append(z);
              //            const_cast<ZDvidSynapseEnsemble&>(*this).downloadUnsync(z);
              /*
            if (synapseSlice.isEmpty()) {
              synapseSlice.push_back(QMap<int, ZDvidSynapse>());
            }
            */
            }
          }
        }
      }
      const_cast<ZDvidSynapseEnsemble&>(*this).downloadUnsync(zs);
    }

    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z < m_dvidInfo.getStartCoordinates().getSliceCoord(m_sliceAxis) ||
          z > m_dvidInfo.getEndCoordinates().getSliceCoord(m_sliceAxis)) {
        continue;
      }

      SynapseSlice &synapseSlice =
          const_cast<ZDvidSynapseEnsemble&>(*this).getSliceUnsync(z, ADJUST_FULL);

      //Paint unselected synapses
      for (int i = 0; i < synapseSlice.size(); ++i) {
        QMap<int, ZDvidSynapse> &synapseMap = synapseSlice[i];
        for (QMap<int, ZDvidSynapse>::const_iterator iter = synapseMap.begin();
             iter != synapseMap.end(); ++iter) {
          const ZDvidSynapse &synapse = iter.value();
          if (!synapse.isSelected()) {
            EDisplayStyle tmpOption = option;
            if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN &&
                hasVisualEffect(neutu::display::VE_GROUP_HIGHLIGHT)) {
              tmpOption = EDisplayStyle::SKELETON;
            }
            synapse.display(painter, slice, tmpOption, sliceAxis);
          }
        }
      }  
    }

    //Paint selected synapses
    const std::set<ZIntPoint>& selected = m_selector.getSelectedSet();
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      ZDvidSynapse &synapse =
          const_cast<ZDvidSynapseEnsemble&>(*this).getSynapseUnsync(
            *iter, EDataScope::LOCAL);
      synapse.display(painter, slice, option, sliceAxis);
    }
  }
}

void ZDvidSynapseEnsemble::clearCache()
{
  m_synapseEnsemble.clear();
  m_dataFetcher->clearLastFetching();
}

void ZDvidSynapseEnsemble::removeSynapseLink(
    const ZIntPoint &v1, const ZIntPoint &v2)
{
  QMutexLocker locker(&m_dataMutex);

  if (m_reader.good()) {
    ZDvidWriter &writer = m_writer;
    if (writer.good()) {
      ZJsonObject obj1 = m_reader.readSynapseJson(v1);
      if (ZDvidSynapse::RemoveRelation(obj1, v2)) {
        writer.writeSynapse(obj1);
      }

      ZJsonObject obj2 = m_reader.readSynapseJson(v2);
      if (ZDvidSynapse::RemoveRelation(obj2, v1)) {
        writer.writeSynapse(obj2);
      }
    }
  }
}

void ZDvidSynapseEnsemble::moveSynapse(
    const ZIntPoint &from, const ZIntPoint &to, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  moveSynapseUnsync(from, to, scope);
}

void ZDvidSynapseEnsemble::moveSynapseUnsync(
    const ZIntPoint &from, const ZIntPoint &to, EDataScope scope)
{
  if (from != to) {
    switch (scope) {
    case EDataScope::GLOBAL:
    case EDataScope::SYNC:
    {
      ZDvidWriter &writer = m_writer;
      if (writer.good()) {
        writer.moveSynapse(from, to);
        if (writer.isStatusOk()) {
          moveSynapseUnsync(from, to, EDataScope::LOCAL);
        }
      }
    }
    break;
    case EDataScope::LOCAL:
    {
      ZDvidSynapse &synapse = getSynapseUnsync(from, EDataScope::LOCAL);
      if (synapse.isValid()) {
        synapse.setPosition(to);
        addSynapseUnsync(synapse, EDataScope::LOCAL);
        removeSynapseUnsync(from, EDataScope::LOCAL);
      } else {
        updateUnsync(to);
      }
    }
      break;
    }
  }
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(
    int x, int y, int z, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  return getSynapseUnsync(x, y, z, scope);
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapseUnsync(
    int x, int y, int z, EDataScope scope)
{
  if (scope == EDataScope::SYNC) {
    updateUnsync(x, y, z);
  }

  if (hasLocalSynapseUnsync(x, y, z)) {
    int sx = x;
    int sy = y;
    int sz = z;
    zgeom::shiftSliceAxis(sx, sy, sz, m_sliceAxis);

    return getSliceUnsync(sz).getMap(sy)[sx];
  } else {
    if (scope == EDataScope::LOCAL) {
      return m_emptySynapse;
    } else if (scope == EDataScope::GLOBAL) {
      updateUnsync(x, y, z);
    }
  }

  return getSynapseUnsync(x, y, z, EDataScope::LOCAL);
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(
    const ZIntPoint &center, EDataScope scope)
{
  QMutexLocker locker(&m_dataMutex);

  return getSynapseUnsync(center, scope);
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapseUnsync(
    const ZIntPoint &center, EDataScope scope)
{
  return getSynapseUnsync(center.getX(), center.getY(), center.getZ(), scope);
}

bool ZDvidSynapseEnsemble::toggleHitSelectUnsync()
{
  bool selecting = true;

  std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
  for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
       iter != selectedList.end(); ++iter) {
    const ZIntPoint &pt = *iter;

    if (pt == m_hitPoint) {
      selecting = false;
      break;
    }
  }

  ZDvidSynapse &synapse = getSynapseUnsync(m_hitPoint, EDataScope::LOCAL);
  synapse.setSelected(selecting);
  m_selector.setSelection(m_hitPoint, selecting);

  return selecting;
}

void ZDvidSynapseEnsemble::selectHitUnsync(bool appending)
{
  selectElementUnsync(m_hitPoint, appending);
}

void ZDvidSynapseEnsemble::selectElementUnsync(const ZIntPoint &pt, bool appending)
{
  if (!appending) {
    deselectSubUnsync();
  }
  m_selector.selectObject(pt);
  ZDvidSynapse &synapse = getSynapseUnsync(pt, EDataScope::LOCAL);
  if (synapse.isValid()) {
    synapse.setSelected(true);
  }
}

void ZDvidSynapseEnsemble::toggleHitSelectWithPartnerUnsync()
{
  if (toggleHitSelectUnsync()) {
    ZDvidSynapse &synapse = getSynapseUnsync(m_hitPoint, EDataScope::LOCAL);
    updatePartner(synapse);
  }
}

void ZDvidSynapseEnsemble::toggleHitSelectWithPartner()
{
  QMutexLocker locker(&m_dataMutex);

  if (toggleHitSelectUnsync()) {
    ZDvidSynapse &synapse = getSynapseUnsync(m_hitPoint, EDataScope::LOCAL);
    updatePartner(synapse);
  }
}

void ZDvidSynapseEnsemble::updatePartnerUnsync(const ZIntPoint &pt)
{
  ZDvidSynapse &synapse = getSynapseUnsync(pt, ZDvidSynapseEnsemble::EDataScope::LOCAL);
  if (synapse.isValid()) {
    updatePartner(synapse);
  }
}

void ZDvidSynapseEnsemble::updatePartner(const ZIntPoint &pt)
{
  QMutexLocker locker(&m_dataMutex);

  updatePartnerUnsync(pt);
}

void ZDvidSynapseEnsemble::updatePartner(ZDvidSynapse &synapse)
{
  if (synapse.isValid()) {
    synapse.clearPartner();

    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray objArray = m_reader.readJsonArray(
          dvidUrl.getSynapseUrl(synapse.getPosition(), 1, 1, 1));

    if (!objArray.isEmpty()) {
      ZJsonObject obj(objArray.value(0));
      synapse.loadJsonObject(obj, dvid::EAnnotationLoadMode::PARTNER_RELJSON);
      synapse.updatePartner();
      synapse.updatePartnerProperty(m_reader);
#if 0
      if (obj.hasKey("Rels")) {
        ZJsonArray jsonArray(obj.value("Rels"));
        if (jsonArray.size() > 0) {
          for (size_t i = 0; i < jsonArray.size(); ++i) {
            ZJsonObject partnerJson(jsonArray.value(i));
            if (partnerJson.hasKey("To")) {
              ZJsonArray posJson(partnerJson.value("To"));
              std::vector<int> coords = posJson.toIntegerArray();
              synapse.addPartner(coords[0], coords[1], coords[2]);
            }
          }
        }
      }
#endif
    }
  }
}

void ZDvidSynapseEnsemble::updateRadiusUnsync()
{
  for (auto &s : m_synapseEnsemble) {
    s.updateRadius();
  }
}

void ZDvidSynapseEnsemble::updateRadius()
{
  QMutexLocker locker(&m_dataMutex);
  updateRadiusUnsync();
}

void ZDvidSynapseEnsemble::selectHitWithPartner(bool appending)
{
  QMutexLocker locker(&m_dataMutex);

  selectHitWithPartnerUnsync(appending);
}

void ZDvidSynapseEnsemble::selectHitWithPartnerUnsync(bool appending)
{
  selectHitUnsync(appending);
  ZDvidSynapse &selectedSynapse = getSynapseUnsync(m_hitPoint, EDataScope::LOCAL);
  if (!selectedSynapse.isValid()) {
    return;
  }

  updatePartner(selectedSynapse);
}

void ZDvidSynapseEnsemble::selectWithPartner(
    const ZIntPoint &pt, bool appending)
{
  QMutexLocker locker(&m_dataMutex);

  selectWithPartnerUnsync(pt, appending);
}

void ZDvidSynapseEnsemble::selectWithPartnerUnsync(
    const ZIntPoint &pt, bool appending)
{
  selectElementUnsync(pt, appending);
  ZDvidSynapse &selectedSynapse = getSynapseUnsync(pt, EDataScope::LOCAL);
  if (!selectedSynapse.isValid()) {
    return;
  }

  updatePartner(selectedSynapse);
}

bool ZDvidSynapseEnsemble::hit(const ZIntPoint &pt)
{
  return hit(pt.getX(), pt.getY(), pt.getZ());
}

bool ZDvidSynapseEnsemble::hit(double x, double y, double z)
{
  QMutexLocker locker(&m_dataMutex);

  const int sliceRange = 5;

  ZIntPoint hitPoint(neutu::iround(x), neutu::iround(y), neutu::iround(z));

  hitPoint.shiftSliceAxis(getSliceAxis());

  for (int slice = -sliceRange; slice <= sliceRange; ++slice) {
    int cz = hitPoint.getZ() + slice;

    SynapseIterator siter(this, cz);

    while (siter.hasNext()) {
      ZDvidSynapse &synapse = siter.next();
      if (synapse.hit(x, y, z)) {
        m_hitPoint = synapse.getPosition();
        return true;
      }
    }
  }

  return false;
}

void ZDvidSynapseEnsemble::deselectSubUnsync()
{
  std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
  for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
       iter != selectedList.end(); ++iter) {
    const ZIntPoint &pt = *iter;
    ZDvidSynapse &synapse = getSynapseUnsync(pt, EDataScope::LOCAL);
    if (synapse.isValid()) {
      synapse.setSelected(false);
    }
  }
  m_selector.deselectAll();
}

void ZDvidSynapseEnsemble::deselectUnsync(bool recursive)
{
  setSelected(false);
  if (recursive) {
    deselectSubUnsync();
  }
}


bool ZDvidSynapseEnsemble::hasSelected() const
{
  return !m_selector.getSelectedSet().empty();
}

std::ostream& operator<< (std::ostream &stream, const ZDvidSynapseEnsemble &se)
{
  QMutexLocker locker(&se.m_dataMutex);

  ZDvidSynapseEnsemble::SynapseIterator siter(&se);

  stream << "Synapses (+" << se.m_startZ << "): " << std::endl;
  while (siter.hasNext()) {
    stream << "  " << siter.next() << std::endl;
  }

  return stream;
}

std::ostream& operator<< (
    std::ostream &stream, const ZDvidSynapseEnsemble::SynapseSlice &se)
{
  stream << "Synapse slice (+" << se.m_startY << "):" << std::endl;
  for (ZDvidSynapseEnsemble::SynapseSlice::const_iterator iter = se.begin();
       iter != se.end(); ++iter) {
    const ZDvidSynapseEnsemble::SynapseMap &synapseMap = *iter;
    if (!synapseMap.isEmpty()) {
      for (ZDvidSynapseEnsemble::SynapseMap::const_iterator
           iter = synapseMap.begin(); iter != synapseMap.end(); ++iter) {
        stream << "  " << iter.value() << std::endl;
      }
    }
  }

  return stream;
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapseEnsemble)

///////////////////Helper Classes///////////////////
QVector<ZDvidSynapseEnsemble::SynapseSlice>
ZDvidSynapseEnsemble::SynapseIterator::m_emptyZ;
QVector<ZDvidSynapseEnsemble::SynapseMap>
ZDvidSynapseEnsemble::SynapseIterator::m_emptyY;
QMap<int, ZDvidSynapse> ZDvidSynapseEnsemble::SynapseIterator::m_emptyX;

ZDvidSynapseEnsemble::SynapseIterator::SynapseIterator(const ZDvidSynapseEnsemble *se) :
  m_zIterator(m_emptyZ), m_yIterator(m_emptyY), m_xIterator(m_emptyX)
{
  if (se != NULL) {
    m_zIterator = QVectorIterator<SynapseSlice>(se->m_synapseEnsemble);
    if (m_zIterator.hasNext()) {
      m_yIterator = QVectorIterator<SynapseMap>(m_zIterator.next());
      if (m_yIterator.hasNext()) {
        m_xIterator = QMapIterator<int, ZDvidSynapse>(m_yIterator.next());
      }
    }
  }
}

ZDvidSynapseEnsemble::SynapseIterator::SynapseIterator(
    const ZDvidSynapseEnsemble *se, int z) :
  m_zIterator(m_emptyZ), m_yIterator(m_emptyY), m_xIterator(m_emptyX)
{
  int index = z - se->getMinZUnsync();
  if (index >= 0) {
    if (se != NULL) {
      if (se->m_synapseEnsemble.size() > index) {
        m_yIterator = QVectorIterator<SynapseMap>(se->m_synapseEnsemble[index]);
        if (m_yIterator.hasNext()) {
          m_xIterator = QMapIterator<int, ZDvidSynapse>(m_yIterator.next());
        }
      }
    }
  }
}

void ZDvidSynapseEnsemble::SynapseIterator::skipEmptyIterator()
{
  while (!m_xIterator.hasNext()) {
    if (m_yIterator.hasNext()) {
      m_xIterator = QMapIterator<int, ZDvidSynapse>(m_yIterator.next());
    } else {
      if (m_zIterator.hasNext()) {
        m_yIterator = QVectorIterator<SynapseMap>(m_zIterator.next());
      } else { //All iterators reach the end
        break;
      }
    }
  }
}

bool ZDvidSynapseEnsemble::SynapseIterator::hasNext() const
{
  const_cast<ZDvidSynapseEnsemble::SynapseIterator&>(*this).skipEmptyIterator();

  return m_xIterator.hasNext();
}

ZDvidSynapse& ZDvidSynapseEnsemble::SynapseIterator::next()
{
  skipEmptyIterator();

  return const_cast<ZDvidSynapse&>(m_xIterator.next().value());
}


///////////////////////////////////////////////
ZDvidSynapseEnsemble::SynapseMap::SynapseMap(EDataStatus status)
{
  m_status = status;
}

void ZDvidSynapseEnsemble::SynapseMap::updateRadius()
{
  for (auto &s : *this) {
    s.setDefaultRadius();
  }
}

///////////////////////////////////////////
ZDvidSynapseEnsemble::SynapseMap ZDvidSynapseEnsemble::SynapseSlice::m_emptyMap(
    ZDvidSynapseEnsemble::EDataStatus::NONE);

ZDvidSynapseEnsemble::SynapseSlice::SynapseSlice(EDataStatus status)
{
  m_startY = 0;
  m_status = status;
}

bool ZDvidSynapseEnsemble::SynapseSlice::contains(int x, int y) const
{
  const SynapseMap &synapseMap = getMap(y);
  if (synapseMap.isValid()) {
    return synapseMap.contains(x);
  }

  return false;
}

bool ZDvidSynapseEnsemble::SynapseSlice::isReady(
    const QRect &rect, const QRect &range) const
{
  if (m_status == EDataStatus::READY) {
    return true;
  }

  if (m_status == EDataStatus::PARTIAL_READY) {
//    qDebug() << "Data rect: " << m_dataRect;
//    qDebug() << "New rect: " << rect;

    QRect dataRect = rect;
    if (!range.isEmpty()) {
      dataRect = rect.intersected(range);
    }

    if (!dataRect.isValid()) {
      return false;
    }

    return m_dataRect.contains(dataRect);
  }

  return false;
}

void ZDvidSynapseEnsemble::SynapseSlice::setDataRect(const QRect &rect)
{
  m_dataRect = rect;
}

bool ZDvidSynapseEnsemble::SynapseSlice::isReady(const QRect &rect) const
{
  return isReady(rect, m_dataRect);
}

void ZDvidSynapseEnsemble::SynapseSlice::updateRadius()
{
  for (auto &s : *this) {
    s.updateRadius();
  }
}


ZDvidSynapseEnsemble::SynapseMap&
ZDvidSynapseEnsemble::SynapseSlice::getMap(int y)
{
  return const_cast<ZDvidSynapseEnsemble::SynapseMap&>(
        static_cast<const ZDvidSynapseEnsemble::SynapseSlice&>(*this).getMap(y));
}

const ZDvidSynapseEnsemble::SynapseMap&
ZDvidSynapseEnsemble::SynapseSlice::getMap(int y) const
{
  if (isValid()) {
    int yIndex = y - m_startY;
    if (yIndex >= 0 && yIndex < size()) {
      return (*this)[yIndex];
    }
  }

  return m_emptyMap;
}

ZDvidSynapseEnsemble::SynapseMap&
ZDvidSynapseEnsemble::SynapseSlice::getMap(int y, EAdjustment adjust)
{
  if (!isValid()) {
    return m_emptyMap;
  }

  if (adjust == ADJUST_NONE) {
    return getMap(y);
  }

  int yIndex = y - m_startY;
  if (yIndex >= size()) {
    if (isEmpty()) {
      m_startY = y;
      resize(1);
      yIndex = 0;
    } else {
      resize(yIndex + 1);
    }
  } else if (yIndex < 0) {
    if (adjust == ADJUST_FULL) {
      SynapseSlice oldSlice = *this;
      clear();
      resize(oldSlice.size() - yIndex);
      for (int i = 0; i < oldSlice.size(); ++i) {
        (*this)[i - yIndex] = oldSlice[i];
      }
      m_startY = y;
      yIndex = 0;
    }
  }

  return (*this)[yIndex];
}

void ZDvidSynapseEnsemble::SynapseSlice::addSynapse(
    const ZDvidSynapse &synapse, neutu::EAxis sliceAxis)
{
  ZIntPoint center = synapse.getPosition();
  center.shiftSliceAxis(sliceAxis);
  SynapseMap& synapseMap = getMap(center.getY(), ADJUST_FULL);

  synapseMap[center.getX()] = synapse;
}
