#include "zsparsestack.h"

#include <fstream>

#include "zstack.hxx"
#include "neutubeconfig.h"
#include "misc/miscutility.h"
#include "zhdf5reader.h"
#include "zhdf5writer.h"

//#define MAX_STACK_VOLUME 1847483647
#define MAX_STACK_VOLUME 923741823

//#define MAX_STACK_VOLUME 1000

ZSparseStack::ZSparseStack() :
  m_objectMask(NULL), m_stackGrid(NULL), m_stack(NULL), m_baseValue(1)
{
}

ZSparseStack::~ZSparseStack()
{
  deprecate(ALL_COMPONET);
}

void ZSparseStack::deprecateDependent(EComponent component)
{
  switch (component) {
  case GREY_SCALE:
  case OBJECT_MASK:
    deprecate(STACK);
    break;
  default:
    break;
  }
}

void ZSparseStack::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case STACK:
    delete m_stack;
    m_stack = NULL;
    break;
  case GREY_SCALE:
    delete m_stackGrid;
    m_stackGrid = NULL;
    break;
  case OBJECT_MASK:
    delete m_objectMask;
    m_objectMask = NULL;
    break;
  case ALL_COMPONET:
    deprecate(GREY_SCALE);
    deprecate(OBJECT_MASK);
    break;
  }
}

bool ZSparseStack::isDeprecated(EComponent component) const
{
  switch (component) {
  case STACK:
    return m_stack == NULL;
  default:
    break;
  }

  return false;
}

void ZSparseStack::setBaseValue(int baseValue)
{
  if (m_baseValue != baseValue) {
    deprecate(STACK);
    m_baseValue = baseValue;
  }
}

void ZSparseStack::assignStackValue(
    ZStack *stack, const ZObject3dScan &obj, const ZStackBlockGrid &stackGrid,
    const int baseValue)
{
  if (stackGrid.isEmpty() || stackGrid.getStackArray().empty()) {
    for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = obj.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          stack->setIntValue(x, y, z, 0, 255);
        }
      }
    }
  } else {
    for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = obj.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          int v = stackGrid.getValue(x, y, z) + baseValue;
          stack->setIntValue(x, y, z, 0, v);
        }
      }
    }
  }
}

void ZSparseStack::assignStackValue(
    ZStack *stack, const ZObject3dScan &obj, const ZObject3dScan &border,
    const ZStackBlockGrid &stackGrid,
    const int baseValue)
{
  if (stackGrid.isEmpty() || stackGrid.getStackArray().empty()) {
    for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = obj.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          stack->setIntValue(x, y, z, 0, 255);
        }
      }
    }
  } else {
    for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = obj.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          int v = stackGrid.getValue(x, y, z) + baseValue;
          stack->setIntValue(x, y, z, 0, v);
        }
      }
    }

    for (size_t i = 0; i < border.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = border.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          if (stack->getIntValue(x, y, z, 0) > baseValue) {
            stack->setIntValue(x, y, z, 0, baseValue);
          }
        }
      }
    }
  }
}

size_t ZSparseStack::GetMaxStackVolume()
{
  return MAX_STACK_VOLUME;
}

bool ZSparseStack::DownsampleRequired(const ZIntCuboid &box)
{
  return box.getVolume() > MAX_STACK_VOLUME;
}

bool ZSparseStack::downsampleRequired() const
{
  if (m_objectMask != NULL) {
    return DownsampleRequired(m_objectMask->getBoundBox());
  }

  return false;
}

ZStack* ZSparseStack::makeDsStack(int xintv, int yintv, int zintv)
{
  ZStack *out = NULL;
  if (m_objectMask != NULL && m_stackGrid != NULL) {
    ZIntCuboid cuboid = m_objectMask->getBoundBox();

    if (!m_objectMask->isEmpty() && !cuboid.isEmpty()) {
      ZObject3dScan *obj = m_objectMask->subobject(cuboid, NULL, NULL);
      ZIntPoint dsIntv(xintv, yintv, zintv);

      if (!dsIntv.isZero()) {
        obj->downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

        ZStackBlockGrid *dsGrid =
            m_stackGrid->makeDownsample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
        out =  new ZStack(GREY, obj->getBoundBox(), 1);
        out->setZero();
        assignStackValue(out, *obj, *dsGrid, m_baseValue);
        out->setDsIntv(dsIntv);
        delete dsGrid;
        delete obj;
      } else {
        out = new ZStack(GREY, cuboid, 1);
        out->setZero();
        assignStackValue(out, *obj, *m_stackGrid, m_baseValue);
        delete obj;
      }
    }
  }

  return out;
}

ZStack* ZSparseStack::makeIsoDsStack(size_t maxVolume, bool preservingGap)
{
  ZStack *out = NULL;
  if (m_objectMask != NULL && m_stackGrid != NULL) {
    ZIntCuboid cuboid = m_objectMask->getBoundBox();

    if (!m_objectMask->isEmpty() && !cuboid.isEmpty()) {
      ZObject3dScan *obj = m_objectMask->subobject(cuboid, NULL, NULL);
      int dsIntv = misc::getIsoDsIntvFor3DVolume(cuboid, maxVolume, true);

#ifdef _DEBUG_
      std::cout << "Downsampling: " << dsIntv << std::endl;
#endif

      if (dsIntv > 0) {
        ZObject3dScan border;
        if (preservingGap) {
          border = obj->getComplementObject();
          border.downsampleMax(dsIntv, dsIntv, dsIntv);
        }
        obj->downsampleMax(dsIntv, dsIntv, dsIntv);
        if (preservingGap) {
          border.intersect(*obj);
        }

        ZStackBlockGrid *dsGrid =
            m_stackGrid->makeDownsample(dsIntv, dsIntv, dsIntv);
        out =  new ZStack(GREY, obj->getBoundBox(), 1);
        out->setZero();
        assignStackValue(out, *obj, border, *dsGrid, m_baseValue);
        out->setDsIntv(ZIntPoint(dsIntv, dsIntv, dsIntv));
        delete dsGrid;
        delete obj;
      } else {
        out = new ZStack(GREY, cuboid, 1);
        out->setZero();
        assignStackValue(out, *obj, *m_stackGrid, m_baseValue);
        delete obj;
      }
    }
  }

  return out;
}

ZStack* ZSparseStack::makeStack(
    const ZIntCuboid &box, size_t maxVolume, bool preservingGap,
    ZIntPoint *dsIntv)
{
  ZStack *out = NULL;
  if (m_objectMask != NULL && m_stackGrid != NULL) {
    ZIntCuboid cuboid = m_objectMask->getBoundBox();
    if (!box.isEmpty()) {
      cuboid.intersect(box);
    }
    if (!m_objectMask->isEmpty() && !cuboid.isEmpty()) {
      size_t volume = cuboid.getVolume();
      double dsRatio = (double) volume / maxVolume;
      ZObject3dScan *obj = m_objectMask->subobject(cuboid, NULL, NULL);

      if (dsRatio > 1.0) {
        ZIntPoint tmpDsIntv = misc::getDsIntvFor3DVolume(dsRatio);

        ZObject3dScan border;
        if (preservingGap) {
          border = obj->getComplementObject();
          border.downsampleMax(tmpDsIntv);
        }

        obj->downsampleMax(tmpDsIntv.getX(), tmpDsIntv.getY(), tmpDsIntv.getZ());

        if (preservingGap) {
          border.intersect(*obj);
        }

        ZStackBlockGrid *dsGrid = m_stackGrid->makeDownsample(
              tmpDsIntv.getX(), tmpDsIntv.getY(), tmpDsIntv.getZ());
#ifdef _DEBUG_2
  return NULL;
#endif
        out =  new ZStack(GREY, obj->getBoundBox(), 1);
        out->setZero();
        assignStackValue(out, *obj, border, *dsGrid, m_baseValue);
        if (dsIntv != NULL) {
          *dsIntv = tmpDsIntv;
        }
        out->setDsIntv(tmpDsIntv);
        delete dsGrid;
        delete obj;
      } else {
        out = new ZStack(GREY, cuboid, 1);
        out->setZero();
        assignStackValue(out, *obj, *m_stackGrid, m_baseValue);
        if (dsIntv != NULL) {
          dsIntv->set(0, 0, 0);
        }

        delete obj;
      }
    }
  }

  return out;
}

ZStack* ZSparseStack::makeStack(const ZIntCuboid &box, bool preservingGap)
{
  return makeStack(box, MAX_STACK_VOLUME, preservingGap, NULL);
}

ZStack* ZSparseStack::getStack()
{
  if (m_objectMask == NULL || m_stackGrid == NULL) {
    return NULL;
  }

  if (isDeprecated(STACK)) {
    ZIntCuboid cuboid = m_objectMask->getBoundBox();
    if (!m_objectMask->isEmpty()) {
      size_t volume = cuboid.getVolume();
      double dsRatio = (double) volume / MAX_STACK_VOLUME;
      if (dsRatio > 1.0) {
        ZObject3dScan obj = *m_objectMask;
        m_dsIntv = misc::getDsIntvFor3DVolume(dsRatio);
//        int intv = misc::getIsoDsIntvFor3DVolume(dsRatio);
//        m_dsIntv.set(intv, intv, intv);

#ifdef _DEBUG_
        std::cout << "Downsampling:" << m_dsIntv.toString() << std::endl;
#endif

        obj.downsampleMax(m_dsIntv.getX(), m_dsIntv.getY(), m_dsIntv.getZ());

        ZStackBlockGrid *dsGrid = m_stackGrid->makeDownsample(
              m_dsIntv.getX(), m_dsIntv.getY(), m_dsIntv.getZ());
#ifdef _DEBUG_2
        return NULL;
#endif
        m_stack =  new ZStack(GREY, obj.getBoundBox(), 1);
        m_stack->setZero();
        assignStackValue(m_stack, obj, *dsGrid, m_baseValue);
        m_stack->setDsIntv(m_dsIntv);
        delete dsGrid;
      } else {
        m_stack = new ZStack(GREY, cuboid, 1);
        m_stack->setZero();
        assignStackValue(m_stack, *m_objectMask, *m_stackGrid, m_baseValue);
        m_dsIntv.set(0, 0, 0);
      }
    }
  }

#ifdef _DEBUG_2
  m_stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

  return m_stack;
}

const ZStack* ZSparseStack::getStack() const
{
  return dynamic_cast<const ZStack*>(
        const_cast<ZSparseStack*>(this)->getStack());
}

#if 0
ZStack* ZSparseStack::toDownsampledStack(int xIntv, int yIntv, int zIntv)
{
  if (m_objectMask == NULL || m_stackGrid == NULL) {
    return NULL;
  }

  ZStack *stack = NULL;
#if 0
  ZIntCuboid cuboid = m_objectMask->getBoundBox();
  if (!m_objectMask->isEmpty()) {
    ZObject3dScan obj = *m_objectMask;
    obj.downsampleMax(xIntv, yIntv, zIntv);


    size_t volume = cuboid.getVolume();
    if (volume > MAX_STACK_VOLUME) {
      ZObject3dScan obj = *m_objectMask;

      if (volume / 8 > MAX_STACK_VOLUME) {
        m_dsIntv.set(3, 3, 1);
      } else {
        m_dsIntv.set(1, 1, 1);
      }
      obj.downsampleMax(m_dsIntv.getX(), m_dsIntv.getY(), m_dsIntv.getZ());

      ZStackBlockGrid *dsGrid = m_stackGrid->makeDownsample(
            m_dsIntv.getX(), m_dsIntv.getY(), m_dsIntv.getZ());

      m_stack =  new ZStack(GREY, obj.getBoundBox(), 1);
      m_stack->setZero();
      assignStackValue(m_stack, obj, *dsGrid);
      delete dsGrid;
    } else {
      m_stack = new ZStack(GREY, cuboid, 1);
      m_stack->setZero();
      assignStackValue(m_stack, *m_objectMask, *m_stackGrid);
    }
  }
#endif
  return stack;
}
#endif

size_t ZSparseStack::getObjectVolume() const
{
  if (m_objectMask == NULL) {
    return 0;
  }

  return m_objectMask->getVoxelNumber();
}

void ZSparseStack::shakeOff()
{
  if (m_objectMask != NULL) {
    std::vector<ZObject3dScan> objArray =
        m_objectMask->getConnectedComponent(ZObject3dScan::ACTION_NONE);
    if (objArray.size() > 1) {
      size_t maxV = objArray[0].getVoxelNumber();
      size_t selected = 0;
      for (size_t i = 1; i < objArray.size(); ++i) {
        const ZObject3dScan &obj = objArray[i];
        size_t v = obj.getVoxelNumber();
        if (v > maxV) {
          maxV = v;
          selected = i;
        }
      }
      *m_objectMask = objArray[selected];
    }
  }
}

ZStack* ZSparseStack::getSlice(int z) const
{
  ZObject3dScan slice = m_objectMask->getSlice(z);
  ZIntCuboid box = slice.getBoundBox();
  ZStack *stack = new ZStack(GREY, box, 1);
  stack->setZero();
  assignStackValue(stack, slice, *m_stackGrid, 1);

  return stack;
}

ZStack* ZSparseStack::getMip() const
{
  ZStack *stack = NULL;
  if (m_objectMask != NULL) {
    ZIntCuboid box = m_objectMask->getBoundBox();
    box.setFirstZ(0);
    box.setLastZ(0);
    stack = new ZStack(GREY, box, 1);
    ZObject3dScan::ConstSegmentIterator iterator(m_objectMask);
    while (iterator.hasNext()) {
      const ZObject3dScan::Segment &seg = iterator.next();
      int y = seg.getY();
//      int z = seg.getZ();
      for (int x = seg.getStart(); x <= seg.getEnd(); ++x) {
        stack->setIntValue(x, y, 0, 0, 164);
      }
    }

  }

  return stack;
}

void ZSparseStack::setGreyScale(ZStackBlockGrid *stackGrid)
{
  if (m_stackGrid != stackGrid) {
    deprecate(GREY_SCALE);
    m_stackGrid = stackGrid;
  }
}

void ZSparseStack::setObjectMask(ZObject3dScan *obj)
{
  if (m_objectMask != obj) {
    deprecate(OBJECT_MASK);
    m_objectMask = obj;
    /*
    if (obj != NULL) {
      obj->setColor(255, 255, 255, 255);
    }
    */
  }
}

void ZSparseStack::merge(ZSparseStack &sparseStack)
{
  if (!sparseStack.isEmpty()) {
    m_objectMask->unify(*(sparseStack.getObjectMask()));
    m_stackGrid->consume(sparseStack.m_stackGrid);
    deprecateDependent(GREY_SCALE);
    sparseStack.deprecateDependent(GREY_SCALE);
  }
}

ZIntCuboid ZSparseStack::getBoundBox() const
{
  ZIntCuboid box;
  if (m_objectMask != NULL) {
    box = m_objectMask->getBoundBox();
  }

  return box;
}

bool ZSparseStack::isEmpty() const
{
  if (m_objectMask != NULL) {
    return m_objectMask->isEmpty();
  }

  return true;
}

void ZSparseStack::read(std::istream &stream)
{
  deprecate(ALL_COMPONET);
  m_objectMask = new ZObject3dScan;
  m_objectMask->read(stream);
  m_stackGrid = new ZStackBlockGrid;
  m_stackGrid->read(stream);
}

void ZSparseStack::write(std::ostream &stream) const
{
  if (m_objectMask != NULL) {
    if (stream.good()) {
      m_objectMask->write(stream);
      if (m_stackGrid != NULL) {
        m_stackGrid->write(stream);
      }
    }
  }
}

bool ZSparseStack::save(const std::string &filePath) const
{
  std::ofstream stream(filePath.c_str(), std::ios_base::binary);
  if (stream.good()) {
    write(stream);
    return true;
  }

  return false;
}

bool ZSparseStack::load(const std::string &filePath)
{
  std::ifstream stream(filePath.c_str(), std::ios_base::binary);
  if (stream.good()) {
    read(stream);
    return true;
  }

  return false;
}

void ZSparseStack::printInfo() const
{
  if (isEmpty()) {
    std::cout << "Empty sparse stack." << std::endl;
  } else {
    if (m_objectMask != NULL) {
      m_objectMask->printInfo();
    }
  }
}
