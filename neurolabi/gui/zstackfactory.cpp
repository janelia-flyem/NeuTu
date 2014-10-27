#include "zstackfactory.h"

#if _QT_GUI_USED_
#include <QPixmap>
#include <QPainter>
#endif

#include "neutubeconfig.h"
#if _QT_GUI_USED_
#include "zstroke2d.h"
#endif
#include "zpointarray.h"
#include "tz_stack.h"
#include "zweightedpointarray.h"
#include "math.h"

ZStackFactory::ZStackFactory()
{
}

ZStackFactory::~ZStackFactory()
{

}

Stack *ZStackFactory::pileMatched(const std::vector<Stack*> stackArray)
{
  if (stackArray.empty()) {
    return NULL;
  }

  int kind = C_Stack::kind(stackArray[0]);
  int width = C_Stack::width(stackArray[0]);
  int height = C_Stack::height(stackArray[0]);
  int depth = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    depth += C_Stack::depth(*iter);
  }

  Stack *out = C_Stack::make(kind, width, height, depth);

  int currentPlane = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    const Stack* stack = *iter;
    Stack substack = C_Stack::sliceView(
          out, currentPlane, currentPlane + C_Stack::depth(stack) - 1);
    C_Stack::copyValue(stack, &substack);
    currentPlane += C_Stack::depth(stack);
  }

  return out;
}

ZStack* ZStackFactory::makeStack(ZStack *stack) const
{
  return stack;
}

ZStack* ZStackFactory::makeVirtualStack(int width, int height, int depth)
{
  if (width <= 0 || height <= 0 || depth <= 0) {
    return NULL;
  }

  return new ZStack(GREY, width, height, depth, 1, true);
}

ZStack* ZStackFactory::makeVirtualStack(const ZIntCuboid &box)
{
  ZStack *stack =
      makeVirtualStack(box.getWidth(), box.getHeight(), box.getDepth());
  if (stack != NULL) {
    stack->setOffset(box.getFirstCorner());
  }

  return stack;
}

ZStack* ZStackFactory::makeOneStack(
    int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, nchannel);
  stack->setOne();

  return stack;
}

ZStack* ZStackFactory::makeZeroStack(
    int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, nchannel);
  stack->setZero();

  return stack;
}

ZStack* ZStackFactory::makeZeroStack(
    int kind, int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(kind, width, height, depth, nchannel);
  stack->setZero();

  return stack;
}

ZStack* ZStackFactory::makeZeroStack(const ZIntCuboid box, int nchannel)
{
  ZStack *stack = makeZeroStack(
        box.getWidth(), box.getHeight(), box.getDepth(), nchannel);
  stack->setOffset(box.getFirstCorner());

  return stack;
}

ZStack* ZStackFactory::makeZeroStack(
    int kind, const ZIntCuboid box, int nchannel)
{
  ZStack *stack = makeZeroStack(
        kind, box.getWidth(), box.getHeight(), box.getDepth(), nchannel);
  stack->setOffset(box.getFirstCorner());

  return stack;
}

ZStack* ZStackFactory::makeIndexStack(int width, int height, int depth)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  size_t voxelNumber = stack->getVoxelNumber();
  uint8_t *array = stack->array8();
  for (size_t i = 0; i < voxelNumber; ++i) {
    array[i] = i;
  }

  return stack;
}

ZStack* ZStackFactory::makeUniformStack(int width, int height, int depth, int v)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  size_t voxelNumber = stack->getVoxelNumber();
  uint8_t *array = stack->array8();
  for (size_t i = 0; i < voxelNumber; ++i) {
    array[i] = v;
  }

  return stack;
}

ZStack* ZStackFactory::makePolygonPicture(const ZStroke2d &curve)
{
#ifdef _QT_GUI_USED_
  ZStack *stack = NULL;
  ZCuboid box = curve.getBoundBox();
  //box.expand(2.0);

  ZIntCuboid stackBox(iround(box.firstCorner().x()),
                      iround(box.firstCorner().y()),
                      iround(box.firstCorner().z()),
                      iround(box.lastCorner().x()),
                      iround(box.lastCorner().y()),
                      iround(box.lastCorner().z()));

  QPixmap *pix = new QPixmap(stackBox.getWidth(), stackBox.getHeight());
  pix->fill(Qt::black);
  QPainter *painter = new QPainter(pix);
  QTransform transform;
  transform.translate(-stackBox.getFirstCorner().getX(),
                      -stackBox.getFirstCorner().getY());
  painter->setTransform(transform);
  painter->setPen(Qt::white);
  painter->setBrush(Qt::white);
  QPolygonF polygon;
  for (size_t i = 0; i < curve.getPointNumber(); ++i) {
    double x = 0;
    double y = 0;
    curve.getPoint(&x, &y, i);
    polygon.append(QPointF(x, y));
  }
  painter->drawPolygon(polygon);

  QImage image = pix->toImage();

#ifdef _DEBUG_2
  pix->save((GET_DATA_DIR + "/test.tif").c_str());
  //delete pix;
  delete painter;
#endif

  stack = new ZStack(GREY, pix->width(), pix->height(), 1, 1);
  size_t offset = 0;
  int height = stack->height();
  int width = stack->width();
  uint8_t *array = stack->array8();
#ifdef _DEBUG_2
  tic();
#endif
  for (int y = 0; y < height; ++y) {
    const uchar* scanLine = image.scanLine(y);
    for (int x = 0; x < width; ++x) {
      array[offset++] = scanLine[x * 4];
      //array[offset++] = qRed(image.pixel(x, y));
    }
  }
#ifdef _DEBUG_2
  ptoc();
#endif
  stack->setOffset(stackBox.getFirstCorner().getX(),
                   stackBox.getFirstCorner().getY(), curve.getZ());
  stack->printInfo();

  delete painter;
  delete pix;

  return stack;
#else

  return NULL;
#endif
}

ZStack* ZStackFactory::makeDensityMap(const ZPointArray &ptArray, double sigma)
{
  ZCuboid boundBox = ptArray.getBoundBox();
  double radius = sigma * 2;
  ZPoint pt1 = boundBox.firstCorner();
  pt1 -= radius;

  ZPoint pt2 = boundBox.lastCorner();
  pt2 += radius;


  ZIntCuboid stackBox(pt1.toIntPoint(), pt2.toIntPoint());
  ZStack *stack = NULL;

  if (ptArray.size() < 1000) {
    stack = makeZeroStack(FLOAT64, stackBox);
    ZPointArray tmpPtArray = ptArray;
    std::sort(tmpPtArray.begin(), tmpPtArray.end(), ZPoint::ZCompare());

    size_t offset = 0;
    double inv = 0.5 / sigma / sigma;

    double *array = stack->array64();

    int x0 = stackBox.getFirstCorner().getX();
    int y0 = stackBox.getFirstCorner().getY();
    int z0 = stackBox.getFirstCorner().getZ();

    int x1 = stackBox.getLastCorner().getX();
    int y1 = stackBox.getLastCorner().getY();
    int z1 = stackBox.getLastCorner().getZ();


    for (int z = z0; z <= z1; ++z) {
      ZPointArray::const_iterator beginIter =
          std::lower_bound(tmpPtArray.begin(), tmpPtArray.end(),
                           ZPoint(0, 0, z - radius), ZPoint::ZCompare());
      ZPointArray::const_iterator endIter =
          std::upper_bound(tmpPtArray.begin(), tmpPtArray.end(),
                           ZPoint(0, 0, z + radius), ZPoint::ZCompare());
#ifdef _DEBUG_
      std::cout << z << "/" << z1 << std::endl;
#endif

      ZPointArray subArray;
      for (ZPointArray::const_iterator iter = beginIter;
           iter != endIter; ++iter) {
        subArray.append(*iter);
      }
      sort(subArray.begin(), subArray.end(), ZPoint::YCompare());

      for (int y = y0; y <= y1; ++y) {
        beginIter =
            std::lower_bound(subArray.begin(), subArray.end(),
                             ZPoint(0, y - radius, 0), ZPoint::YCompare());
        endIter =
            std::upper_bound(subArray.begin(), subArray.end(),
                             ZPoint(0, y + radius, 0), ZPoint::YCompare());

        for (int x = x0; x <= x1; ++x) {
          for (ZPointArray::const_iterator iter = beginIter;
               iter != endIter; ++iter) {
            const ZPoint &pt = *iter;
            double dx = x - pt.x();
            if (fabs(dx) < radius) {
              double dy = y - pt.y();
              double dz = z - pt.z();
              double v = exp(-(dx * dx  + dy * dy  + dz * dz) * inv);
              array[offset] += v;
            }
          }
          ++offset;
        }
      }
    }
  } else {
    stack = makeZeroStack(GREY, stackBox);
    Filter_3d *filter = Gaussian_Filter_3d(sigma, sigma, sigma);

    for (ZPointArray::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZPoint &pt = *iter;
      stack->setIntValue(iround(pt.x()), iround(pt.y()), iround(pt.z()), 0, 1);
    }

    Stack *stack2 = Filter_Stack(stack->c_stack(), filter);

    Kill_FMatrix(filter);

    ZStack *out = new ZStack;
    out->consume(stack2);
    out->setOffset(stack->getOffset());

    delete stack;
    stack = out;
  }

  return stack;
}

ZStack* ZStackFactory::makeDensityMap(
    const ZWeightedPointArray &ptArray, double sigma)
{
  ZCuboid boundBox = ptArray.getBoundBox();
  double radius = sigma * 2;
  ZPoint pt1 = boundBox.firstCorner();
  pt1 -= radius;

  ZPoint pt2 = boundBox.lastCorner();
  pt2 += radius;


  ZIntCuboid stackBox(pt1.toIntPoint(), pt2.toIntPoint());
  ZStack *stack = NULL;

  if (ptArray.size() < 1000) {
    stack = makeZeroStack(FLOAT64, stackBox);
    ZWeightedPointArray tmpPtArray = ptArray;
    std::sort(tmpPtArray.begin(), tmpPtArray.end(), ZPoint::ZCompare());

    size_t offset = 0;
    double inv = 0.5 / sigma / sigma;

    double *array = stack->array64();

    int x0 = stackBox.getFirstCorner().getX();
    int y0 = stackBox.getFirstCorner().getY();
    int z0 = stackBox.getFirstCorner().getZ();

    int x1 = stackBox.getLastCorner().getX();
    int y1 = stackBox.getLastCorner().getY();
    int z1 = stackBox.getLastCorner().getZ();


    for (int z = z0; z <= z1; ++z) {
      ZWeightedPointArray::const_iterator beginIter =
          std::lower_bound(tmpPtArray.begin(), tmpPtArray.end(),
                           ZPoint(0, 0, z - radius), ZPoint::ZCompare());
      ZWeightedPointArray::const_iterator endIter =
          std::upper_bound(tmpPtArray.begin(), tmpPtArray.end(),
                           ZPoint(0, 0, z + radius), ZPoint::ZCompare());
#ifdef _DEBUG_
      std::cout << z << "/" << z1 << std::endl;
#endif

      ZWeightedPointArray subArray;
      for (ZWeightedPointArray::const_iterator iter = beginIter;
           iter != endIter; ++iter) {
        subArray.append(*iter);
      }
      sort(subArray.begin(), subArray.end(), ZPoint::YCompare());

      for (int y = y0; y <= y1; ++y) {
        beginIter =
            std::lower_bound(subArray.begin(), subArray.end(),
                             ZPoint(0, y - radius, 0), ZPoint::YCompare());
        endIter =
            std::upper_bound(subArray.begin(), subArray.end(),
                             ZPoint(0, y + radius, 0), ZPoint::YCompare());

        for (int x = x0; x <= x1; ++x) {
          for (ZWeightedPointArray::const_iterator iter = beginIter;
               iter != endIter; ++iter) {
            const ZWeightedPoint &pt = *iter;
            double dx = x - pt.x();
            if (fabs(dx) < radius) {
              double dy = y - pt.y();
              double dz = z - pt.z();
              double v = exp(-(dx * dx  + dy * dy  + dz * dz) * inv);
              array[offset] += v * pt.weight();
            }
          }
          ++offset;
        }
      }
    }
  } else {
    stack = makeZeroStack(GREY, stackBox);
    Filter_3d *filter = Gaussian_Filter_3d(sigma, sigma, sigma);

    for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZWeightedPoint &pt = *iter;
      stack->setIntValue(iround(pt.x()), iround(pt.y()), iround(pt.z()), 0,
                         iround(pt.weight()));
    }

    Stack *stack2 = Filter_Stack(stack->c_stack(), filter);

    Kill_FMatrix(filter);

    ZStack *out = new ZStack;
    out->consume(stack2);
    out->setOffset(stack->getOffset());

    delete stack;
    stack = out;
  }

  return stack;
}

ZStack* ZStackFactory::makeAlphaBlend(const ZStack &/*stack1*/, const ZStack &/*stack2*/,
                              double /*alpha*/)
{
  return NULL;
}

ZStack* ZStackFactory::makeSeedStack(const ZWeightedPointArray &ptArray)
{
  ZCuboid boundBox = ptArray.getBoundBox();
  ZPoint pt1 = boundBox.firstCorner();
  ZPoint pt2 = boundBox.lastCorner();

  ZIntCuboid stackBox(pt1.toIntPoint(), pt2.toIntPoint());
  ZStack *stack = makeZeroStack(GREY, stackBox);

  for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const ZWeightedPoint &pt = *iter;
    int x = iround(pt.x());
    int y = iround(pt.y());
    int z = iround(pt.z());
    int v = iround(pt.weight());

    stack->setIntValue(x, y, z, 0, v);
    stack->setIntValue(x - 1, y, z, 0, v);
    stack->setIntValue(x + 1, y, z, 0, v);
    stack->setIntValue(x, y - 1, z, 0, v);
    stack->setIntValue(x, y + 1, z, 0, v);
    stack->setIntValue(x, y, z - 1, 0, v);
    stack->setIntValue(x, y, z + 1, 0, v);
  }

  return stack;
}
