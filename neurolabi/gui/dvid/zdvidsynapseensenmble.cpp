#include "zdvidsynapseensenmble.h"

#include<QRect>

#include "zdvidurl.h"
#include "zpainter.h"
#include "tz_math.h"
#include "dvid/zdvidwriter.h"
#include "zstackview.h"


ZDvidSynapseEnsemble::SynapseSlice
ZDvidSynapseEnsemble::m_emptySlice(ZDvidSynapseEnsemble::STATUS_NULL);
ZDvidSynapse ZDvidSynapseEnsemble::m_emptySynapse;

ZDvidSynapseEnsemble::ZDvidSynapseEnsemble()
{
  init();
}

void ZDvidSynapseEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (m_reader.open(target)) {
    m_dvidInfo = m_reader.readGrayScaleInfo();
    m_startZ = m_dvidInfo.getStartCoordinates().getSliceCoord(m_sliceAxis);
//    m_startY = m_dvidInfo.getStartCoordinates().getY();
  }
}

void ZDvidSynapseEnsemble::init()
{
  m_startZ = 0;
  m_type = TYPE_DVID_SYNAPE_ENSEMBLE;
  m_view = NULL;
  m_maxPartialArea = 1024 * 1024;
  m_sliceAxis = NeuTube::Z_AXIS;
}

ZIntCuboid ZDvidSynapseEnsemble::update(const ZIntCuboid &box)
{
  ZIntCuboid dataBox = box;
  if (!m_dataRange.isEmpty()) {
    dataBox.intersect(m_dataRange);
  }

  if (!dataBox.isEmpty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getSynapseUrl(dataBox));

    for (size_t i = 0; i < obj.size(); ++i) {
      ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (synapseJson.hasKey("Pos")) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(synapseJson);
        addSynapse(synapse, DATA_LOCAL);
      }
    }
  }

  return dataBox;
}

void ZDvidSynapseEnsemble::update(int x, int y, int z)
{
  ZDvidSynapse synapse = m_reader.readSynapse(x, y, z);
  if (synapse.isValid()) {
    addSynapse(synapse, DATA_LOCAL);
  } else {
    removeSynapse(x, y, z, DATA_LOCAL);
  }
}

void ZDvidSynapseEnsemble::update(const ZIntPoint &pt)
{
  update(pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidSynapseEnsemble::attachView(ZStackView *view)
{
  m_view = view;
}

void ZDvidSynapseEnsemble::download(int z)
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

  if (currentArea > 0 && currentArea < m_maxPartialArea) {
    QRect viewPort = m_view->getViewParameter().getViewPort();
    ZIntCuboid box(
          viewPort.left(), viewPort.top(), blockBox.getFirstCorner().getZ(),
          viewPort.right(), viewPort.bottom(), blockBox.getLastCorner().getZ());
    box.shiftSliceAxisInverse(m_sliceAxis);
    update(box);
    for (int cz = blockBox.getFirstCorner().getZ();
         cz <= blockBox.getLastCorner().getZ(); ++cz) {
      SynapseSlice &slice = getSlice(cz, ADJUST_FULL);
      slice.setDataRect(viewPort);
      slice.setStatus(STATUS_PARTIAL_READY);
    }
  } else {
    ZIntPoint lastCorner = m_dvidInfo.getEndCoordinates();
    ZIntPoint firstCorner = m_dvidInfo.getStartCoordinates();

    firstCorner.shiftSliceAxis(m_sliceAxis);
    lastCorner.shiftSliceAxis(m_sliceAxis);

    int width = lastCorner.getX() - firstCorner.getX() + 1;
    int height = lastCorner.getY() - firstCorner.getY() + 1;
    ZIntCuboid box;

    box.setFirstCorner(firstCorner.getX(), firstCorner.getY(),
                       blockBox.getFirstCorner().getZ());
    box.setSize(width, height, blockBox.getDepth());

    box.shiftSliceAxisInverse(m_sliceAxis);

    box = update(box);

    for (int cz = blockBox.getFirstCorner().getZ();
         cz <= blockBox.getLastCorner().getZ(); ++cz) {
      SynapseSlice &slice = getSlice(cz, ADJUST_FULL);
      if (m_dataRange.isEmpty()) {
        slice.setStatus(STATUS_READY);
      } else {
        slice.setDataRect(
              QRect(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                    box.getWidth(), box.getHeight()));
        slice.setStatus(STATUS_PARTIAL_READY);
      }
    }
  }
}

void ZDvidSynapseEnsemble::downloadForLabel(uint64_t label)
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getSynapseUrl(label));

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZDvidSynapse synapse;
    synapse.loadJsonObject(synapseJson);
    if (synapse.isValid()) {
      addSynapse(synapse, DATA_LOCAL);
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

ZDvidSynapseEnsemble::SynapseSlice& ZDvidSynapseEnsemble::getSlice(
    int z, EAdjustment adjust)
{
  if (adjust == ADJUST_NONE) {
    return getSlice(z);
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
ZDvidSynapseEnsemble::getSlice(int z) const
{
  int zIndex = z - m_startZ;

  if (zIndex >= 0 && zIndex < m_synapseEnsemble.size()) {
    return m_synapseEnsemble[zIndex];
  }

  return m_emptySlice;
}

ZDvidSynapseEnsemble::SynapseSlice& ZDvidSynapseEnsemble::getSlice(int z)
{
  return const_cast<ZDvidSynapseEnsemble::SynapseSlice&>(
        static_cast<const ZDvidSynapseEnsemble&>(*this).getSlice(z));
}

ZDvidSynapseEnsemble::SynapseMap &ZDvidSynapseEnsemble::getSynapseMap(
    int y, int z, EAdjustment adjust)
{
  SynapseSlice &slice = getSlice(z, adjust);

  return slice.getMap(y, adjust);
}

const ZDvidSynapseEnsemble::SynapseMap &ZDvidSynapseEnsemble::getSynapseMap(
    int y, int z) const
{
  const ZDvidSynapseEnsemble::SynapseSlice &slice = getSlice(z);

  return slice.getMap(y);
}

ZDvidSynapseEnsemble::SynapseMap &ZDvidSynapseEnsemble::getSynapseMap(int y, int z)
{
  return const_cast<ZDvidSynapseEnsemble::SynapseMap&>(
        static_cast<const ZDvidSynapseEnsemble&>(*this).getSynapseMap(y, z));
  /*
  int yIndex = y - m_startY;

  if (slice.size() <= yIndex) {
    slice.resize(yIndex + 1);
  }

  return slice[yIndex];
  */
}

int ZDvidSynapseEnsemble::getMinZ() const
{
  return m_startZ;
}

int ZDvidSynapseEnsemble::getMaxZ() const
{
  return m_startZ + m_synapseEnsemble.size() - 1;
}

bool ZDvidSynapseEnsemble::hasLocalSynapse(int x, int y, int z) const
{
  ZGeometry::shiftSliceAxis(x, y, z, m_sliceAxis);

  int zIndex = z - m_startZ;


  if (zIndex < 0 || z >= m_synapseEnsemble.size()) {
    return false;
  }

  const SynapseSlice &slice = getSlice(z);

  return slice.contains(x, y);

//  return m_synapseEnsemble[zIndex][yIndex].contains(x);
}

bool ZDvidSynapseEnsemble::removeSynapse(
    const ZIntPoint &pt, EDataScope scope)
{
  return removeSynapse(pt.getX(), pt.getY(), pt.getZ(), scope);
}

bool ZDvidSynapseEnsemble::removeSynapse(
    int x, int y, int z, EDataScope scope)
{
  if (scope == ZDvidSynapseEnsemble::DATA_LOCAL) {
    if (hasLocalSynapse(x, y, z)) {
      int sx = x;
      int sy = y;
      int sz = z;
      ZGeometry::shiftSliceAxis(sx, sy, sz, m_sliceAxis);
      getSynapseMap(sy, sz).remove(sx);
      getSelector().deselectObject(ZIntPoint(x, y, z));

      return true;
    }
  } else {
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
      writer.deleteSynapse(x, y, z);
    }

    if (writer.isStatusOk()) {
      return removeSynapse(x, y, z, DATA_LOCAL);
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

void ZDvidSynapseEnsemble::addSynapse(
    const ZDvidSynapse &synapse, EDataScope scope)
{
  if (scope == DATA_LOCAL) {
    ZIntPoint center = synapse.getPosition();
    center.shiftSliceAxis(m_sliceAxis);
    SynapseMap &synapseMap =
        getSynapseMap(center.getY(), center.getZ(), ADJUST_FULL);

    ZDvidSynapse &targetSynapse = synapseMap[center.getX()];
    if (!targetSynapse.isSelected() && synapse.isSelected()) {
      getSelector().selectObject(synapse.getPosition());
    }

    bool isSelected = targetSynapse.isSelected() || synapse.isSelected();
    targetSynapse = synapse;

    if (isSelected) {
      targetSynapse.setSelected(isSelected);
      updatePartner(targetSynapse);
    }
  } else {
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
      writer.writeSynapse(synapse);
      if (writer.isStatusOk()) {
        addSynapse(synapse, DATA_LOCAL);
      }
    }
  }
}

void ZDvidSynapseEnsemble::setRange(const ZIntCuboid &dataRange)
{
  m_dataRange = dataRange;
}

void ZDvidSynapseEnsemble::updateFromCache(int z)
{
  if (m_sliceCache.contains(z)) {
    SynapseSlice &slice =
        const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z, ADJUST_FULL);
    if (m_sliceCache[z]->isReady()) {
      slice = *(m_sliceCache[z]);
    }
  }
}

void ZDvidSynapseEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    NeuTube::EAxis sliceAxis) const
{
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

      rangeRect.setTopLeft(QPoint(range.getFirstCorner().getX(),
                                  range.getFirstCorner().getY()));
      rangeRect.setSize(QSize(range.getWidth(), m_dataRange.getHeight()));
    }

    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z >= m_dvidInfo.getStartCoordinates().getZ() ||
          z <= m_dvidInfo.getEndCoordinates().getZ()) {
        SynapseSlice &synapseSlice =
            const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z, ADJUST_FULL);
        bool isReady = synapseSlice.isReady();

        if (!isReady && m_view != NULL) {
          isReady = synapseSlice.isReady(
                m_view->getViewPort(NeuTube::COORD_STACK), rangeRect);
        }
        if (!isReady) {
          int blockZ = m_dvidInfo.getBlockIndexZ(z);
          if (blockZ != currentBlockZ) {
            currentBlockZ = blockZ;
            const_cast<ZDvidSynapseEnsemble&>(*this).download(z);
            /*
            if (synapseSlice.isEmpty()) {
              synapseSlice.push_back(QMap<int, ZDvidSynapse>());
            }
            */
          }
        }
      }
    }

    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z < m_dvidInfo.getStartCoordinates().getSliceCoord(m_sliceAxis) ||
          z > m_dvidInfo.getEndCoordinates().getSliceCoord(m_sliceAxis)) {
        continue;
      }

      SynapseSlice &synapseSlice =
          const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z, ADJUST_FULL);

      for (int i = 0; i < synapseSlice.size(); ++i) {
        QMap<int, ZDvidSynapse> &synapseMap = synapseSlice[i];
        for (QMap<int, ZDvidSynapse>::const_iterator iter = synapseMap.begin();
             iter != synapseMap.end(); ++iter) {
          const ZDvidSynapse &synapse = iter.value();
          synapse.display(painter, slice, option, sliceAxis);
        }
      }

      const std::set<ZIntPoint>& selected = m_selector.getSelectedSet();
      for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
           iter != selected.end(); ++iter) {
        ZDvidSynapse &synapse =
            const_cast<ZDvidSynapseEnsemble&>(*this).getSynapse(*iter, DATA_LOCAL);
        synapse.display(painter, slice, option, sliceAxis);
      }
    }
  }
}

void ZDvidSynapseEnsemble::removeSynapseLink(
    const ZIntPoint &v1, const ZIntPoint &v2)
{
  if (m_reader.good()) {
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
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
  if (from != to) {
    switch (scope) {
    case DATA_GLOBAL:
    case DATA_SYNC:
    {
      ZDvidWriter writer;
      if (writer.open(m_dvidTarget)) {
        writer.moveSynapse(from, to);
        if (writer.isStatusOk()) {
          writer.addSynapseProperty(to, "user", NeuTube::GetCurrentUserName());
          moveSynapse(from, to, DATA_LOCAL);
          ZDvidSynapse &synapse = getSynapse(to, DATA_LOCAL);
          if (synapse.isValid()) {
            synapse.setUserName(NeuTube::GetCurrentUserName());
          }
        }
      }
    }
    break;
    case DATA_LOCAL:
    {
      ZDvidSynapse &synapse = getSynapse(from, DATA_LOCAL);
      if (synapse.isValid()) {
        synapse.setPosition(to);
        addSynapse(synapse, DATA_LOCAL);
        removeSynapse(from, DATA_LOCAL);
      } else {
        update(to);
      }
    }
      break;
    }
  }
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(
    int x, int y, int z, EDataScope scope)
{
  if (scope == DATA_SYNC) {
    update(x, y, z);
  }

  if (hasLocalSynapse(x, y, z)) {
    int sx = x;
    int sy = y;
    int sz = z;
    ZGeometry::shiftSliceAxis(sx, sy, sz, m_sliceAxis);

    return getSlice(sz).getMap(sy)[sx];
  } else {
    if (scope == DATA_LOCAL) {
      return m_emptySynapse;
    } else if (scope == DATA_GLOBAL) {
      update(x, y, z);
    }
  }

  return getSynapse(x, y, z, DATA_LOCAL);
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(
    const ZIntPoint &center, EDataScope scope)
{
  return getSynapse(center.getX(), center.getY(), center.getZ(), scope);
}

bool ZDvidSynapseEnsemble::toggleHitSelect()
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

  ZDvidSynapse &synapse = getSynapse(m_hitPoint, DATA_LOCAL);
  synapse.setSelected(selecting);
  m_selector.setSelection(m_hitPoint, selecting);

  return selecting;
}

void ZDvidSynapseEnsemble::selectHit(bool appending)
{
  if (!appending) {
    std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
    for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
         iter != selectedList.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      ZDvidSynapse &synapse = getSynapse(pt, DATA_LOCAL);
      if (synapse.isValid()) {
        synapse.setSelected(false);
      }
    }
    m_selector.deselectAll();
  }
  m_selector.selectObject(m_hitPoint);
  ZDvidSynapse &synapse = getSynapse(m_hitPoint, DATA_LOCAL);
  if (synapse.isValid()) {
    synapse.setSelected(true);
  }
}

void ZDvidSynapseEnsemble::toggleHitSelectWithPartner()
{
  if (toggleHitSelect()) {
    ZDvidSynapse &synapse = getSynapse(m_hitPoint, DATA_LOCAL);
    updatePartner(synapse);
  }
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
    }
  }
}

void ZDvidSynapseEnsemble::selectHitWithPartner(bool appending)
{
  selectHit(appending);
  ZDvidSynapse &selectedSynapse = getSynapse(m_hitPoint, DATA_LOCAL);
  if (!selectedSynapse.isValid()) {
    return;
  }

  updatePartner(selectedSynapse);
}

bool ZDvidSynapseEnsemble::hit(double x, double y, double z)
{
  const int sliceRange = 5;

  ZIntPoint hitPoint(iround(x), iround(y), iround(z));

  hitPoint.shiftSliceAxis(getSliceAxis());

  for (int slice = -sliceRange; slice <= sliceRange; ++slice) {
    int cz = iround(hitPoint.getZ() + slice);

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

bool ZDvidSynapseEnsemble::hasSelected() const
{
  return !m_selector.getSelectedSet().empty();
}

std::ostream& operator<< (std::ostream &stream, const ZDvidSynapseEnsemble &se)
{
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

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapseEnsemble)

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
  if (se != NULL) {
    if (se->m_synapseEnsemble.size() > z) {
      m_yIterator = QVectorIterator<SynapseMap>(se->m_synapseEnsemble[z]);
      if (m_yIterator.hasNext()) {
        m_xIterator = QMapIterator<int, ZDvidSynapse>(m_yIterator.next());
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

///////////////////////////////////////////
ZDvidSynapseEnsemble::SynapseMap ZDvidSynapseEnsemble::SynapseSlice::m_emptyMap(
    ZDvidSynapseEnsemble::STATUS_NULL);

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
  if (m_status == STATUS_READY) {
    return true;
  }

  if (m_status == STATUS_PARTIAL_READY) {
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
    const ZDvidSynapse &synapse, NeuTube::EAxis sliceAxis)
{
  ZIntPoint center = synapse.getPosition();
  center.shiftSliceAxis(sliceAxis);
  SynapseMap& synapseMap = getMap(center.getY(), ADJUST_FULL);

  synapseMap[center.getX()] = synapse;
}
