#include "zdvidsynapseensenmble.h"
#include "zdvidurl.h"
#include "zpainter.h"

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

void ZDvidSynapseEnsemble::download(int z)
{
  int width = m_dvidInfo.getEndCoordinates().getX() -
      m_dvidInfo.getStartCoordinates().getX() + 1;
  int height = m_dvidInfo.getEndCoordinates().getY() -
      m_dvidInfo.getStartCoordinates().getY() + 1;

  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = m_reader.readJsonArray(
        dvidUrl.getSynapseUrl(
          m_dvidInfo.getStartCoordinates().getX(),
          m_dvidInfo.getStartCoordinates().getY(),
          z, width, height, 1));

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (synapseJson.hasKey("Pos")) {
      ZJsonArray posJson(synapseJson.value("Pos"));
      int x = ZJsonParser::integerValue(posJson.at(0));
      int y = ZJsonParser::integerValue(posJson.at(1));
      int z = ZJsonParser::integerValue(posJson.at(2));

      ZDvidSynapse& synapse = getSynapse(x, y, z);
      synapse.loadJsonObject(synapseJson);
    }
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

void ZDvidSynapseEnsemble::removeSynapse(int x, int y, int z)
{
  getSynapseMap(y, z).remove(x);
}

void ZDvidSynapseEnsemble::addSynapse(const ZDvidSynapse &synapse)
{
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

  getSynapse(synapse.getPosition()) = synapse;
}

void ZDvidSynapseEnsemble::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (slice >= 0) {
    int z = painter.getZ(slice);

    QVector<QMap<int, ZDvidSynapse> > &synapseSlice =
        const_cast<ZDvidSynapseEnsemble&>(*this).getSlice(z);
    if (synapseSlice.empty()) {
      const_cast<ZDvidSynapseEnsemble&>(*this).download(z);
    }

    for (int i = 0; i < synapseSlice.size(); ++i) {
      QMap<int, ZDvidSynapse> &synapseMap = synapseSlice[i];
      for (QMap<int, ZDvidSynapse>::const_iterator iter = synapseMap.begin();
           iter != synapseMap.end(); ++iter) {
        const ZDvidSynapse &synapse = iter.value();
        synapse.display(painter, slice, option);
      }
    }
  }
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(int x, int y, int z)
{
  QMap<int, ZDvidSynapse> &synapseMap = getSynapseMap(y, z);
  if (!synapseMap.contains(x)) {
    synapseMap[x] = ZDvidSynapse();
  }

  return synapseMap[x];
}

ZDvidSynapse& ZDvidSynapseEnsemble::getSynapse(const ZIntPoint &center)
{
  return getSynapse(center.getX(), center.getY(), center.getZ());
}

void ZDvidSynapseEnsemble::selectHit(bool appending)
{
  if (!appending) {
    std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
    for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
         iter != selectedList.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      getSynapse(pt).setSelected(false);
    }
    m_selector.deselectAll();
  }
  m_selector.selectObject(m_hitPoint);
  getSynapse(m_hitPoint).setSelected(true);
}

bool ZDvidSynapseEnsemble::hit(double x, double y, double z)
{
  SynapseIterator siter(this, z);

  while (siter.hasNext()) {
    ZDvidSynapse &synapse = siter.next();
    if (synapse.hit(x, y, z)) {
      m_hitPoint = synapse.getPosition();
      return true;
    }
  }

  return false;
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
