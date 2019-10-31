#include "zstackfactory.h"

#if _QT_GUI_USED_
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include "zcolorscheme.h"
#include "zpixmap.h"
#include "zpainter.h"
#endif

#include <algorithm>
#include "neutubeconfig.h"
#if _QT_GUI_USED_
#include "zstroke2d.h"
#endif
#include "geometry/zpointarray.h"
#include "tz_stack.h"
#include "zweightedpointarray.h"
#include "math.h"
#include "tz_color.h"
#include "zobject3dscanarray.h"
#include "tz_stack_bwmorph.h"
#include "c_stack.h"
#include "geometry/zintcuboid.h"
#include "zswctree.h"
#include "zstackarray.h"
#include "zarray.h"
#include "misc/miscutility.h"

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

ZStack* ZStackFactory::MakeVirtualStack(int width, int height, int depth)
{
  if (width <= 0 || height <= 0 || depth <= 0) {
    return NULL;
  }

  return new ZStack(GREY, width, height, depth, 1, true);
}

ZStack* ZStackFactory::MakeVirtualStack(const ZIntCuboid &box)
{
  ZStack *stack =
      MakeVirtualStack(box.getWidth(), box.getHeight(), box.getDepth());
  if (stack != NULL) {
    stack->setOffset(box.getFirstCorner());
  }

  return stack;
}

ZStack* ZStackFactory::MakeOneStack(
    int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, nchannel);
  stack->setOne();

  return stack;
}

ZStack* ZStackFactory::MakeZeroStack(
    int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, nchannel);
  stack->setZero();

  return stack;
}

ZStack* ZStackFactory::MakeZeroStack(
    int kind, int width, int height, int depth, int nchannel)
{
  ZStack *stack = new ZStack(kind, width, height, depth, nchannel);
  stack->setZero();

  return stack;
}

ZStack* ZStackFactory::makeSlice(const ZStack &stack, int z)
{
  ZStack *out = NULL;

  int slice = z - stack.getOffset().getZ();
  if (stack.channelNumber() == 1) {
    Stack sliceView = C_Stack::sliceView(stack.c_stack(), slice);
    out = new ZStack;
    out->consume(C_Stack::clone(&sliceView));
  } else if (stack.channelNumber() > 1) {
    out = MakeZeroStack(stack.kind(), stack.width(), stack.height(), 1,
                        stack.channelNumber());
    for (int c = 0; c < stack.channelNumber(); ++c) {
      Stack sliceView = C_Stack::sliceView(stack.data(), slice, c);
      C_Stack::copyValue(&sliceView, out->c_stack(c));
    }
  }

  if (out != NULL) {
    out->setOffset(stack.getOffset().getX(), stack.getOffset().getY(), z);
  }

  return out;
}

ZStack* ZStackFactory::MakeZeroStack(const ZIntCuboid box, int nchannel)
{
  ZStack *stack = MakeZeroStack(
        box.getWidth(), box.getHeight(), box.getDepth(), nchannel);
  stack->setOffset(box.getFirstCorner());

  return stack;
}

ZStack* ZStackFactory::MakeZeroStack(
    int kind, const ZIntCuboid box, int nchannel)
{
  ZStack *stack = MakeZeroStack(
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

ZStack* ZStackFactory::MakeUniformStack(int width, int height, int depth, int v)
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

  ZIntCuboid stackBox(neutu::iround(box.firstCorner().x()),
                      neutu::iround(box.firstCorner().y()),
                      neutu::iround(box.firstCorner().z()),
                      neutu::iround(box.lastCorner().x()),
                      neutu::iround(box.lastCorner().y()),
                      neutu::iround(box.lastCorner().z()));

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
  //delete painter;
#endif

  Stack *stackData = C_Stack::make(GREY, pix->width(), pix->height(), 1);
  size_t offset = 0;
  int height = C_Stack::height(stackData);
  int width = C_Stack::width(stackData);
  uint8_t *array = C_Stack::array8(stackData);
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

  stack = new ZStack;
  stack->consume(stackData);
  /*
  stack = new ZStack(GREY, pix->width(), pix->height(), 1, 1);

  Stack_Fill_2dhole(stackData, stack->c_stack(), 255, 1);

  C_Stack::kill(stackData);
  */

#ifdef _DEBUG_2
  ptoc();
#endif
  stack->setOffset(stackBox.getFirstCorner().getX(),
                   stackBox.getFirstCorner().getY(), curve.getZ());

#ifdef _DEBUG_
  stack->printInfo();
#endif

  delete painter;
  delete pix;

  return stack;
#else

  return NULL;
#endif
}

ZStack* ZStackFactory::MakeDensityMap(const ZPointArray &ptArray, double sigma)
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
    stack = MakeZeroStack(FLOAT64, stackBox);
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
    stack = MakeZeroStack(GREY, stackBox);
    Filter_3d *filter = Gaussian_Filter_3d(sigma, sigma, sigma);

    for (ZPointArray::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZPoint &pt = *iter;
      stack->addIntValue(
            neutu::iround(pt.x()),
            neutu::iround(pt.y()),
            neutu::iround(pt.z()), 0, 1);
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

ZStack* ZStackFactory::MakeDensityMap(
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
    stack = MakeZeroStack(FLOAT64, stackBox);
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
    stack = MakeZeroStack(GREY, stackBox);

    for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZWeightedPoint &pt = *iter;
      stack->addIntValue(
            neutu::iround(pt.x()),
            neutu::iround(pt.y()),
            neutu::iround(pt.z()), 0,
            neutu::iround(pt.weight()));
    }

    if (sigma > 0.0) {
      Filter_3d *filter = Gaussian_Filter_3d(sigma, sigma, sigma);
      Stack *stack2 = Filter_Stack(stack->c_stack(), filter);

      Kill_FMatrix(filter);

      ZStack *out = new ZStack;
      out->consume(stack2);
      out->setOffset(stack->getOffset());

      delete stack;
      stack = out;
    }
  }

  return stack;
}

ZStack* ZStackFactory::makeAlphaBlend(const ZStack &/*stack1*/, const ZStack &/*stack2*/,
                              double /*alpha*/)
{
  return NULL;
}

ZStack* ZStackFactory::MakeSeedStack(const ZWeightedPointArray &ptArray)
{
  ZCuboid boundBox = ptArray.getBoundBox();
  ZPoint pt1 = boundBox.firstCorner();
  ZPoint pt2 = boundBox.lastCorner();

  ZIntCuboid stackBox(pt1.toIntPoint(), pt2.toIntPoint());
  ZStack *stack = MakeZeroStack(GREY, stackBox);

  for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const ZWeightedPoint &pt = *iter;
    int x = neutu::iround(pt.x());
    int y = neutu::iround(pt.y());
    int z = neutu::iround(pt.z());
    int v = neutu::iround(pt.weight());

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

ZStack* ZStackFactory::MakeColorStack(const ZStack &stack, double h, double s)
{
  ZStack *colorStack = MakeZeroStack(COLOR, stack.getBoundBox(), 1);

  Rgb_Color color;

  size_t voxelNumber = stack.getVoxelNumber();

  const uint8_t *signalArray = stack.array8();
  color_t *colorArray = colorStack->arrayc();

  for (size_t i = 0; i < voxelNumber; ++i) {
    double v = signalArray[i];
    v /= 255.0;
    Set_Color_Hsv(&color, h, s, v);
    colorArray[i][0] = color.r;
    colorArray[i][1] = color.g;
    colorArray[i][2] = color.b;
  }

  return colorStack;
}

ZStack* ZStackFactory::MakeColorStack(
    const ZStack &stack, const ZStack &mask, double h, double s)
{
  ZIntCuboid boundBox = stack.getBoundBox();
  ZStack *colorStack = MakeZeroStack(COLOR, boundBox, 1);

  Rgb_Color color;

  //size_t voxelNumber = stack.getVoxelNumber();

  const uint8_t *signalArray = stack.array8();
  color_t *colorArray = colorStack->arrayc();

  size_t i = 0;

  int minX = boundBox.getFirstCorner().getX();
  int minY = boundBox.getFirstCorner().getY();
  int minZ = boundBox.getFirstCorner().getZ();
  int maxX = boundBox.getLastCorner().getX();
  int maxY = boundBox.getLastCorner().getY();
  int maxZ = boundBox.getLastCorner().getZ();


  for (int z = minZ; z <= maxZ; ++z) {
    for (int y = minY; y <= maxY; ++y) {
      for (int x = minX; x <= maxX; ++x) {
        if (mask.getIntValue(x, y, z) > 0) {
          double v = signalArray[i];
          v /= 255.0;
          Set_Color_Hsv(&color, h, s, v);
          colorArray[i][0] = color.r;
          colorArray[i][1] = color.g;
          colorArray[i][2] = color.b;
        } else {
          colorArray[i][0] = signalArray[i];
          colorArray[i][1] = signalArray[i];
          colorArray[i][2] = signalArray[i];
        }
        ++i;
      }
    }
  }

  return colorStack;
}

ZStack* ZStackFactory::MakeColorStack(const ZStack &stack, const ZStack &labelField)
{
  ZIntCuboid boundBox = stack.getBoundBox();
  ZStack *colorStack = MakeZeroStack(COLOR, boundBox, 1);

  Rgb_Color color;

  //size_t voxelNumber = stack.getVoxelNumber();

  const uint8_t *signalArray = stack.array8();
  color_t *colorArray = colorStack->arrayc();

  size_t i = 0;

  int minX = boundBox.getFirstCorner().getX();
  int minY = boundBox.getFirstCorner().getY();
  int minZ = boundBox.getFirstCorner().getZ();
  int maxX = boundBox.getLastCorner().getX();
  int maxY = boundBox.getLastCorner().getY();
  int maxZ = boundBox.getLastCorner().getZ();


  for (int z = minZ; z <= maxZ; ++z) {
    for (int y = minY; y <= maxY; ++y) {
      for (int x = minX; x <= maxX; ++x) {
        int label = labelField.getIntValue(x, y, z);
        if (label > 0) {
          double v = 0.0;
          double h = 0.0;
          double s = 0.0;

          Set_Color_Discrete(&color, label - 1);
          Rgb_Color_To_Hsv(&color, &h, &s, &v);

          v = signalArray[i];
          v /= 255.0;

          Set_Color_Hsv(&color, h, s, v);
          colorArray[i][0] = color.r;
          colorArray[i][1] = color.g;
          colorArray[i][2] = color.b;
        } else {
          colorArray[i][0] = signalArray[i];
          colorArray[i][1] = signalArray[i];
          colorArray[i][2] = signalArray[i];
        }
        ++i;
      }
    }
  }

  return colorStack;
}

//ZStack* ZStackFactory::makeSeedStack(const ZObject3dScanArray &objArray)
//{

//}

ZStack* ZStackFactory::MakeRgbStack(
      const ZStack &redStack, const ZStack &greenStack, const ZStack &blueStack)
{
  if (redStack.kind() != GREY || greenStack.kind() != GREY ||
      blueStack.kind() != GREY) {
    return NULL;
  }

  ZIntCuboid boundBox = redStack.getBoundBox();

  //Get bound box
  boundBox.join(greenStack.getBoundBox());
  boundBox.join(blueStack.getBoundBox());

  if (boundBox.isEmpty()) {
    return NULL;
  }

  ZStack *output = ZStackFactory::MakeZeroStack(COLOR, boundBox, 1);

  ZStack* channels[3];

  //Paste all stacks into the bound box
  channels[0] = ZStackFactory::MakeZeroStack(GREY, boundBox, 1);
  redStack.paste(channels[0]);

  channels[1] = ZStackFactory::MakeZeroStack(GREY, boundBox, 1);
  greenStack.paste(channels[1]);

  channels[2] = ZStackFactory::MakeZeroStack(GREY, boundBox, 1);
  blueStack.paste(channels[2]);

  size_t voxelNumber = output->getVoxelNumber();
  color_t *outputArray = output->arrayc();
  for (int c = 0; c < 3; ++c) {
    uint8_t *array = channels[c]->array8();
    for (size_t i = 0; i < voxelNumber; ++i) {
      outputArray[i][c] = array[i];
    }
    delete channels[c];
  }

  return output;
}

ZStack* ZStackFactory::CompositeForeground(const ZStackArray &stackArray)
{
  ZStack *stack = NULL;

  if (!stackArray.empty()) {
    const ZStackPtr &firstStack = stackArray.front();

    ZIntCuboid boundBox = stackArray.getBoundBox();
    stack = ZStackFactory::MakeZeroStack(
          firstStack->kind(), boundBox, firstStack->channelNumber());

    for (const ZStackPtr &substack : stackArray) {
      substack->paste(stack, 0);
    }
  }

  return stack;
}

ZStack* ZStackFactory::CompositeForeground(
    const ZStack &stack1, const ZStack &stack2)
{
  if (stack1.kind() != stack2.kind() ||
      stack1.channelNumber() != stack2.channelNumber()) {
    return NULL;
  }

  ZIntCuboid boundBox = stack1.getBoundBox();
  boundBox.join(stack2.getBoundBox());

  ZStack *stack = ZStackFactory::MakeZeroStack(
        stack1.kind(), boundBox, stack1.channelNumber());
  stack1.paste(stack, 0);
  stack2.paste(stack, 0);

  return stack;
}

ZStack* ZStackFactory::MakeBinaryStack(
    const ZObject3dScanArray &objArray, int v)
{
  ZStack *stack = NULL;
  if (!objArray.empty()) {
    ZIntCuboid boundBox = objArray.getBoundBox();
    stack = MakeZeroStack(GREY, boundBox);

    int offset[3];
    offset[0] = -stack->getOffset().getX();
    offset[1] = -stack->getOffset().getY();
    offset[2] = -stack->getOffset().getZ();

    for (ZObject3dScanArray::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan &obj = **iter;
      obj.drawStack(stack->c_stack(), v, offset);
    }
  }

  return stack;
}

/*
ZStack* ZStackFactory::MakeLabelStack(ZSwcTree *obj, int v)
{
  ZIntCuboid box = obj->getBoundBox().toIntCuboid();

  return NULL;
//  ZStack *stack = MakeStack()
}
*/

ZStack* ZStackFactory::MakeColorStack(const ZObject3dScanArray &objArray)
{
  ZStack *stack = NULL;

#if defined(_QT_GUI_USED_)
  if (!objArray.empty()) {
    ZIntCuboid boundBox = objArray.getBoundBox();
    stack = MakeZeroStack(GREY, boundBox, 3);

    int offset[3];
    offset[0] = -stack->getOffset().getX();
    offset[1] = -stack->getOffset().getY();
    offset[2] = -stack->getOffset().getZ();

    for (ZObject3dScanArray::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan &obj = **iter;
      obj.drawStack(stack->c_stack(0), obj.getColor().red(), offset);
      obj.drawStack(stack->c_stack(1), obj.getColor().green(), offset);
      obj.drawStack(stack->c_stack(2), obj.getColor().blue(), offset);
    }
  }
#endif

  return stack;
}

ZStack* ZStackFactory::MakeStrokeMask(const std::vector<ZStroke2d *> strokeList)
{
  ZStack *stack = NULL;

#if _QT_GUI_USED_
  ZIntCuboid box;
  for (ZStroke2d *stroke : strokeList) {
    ZIntCuboid subbox;
    stroke->boundBox(&subbox);
    box.join(subbox);
  }
  if (!box.isEmpty()) {
    box.expandX(1);
    box.expandY(1);
    stack = MakeZeroStack(GREY, box);
  }
  for (ZStroke2d *stroke : strokeList) {
    stroke->labelStack(stack);
  }
#endif

  return stack;
}

ZStack* ZStackFactory::MakeStrokeProjMask(const std::vector<ZStroke2d *> strokeList)
{
  ZStack *stack = NULL;

#if _QT_GUI_USED_
  ZIntCuboid box;
  for (ZStroke2d *stroke : strokeList) {
    ZIntCuboid subbox;
    stroke->boundBox(&subbox);
    box.join(subbox);
  }
  if (!box.isEmpty()) {
    box.setFirstZ(0);
    box.setLastZ(0);
    box.expandX(1);
    box.expandY(1);
    stack = MakeZeroStack(GREY, box);
  }
  for (ZStroke2d *stroke : strokeList) {
    stroke->labelProjStack(stack);
  }
#endif

  return stack;
}

ZStack* ZStackFactory::MakeStrokeProjMask(
    const std::vector<ZStroke2d *> strokeList, int value)
{
  ZStack *stack = NULL;

#if _QT_GUI_USED_
  ZIntCuboid box;
  for (ZStroke2d *stroke : strokeList) {
    ZIntCuboid subbox;
    stroke->boundBox(&subbox);
    box.join(subbox);
  }
  if (!box.isEmpty()) {
    box.setFirstZ(0);
    box.setLastZ(0);
    box.expandX(1);
    box.expandY(1);
    stack = MakeZeroStack(GREY, box);
  }
  for (ZStroke2d *stroke : strokeList) {
    stroke->labelProjStack(stack, value);
  }
#endif

  return stack;
}

ZStack* ZStackFactory::MakeStrokeMask(
    const std::vector<ZStroke2d *> strokeList, int z)
{
  ZStack *stack = NULL;

#if _QT_GUI_USED_
  ZIntCuboid box;
  for (ZStroke2d *stroke : strokeList) {
    ZIntCuboid subbox;
    if (stroke->getZ() == z || stroke->isPenetrating()) {
      stroke->boundBox(&subbox);
      box.join(subbox);
    }
  }
  if (!box.isEmpty()) {
    box.setFirstZ(z);
    box.setLastZ(z);
    box.expandX(1);
    box.expandY(1);
    stack = MakeZeroStack(GREY, box);
  }
  for (ZStroke2d *stroke : strokeList) {
    if (stroke->getZ() == z || stroke->isPenetrating()) {
      stroke->labelStack(stack);
    }
  }
#endif

  return stack;
}

ZStack* ZStackFactory::Compose(const std::vector<ZStack *> &stackArray)
{
  ZIntCuboid box;
  for (const ZStack *stack : stackArray) {
    if (stack) {
      box.join(stack->getBoundBox());
    }
  }

  ZStack *stack = nullptr;
  if (!box.isEmpty()) {
    stack = MakeZeroStack(GREY, box);
    for (const ZStack *block : stackArray) {
      stack->setBlockValue(block);
    }
  }

  return stack;
}

ZStack* ZStackFactory::MakeLabelBinaryStack(
    const std::vector<ZArray*> &labelArray, uint64_t v)
{
  ZIntCuboid box;
  for (ZArray *array : labelArray) {
    if (array) {
      box.join(misc::GetBoundBox(array));
    }
  }

  ZStack *stack = nullptr;
  if (!box.isEmpty()) {
    stack = MakeZeroStack(GREY, box);
    int stackWidth = stack->width();
    int stackHeight = stack->height();
    int stackArea = stackWidth * stackHeight;

    uint8_t *out = stack->array8();

    for (ZArray *array : labelArray) {
      if (array) {
        uint64_t *label = array->getDataPointer<uint64_t>();
        size_t arrayOffset = 0;
        size_t stackOffset = 0;
        int x0 = array->getStartCoordinate(0);
        int y0 = array->getStartCoordinate(1);
        int z0 = array->getStartCoordinate(2);
        int width = array->getDim(0);
        int height = array->getDim(1);
        int depth = array->getDim(2);
        int dz = z0 - stack->getOffset().getZ();
        int dy = y0 - stack->getOffset().getY();
        int dx = x0 - stack->getOffset().getX();

        for (int z = 0; z < depth; ++z) {
          stackOffset = (dz + z) * stackArea + dy * stackWidth + dx;
          for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
              if (label[arrayOffset++] == v) {
                out[stackOffset] = 1;
              }
              ++stackOffset;
            }
            stackOffset += stackWidth - width;
          }
        }
      }
    }
  }

  return stack;
}

ZStack* ZStackFactory::MakeLabelColorStack(
    const std::vector<ZArray*> &labelArray)
{
  ZStack *stack = nullptr;

#if defined(_QT_APPLICATION_)
  ZIntCuboid box;
  for (ZArray *array : labelArray) {
    if (array) {
      box.join(misc::GetBoundBox(array));
    }
  }

  if (!box.isEmpty()) {
    stack = MakeZeroStack(COLOR, box);
    int stackWidth = stack->width();
    int stackHeight = stack->height();
    int stackArea = stackWidth * stackHeight;

    color_t *out = stack->arrayc();

    ZColorScheme colorScheme;
    colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);

    for (ZArray *array : labelArray) {
      if (array) {
        uint64_t *label = array->getDataPointer<uint64_t>();
        size_t arrayOffset = 0;
        size_t stackOffset = 0;
        int x0 = array->getStartCoordinate(0);
        int y0 = array->getStartCoordinate(1);
        int z0 = array->getStartCoordinate(2);
        int width = array->getDim(0);
        int height = array->getDim(1);
        int depth = array->getDim(2);
        int dz = z0 - stack->getOffset().getZ();
        int dy = y0 - stack->getOffset().getY();
        int dx = x0 - stack->getOffset().getX();

        for (int z = 0; z < depth; ++z) {
          stackOffset = (dz + z) * stackArea + dy * stackWidth + dx;
          for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
              QColor color = colorScheme.getColor(label[arrayOffset++]);
              out[stackOffset][0] = color.red();
              out[stackOffset][1] = color.green();
              out[stackOffset][2] = color.blue();
              ++stackOffset;
            }
            stackOffset += stackWidth - width;
          }
        }
      }
    }
  }
#endif

  return stack;
}

ZStack* ZStackFactory::LoadFromFile(const std::string &path)
{
  ZStack *stack = new ZStack;
  stack->load(path, false);

  return stack;
}
