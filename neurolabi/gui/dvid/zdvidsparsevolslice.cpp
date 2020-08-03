#include "zdvidsparsevolslice.h"

#include "neutubeconfig.h"
#include "logging/zqslog.h"
#include "zpainter.h"
#include "geometry/zintcuboid.h"

ZDvidSparsevolSlice::ZDvidSparsevolSlice() : ZObject3dScan()/*, m_currentZ(-1)*/
{
  m_type = GetType();
  setHitProtocal(ZStackObject::EHitProtocol::HIT_NONE);
//  setHittable(false);
  m_externalReader = NULL;
  m_isFullView = false;
}

ZDvidSparsevolSlice::ZDvidSparsevolSlice(const ZDvidSparsevolSlice &obj) :
  ZObject3dScan(obj)
{
  m_currentViewParam = obj.m_currentViewParam;
  m_isFullView = obj.m_isFullView;
//  m_currentZ = obj.m_currentZ;
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

bool ZDvidSparsevolSlice::isSliceVisible(int /*z*/, neutu::EAxis axis) const
{
  if (m_sliceAxis != axis) {
    return false;
  }

  return isVisible();
}

bool ZDvidSparsevolSlice::updateRequired(const ZStackViewParam &viewParam) const
{
  bool required = true;

  if (m_currentViewParam.contains(viewParam)) {
    required = false;
  } else if (m_isFullView) {
    if (m_currentViewParam.onSamePlane(viewParam)) {
      required = false;
    }
  }

  return required;
}

#if 0
bool ZDvidSparsevolSlice::updateRequired(int z) const
{
  bool required = true;

  if (z >= getMinZ() && z <= getMaxZ()) {
    if (m_isFullView) {
      required = false;
    }
  }

  return required;
}
#endif

void ZDvidSparsevolSlice::forceUpdate(int z)
{
  m_currentViewParam.setCutDepth(ZPoint(0, 0, 0), z);
  if (m_externalReader != NULL) {
    m_externalReader->readBody(
          getLabel(), m_currentViewParam.getCutCenter().getValue(
            m_sliceAxis),
          m_sliceAxis, true, this);
  } else {
    m_reader.readBody(
          getLabel(), m_currentViewParam.getCutCenter().getValue(
            m_sliceAxis),
          m_sliceAxis, true, this);
  }
  m_isFullView = true;
}


bool ZDvidSparsevolSlice::update(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return false;
  }

  if (!viewParam.isValid()) {
    return false;
  }

  bool updated = false;

  ZStackViewParam newViewParam = viewParam;

  if (updateRequired(newViewParam)) {
    if (newViewParam.getArea(neutu::data3d::ESpace::MODEL) < 800 * 600) {
      forceUpdate(newViewParam, true);
      m_isFullView = false;
    } else {
      forceUpdate(newViewParam.getCutCenter().getValue(
                    newViewParam.getSliceAxis()));
    }


    updated = true;

    m_currentViewParam = newViewParam;
  }

  return updated;
}

void ZDvidSparsevolSlice::update()
{
  update(m_currentViewParam);
//  m_reader.readBody(getLabel(), m_currentZ, m_sliceAxis, true, this);
}

void ZDvidSparsevolSlice::forceUpdate(
    const ZStackViewParam &viewParam, bool ignoringHidden)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  if ((!ignoringHidden) || isVisible()) {
    ZIntCuboid box = zgeom::GetIntBoundBox(viewParam.getCutRect());
    /*
    QRect viewPort = viewParam.getViewPort();
    ZIntCuboid box;
    box.setMinCorner(viewPort.left(), viewPort.top(), viewParam.getZ());
    box.setSize(viewPort.width(), viewPort.height(), 1);

    box.shiftSliceAxisInverse(m_sliceAxis);
    */

    if (m_externalReader != NULL) {
      m_externalReader->readBody(getLabel(), box, true, this);
    } else {
      m_reader.readBody(getLabel(), box, true, this);
    }
  }
}

/*
void ZDvidSparsevolSlice::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    neutu::EAxis sliceAxis) const
{
  if (slice >= 0) {
//    const_cast<ZDvidSparsevolSlice&>(*this).update(painter.getZOffset() + slice);
    ZObject3dScan::display(painter, slice, option, sliceAxis);
  }
}
*/
