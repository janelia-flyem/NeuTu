#include "utilities.h"

#include "common/math.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zviewproj.h"
#include "widgets/zimagewidget.h"
//#include "zpositionmapper.h"
#include "zstackdocutil.h"
#include "zstack.hxx"
#include "geometry/zgeometry.h"

QString neutu::mvc::ComposeStackDataInfo(
    ZStackDoc *doc, const ZIntPoint &pos, neutu::mvc::ViewInfoFlags f)
{
  QString info;

  int x = pos.getX();
  int y = pos.getY();
  int z = pos.getZ();

  ZStack *stack = doc->getStack();

  if (HasFlag(f, neutu::mvc::ViewInfoFlag::DATA_COORD)) {
    info = QString("(%1, %2, %3)").arg(x).arg(y).arg(z);
  }

  if (!info.isEmpty()) {
    info += QString(": ");
  }

  if (stack != NULL) {
    if (HasFlag(f, neutu::mvc::ViewInfoFlag::IMAGE_VALUE)) {
      if (!stack->isVirtual()) {
        if (stack->channelNumber() == 1) {
          info += QString("%4").arg(stack->getIntValue(x, y, z));
        } else {
          info += QString("(");
          for (int i=0; i<stack->channelNumber(); i++) {
            if (i==0) {
              info += QString("%1").arg(stack->getIntValue(x, y, z, i));
            } else {
              info += QString(", %1").arg(stack->getIntValue(x, y, z, i));
            }
          }
          info += QString(")");
        }
      }
    }
  }

  if (HasFlag(f, neutu::mvc::ViewInfoFlag::MASK_VALUE)) {
    ZStack *mask = doc->stackMask();
    if (mask != NULL) {
      if (!info.isEmpty()) {
        info += " | ";
      }
      info += "Mask: ";
      if (mask->channelNumber() == 1) {
        info += QString("%4").arg(mask->getIntValue(x, y, z));
      } else {
        info += QString("(");
        for (int i=0; i<mask->channelNumber(); i++) {
          if (i==0) {
            info += QString("%1").arg(mask->getIntValue(x, y, z, i));
          } else {
            info += QString(", %1").arg(mask->getIntValue(x, y, z, i));
          }
        }
        info += QString(")");
      }
    }
  }

  return info;
}

QString neutu::mvc::ComposeViewInfo(ZStackView *view, const ZPoint &dataPos)
{
  QString info;

  /*
  int z = view->sliceIndex();
  if (view->buddyPresenter()->interactiveContext().isProjectView()) {
    z = -1;
  }
  */

  //    QPointF pos = imageWidget()->canvasCoordinate(widgetPos);
//  ZPoint pos(dataPos.x(), dataPos.y(), 0);

//  ZViewProj vp = view->getViewProj();
  ZSliceViewTransform t = view->getSliceViewTransform();
  ZStackDoc *doc = view->buddyDocument().get();
  if (view->viewingInfo(neutu::mvc::ViewInfoFlag::WINDOW_SCALE)) {
    if (t.getScale() > 0.00001) {
      if (doc->getResolution().getUnit() ==
          ZResolution::EUnit::UNIT_NANOMETER) {
        double s = neutu::iround(
              view->imageWidget()->screenSize().width() / t.getScale() *
              doc->getResolution().voxelSizeX());
        QString unit = "nm";
        if (s > 1000.0) {
          s /= 1000.0;
          unit = "um";
        }
        if (unit == "nm" || s > 10.0) {
          info += QString(" Screen Width: ~%1").arg(neutu::iround(s)) + unit;
        } else {
          info += QString(" Screen Width: ~") + QString::number(s, 'g', 2) + unit;
        }
      } else {
        info += QString(" Screen Width: ~%1 pixels").arg(
              neutu::iround(view->imageWidget()->screenSize().width() / t.getScale()));
      }
      info += QString(" x%1").arg(t.getScale(), 0, 'g', 3);
      info += "  ";
    }
  }

//  ZPoint pt = t.inverseTransform(pos);
  info += ComposeStackDataInfo(
        doc, dataPos.toIntPoint(), view->getViewInfoFlag());


  return info;
}

/*
ZPoint neutu::mvc::MapWidgetPosToData(
    const ZStackView *view, const ZPoint &widgetPos)
{
  neutu::EAxis axis = view->getSliceAxis();
  ZViewProj vp = view->getViewProj();

  ZStackDoc *doc = view->buddyDocument().get();
  ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(*doc, axis);
  int z0 = box.getMinCorner().getZ();
  ZPoint stackPos = ZPositionMapper::WidgetToStack(widgetPos, vp, z0);

  ZPoint dataPos;
  if (axis == neutu::EAxis::ARB) {
    dataPos = ZPositionMapper::StackToData(stackPos, view->getAffinePlane());
  } else {
    dataPos = ZPositionMapper::StackToData(stackPos, axis);
  }

  return dataPos;
}
*/
