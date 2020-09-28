#include "zflyemorthoviewhelper.h"
#include "zcrosshair.h"
#include "zflyemorthomvc.h"
#include "zflyemorthodoc.h"
#include "mvc/zstackview.h"

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
    return getMasterMvc()->getMainView();
  }

  return NULL;
}

neutu::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(
    neutu::EAxis a1, neutu::EAxis a2)
{
  if (a1 > a2) {
    return getAlignAxis(a2, a1);
  }

  switch (a1) {
  case neutu::EAxis::X:
    if (a2 == neutu::EAxis::Y) {
      return neutu::EAxis::Z;
    } else if (a2 == neutu::EAxis::Z) {
      return neutu::EAxis::X;
    }
    break;
  case neutu::EAxis::Y:
    if (a2 == neutu::EAxis::Z) {
      return neutu::EAxis::Y;
    }
    break;
  default:
    break;
  }

  return neutu::EAxis::X;
}

neutu::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(const ZFlyEmOrthoMvc *mvc)
{
  return getAlignAxis(mvc->getMainView()->getSliceAxis(),
                      getMasterMvc()->getMainView()->getSliceAxis());
}

ZPoint ZFlyEmOrthoViewHelper::getCrossCenter() const
{
  ZPoint center;
  if (getMasterMvc() != NULL) {
    center = getMasterMvc()->getMainView()->getCutCenter();
    /*
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
    */
  }

  return center;
}

void ZFlyEmOrthoViewHelper::syncCrossHair(ZFlyEmOrthoMvc *mvc)
{
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_
    std::cout << "Sync crosshair from " << neutu::EnumValue(getMasterView()->getSliceAxis())
              << " to " << neutu::EnumValue(mvc->getMainView()->getSliceAxis())
              << std::endl;
#endif
//    ZCrossHair *crossHair = mvc->getCompleteDocument()->getCrossHair();
//    ZPoint crossCenter = getMasterDoc()->getCrossHair()->getCenter();
    ZPoint mappedCrossCenter = getCrossCenter();
//    mappedCrossCenter.shiftSliceAxisInverse(getMasterMvc);
    mvc->getMainView()->setDepth(
          mappedCrossCenter.getValue(mvc->getMainView()->getSliceAxis()));
    mvc->getMainView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);

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

void ZFlyEmOrthoViewHelper::syncViewPort(ZFlyEmOrthoMvc */*mvc*/)
{
#if 0
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_2
    std::cout << "Sync viewport from " << getMasterView()->getSliceAxis()
              << " to " << mvc->getView()->getSliceAxis() << std::endl;
#endif

    ZViewProj viewProj = getMasterView()->getViewProj();
    ZViewProj newViewProj = mvc->getView()->getViewProj();
    ZPoint mappedCrossCenter = getCrossCenter();
    neutu::EAxis slaveAxis = mvc->getView()->getSliceAxis();

    if (slaveAxis == neutu::EAxis::ARB) {
      return;
    }

    ZCrossHair *refCross = mvc->getCompleteDocument()->getCrossHair();

    ZPoint refCenter = refCross->getCenter();
    refCenter.shiftSliceAxis(slaveAxis);

    QPointF refCrossPos = QPointF(refCenter.getX(), refCenter.getY());

    switch (slaveAxis) {
    case neutu::EAxis::Z:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getY()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getZ());
      break;
    case neutu::EAxis::X:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getZ(), mappedCrossCenter.getY()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getX());
      break;
    case neutu::EAxis::Y:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getZ()),
            refCrossPos);
//      mvc->getView()->setZ(mappedCrossCenter.getY());
      break;
    case neutu::EAxis::ARB:
      break;
    }

    //To avoid misalignment caused by rounding error
    neutu::EAxis axis = getAlignAxis(mvc);
    switch (axis) {
    case neutu::EAxis::Y:
      newViewProj.setX0(viewProj.getX0());
      break;
    case neutu::EAxis::X:
      newViewProj.setY0(viewProj.getY0());
      break;
    default:
      break;
    }

    mvc->getView()->setViewProj(newViewProj);
    mvc->getView()->setZ(mappedCrossCenter.getSliceCoord(slaveAxis));
    mvc->getView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);

#ifdef _DEBUG_2
    getMasterMvc()->getView()->printViewParam();
    mvc->getView()->printViewParam();
#endif
  }
#endif

}

