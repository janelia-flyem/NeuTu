#include "zimageslicefactory.h"

#include "common/math.h"
#include "geometry/zaffinerect.h"
#include "zslicecanvas.h"
#include "utilities.h"

ZImageSliceFactory::ZImageSliceFactory()
{
}


ZSliceCanvas* ZImageSliceFactory::Make(
      const ZStack &stack, const ZModelViewTransform &transform,
      double width, double height, ZSliceCanvas *result)
{
  if (result == nullptr) {
    result = new ZSliceCanvas;
  }

  ZAffineRect rect;
  rect.setPlane(transform.getCutPlane());
  rect.setSize(width, height);

  QImage image = neutu::vis2d::GetSlice(stack, rect);
  ZSliceCanvas canvas;
  ZSliceViewTransform t;
  t.setCutPlane(transform.getSliceAxis(), rect.getCenter());
//  t.setAnchor((rect.getWidth() - 1) / 2.0, (rect.getHeight() - 1) / 2.0);
  t.setAnchor(neutu::ifloor((rect.getWidth()) / 2.0),
              neutu::ifloor((rect.getHeight()) / 2.0));

  result->setTransform(t);
  result->fromImage(image);

  return result;
}
