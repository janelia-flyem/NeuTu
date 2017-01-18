#include "zdvidsparsevolslice.h"

#include "neutubeconfig.h"
#include "zpainter.h"

ZDvidSparsevolSlice::ZDvidSparsevolSlice() : ZObject3dScan(), m_currentZ(-1)
{
  m_type = GetType();
  setHittable(false);
  m_externalReader = NULL;
}

ZDvidSparsevolSlice::ZDvidSparsevolSlice(const ZDvidSparsevolSlice &obj) :
  ZObject3dScan(obj)
{
  m_currentZ = obj.m_currentZ;
  m_dvidTarget = obj.m_dvidTarget;
  m_externalReader = obj.m_externalReader;
}

ZDvidSparsevolSlice::~ZDvidSparsevolSlice()
{
  ZOUT(LTRACE(), 5) << "Deconstructing ZDvidSparsevolSlice" << this;
}

void ZDvidSparsevolSlice::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  m_reader.open(target);
}

void ZDvidSparsevolSlice::setReader(ZDvidReader *reader)
{
  m_externalReader = reader;
}

bool ZDvidSparsevolSlice::isSliceVisible(int /*z*/, NeuTube::EAxis axis) const
{
  if (m_sliceAxis != axis) {
    return false;
  }

  return isVisible();
}

bool ZDvidSparsevolSlice::update(int z)
{
  bool updated = false;
  if (m_currentZ != z) {
    m_currentZ = z;
    if (z < getMinZ() || z > getMaxZ()) {
      if (m_externalReader != NULL) {
        m_externalReader->readBody(
              getLabel(), m_currentZ, m_sliceAxis, true, this);
      } else {
        m_reader.readBody(getLabel(), m_currentZ, m_sliceAxis, true, this);
      }
      updated = true;
    }
  }

  return updated;
}

void ZDvidSparsevolSlice::update()
{
  m_reader.readBody(getLabel(), m_currentZ, m_sliceAxis, true, this);
}

void ZDvidSparsevolSlice::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    NeuTube::EAxis sliceAxis) const
{
  if (slice >= 0) {
    const_cast<ZDvidSparsevolSlice&>(*this).update(painter.getZOffset() + slice);
    ZObject3dScan::display(painter, slice, option, sliceAxis);
  }
}
