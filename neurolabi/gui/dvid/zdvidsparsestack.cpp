#include "zdvidsparsestack.h"
#include <QImage>
#include <QtConcurrentRun>

#include "zdvidinfo.h"
#include "zdvidreader.h"
#include "zpainter.h"
#include "zimage.h"

ZDvidSparseStack::ZDvidSparseStack()
{
  init();
}

ZDvidSparseStack::~ZDvidSparseStack()
{
#ifdef _DEBUG_
  std::cout << "Deleting dvid sparsestack: " << ": " << getSource() << std::endl;
#endif
}

void ZDvidSparseStack::init()
{
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  m_type = GetType();
  m_isValueFilled = false;
  m_label = 0;
}

ZStack* ZDvidSparseStack::getSlice(int z) const
{
  ZStack *stack = NULL;

  const ZObject3dScan *objectMask =
      const_cast<ZDvidSparseStack&>(*this).getObjectMask();
  if (objectMask != NULL) {
    ZObject3dScan slice = objectMask->getSlice(z);
    ZIntCuboid box = slice.getBoundBox();
    stack = new ZStack(GREY, box, 1);
    stack->setZero();

    size_t stripeNumber = slice.getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = slice.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      int segmentNumber = stripe.getSegmentNumber();
      for (int j = 0; j < segmentNumber; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          int v = getValue(x, y, z);
          stack->setIntValue(x, y, z, 0, v);
        }
      }
    }
  }

  return stack;
}

void ZDvidSparseStack::initBlockGrid()
{

  if (m_dvidReader.open(getDvidTarget())) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    m_sparseStack.setGreyScale(grid);
    grid->setMinPoint(dvidInfo.getStartCoordinates());
    grid->setBlockSize(dvidInfo.getBlockSize());
    grid->setGridSize(dvidInfo.getGridSize());
  }
}

void ZDvidSparseStack::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  initBlockGrid();
}

int ZDvidSparseStack::getValue(int x, int y, int z) const
{
  int v = 0;
  ZStackBlockGrid *stackGrid = const_cast<ZStackBlockGrid*>(
        m_sparseStack.getStackGrid());
  if (stackGrid != NULL) {
    ZStackBlockGrid::Location location = stackGrid->getLocation(x, y, z);
    const ZIntPoint& blockIndex = location.getBlockIndex();
    if (stackGrid->getStack(blockIndex) == NULL) {
      ZIntCuboid box = stackGrid->getBlockBox(blockIndex);
      ZStack *stack = m_dvidReader.readGrayScale(box);
      stackGrid->consumeStack(blockIndex, stack);
    }

    ZStack *blockStack = stackGrid->getStack(blockIndex);
    v = blockStack->getIntValueLocal(location.getLocalPosition().getX(),
                                     location.getLocalPosition().getY(),
                                     location.getLocalPosition().getZ());
  }

  return v;
}

void ZDvidSparseStack::display(
    ZPainter &painter, int slice, EDisplayStyle option, NeuTube::EAxis sliceAxis) const
{
  if (loadingObjectMask()) {
    ZObject3dScan *obj = m_dvidReader.readBody(
          getLabel(), painter.getZ(slice), NeuTube::Z_AXIS, NULL);
    obj->setColor(getColor());
    obj->display(painter, slice, option, sliceAxis);
    delete obj;
  } else {
    ZObject3dScan *obj = const_cast<ZDvidSparseStack&>(*this).getObjectMask();
    if (obj != NULL) {
      obj->display(painter, slice, option, sliceAxis);
    }
  }
}

ZIntCuboid ZDvidSparseStack::getBoundBox() const
{
  ZIntCuboid box;
  ZObject3dScan *obj = const_cast<ZDvidSparseStack&>(*this).getObjectMask();
  if (obj != NULL) {
    box = obj->getBoundBox();
  }

  return box;
}

QString ZDvidSparseStack::getLoadBodyThreadId() const
{
  return "ZDvidSparseStack::loadBodyAsync";
}

void ZDvidSparseStack::downloadBodyMask()
{
  if (m_dvidReader.isReady()) {
    ZObject3dScan *obj = new ZObject3dScan;
    m_dvidReader.readBody(getLabel(), obj);
    m_sparseStack.setObjectMask(obj);
    pushAttribute();
  }
}

void ZDvidSparseStack::loadBody(uint64_t bodyId, bool canonizing)
{
  m_isValueFilled = false;

  ZObject3dScan *obj = new ZObject3dScan;
  m_dvidReader.readBody(bodyId, obj);
  m_sparseStack.setObjectMask(obj);
  setLabel(bodyId);
  if (canonizing) {
    obj->canonize();
  }
}

void ZDvidSparseStack::loadBodyAsync(uint64_t bodyId)
{
  m_isValueFilled = false;
  setLabel(bodyId);

  QString threadId = getLoadBodyThreadId();
  if (m_futureMap.contains(threadId)) {
    if (!m_futureMap[threadId].isRunning()) {
      m_futureMap.remove(threadId);
    }
  }

  if (!m_futureMap.contains(threadId)) {
    QFuture<void> future =
        QtConcurrent::run(this, &ZDvidSparseStack::loadBody, bodyId, true);
    m_futureMap[threadId] = future;
  }
}

void ZDvidSparseStack::setMaskColor(const QColor &color)
{
  setColor(color);
  pushMaskColor();
}

void ZDvidSparseStack::setLabel(uint64_t bodyId)
{
  m_label = bodyId;
//  pushLabel();
}

const ZIntPoint& ZDvidSparseStack::getDownsampleInterval() const
{
  return m_sparseStack.getDownsampleInterval();
}

bool ZDvidSparseStack::fillValue(const ZIntCuboid &box)
{
  if (m_isValueFilled) {
    return true;
  }

  //  bool changed = false;
  int blockCount = 0;
  ZObject3dScan *objMask = getObjectMask();
  if (objMask != NULL) {
    if (!objMask->isEmpty()) {
      qDebug() << "Downloading grayscale ...";
      //    tic();
      ZDvidInfo dvidInfo;
      dvidInfo.setFromJsonString(
            m_dvidReader.readInfo(getDvidTarget().getGrayScaleName().c_str()).
            toStdString());
      ZObject3dScan blockObj =
          dvidInfo.getBlockIndex(*objMask);
      ZStackBlockGrid *grid = m_sparseStack.getStackGrid();

      size_t stripeNumber = blockObj.getStripeNumber();
      ZIntCuboid blockBox;
      blockBox.setFirstCorner(dvidInfo.getBlockIndex(box.getFirstCorner()));
      blockBox.setLastCorner(dvidInfo.getBlockIndex(box.getLastCorner()));


      for (size_t s = 0; s < stripeNumber; ++s) {
        const ZObject3dStripe &stripe = blockObj.getStripe(s);
        int segmentNumber = stripe.getSegmentNumber();
        int y = stripe.getY();
        int z = stripe.getZ();
        for (int i = 0; i < segmentNumber; ++i) {
          int x0 = stripe.getSegmentStart(i);
          int x1 = stripe.getSegmentEnd(i);

          for (int x = x0; x <= x1; ++x) {
            if (blockBox.contains(x, y, z)) {
              const ZIntPoint blockIndex =
                  ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();

              if (grid->getStack(blockIndex) == NULL) {
                ZIntCuboid box = grid->getBlockBox(blockIndex);
                ZStack *stack = m_dvidReader.readGrayScale(box);
                grid->consumeStack(blockIndex, stack);
                ++blockCount;
                //              changed = true;
              }
            }
          }
        }
      }
      //    ptoc();

      qDebug() << blockCount << " blocks downloaded.";
    }
  }

  return (blockCount > 0);
}

bool ZDvidSparseStack::fillValue()
{
//  bool changed = false;
  int blockCount = 0;
  ZObject3dScan *objMask = getObjectMask();
  if (objMask != NULL) {
    if (!objMask->isEmpty()) {
      //    std::cout << "Downloading grayscale ..." << std::end;
      qDebug() << "Downloading grayscale ...";
      ZDvidInfo dvidInfo;
      dvidInfo.setFromJsonString(
            m_dvidReader.readInfo(getDvidTarget().getGrayScaleName().c_str()).
            toStdString());
      ZObject3dScan blockObj =
          dvidInfo.getBlockIndex(*objMask);
      ZStackBlockGrid *grid = m_sparseStack.getStackGrid();

      size_t stripeNumber = blockObj.getStripeNumber();
      for (size_t s = 0; s < stripeNumber; ++s) {
        const ZObject3dStripe &stripe = blockObj.getStripe(s);
        int segmentNumber = stripe.getSegmentNumber();
        int y = stripe.getY();
        int z = stripe.getZ();
        for (int i = 0; i < segmentNumber; ++i) {
          int x0 = stripe.getSegmentStart(i);
          int x1 = stripe.getSegmentEnd(i);

          for (int x = x0; x <= x1; ++x) {
            const ZIntPoint blockIndex =
                ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();

            if (grid->getStack(blockIndex) == NULL) {
              ZIntCuboid box = grid->getBlockBox(blockIndex);
              ZStack *stack = m_dvidReader.readGrayScale(box);
              grid->consumeStack(blockIndex, stack);
              ++blockCount;
            }
          }
        }
      }

      m_isValueFilled = true;

      qDebug() << blockCount << " blocks downloaded.";
    }
  }

  return blockCount > 0;
//  return changed;
}

ZStack* ZDvidSparseStack::getStack()
{
  if (fillValue()) {
    m_sparseStack.deprecate(ZSparseStack::STACK);
  }

  return m_sparseStack.getStack();
}

ZStack* ZDvidSparseStack::getStack(const ZIntCuboid &updateBox)
{
  if (fillValue(updateBox)) {
    m_sparseStack.deprecate(ZSparseStack::STACK);
  }

  return m_sparseStack.getStack();
}

uint64_t ZDvidSparseStack::getLabel() const
{
  return m_label;
}

bool ZDvidSparseStack::loadingObjectMask() const
{
  QString threadId = getLoadBodyThreadId();
  if (m_futureMap.contains(threadId)) {
    if (m_futureMap[threadId].isRunning()) {
      return true;
    }
  }

  return false;
}

void ZDvidSparseStack::finishObjectMaskLoading()
{
  QString threadId = getLoadBodyThreadId();
  if (m_futureMap.contains(threadId)) {
    if (m_futureMap[threadId].isRunning()) {
      m_futureMap[threadId].waitForFinished();
    }
  }
}

void ZDvidSparseStack::syncObjectMask()
{
  finishObjectMaskLoading();
  pushAttribute();
}

/*
const ZObject3dScan* ZDvidSparseStack::getObjectMask() const
{
  finishObjectMaskLoading();

  return m_sparseStack.getObjectMask();
}
*/

ZObject3dScan* ZDvidSparseStack::getObjectMask()
{
  syncObjectMask();

  return m_sparseStack.getObjectMask();
  /*
  return const_cast<ZObject3dScan*>(
        static_cast<const ZDvidSparseStack&>(*this).getObjectMask());
        */
}

const ZSparseStack* ZDvidSparseStack::getSparseStack() const
{
  return &m_sparseStack;
}

ZSparseStack* ZDvidSparseStack::getSparseStack()
{
  fillValue();

  return &m_sparseStack;
}

bool ZDvidSparseStack::hit(double x, double y, double z)
{
  ZObject3dScan *objectMask = getObjectMask();
  if (objectMask != NULL) {
    if (objectMask->hit(x, y, z)) {
      m_hitPoint = objectMask->getHitPoint();
      return true;
    }
  }

  return false;
}

bool ZDvidSparseStack::hit(double x, double y, NeuTube::EAxis axis)
{
  ZObject3dScan *objectMask = getObjectMask();
  if (objectMask != NULL) {
    if (objectMask->hit(x, y, axis)) {
      m_hitPoint = objectMask->getHitPoint();
      return true;
    }
  }

  return false;
}

bool ZDvidSparseStack::isEmpty() const
{
  return m_sparseStack.isEmpty();
}

ZDvidSparseStack* ZDvidSparseStack::getCrop(const ZIntCuboid &box) const
{
  ZDvidSparseStack *stack = new ZDvidSparseStack;
  stack->setDvidTarget(getDvidTarget());
  stack->m_sparseStack.setBaseValue(m_sparseStack.getBaseValue());
  ZObject3dScan *submask = new ZObject3dScan;
  const_cast<ZDvidSparseStack&>(*this).getObjectMask()->subobject(box, submask);
  stack->m_sparseStack.setObjectMask(submask);

  return stack;
}

void ZDvidSparseStack::deprecateStackBuffer()
{
  m_sparseStack.deprecate(ZSparseStack::STACK);
}

void ZDvidSparseStack::pushLabel()
{
  ZObject3dScan *obj = m_sparseStack.getObjectMask();
  if (obj != NULL) {
    obj->setLabel(m_label);
  }
}

void ZDvidSparseStack::pushMaskColor()
{
  ZObject3dScan *obj = m_sparseStack.getObjectMask();
  if (obj != NULL) {
    obj->setColor(getColor());
  }
}

void ZDvidSparseStack::pushAttribute()
{
  pushLabel();
  pushMaskColor();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSparseStack)
