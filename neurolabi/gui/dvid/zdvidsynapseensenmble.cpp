#include "zdvidsynapseensenmble.h"
#include "zdvidurl.h"
#include "zpainter.h"

ZDvidSynapseEnsenmble::ZDvidSynapseEnsenmble()
{
  m_startZ = 0;
  m_startY = 0;
}

void ZDvidSynapseEnsenmble::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  m_reader.open(target);

  m_dvidInfo = m_reader.readGrayScaleInfo();
  m_startZ = m_dvidInfo.getStartCoordinates().getZ();
  m_startY = m_dvidInfo.getStartCoordinates().getY();
}

void ZDvidSynapseEnsenmble::download(int z)
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
}

QVector<QMap<int, ZDvidSynapse> >& ZDvidSynapseEnsenmble::getSlice(int z)
{
  int zIndex = z - m_startZ;

  if (m_synapseEnsemble.size() <= zIndex) {
    m_synapseEnsemble.resize(zIndex + 1);
  }


  return m_synapseEnsemble[zIndex];
}

QMap<int, ZDvidSynapse> &ZDvidSynapseEnsenmble::getSynapseMap(int y, int z)
{
  QVector<QMap<int, ZDvidSynapse> > &slice = getSlice(z);

  int yIndex = y - m_startY;

  if (slice.size() <= yIndex) {
    slice.resize(yIndex + 1);
  }

  return slice[yIndex];
}

void ZDvidSynapseEnsenmble::removeSynapse(int x, int y, int z)
{
  getSynapseMap(y, z).remove(x);
}

void ZDvidSynapseEnsenmble::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (slice >= 0) {
    int z = painter.getZ(slice);

    QVector<QMap<int, ZDvidSynapse> > &synapseSlice =
        const_cast<ZDvidSynapseEnsenmble&>(*this).getSlice(z);
    if (synapseSlice.empty()) {
      const_cast<ZDvidSynapseEnsenmble&>(*this).download(z);
    }

    for (int i = 0; i < synapseSlice.size(); ++i) {
      QMap<int, ZDvidSynapse> &synapseMap = synapseSlice[i];
      for (QMap<int, ZDvidSynapse>::const_iterator iter = synapseMap.begin();
           iter != synapseMap.end(); ++iter) {
        iter.value().display(painter, slice, option);
      }
    }
  }
}
