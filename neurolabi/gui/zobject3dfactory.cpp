#include "zobject3dfactory.h"
#include "zstack.hxx"
#include "zstackfactory.h"

ZObject3dFactory::ZObject3dFactory()
{
}

ZObject3dArray* ZObject3dFactory::makeRegionBoundary(const ZStack &stack)
{
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
          offset++;
        }
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
          offset += width;
        }
      }
    }
  }

  //z scan
  for (int y = 0; y < depth; ++y) {
    for (int x = 0; x < width; ++x) {
      offset = width * y + x + area;
      for (int z = 1; z < depth - 1; ++z) {
        if (originalStack->array[offset] != originalStack->array[offset - area] ||
            originalStack->array[offset] != originalStack->array[offset + area]) {
          maskStack->array[offset] = 1;
          offset += area;
        }
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
            obj->append(x, y, z);
          }
        }
        ++offset;
      }
    }
  }

  //Create output array
  ZObject3dArray *out = new ZObject3dArray;
  out->setSourceSize(width, height, depth);
  int label = 0;
  for (ZObject3dArray::iterator iter = tmpArray.begin();
       iter != tmpArray.end(); ++iter, ++label) {
    ZObject3d *obj = *iter;
    if (!obj->isEmpty()) {
      obj->setLabel(label);
      out->push_back(obj);
    } else {
      delete obj;
    }
    *iter = NULL;
  }

  delete mask;

  return out;
}
