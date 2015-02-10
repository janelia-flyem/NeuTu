#include "zobject3dfactory.h"
#include "zstack.hxx"
#include "zstackfactory.h"
#include "zobject3d.h"
#include "zobject3darray.h"
#include "zobject3dscan.h"
#include "neutubeconfig.h"
#include "zobject3dscanarray.h"
#include "zstackfactory.h"
#include "zclosedcurve.h"
#include "zstroke2d.h"

ZObject3dFactory::ZObject3dFactory()
{
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
  ZStack *mask = ZStackFactory::makeZeroStack(
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

  //y scan
  for (int z = 0; z < depth; ++z) {
    for (int x = 0; x < width; ++x) {
      offset = area * z + x + width;
      for (int y = 1; y < height - 1; ++y) {
        if (originalStack->array[offset] != originalStack->array[offset - width] ||
            originalStack->array[offset] != originalStack->array[offset + width]) {
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
        if (originalStack->array[offset] != originalStack->array[offset - area] ||
            originalStack->array[offset] != originalStack->array[offset + area]) {
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
          int v = originalStack->array[offset];
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

ZObject3dScanArray* ZObject3dFactory::MakeObject3dScanArray(const ZStack &stack)
{
  ZObject3dScanArray *objArray = NULL;

  if (stack.hasData()) {
    std::map<int, ZObject3dScan*> *bodySet =
        ZObject3dScan::extractAllObject(
          stack.array8(), stack.width(), stack.height(), stack.depth(), 0, NULL);
    objArray = new ZObject3dScanArray;
    for (std::map<int, ZObject3dScan*>::const_iterator iter = bodySet->begin();
         iter != bodySet->end(); ++iter) {
      ZObject3dScan *obj = iter->second;
      if (iter->first > 0) {
        obj->translate(stack.getOffset());
        obj->setLabel(iter->first);
        objArray->push_back(*obj);
      }
      delete obj;
    }
  }

  return objArray;
}

ZObject3dScan* ZObject3dFactory::MakeFilledMask(
    const ZClosedCurve &curve, int z, ZObject3dScan *result)
{
  if (result != NULL) {
    result->clear();
  }

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

  return result;
}
