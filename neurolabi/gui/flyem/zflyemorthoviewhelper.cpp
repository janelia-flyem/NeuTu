#include "zflyemorthoviewhelper.h"
#include "zcrosshair.h"
#include "zflyemorthomvc.h"
#include "zflyemorthodoc.h"
#include "zstackview.h"

ZFlyEmOrthoViewHelper::ZFlyEmOrthoViewHelper()
{
  m_mvc = NULL;
}

void ZFlyEmOrthoViewHelper::attach(ZFlyEmOrthoMvc *mvc)
{
  m_mvc = mvc;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoViewHelper::getMasterMvc() const
{
  return m_mvc;
}

ZFlyEmOrthoDoc* ZFlyEmOrthoViewHelper::getMasterDoc() const
{
  if (getMasterMvc() != NULL) {
    return getMasterMvc()->getCompleteDocument();
  }

  return NULL;
}

ZStackView* ZFlyEmOrthoViewHelper::getMasterView() const
{
  if (getMasterMvc() != NULL) {
    return getMasterMvc()->getView();
  }

  return NULL;
}

neutube::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(
    neutube::EAxis a1, neutube::EAxis a2)
{
  if (a1 > a2) {
    return getAlignAxis(a2, a1);
  }

  switch (a1) {
  case neutube::EAxis::X:
    if (a2 == neutube::EAxis::Y) {
      return neutube::EAxis::Z;
    } else if (a2 == neutube::EAxis::Z) {
      return neutube::EAxis::X;
    }
    break;
  case neutube::EAxis::Y:
    if (a2 == neutube::EAxis::Z) {
      return neutube::EAxis::Y;
    }
    break;
  default:
    break;
  }

  return neutube::EAxis::X;
}

neutube::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(const ZFlyEmOrthoMvc *mvc)
{
  return getAlignAxis(mvc->getView()->getSliceAxis(),
                      getMasterMvc()->getView()->getSliceAxis());
}

ZPoint ZFlyEmOrthoViewHelper::getCrossCenter() const
{
  ZPoint center;
  if (getMasterMvc() != NULL) {
    center = getMasterDoc()->getCrossHair()->getCenter();
    center.shiftSliceAxis(getMasterView()->getSliceAxis());
#ifdef _DEBUG_
    std::cout << "Cross hair center: " << center.toString() << std::endl;
#endif
    ZViewProj viewProj = getMasterView()->getViewProj();
    QPointF pt = viewProj.mapPointBackF(QPointF(center.getX(), center.getY()));
    center.setX(pt.x());
    center.setY(pt.y());
    center.setZ(getMasterView()->getCurrentZ());
    center.shiftSliceAxisInverse(getMasterView()->getSliceAxis());
  }

  return center;
}

void ZFlyEmOrthoViewHelper::syncCrossHair(ZFlyEmOrthoMvc *mvc)
{
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_
    std::cout << "Sync crosshair from " << neutube::EnumValue(getMasterView()->getSliceAxis())
              << " to " << neutube::EnumValue(mvc->getView()->getSliceAxis()) << std::endl;
#endif
//    ZCrossHair *crossHair = mvc->getCompleteDocument()->getCrossHair();
//    ZPoint crossCenter = getMasterDoc()->getCrossHair()->getCenter();
    ZPoint mappedCrossCenter = getCrossCenter();
//    mappedCrossCenter.shiftSliceAxisInverse(getMasterMvc);
    mvc->getView()->setZ(
          mappedCrossCenter.getSliceCoord(mvc->getView()->getSliceAxis()));
    mvc->getView()->updateImageScreen(ZStackView::UPDATE_QUEUED);

#if 0
    NeuTube::EAxis axis = getAlignAxis(mvc);
    switch (axis) {
    case NeuTube::X_AXIS:
//      crossHair->setX(crossCenter.getX());
      mvc->getView()->setZ(mappedCrossCenter.getX());
      break;
    case NeuTube::Y_AXIS:
//      crossHair->setY(crossCenter.getY());
      mvc->getView()->setZ(mappedCrossCenter.getY());
      break;
    case NeuTube::Z_AXIS:
      if (mvc->getView()->getSliceAxis() == NeuTube::X_AXIS) {
//        crossHair->setZ(crossCenter.getY());
        mvc->getView()->setZ(mappedCrossCenter.getX());
      } else {
//        crossHair->setZ(crossCenter.getX());
        mvc->getView()->setZ(mappedCrossCenter.getY());
      }
      break;
    }
#endif

#ifdef _DEBUG_2
    getMasterMvc()->getView()->printViewParam();
    mvc->getView()->printViewParam();
#endif
  }
}

void ZFlyEmOrthoViewHelper::syncViewPort(ZFlyEmOrthoMvc *mvc)
{
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_2
    std::cout << "Sync viewport from " << getMasterView()->getSliceAxis()
              << " to " << mvc->getView()->getSliceAxis() << std::endl;
#endif

    ZViewProj viewProj = getMasterView()->getViewProj();
    ZViewProj newViewProj = mvc->getView()->getViewProj();
    ZPoint mappedCrossCenter = getCrossCenter();
    neutube::EAxis slaveAxis = mvc->getView()->getSliceAxis();

    if (slaveAxis == neutube::EAxis::ARB) {
      return;
    }

    ZCrossHair *refCross = mvc->getCompleteDocument()->getCrossHair();

    ZPoint refCenter = refCross->getCenter();
    refCenter.shiftSliceAxis(slaveAxis);

    QPointF refCrossPos = QPointF(refCenter.getX(), refCenter.getY());

    switch (slaveAxis) {
    case neutube::EAxis::Z:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getY()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getZ());
      break;
    case neutube::EAxis::X:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getZ(), mappedCrossCenter.getY()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getX());
      break;
    case neutube::EAxis::Y:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getZ()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getY());
      break;
    case neutube::EAxis::ARB:
      break;
    }

    //To avoid misalignment caused by rounding error
    neutube::EAxis axis = getAlignAxis(mvc);
    switch (axis) {
    case neutube::EAxis::Y:
      newViewProj.setX0(viewProj.getX0());
      break;
    case neutube::EAxis::X:
      newViewProj.setY0(viewProj.getY0());
      break;
    default:
      break;
    }

    mvc->getView()->setZ(mappedCrossCenter.getSliceCoord(slaveAxis));
    mvc->getView()->setViewProj(newViewProj);
    mvc->getView()->updateImageScreen(ZStackView::UPDATE_QUEUED);

#ifdef _DEBUG_2
    getMasterMvc()->getView()->printViewParam();
    mvc->getView()->printViewParam();
#endif
  }
}

