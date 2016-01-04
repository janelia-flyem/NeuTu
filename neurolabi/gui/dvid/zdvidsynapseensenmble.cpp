#include "zdvidsynapseensenmble.h"
#include "zdvidurl.h"
#include "zpainter.h"
#include "tz_math.h"
#include "dvid/zdvidwriter.h"

ZDvidSynapseEnsemble::ZDvidSynapseEnsemble()
{
  m_startZ = 0;
  m_startY = 0;
  m_type = TYPE_DVID_SYNAPE_ENSEMBLE;
}

void ZDvidSynapseEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (m_reader.open(target)) {
    m_dvidInfo = m_reader.readGrayScaleInfo();
    m_startZ = m_dvidInfo.getStartCoordinates().getZ();
    m_startY = m_dvidInfo.getStartCoordinates().getY();
  }
}

void ZDvidSynapseEnsemble::update(int x, int y, int z)
{
  ZDvidSynapse synapse = m_reader.readSynapse(x, y, z);
  if (synapse.isValid()) {
    addSynapse(synapse, DATA_LOCAL);
  } else {
    removeSynapse(synapse.getPosition(), DATA_LOCAL);
  }
}

void ZDvidSynapseEnsemble::update(const ZIntPoint &pt)
{
  update(pt.getX(), pt.getY(), pt.getZ());
}

void ZDvidSynapseEnsemble::download(int z)
{
  int width = m_dvidInfo.getEndCoordinates().getX() -
      m_dvidInfo.getStartCoordinates().getX() + 1;
  int height = m_dvidInfo.getEndCoordinates().getY() -
      m_dvidInfo.getStartCoordinates().getY() + 1;

  ZIntCuboid blockBox =
      m_dvidInfo.getBlockBox(0, 0, m_dvidInfo.getBlockIndexZ(z));

  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = m_reader.readJsonArray(
        dvidUrl.getSynapseUrl(
          m_dvidInfo.getStartCoordinates().getX(),
          m_dvidInfo.getStartCoordinates().getY(),
          blockBox.getFirstCorner().getZ(),
          width, height, blockBox.getDepth()));

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (synapseJson.hasKey("Pos")) {
//      ZJsonArray posJson(synapseJson.value("Pos"));
//      int x = ZJsonParser::integerValue(posJson.at(0));
//      int y = ZJsonParser::integerValue(posJson.at(1));
//      int z = ZJsonParser::integerValue(posJson.at(2));

      ZDvidSynapse synapse;
      synapse.loadJsonObject(synapseJson);
      addSynapse(synapse, DATA_LOCAL);
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

QVector<QMap<int, ZDvidSynapse> >& ZDvidSynapseEnsemble::getSlice(int z)
{
  int zIndex = z - m_startZ;

  if (m_synapseEnsemble.size() <= zIndex) {
    m_synapseEnsemble.resize(zIndex + 1);
  }

  return m_synapseEnsemble[zIndex];
}

QMap<int, ZDvidSynapse> &ZDvidSynapseEnsemble::getSynapseMap(int y, int z)
{
  QVector<QMap<int, ZDvidSynapse> > &slice = getSlice(z);

  int yIndex = y - m_startY;

  if (slice.size() <= yIndex) {
    slice.resize(yIndex + 1);
  }

  return slice[yIndex];
}

bool ZDvidSynapseEnsemble::hasLocalSynapse(int x, int y, int z) const
{
  int zIndex = z - m_startZ;
  int yIndex = y - m_startY;

  if (zIndex < 0 || z >= m_synapseEnsemble.size()) {
    return false;
  } else {
    if (yIndex < 0 || m_synapseEnsemble[zIndex].size() <= y) {
      return false;
    }
  }

  return m_synapseEnsemble[zIndex][yIndex].contains(x);
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
      getSynapseMap(y, z).remove(x);
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
    int zDiff = synapse.getPosition().getZ() - m_startZ;
    if (zDiff < 0) {
      if (!m_synapseEnsemble.isEmpty()) {
        QVector<QVector<QMap<int, ZDvidSynapse> > > se = m_synapseEnsemble;
        m_synapseEnsemble.clear();
        m_synapseEnsemble.resize(se.size() - zDiff);
        for (int i = 0; i < se.size(); ++i) {
          m_synapseEnsemble[i - zDiff] = se[i];
        }
      }
      m_startZ += zDiff;
    }

    int yDiff = synapse.getPosition().getY() - m_startY;
    if (yDiff < 0) {
      for (QVector<QVector<QMap<int, ZDvidSynapse> > >::iterator
           iter = m_synapseEnsemble.begin(); iter != m_synapseEnsemble.end();
           ++iter) {
        QVector<QMap<int, ZDvidSynapse> > &slice = *iter;
        if (!slice.isEmpty()) {
          QVector<QMap<int, ZDvidSynapse> > oldSlice = slice;
          slice.clear();
          slice.resize(oldSlice.size() - yDiff);
          for (int i = 0; i < oldSlice.size(); ++i) {
            slice[i - yDiff] = oldSlice[i];
          }
        }
      }
      m_startY += yDiff;
    }

    QMap<int, ZDvidSynapse> &synapseMap =
        getSynapseMap(synapse.getPosition().getY(), synapse.getPosition().getZ());
    synapseMap[synapse.getPosition().getX()] = synapse;
    if (synapse.isSelected()) {
      getSelector().selectObject(synapse.getPosition());
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

void ZDvidSynapseEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (slice >= 0) {
    const int sliceRange = 5;

    int currentBlockZ = m_dvidInfo.getStartBlockIndex().getZ() - 1;
    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z >= m_dvidInfo.getStartCoordinates().getZ() ||
          z <= m_dvidInfo.getEndCoordinates().getZ()) {
        QVector<QMap<int, ZDvidSynapse> > &synapseSlice =
            const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z);
        if (synapseSlice.empty()) {
          int blockZ = m_dvidInfo.getBlockIndexZ(z);
          if (blockZ != currentBlockZ) {
            currentBlockZ = blockZ;
            const_cast<ZDvidSynapseEnsemble&>(*this).download(z);
            if (synapseSlice.isEmpty()) {
              synapseSlice.push_back(QMap<int, ZDvidSynapse>());
            }
          }
        }
      }
    }

    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z < m_dvidInfo.getStartCoordinates().getZ() ||
          z > m_dvidInfo.getEndCoordinates().getZ()) {
        continue;
      }
      QVector<QMap<int, ZDvidSynapse> > &synapseSlice =
          const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z);

      for (int i = 0; i < synapseSlice.size(); ++i) {
        QMap<int, ZDvidSynapse> &synapseMap = synapseSlice[i];
        for (QMap<int, ZDvidSynapse>::const_iterator iter = synapseMap.begin();
             iter != synapseMap.end(); ++iter) {
          const ZDvidSynapse &synapse = iter.value();
          synapse.display(painter, slice, option);
        }
      }

      const std::set<ZIntPoint>& selected = m_selector.getSelectedSet();
      for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
           iter != selected.end(); ++iter) {
        ZDvidSynapse &synapse =
            const_cast<ZDvidSynapseEnsemble&>(*this).getSynapse(*iter, DATA_LOCAL);
        synapse.display(painter, slice, option);
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
    const ZIntPoint &from, const ZIntPoint &to)
{
  if (from != to) {
    ZDvidWriter writer;
    if (writer.open(m_dvidTarget)) {
      writer.moveSynapse(from, to);
      if (writer.isStatusOk()) {
        ZDvidSynapse &synapse = getSynapse(from, DATA_LOCAL);
        if (synapse.isValid()) {
          synapse.setPosition(to);
          addSynapse(synapse, DATA_LOCAL);
          removeSynapse(from, DATA_LOCAL);
        } else {
          update(to);
        }
      }
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
    return m_synapseEnsemble[z][y][x];
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

  for (int slice = -sliceRange; slice <= sliceRange; ++slice) {
    int cz = iround(z + slice);

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

  stream << "Synapses: " << std::endl;
  while (siter.hasNext()) {
    stream << "  " << siter.next() << std::endl;
  }

  return stream;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapseEnsemble)

///////////////////Helper Classes///////////////////
ZDvidSynapseEnsemble::SynapseIterator::SynapseIterator(const ZDvidSynapseEnsemble *se) :
  m_zIterator(m_emptyZ), m_yIterator(m_emptyY), m_xIterator(m_emptyX)
{
  if (se != NULL) {
    m_zIterator = QVectorIterator<QVector<QMap<int, ZDvidSynapse> > >(
          se->m_synapseEnsemble);
    if (m_zIterator.hasNext()) {
      m_yIterator = QVectorIterator<QMap<int, ZDvidSynapse> >(m_zIterator.next());
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
      m_yIterator = QVectorIterator<QMap<int, ZDvidSynapse> >(
            se->m_synapseEnsemble[z]);
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
        m_yIterator = QVectorIterator<QMap<int, ZDvidSynapse> >(m_zIterator.next());
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

