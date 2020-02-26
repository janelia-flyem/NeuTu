#include "utilities.h"

#include "common/math.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zviewproj.h"
#include "widgets/zimagewidget.h"
#include "zpositionmapper.h"
#include "zstackdocutil.h"
#include "zstack.hxx"
#include "geometry/zgeometry.h"

QString neutu::mvc::ComposeStackDataInfo(
    ZStackDoc *doc, double cx, double cy, int z, neutu::EAxis axis,
    neutu::mvc::ViewInfoFlags f)
{
  QString info;

  int x = std::floor(cx);
  int y = std::floor(cy);

  int wx = x;
  int wy = y;
  int wz = z;

  zgeom::shiftSliceAxisInverse(wx, wy, wz, axis);

  ZStack *stack = doc->getStack();

  if (x >= 0 && y >= 0) {
    if (HasFlag(f, neutu::mvc::ViewInfoFlag::RAW_STACK_COORD)) {
      std::ostringstream stream;

      stream << "(";
      if (x >= 0) {
        stream << x << ", ";
      }
      if (y >= 0) {
        stream << y;
      }
      if (z >= 0) {
        stream << " , " << z;
      }

      stream << ")";
      info = stream.str().c_str();
    }

    if (z < 0) {
      info += " (MIP)";
    }

    if (!info.isEmpty()) {
      info += QString(": ");
    }

    if (stack != NULL) {
      if (HasFlag(f, neutu::mvc::ViewInfoFlag::IMAGE_VALUE)) {
        if (!stack->isVirtual()) {
          if (stack->channelNumber() == 1) {
            info += QString("%4").arg(stack->value(wx, wy, wz));
          } else {
            info += QString("(");
            for (int i=0; i<stack->channelNumber(); i++) {
              if (i==0) {
                info += QString("%1").arg(stack->value(wx, wy, wz, i));
              } else {
                info += QString(", %1").arg(stack->value(wx, wy, wz, i));
              }
            }
            info += QString(")");
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
            info += QString("%4").arg(mask->value(wx, wy, wz));
          } else {
            info += QString("(");
            for (int i=0; i<mask->channelNumber(); i++) {
              if (i==0) {
                info += QString("%1").arg(mask->value(wx, wy, wz, i));
              } else {
                info += QString(", %1").arg(mask->value(wx, wy, wz, i));
              }
            }
            info += QString(")");
          }
        }
      }

      if (HasFlag(f, neutu::mvc::ViewInfoFlag::DATA_COORD)) {
        if (stack->hasOffset()) {
          ZIntPoint stackOffset = doc->getStackOffset();
          if (!info.isEmpty()) {
            info += "; ";
          }

          info += QString("(%1, %2, %3)").
              arg(stackOffset.getX() + wx).arg(stackOffset.getY() + wy).
              arg(stackOffset.getZ() + wz);
        }
      }
    }
  }

  return info;
}

QString neutu::mvc::ComposeViewInfo(ZStackView *view, const QPoint &widgetPos)
{
  QString info;

  int z = view->sliceIndex();
  if (view->buddyPresenter()->interactiveContext().isProjectView()) {
    z = -1;
  }

  //    QPointF pos = imageWidget()->canvasCoordinate(widgetPos);
  ZPoint pos(widgetPos.x(), widgetPos.y(), z);

  ZViewProj vp = view->getViewProj();
  ZStackDoc *doc = view->buddyDocument().get();
  if (view->viewingInfo(neutu::mvc::ViewInfoFlag::WINDOW_SCALE)) {
    if (vp.getZoom() > 0.00001) {
      if (doc->getResolution().getUnit() ==
          ZResolution::EUnit::UNIT_NANOMETER) {
        double s = neutu::iround(
              view->imageWidget()->screenSize().width() / vp.getZoom() *
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
              neutu::iround(view->imageWidget()->screenSize().width() / vp.getZoom()));
      }
      info += "  ";
    }
  }

  neutu::EAxis axis = view->getSliceAxis();
  if (doc->hasStackData()) {
    ZPoint pt = ZPositionMapper::WidgetToRawStack(pos, vp);
    info += ComposeStackDataInfo(
          doc, pt.x(), pt.y(), pt.z(), axis, view->getViewInfoFlag());
  } else {
    ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(*doc, axis);

    QPointF stackPos = ZPositionMapper::WidgetToStack(
          widgetPos.x(), widgetPos.y(), vp);
    ZPoint dataPos;
    if (axis == neutu::EAxis::ARB) {
      if (view->viewingInfo(neutu::mvc::ViewInfoFlag::DATA_COORD)) {
        dataPos = ZPositionMapper::StackToData(
              ZPoint(stackPos.x(), stackPos.y(),
                     view->getZ(neutu::ECoordinateSystem::STACK)),
              view->getViewCenter().toPoint(), view->getAffinePlane());
        info += QString("(%1, %2, %3)").
            arg(neutu::iround(dataPos.getX())).
            arg(neutu::iround(dataPos.getY())).
            arg(neutu::iround(dataPos.getZ()));
      }
    } else {
      dataPos = ZPositionMapper::StackToData(
            ZPositionMapper::WidgetToStack(
              pos, vp, box.getFirstCorner().getZ()), axis);
      if (view->viewingInfo(neutu::mvc::ViewInfoFlag::RAW_STACK_COORD)) {
        info += QString("(%1, %2, %3)").arg(pos.x()).arg(pos.y()).arg(z);
      }
      if (view->viewingInfo(neutu::mvc::ViewInfoFlag::DATA_COORD)) {
        if (!info.isEmpty()) {
          info += "; ";
        }
        info += QString("(%1, %2, %3)").
            arg(iround(dataPos.getX())).arg(iround(dataPos.getY())).
            arg(iround(dataPos.getZ()));
      }
    }
  }

  return info;
}

ZPoint neutu::mvc::MapWidgetPosToData(
    const ZStackView *view, const ZPoint &widgetPos)
{
  neutu::EAxis axis = view->getSliceAxis();
  ZViewProj vp = view->getViewProj();

  ZStackDoc *doc = view->buddyDocument().get();
  ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(*doc, axis);
  int z0 = box.getFirstCorner().getZ();
  ZPoint stackPos = ZPositionMapper::WidgetToStack(widgetPos, vp, z0);

  ZPoint dataPos;
  if (axis == neutu::EAxis::ARB) {
    dataPos = ZPositionMapper::StackToData(stackPos, view->getAffinePlane());
  } else {
    dataPos = ZPositionMapper::StackToData(stackPos, axis);
  }

  return dataPos;
}
