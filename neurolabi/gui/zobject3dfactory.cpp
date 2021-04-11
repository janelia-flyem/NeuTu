#include "zobject3dfactory.h"

#include <unordered_map>

#include "zstack.hxx"
#include "zstackfactory.h"
#include "zobject3d.h"
#include "zobject3darray.h"
#include "zobject3dscan.h"
#include "neutubeconfig.h"
#include "zobject3dscanarray.h"
#include "zstackfactory.h"
#include "zclosedcurve.h"
#include "zarray.h"
#include "zrandomgenerator.h"
#include "geometry/zintcuboid.h"
#include "zstackarray.h"
#include "zstackutil.h"
#include "misc/miscutility.h"

#if defined(_QT_GUI_USED_)
#  include "zstroke2d.h"
#endif

ZObject3dFactory::ZObject3dFactory()
{
}

ZStack* ZObject3dFactory::MakeBoundaryStack(const ZStack &stack)
{
  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::MakeZeroStack(
        stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);

  size_t area = C_Stack::area(originalStack);
  size_t offset = 0;
  //x scan
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      offset++;
      for (int x = 1; x < width - 1; ++x) {
        if (originalStack->array[offset] != originalStack->array[offset - 1] ||
            originalStack->array[offset] != originalStack->array[offset + 1]) {
          maskStack->array[offset] = originalStack->array[offset];
        }
        offset++;
      }
      offset++;
    }
  }

  uint8_t *originalArray = originalStack->array;
  //y scan
  for (int z = 0; z < depth; ++z) {
    for (int x = 0; x < width; ++x) {
      offset = area * z + x + width;
      for (int y = 1; y < height - 1; ++y) {
        if (originalArray[offset] != originalArray[offset - width] ||
            originalArray[offset] != originalArray[offset + width]) {
          maskStack->array[offset] = originalStack->array[offset];
        }
        offset += width;
      }
    }
  }

  //z scan
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      offset = width * y + x + area;
      for (int z = 1; z < depth - 1; ++z) {
        if (originalArray[offset] != originalArray[offset - area] ||
            originalArray[offset] != originalArray[offset + area]) {
          maskStack->array[offset] = originalStack->array[offset];
        }
        offset += area;
      }
    }
  }

  return mask;
}

ZObject3dArray* ZObject3dFactory::MakeRegionBoundary(
    const ZStack &stack, EOutputForm option)
{
#ifdef _DEBUG_2
  stack.save(GET_DATA_DIR + "/test.tif");
#endif

  if (stack.kind() != GREY) {
    return NULL;
  }

  if (stack.isVirtual()) {
    return NULL;
  }

  ZObject3dArray tmpArray(256);

  for (size_t i = 0; i < 256; ++i) {
    tmpArray[i] = new ZObject3d;
  }

  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::MakeZeroStack(
        stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);

  size_t area = C_Stack::area(originalStack);
  size_t offset = 0;
  //x scan
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      offset++;
      for (int x = 1; x < width - 1; ++x) {
        if (originalStack->array[offset] != originalStack->array[offset - 1] ||
            originalStack->array[offset] != originalStack->array[offset + 1]) {
          maskStack->array[offset] = 1;
        }
        offset++;
      }
      offset++;
    }
  }

  uint8_t *originalArray = originalStack->array;
  //y scan
  for (int z = 0; z < depth; ++z) {
    for (int x = 0; x < width; ++x) {
      offset = area * z + x + width;
      for (int y = 1; y < height - 1; ++y) {
        if (originalArray[offset] != originalArray[offset - width] ||
            originalArray[offset] != originalArray[offset + width]) {
          maskStack->array[offset] = 1;
        }
        offset += width;
      }
    }
  }

  //z scan
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      offset = width * y + x + area;
      for (int z = 1; z < depth - 1; ++z) {
        if (originalArray[offset] != originalArray[offset - area] ||
            originalArray[offset] != originalArray[offset + area]) {
          maskStack->array[offset] = 1;
        }
        offset += area;
      }
    }
  }

  //Extract objects
  offset = 0;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        if (maskStack->array[offset] == 1) {
          int v = originalArray[offset];
          if (v > 0) {
            ZObject3d *obj = tmpArray[v];
            obj->append(x + stack.getOffset().getX(),
                        y + stack.getOffset().getY(),
                        z + stack.getOffset().getZ());
          }
        }
        ++offset;
      }
    }
  }

  //Create output array
  ZObject3dArray *out = new ZObject3dArray(tmpArray.size());
  out->setSourceSize(width, height, depth);
  int label = 0;
  for (ZObject3dArray::iterator iter = tmpArray.begin();
       iter != tmpArray.end(); ++iter, ++label) {
    ZObject3d *obj = *iter;
    if (obj != NULL) {
      obj->setLabel(label);
      (*out)[label] = obj;
    }
    *iter = NULL;
  }

  ZObject3dArray *out2 = NULL;

  if (option == OUTPUT_COMPACT) {
    out2 = new ZObject3dArray;
    for (ZObject3dArray::iterator iter = out->begin(); iter != out->end();
         ++iter) {
      ZObject3d *obj = *iter;
      if (!obj->isEmpty()) {
        out2->push_back(obj);
      } else {
        delete obj;
      }
      *iter = NULL;
    }
  }

  if (out2 != NULL) {
    delete out;
    out = out2;
  }

  delete mask;

  return out;
}

ZObject3dScan* ZObject3dFactory::MakeObject3dScan(
    const ZStack &stack, ZObject3dScan *out)
{
  if (out == NULL) {
    out = new ZObject3dScan;
  }

  out->clear();

  out->loadStack(stack);

  return out;
}

ZObject3dScan ZObject3dFactory::MakeObject3dScan(const ZStack &stack)
{
  ZObject3dScan obj;
  MakeObject3dScan(stack, &obj);

  return obj;
}

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    const ZStack &stack, int yStep)
{
  ZObject3dScanArray *objArray = NULL;

  if (stack.hasData()) {
    std::map<uint64_t, ZObject3dScan*> *bodySet =
        ZObject3dScan::extractAllObject(
          stack.array8(), stack.width(), stack.height(), stack.depth(), 0, yStep,
          NULL);
    objArray = new ZObject3dScanArray;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter) {
      ZObject3dScan *obj = iter->second;
      if (iter->first > 0) {
        obj->translate(stack.getOffset());
        obj->setLabel(iter->first);
        objArray->append(obj);
      } else {
        delete obj;
      }
    }
    delete bodySet;
  }

  return objArray;
}

//Add details from high-res bodies to low-res bodies
//The bodies have been scaled to the same scale
//Adjustment: Bl(i) - Bh(j!=i) + Bh(i)
void ZObject3dFactory::AdjustResolution(
      std::map<uint64_t, ZObject3dScan*> &lowResSet,
      std::map<uint64_t, ZObject3dScan*> &highResSet)
{
  for (auto &lowResIter : lowResSet) {
    uint64_t lowResBodyId = lowResIter.first;
    ZObject3dScan *lowResBody = lowResIter.second;
    for (auto &highResIter : highResSet) {
      uint64_t highResBodyId = highResIter.first;
      ZObject3dScan *highResBody = highResIter.second;
      if (lowResBodyId != highResBodyId) {
        lowResBody->subtractSliently(*highResBody);
      }
    }
    for (auto &highResIter : highResSet) {
      uint64_t highResBodyId = highResIter.first;
      ZObject3dScan *highResBody = highResIter.second;
      if (lowResBodyId == highResBodyId) {
        lowResBody->unify(*highResBody);
      }
    }
  }
}

std::map<uint64_t, ZObject3dScan*>* ZObject3dFactory::ExtractAllForegroundObject(
    ZStack &stack, bool upsampling)
{
  std::map<uint64_t, ZObject3dScan*> *bodySet =
      ZObject3dScan::extractAllForegroundObject(
        stack.array8(), stack.width(), stack.height(), stack.depth(),
        neutu::EAxis::Z);
  if (bodySet != NULL) {
    for (auto &bodyIter : *bodySet) {
      ZObject3dScan *body = bodyIter.second;
      body->translate(stack.getOffset());
      if (upsampling) {
        body->upSample(stack.getDsIntv());
      }
    }
  }

  return bodySet;
}

void ZObject3dFactory::DeleteObjectMap(std::map<uint64_t, ZObject3dScan *> *bodySet)
{
  if (bodySet != NULL) {
    for (auto &bodyIter : *bodySet) {
      ZObject3dScan *body = bodyIter.second;
      delete body;
    }
    delete bodySet;
  }
}

//Combine the object map by concatenating objects with the same ID
void ZObject3dFactory::CombineObjectMap(
    std::map<uint64_t, ZObject3dScan *> *masterBodySet,
    std::map<uint64_t, ZObject3dScan *> *bodySet)
{
  for (auto &body : *bodySet) {
    if (masterBodySet->count(body.first) > 0) {
      (*masterBodySet)[body.first]->concat(*(body.second));
    } else {
      (*masterBodySet)[body.first] = body.second;
    }
  }
}

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    ZStackArray &stackArray)
{
  ZObject3dScanArray *result = NULL;

  if (!stackArray.empty()) {
    std::unordered_map<uint64_t, ZObject3dScan*> objMap;

    stackArray.sort(zstack::DsIntvGreaterThan());
    ZStackPtr stack = stackArray[0];

    std::map<uint64_t, ZObject3dScan*> *bodySet =
        ExtractAllForegroundObject(*stack, true);

    std::map<uint64_t, ZObject3dScan*> *allHighResBodySet =
        new std::map<uint64_t, ZObject3dScan*>;
    for (size_t i = 1; i < stackArray.size(); ++i) {
      stack = stackArray[i];
      std::map<uint64_t, ZObject3dScan*> *highResBodySet =
          ExtractAllForegroundObject(*stack, true);
      CombineObjectMap(allHighResBodySet, highResBodySet);
      delete highResBodySet;
    }

    if (!allHighResBodySet->empty()) {
      AdjustResolution(*bodySet, *allHighResBodySet);
    }
    DeleteObjectMap(allHighResBodySet);

    result = new ZObject3dScanArray;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter) {
      ZObject3dScan *obj = iter->second;
      uint64_t label =iter->first;
      if (label > 0) {
        obj->setLabel(label);
        obj->setDsIntv(0);
        if (objMap.count(label) > 0) {
          objMap[label]->concat(*obj);
          delete obj;
        } else {
          result->append(obj);
          objMap[label] = obj;
        }
      } else {
        delete obj;
      }
    }
    delete bodySet;
  }

  return result;
}

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    const ZStack &stack, neutu::EAxis sliceAxis)
{
  ZObject3dScanArray *objArray = NULL;

  if (stack.hasData()) {
    std::map<uint64_t, ZObject3dScan*> *bodySet =
        ZObject3dScan::extractAllObject(
          stack.array8(), stack.width(), stack.height(), stack.depth(), sliceAxis);
    objArray = new ZObject3dScanArray;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter) {
      ZObject3dScan *obj = iter->second;
      if (iter->first > 0) {
        obj->translate(stack.getOffset());
        obj->setLabel(iter->first);
        objArray->append(obj);
      } else {
        delete obj;
      }
    }
  }

  return objArray;
}

std::vector<ZObject3dScan*> ZObject3dFactory::MakeObject3dScanPointerArray(
    const ZStack &stack, int yStep, bool boundaryOnly)
{
  std::vector<ZObject3dScan*> objArray;

  if (stack.hasData()) {
    const ZStack *mask = &stack;
    if (boundaryOnly) {
      mask = MakeBoundaryStack(stack);
    }

    std::map<uint64_t, ZObject3dScan*> *bodySet =
        ZObject3dScan::extractAllObject(
          mask->array8(), mask->width(), mask->height(), mask->depth(),
          0, yStep, NULL);
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter) {
      ZObject3dScan *obj = iter->second;
      if (iter->first > 0) {
        obj->translate(stack.getOffset());
        obj->setLabel(iter->first);
        objArray.push_back(obj);
      } else {
        delete obj;
      }
    }

    if (mask != &stack) {
      delete mask;
    }
  }

  return objArray;
}

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    const ZArray &array, int yStep, ZObject3dScanArray *out, bool foreground)
{
  if (out == NULL) {
    out = new ZObject3dScanArray;
  }

  std::map<uint64_t, ZObject3dScan*> *bodySet = NULL;

  if (array.valueType() == mylib::UINT64_TYPE) {
    if (foreground) {
      bodySet = ZObject3dScan::extractAllForegroundObject(
        array.getDataPointer<uint64_t>(), array.getDim(0), array.getDim(1),
            array.getDim(2), 0, 0, 0, yStep, NULL);
    } else {
      bodySet = ZObject3dScan::extractAllObject(
        array.getDataPointer<uint64_t>(), array.getDim(0), array.getDim(1),
            array.getDim(2), 0, 0, 0, yStep, NULL);
    }
  }

//  out->resize(bodySet->size());

  if (bodySet != NULL) {
    size_t index = 0;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter, ++index) {
      ZObject3dScan *obj = iter->second;
      obj->setLabel(iter->first);
      out->append(obj);

//      std::swap((*out)[index].getStripeArray(), obj->getStripeArray());
//      (*out)[index].setLabel(obj->getLabel());
//      out->push_back(*obj);

//      delete obj;
    }

    delete bodySet;
  } else {
    out = NULL;
  }

  return out;
}

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    const ZStack &stack, neutu::EAxis axis, bool foreground,
    ZObject3dScanArray *out)
{
  if (stack.kind() != GREY && stack.kind() != GREY16) {
    return NULL;
  }

  if (out == NULL) {
    out = new ZObject3dScanArray;
  }

  std::map<uint64_t, ZObject3dScan*> *bodySet = NULL;

  switch (stack.kind()) {
  case GREY:
    if (foreground) {
      bodySet = ZObject3dScan::extractAllForegroundObject(
            stack.array8(), stack.width(), stack.height(), stack.depth(), axis);
    } else {
      bodySet = ZObject3dScan::extractAllObject(
            stack.array8(), stack.width(), stack.height(), stack.depth(), axis);
    }
    break;
  case GREY16:
    if (foreground) {
      bodySet = ZObject3dScan::extractAllForegroundObject(
            stack.array16(), stack.width(), stack.height(), stack.depth(), axis);
    } else {
      bodySet = ZObject3dScan::extractAllObject(
            stack.array16(), stack.width(), stack.height(), stack.depth(), axis);
    }
    break;
  default:
    break;
  }


//  out->resize(bodySet->size());

  if (bodySet != NULL) {
    size_t index = 0;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter, ++index) {
      ZObject3dScan *obj = iter->second;
      obj->setLabel(iter->first);
      obj->setSliceAxis(axis);

      ZIntPoint offset = stack.getOffset();
      offset.shiftSliceAxis(axis);
      obj->translate(offset);

      out->append(obj);

//      ZObject3dScan &outObj = (*out)[index];

//      std::swap(outObj.getStripeArray(), obj->getStripeArray());
//      outObj.setLabel(obj->getLabel());
//      outObj.setSliceAxis(axis);
//      out->push_back(*obj);

//      delete obj;
    }

    delete bodySet;
  } else {
    out = NULL;
  }

  return out;
}


ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(
    const ZArray &array, neutu::EAxis axis, bool foreground,
    ZObject3dScanArray *out)
{
  if (out == NULL) {
    out = new ZObject3dScanArray;
  }

  std::map<uint64_t, ZObject3dScan*> *bodySet = NULL;

  if (array.valueType() == mylib::UINT64_TYPE) {
    if (foreground) {
      bodySet = ZObject3dScan::extractAllForegroundObject(
            array.getDataPointer<uint64_t>(), array.getDim(0), array.getDim(1),
            array.getDim(2), axis);
    } else {
      bodySet = ZObject3dScan::extractAllObject(
            array.getDataPointer<uint64_t>(), array.getDim(0), array.getDim(1),
            array.getDim(2), axis);
    }
  }

//  out->resize(bodySet->size());

  if (bodySet != NULL) {
    size_t index = 0;
    for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter, ++index) {
      ZObject3dScan *obj = iter->second;
      obj->setLabel(iter->first);
      obj->setSliceAxis(axis);
      out->append(obj);


//      ZObject3dScan &outObj = (*out)[index];

//      std::swap(outObj.getStripeArray(), obj->getStripeArray());
//      outObj.setLabel(obj->getLabel());
//      outObj.setSliceAxis(axis);
//      out->push_back(*obj);

//      delete obj;
    }

    delete bodySet;
  } else {
    out = NULL;
  }

  return out;
}

ZObject3dScan* ZObject3dFactory::MakeObject3dScan(
      const std::vector<ZArray*> labelArray, uint64_t v, ZObject3dScan *out)
{
  if (out == NULL) {
    out = new ZObject3dScan;
  }

  for (ZArray *array : labelArray) {
    ZIntCuboid box = misc::GetBoundBox(array);
    int x0 = box.getMinCorner().getX();
    int y0 = box.getMinCorner().getY();
    int z0 = box.getMinCorner().getZ();
    int y1 = box.getMaxCorner().getY();
    int z1 = box.getMaxCorner().getZ();
//    int area = box.getWidth() * box.getHeight();
    size_t offset = 0;
    for (int z = z0; z <= z1; ++z) {
      for (int y = y0; y <= y1; ++y) {
        out->scanArrayV(
              array->getDataPointer<uint64_t>() + offset,
              x0, y, z, box.getWidth(), v);
        offset+= box.getWidth();
      }
    }
  }

  return out;
}

ZObject3dScan* ZObject3dFactory::MakeObject3dScan(
    const std::vector<ZArray*> labelArray, uint64_t v,
    const ZIntCuboid &range, ZObject3dScan *out)
{
  if (out == NULL) {
    out = new ZObject3dScan;
  }

  for (ZArray *array : labelArray) {
    const ZIntCuboid box = misc::GetBoundBox(array);
    int x0 = box.getMinCorner().getX();
    int y0 = box.getMinCorner().getY();
    int z0 = box.getMinCorner().getZ();
    int y1 = box.getMaxCorner().getY();
    int z1 = box.getMaxCorner().getZ();
    size_t offset = 0;
    if (range.hasOverlap(box)) {
      if (range.contains(box)) {
        for (int z = z0; z <= z1; ++z) {
          for (int y = y0; y <= y1; ++y) {
            out->scanArrayV(
                  array->getDataPointer<uint64_t>() + offset,
                  x0, y, z, box.getWidth(), v);
            offset+= box.getWidth();
          }
        }
      } else {
        ZIntCuboid newBox = box;
        newBox.intersect(range);
        int nx0 = newBox.getMinCorner().getX();
        int ny0 = newBox.getMinCorner().getY();
        int nz0 = newBox.getMinCorner().getZ();
        int ny1 = newBox.getMaxCorner().getY();
        int nz1 = newBox.getMaxCorner().getZ();
        int area = box.getWidth() * box.getHeight();
        int dx = nx0 - x0;
        for (int z = nz0; z <= nz1; ++z) {
          for (int y = ny0; y <= ny1; ++y) {
            out->scanArrayV(
                  array->getDataPointer<uint64_t>() + area * (z - z0) +
                  box.getWidth() * (y - y0) + dx,
                  nx0, y, z, newBox.getWidth(), v);
          }
        }
      }
    }
  }

  return out;
}

ZObject3dScan* ZObject3dFactory::MakeFilledMask(
    const ZClosedCurve &curve, int z, ZObject3dScan *result)
{
  if (result != NULL) {
    result->clear();
  }

#if defined(_QT_GUI_USED_)
  if (!curve.isEmpty()) {
    ZStroke2d stroke;
    stroke.setZ(z);
    stroke.setWidth(1.0);
    for (size_t i = 0; i < curve.getLandmarkNumber(); ++i) {
      ZPoint pt = curve.getLandmark(i);
      stroke.append(pt.x(), pt.y());
    }
    ZStack *stack = ZStackFactory::makePolygonPicture(stroke);
    if (stack != NULL) {
      if (result == NULL) {
        result = new ZObject3dScan;
      }

      result->loadStack(*stack);
      delete stack;
    }
  }
#endif

  return result;
}

ZObject3dScan ZObject3dFactory::MakeObject3dScan(const ZIntCuboid &box)
{
  ZObject3dScan obj;
  int z0 = box.getMinCorner().getZ();
  int z1 = box.getMaxCorner().getZ();
  int y0 = box.getMinCorner().getY();
  int y1 = box.getMaxCorner().getY();
  int x0 = box.getMinCorner().getX();
  int x1 = box.getMaxCorner().getX();
  for (int z = z0; z <= z1; ++z) {
    for (int y = y0; y <= y1; ++y) {
      obj.addSegment(z, y, x0, x1, false);
    }
  }
  obj.setCanonized(true);

  return obj;
}

ZObject3dScan ZObject3dFactory::MakeRandomObject3dScan(const ZIntCuboid &box)
{
  ZRandomGenerator &rnd = ZRandomGenerator::GetInstance();

  ZObject3dScan obj;
  int nSeg = rnd.rndint(box.getHeight() * box.getDepth());
  for (int i = 0; i < nSeg; ++i) {
    int z = rnd.rndint(box.getMinCorner().getZ(), box.getMaxCorner().getZ());
    int y = rnd.rndint(box.getMinCorner().getY(), box.getMaxCorner().getY());
    int x1 =rnd.rndint(box.getMinCorner().getX(), box.getMaxCorner().getX());
    int x2 =rnd.rndint(box.getMinCorner().getX(), box.getMaxCorner().getX());
    if (x1 > x2) {
      std::swap(x1, x2);
    }

    obj.addSegment(z, y, x1, x2, false);
  }

  obj.canonize();

  return obj;
}

std::vector<ZObject3d*> ZObject3dFactory::MakeObject3dArray(const ZStack &stack)
{
  std::map<int, ZObject3d*> objMap;
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();
  int x0 = stack.getOffset().getX();
  int y0 = stack.getOffset().getY();
  int z0 = stack.getOffset().getZ();
  int i = 0;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int v = stack.getIntValue(i);
        if (v > 0) {
          ZObject3d *obj = NULL;
          if (objMap.count(v) == 0) {
            obj = new ZObject3d;
            obj->setLabel(v);
            objMap[v] = obj;
          } else {
            obj = objMap[v];
          }

          obj->append(x + x0, y + y0, z + z0);
        }
        ++i;
      }
    }
  }

  std::vector<ZObject3d*> objArray;
  for (std::map<int, ZObject3d*>::iterator iter = objMap.begin();
       iter != objMap.end(); ++iter) {
    objArray.push_back(iter->second);
  }

  return objArray;
}

ZObject3dScan* ZObject3dFactory::MakeBoxObject3dScan(
    const ZIntCuboid &box, ZObject3dScan *obj)
{
  if (obj == NULL) {
    obj = new ZObject3dScan;
  } else {
    obj->clear();
  }

  int x0 = box.getMinCorner().getX();
  int y0 = box.getMinCorner().getY();
  int z0 = box.getMinCorner().getZ();

  int x1 = box.getMaxCorner().getX();
  int y1 = box.getMaxCorner().getY();
  int z1 = box.getMaxCorner().getZ();

  for (int z = z0; z <= z1; ++z) {
    for (int y = y0; y <= y1; ++y) {
      obj->addSegment(z, y, x0, x1, false);
    }
  }
  obj->setCanonized(true);

  return obj;
}

ZObject3d* ZObject3dFactory::MakeBoxObject3d(
    const ZIntCuboid &box, ZObject3d *obj)
{
  if (obj == NULL) {
    obj = new ZObject3d;
  } else {
    obj->clear();
  }

  int x0 = box.getMinCorner().getX();
  int y0 = box.getMinCorner().getY();
  int z0 = box.getMinCorner().getZ();

  int x1 = box.getMaxCorner().getX();
  int y1 = box.getMaxCorner().getY();
  int z1 = box.getMaxCorner().getZ();

  for (int z = z0; z <= z1; ++z) {
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x < x1; ++x) {
        obj->append(x, y, z);
      }
    }
  }

  return obj;
}

#if defined(_QT_GUI_USED_)
ZObject3dScan ZObject3dFactory::MakeObject3dScan(const ZStroke2d &stroke)
{
  ZObject3dScan obj;

  ZStack *stack = stroke.toStack();
  MakeObject3dScan(*stack, &obj);
  delete stack;

  obj.setLabel(stroke.getLabel());

  return obj;
}
#endif
