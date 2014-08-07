#include "zstackfactory.h"

#if _QT_GUI_USED_
#include <QPixmap>
#include <QPainter>
#endif

#include "neutubeconfig.h"
#include "zstroke2d.h"

ZStackFactory::ZStackFactory()
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

