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

NeuTube::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(
    NeuTube::EAxis a1, NeuTube::EAxis a2)
{
  if (a1 > a2) {
    return getAlignAxis(a2, a1);
  }

  switch (a1) {
  case NeuTube::X_AXIS:
    if (a2 == NeuTube::Y_AXIS) {
      return NeuTube::Z_AXIS;
    } else if (a2 == NeuTube::Z_AXIS) {
      return NeuTube::Y_AXIS;
    }
    break;
  case NeuTube::Y_AXIS:
    if (a2 == NeuTube::Z_AXIS) {
      return NeuTube::X_AXIS;
    }
    break;
  default:
    break;
  }

  return NeuTube::X_AXIS;
}

NeuTube::EAxis ZFlyEmOrthoViewHelper::getAlignAxis(const ZFlyEmOrthoMvc *mvc)
{
  return getAlignAxis(mvc->getView()->getSliceAxis(),
                      getMasterMvc()->getView()->getSliceAxis());
}

ZPoint ZFlyEmOrthoViewHelper::getCrossCenter() const
{
  ZPoint center;
  if (getMasterMvc() != NULL) {
    center = getMasterDoc()->getCrossHair()->getCenter();
#ifdef _DEBUG_
    std::cout << "Cross hair center: " << center.toString() << std::endl;
#endif
    ZViewProj viewProj = getMasterView()->getViewProj();
    QPointF pt = viewProj.mapPointBackF(QPointF(center.getX(), center.getY()));
    center.setX(pt.x());
    center.setY(pt.y());
    center.setZ(getMasterView()->getCurrentZ());
  }

  return center;
}

void ZFlyEmOrthoViewHelper::syncCrossHair(ZFlyEmOrthoMvc *mvc)
{
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_
    std::cout << "Sync crosshair from " << getMasterView()->getSliceAxis()
              << " to " << mvc->getView()->getSliceAxis() << std::endl;
#endif
    ZCrossHair *crossHair = mvc->getCompleteDocument()->getCrossHair();
    ZPoint crossCenter = getMasterDoc()->getCrossHair()->getCenter();
    ZPoint mappedCrossCenter = getCrossCenter();


    NeuTube::EAxis axis = getAlignAxis(mvc);
    switch (axis) {
    case NeuTube::X_AXIS:
      crossHair->setX(crossCenter.getX());
      mvc->getView()->setZ(mappedCrossCenter.getY());
      break;
    case NeuTube::Y_AXIS:
      crossHair->setY(crossCenter.getY());
      mvc->getView()->setZ(mappedCrossCenter.getX());
      break;
    case NeuTube::Z_AXIS:
      if (mvc->getView()->getSliceAxis() == NeuTube::X_AXIS) {
        crossHair->setX(crossCenter.getY());
        mvc->getView()->setZ(mappedCrossCenter.getX());
      } else {
        crossHair->setY(crossCenter.getX());
        mvc->getView()->setZ(mappedCrossCenter.getY());
      }
      break;
    }

#ifdef _DEBUG_
    getMasterMvc()->getView()->printViewParam();
    mvc->getView()->printViewParam();
#endif
  }
}

void ZFlyEmOrthoViewHelper::syncViewPort(ZFlyEmOrthoMvc *mvc)
{
  if (getMasterMvc() != NULL) {
#ifdef _DEBUG_
    std::cout << "Sync viewport from " << getMasterView()->getSliceAxis()
              << " to " << mvc->getView()->getSliceAxis() << std::endl;
#endif

    ZViewProj viewProj = getMasterView()->getViewProj();
    ZViewProj newViewProj = mvc->getView()->getViewProj();
    ZPoint mappedCrossCenter = getCrossCenter();

    ZCrossHair *refCross = mvc->getCompleteDocument()->getCrossHair();
    QPointF refCrossPos = QPointF(refCross->getCenter().getX(),
                                  refCross->getCenter().getY());

    NeuTube::EAxis axis = getAlignAxis(mvc);
    switch (axis) {
    case NeuTube::X_AXIS:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getZ()),
            refCrossPos);
      newViewProj.setX0(viewProj.getX0());
      mvc->getView()->setZ(mappedCrossCenter.getY());
      break;
    case NeuTube::Y_AXIS:
      newViewProj.setZoomWithFixedPoint(
            viewProj.getZoom(),
            QPoint(mappedCrossCenter.getZ(), mappedCrossCenter.getY()),
            refCrossPos);
      newViewProj.setY0(viewProj.getY0());
      mvc->getView()->setZ(mappedCrossCenter.getX());
      break;
    case NeuTube::Z_AXIS:
      mappedCrossCenter.shiftSliceAxisInverse(getMasterView()->getSliceAxis());
      if (mvc->getView()->getSliceAxis() == NeuTube::X_AXIS) {
        newViewProj.setZoomWithFixedPoint(
              viewProj.getZoom(),
              QPoint(mappedCrossCenter.getZ(), mappedCrossCenter.getY()),
              refCrossPos);
//        newViewProj.setY0(viewProj.getX0());
        mvc->getView()->setZ(mappedCrossCenter.getX());
      } else {
        newViewProj.setZoomWithFixedPoint(
              viewProj.getZoom(),
              QPoint(mappedCrossCenter.getX(), mappedCrossCenter.getZ()),
              refCrossPos);
//        newViewProj.setX0(viewProj.getY0());
        mvc->getView()->setZ(mappedCrossCenter.getY());
      }
      break;
    }
    mvc->getView()->setViewProj(newViewProj);
    mvc->getView()->updateImageScreen(ZStackView::UPDATE_QUEUED);

#ifdef _DEBUG_
    getMasterMvc()->getView()->printViewParam();
    mvc->getView()->printViewParam();
#endif
  }
}

