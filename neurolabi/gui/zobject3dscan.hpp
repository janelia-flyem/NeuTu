#ifndef ZOBJECT3DSCAN_HPP
#define ZOBJECT3DSCAN_HPP

#include "zobject3dscan.h"

template<class T>
int ZObject3dScan::scanArray(
    const T *array, int x, int y, int z, int width, int x0)
{
  if (array == NULL) {
    return 0;
  }

  if (x < 0 || x >= width) {
    return 0;
  }

  T v = array[x];

  if (isEmpty()) {
//    addStripe(z, y, false);
    addStripeFast(z, y);
    getStripeArray().back().getSegmentArray().reserve(8);
  } else {
    if (m_stripeArray.back().getY() != y || m_stripeArray.back().getZ() != z) {
//      addStripe(z, y, false);
      addStripeFast(z, y);
      getStripeArray().back().getSegmentArray().reserve(8);
    }
  }

  int length = 1;
  if (x < width - 1) {
    while (array[x + length] == v) {
      ++length;
      if (x + length >= width) {
        break;
      }
    }
  }

  x += x0;
//  addSegment(x, x + length - 1, false);

  addSegmentFast(x, x + length - 1);

  return length;
}

template<class T>
int ZObject3dScan::scanArrayShift(
    const T *array, int start, int y, int z, int stride, int dim)
{
  if (array == NULL) {
    return 0;
  }

  int x = start;

  if (x < 0 || x >= dim) {
    return 0;
  }

  T v = array[x * stride];

  if (isEmpty()) {
//    addStripe(z, y, false);
    addStripeFast(z, y);
    getStripeArray().back().getSegmentArray().reserve(8);
  } else {
    if (m_stripeArray.back().getY() != y || m_stripeArray.back().getZ() != z) {
//      addStripe(z, y, false);
      addStripeFast(z, y);
      getStripeArray().back().getSegmentArray().reserve(8);
    }
  }

  int length = 1;
  if (x < dim - 1) {
    while (array[(x + length) * stride] == v) {
      ++length;
      if (x + length >= dim) {
        break;
      }
    }
  }

//  x += x0;
//  addSegment(x, x + length - 1, false);

  addSegmentFast(x, x + length - 1);

  return length;
}

template<class T>
int ZObject3dScan::scanArray(
    const T *array, int x, int y, int z, int width, int dim, int start,
    NeuTube::EAxis axis)
{
  if (array == NULL) {
    return 0;
  }

  ZGeometry::shiftSliceAxis(x, y, z, axis);

  if (x < 0 || x >= dim) {
    return 0;
  }

  size_t stride = 1;
  if (axis == NeuTube::X_AXIS) {
    stride = width;
  }

  T v = array[stride * x];

  if (isEmpty()) {
//    addStripe(z, y, false);
    addStripeFast(z, y);
    getStripeArray().back().getSegmentArray().reserve(8);
  } else {
    if (m_stripeArray.back().getY() != y || m_stripeArray.back().getZ() != z) {
//      addStripe(z, y, false);
      addStripeFast(z, y);
      getStripeArray().back().getSegmentArray().reserve(8);
    }
  }

  int length = 1;
  if (x < dim - 1) {
    while (array[(x + length) * stride] == v) {
      ++length;
      if (x + length >= dim) {
        break;
      }
    }
  }

  x += start;
//  addSegment(x, x + length - 1, false);

  addSegmentFast(x, x + length - 1);

  return length;
}

template<class T>
std::map<uint64_t, ZObject3dScan *> *ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, int startPlane,
    int yStep,
    std::map<uint64_t, ZObject3dScan *> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<uint64_t, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        uint64_t v = array[x];
        std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          obj->blockEvent(true);
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y, z + startPlane, width);

        x += length;
      }
      array += width * yStep;
    }
  }

  for (std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet->begin();
       iter != bodySet->end(); ++iter) {
    ZObject3dScan *obj = iter->second;
    obj->blockEvent(false);
  }

  return bodySet;
}

template<class T>
std::map<uint64_t, ZObject3dScan *> *ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, int x0, int y0, int z0,
    int yStep,
    std::map<uint64_t, ZObject3dScan *> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<uint64_t, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        uint64_t v = array[x];
        std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y + y0, z + z0, width, x0);

        x += length;
      }
      array += width * yStep;
    }
  }

  return bodySet;
}


template<class T>
std::map<uint64_t, ZObject3dScan *> *ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, NeuTube::EAxis axis)
{
  std::map<uint64_t, ZObject3dScan *> *bodySet =
      new std::map<uint64_t, ZObject3dScan*>;

  int sw = width;
  int sh = height;
  int sd = depth;
  size_t strideX = 1;
  size_t strideY = width;
  size_t strideZ = width * height;

  ZGeometry::shiftSliceAxis(sw, sh, sd, axis);
  ZGeometry::shiftSliceAxis(strideX, strideY, strideZ, axis);

  const T *arrayOrigin = array;

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < sd; ++z) {
    for (int y = 0; y < sh; ++y) {
      int x = 0;

      array = arrayOrigin + z * strideZ + y * strideY;
      while (x < sw) {
        uint64_t v = array[x * strideX];
        std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          obj->setSliceAxis(axis);
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArrayShift(array, x, y, z, strideX, sw);

        x += length;
      }
    }
  }

  return bodySet;
}

template<class T>
std::map<uint64_t, ZObject3dScan *> *ZObject3dScan::extractAllForegroundObject(
    const T *array, int width, int height, int depth, NeuTube::EAxis axis)
{
  std::map<uint64_t, ZObject3dScan *> *bodySet =
      new std::map<uint64_t, ZObject3dScan*>;

  int sw = width;
  int sh = height;
  int sd = depth;
  size_t strideX = 1;
  size_t strideY = width;
  size_t strideZ = width * height;

  ZGeometry::shiftSliceAxis(sw, sh, sd, axis);
  ZGeometry::shiftSliceAxis(strideX, strideY, strideZ, axis);

  const T *arrayOrigin = array;

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < sd; ++z) {
    for (int y = 0; y < sh; ++y) {
      int x = 0;

      array = arrayOrigin + z * strideZ + y * strideY;
      while (x < sw) {
        uint64_t v = array[x * strideX];
        if (v > 0) {
          std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet->find(v);
          if (iter == bodySet->end()) {
            obj = new ZObject3dScan;
            obj->setSliceAxis(axis);
            obj->setLabel(v);
            //(*bodySet)[v] = obj;
            bodySet->insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
          } else {
            obj = iter->second;
          }
          int length = obj->scanArrayShift(array, x, y, z, strideX, sw);

          x += length;
        } else {
          ++x;
        }
      }
    }
  }

  return bodySet;
}


template<class T>
std::map<uint64_t, ZObject3dScan *> *ZObject3dScan::extractAllForegroundObject(
    const T *array, int width, int height, int depth, int x0, int y0, int z0,
    int yStep, std::map<uint64_t, ZObject3dScan *> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<uint64_t, ZObject3dScan*>;
  }

  std::vector<std::map<uint64_t, ZObject3dScan*> > m_bodySetArray(20);

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; y += yStep) {
      int x = 0;
      while (x < width) {
        uint64_t v = array[x];
        if (v > 0) {
          std::map<uint64_t, ZObject3dScan*> &currentSet = m_bodySetArray[v % 20];

          std::map<uint64_t, ZObject3dScan*>::iterator iter = currentSet.find(v);
          if (iter == currentSet.end()) {
            obj = new ZObject3dScan;
            obj->setLabel(v);
            obj->getStripeArray().reserve(height);
            //(*bodySet)[v] = obj;
            bodySet->insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
            currentSet.insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
          } else {
            obj = iter->second;
          }
          int length = obj->scanArray(array, x, y + y0, z + z0, width, x0);

          x += length;
        } else {
          ++x;
        }
      }
      array += width * yStep;
    }
  }

  return bodySet;
}


template<class InputIterator>
Stack* ZObject3dScan::makeStack(InputIterator startObject,
                                InputIterator endObject, int *offset)
{
  if (startObject != endObject) {
    Cuboid_I boundBox;
    startObject->getBoundBox(&boundBox);

    InputIterator iter = startObject;
    ++iter;
    //Get Bound box
    for (; iter != endObject; ++iter) {
      Cuboid_I subBoundBox;
      iter->getBoundBox(&subBoundBox);
      Cuboid_I_Union(&boundBox, &subBoundBox, &boundBox);
    }

    int width, height, depth;
    Cuboid_I_Size(&boundBox, &width, &height, &depth);
    //Create stack
    Stack *stack = C_Stack::make(GREY, width, height, depth);
    C_Stack::setZero(stack);

    int stackOffset[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
      stackOffset[i] = -boundBox.cb[i];
    }

    int v = 1;
    for (iter = startObject; iter != endObject; ++iter) {
      iter->drawStack(stack, v++, stackOffset);
    }

    if (offset != NULL) {
      for (int i = 0; i < 3; ++i) {
        offset[i] = boundBox.cb[i];
      }
    }

    return stack;
  }

  return NULL;
}


#endif // ZOBJECT3DSCAN_HPP
