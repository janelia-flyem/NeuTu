#include "zobject3d.h"
#if _QT_GUI_USED_
#include <QtGui>
#endif

#include <string.h>
#include <fstream>
#include <algorithm>
#include "tz_stack_attribute.h"
#include "tz_error.h"
#include "tz_voxel_graphics.h"
#include "tz_tvoxel.h"
#include "tz_math.h"
#include "c_stack.h"
#include "zobject3darray.h"
#include "zstack.hxx"
#include "zpainter.h"
#include "zcuboid.h"

using namespace std;

ZObject3d::ZObject3d(Object_3d *obj) : m_conn(0), m_label(-1),
  m_hitVoxelIndex(-1)
{
  if (obj != NULL) {
    m_voxelArray.resize(obj->size * 3);
    for (size_t i = 0; i < obj->size; i++) {
      set(i, obj->voxels[i]);
    }
  }

  setTarget(TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_OBJ3D;
}

ZObject3d::ZObject3d(const vector<size_t> &indexArray, int width, int height,
                     int dx, int dy, int dz) : m_conn(0), m_label(-1),
  m_hitVoxelIndex(-1)
{
  setSize(indexArray.size());

  size_t i = 0;
  for (vector<size_t>::const_iterator iter = indexArray.begin();
       iter != indexArray.end(); ++iter, ++i) {
    set(i, *iter, width, height, dx, dy, dz);
  }

  setTarget(TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_OBJ3D;
}

ZObject3d::~ZObject3d()
{
  clear();
}

void ZObject3d::set(int index, int x, int y, int z)
{
  index *= 3;
  m_voxelArray[index] = x;
  m_voxelArray[index + 1] = y;
  m_voxelArray[index + 2] = z;
}

void ZObject3d::set(int index, Voxel_t voxel)
{
  index *= 3;
  m_voxelArray[index] = voxel[0];
  m_voxelArray[index + 1] = voxel[1];
  m_voxelArray[index + 2] = voxel[2];
}

void ZObject3d::set(int index, size_t voxelIndex, int width, int height,
                    int dx, int dy, int dz)
{
  int x, y, z;
  Stack_Util_Coord(voxelIndex, width, height, &x, &y, &z);
  set(index, x + dx, y + dy, z + dz);
}

void ZObject3d::append(int x, int y, int z)
{
  m_voxelArray.push_back(x);
  m_voxelArray.push_back(y);
  m_voxelArray.push_back(z);
}

void ZObject3d::append(const ZObject3d &obj, size_t srcOffset)
{
  m_voxelArray.insert(m_voxelArray.end(),
                      obj.m_voxelArray.begin() + srcOffset * 3,
                      obj.m_voxelArray.end());
}

void ZObject3d::appendBackward(const ZObject3d &obj, size_t srcOffset)
{
  size_t voxelNumber = obj.size();
  if (srcOffset >= voxelNumber) {
    return;
  }

  size_t dstOffset = obj.size();
  m_voxelArray.resize(m_voxelArray.size() + obj.size() - srcOffset * 3);
  for (size_t i = voxelNumber - srcOffset; i > 0; --i) {
    set(dstOffset++, obj.getX(i - 1), obj.getY(i - 1), obj.getZ(i - 1));
  }
}

Object_3d* ZObject3d::c_obj() const
{
  m_objWrapper.conn = m_conn;
  m_objWrapper.voxels = (Voxel_t *) (&(m_voxelArray[0]));
  m_objWrapper.size = size();

  return &m_objWrapper;
}

void ZObject3d::setFromCObj(const Object_3d *obj)
{
  m_voxelArray.resize(obj->size * 3);
  size_t offset = 0;
  for (size_t i = 0; i < obj->size; ++i) {
    for (int j = 0; j < 3; ++j) {
      m_voxelArray[offset++] = obj->voxels[i][j];
    }
  }
}

void ZObject3d::setLine(ZPoint start, ZPoint end)
{
  Voxel_t startVoxel;
  Voxel_t endVoxel;

  Set_Tvoxel(startVoxel,
             iround(start.x()), iround(start.y()), iround(start.z()));
  Set_Tvoxel(endVoxel,
             iround(end.x()), iround(end.y()), iround(end.z()));

  Object_3d *obj = Line_To_Object_3d(startVoxel, endVoxel);

  clear();
  for (size_t i = 0; i < OBJECT_3D_SIZE(obj); ++i) {
    append(OBJECT_3D_X(obj, i), OBJECT_3D_Y(obj, i), OBJECT_3D_Z(obj, i));
  }

  Kill_Object_3d(obj);
}

void ZObject3d::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

bool ZObject3d::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);

  return false;
}

void ZObject3d::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    NeuTube::EAxis sliceAxis) const
{  
  UNUSED_PARAMETER(option);

  if (slice < 0 && !isProjectionVisible()) {
    return;
  }

#if _QT_GUI_USED_
  painter.save();
//  z -= iround(painter.getOffset().z());
  int z = slice + iround(painter.getZOffset());

  QPen pen(m_color);
  painter.setPen(pen);
  Object_3d *obj= c_obj();
  std::vector<QPoint> pointArray;

  switch (sliceAxis) {
  case NeuTube::Z_AXIS:
    if (slice < 0) {
      for (size_t i = 0; i < obj->size; i++) {
        pointArray.push_back(QPoint(obj->voxels[i][0], obj->voxels[i][1]));
      }
    } else {
      for (size_t i = 0; i < obj->size; i++) {
        if (obj->voxels[i][2] == z) {
          pointArray.push_back(QPoint(obj->voxels[i][0], obj->voxels[i][1]));
        }
      }
    }
    break;
  case NeuTube::X_AXIS:
  case NeuTube::Y_AXIS:
    if (slice < 0) {
      for (size_t i = 0; i < obj->size; i++) {
        ZIntPoint pt(obj->voxels[i][0], obj->voxels[i][1], obj->voxels[i][2]);
        pt.shiftSliceAxis(sliceAxis);
        pointArray.push_back(QPoint(pt.getX(), pt.getY()));
      }
    } else {
      for (size_t i = 0; i < obj->size; i++) {
        ZIntPoint pt(obj->voxels[i][0], obj->voxels[i][1], obj->voxels[i][2]);
        pt.shiftSliceAxis(sliceAxis);
        if (pt.getZ() == z) {
          pointArray.push_back(QPoint(pt.getX(), pt.getY()));
        }
      }
    }
    break;
  }

  painter.drawPoints(&(pointArray[0]), pointArray.size());

  if (isSelected()) {
    QColor color;
    color.setRedF(m_color.redF() * 0.5 + 0.5);
    color.setGreenF(m_color.greenF() * 0.5 + 0.5);
    color.setBlueF(m_color.blueF() * 0.5 + 0.5);
    QPen pen(color);
    painter.setPen(pen);
    for (std::vector<QPoint>::iterator iter = pointArray.begin();
         iter != pointArray.end(); ++iter) {
      QPoint &pt = *iter;
      pt += QPoint(1, 0);
    }
    painter.drawPoints(&(pointArray[0]), pointArray.size());

    for (std::vector<QPoint>::iterator iter = pointArray.begin();
         iter != pointArray.end(); ++iter) {
      QPoint &pt = *iter;
      pt -= QPoint(1, 0);
    }
    painter.drawPoints(&(pointArray[0]), pointArray.size());
  }
  painter.restore();
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(slice);
  UNUSED_PARAMETER(option);
#endif
}

void ZObject3d::labelStack(Stack *stack) const
{
  labelStack(stack, m_label);
}

void ZObject3d::labelStack(Stack *stack, int label) const
{
  Object_3d *obj = c_obj();

  Image_Array ima;
  ima.array = stack->array;

  switch (Stack_Kind(stack)) {
  case GREY:
    label = (label - 1) % 255 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ima.array8[Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],
                                   obj->voxels[i][2], stack->width,
                                   stack->height, stack->depth)] = label;
    }
    break;
  case GREY16:
    label = (label - 1) % 65535 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ima.array16[Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],
                                    obj->voxels[i][2], stack->width,
                                    stack->height, stack->depth)] = label;
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

void ZObject3d::labelStack(ZStack *stack, int label) const
{
  labelStack(stack->c_stack(), label, stack->getOffset().getX(),
             stack->getOffset().getY(), stack->getOffset().getZ());
}
void ZObject3d::labelStack(ZStack *stack) const
{
  labelStack(stack, m_label);
}

void ZObject3d::labelStack(Stack *stack, int label, int dx, int dy, int dz) const
{
  Object_3d *obj = c_obj();

  Image_Array ima;
  ima.array = stack->array;

  switch (Stack_Kind(stack)) {
  case GREY:
    label = (label - 1) % 255 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ssize_t index = Stack_Util_Offset(obj->voxels[i][0] + dx,
          obj->voxels[i][1] + dy,
          obj->voxels[i][2] + dz, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array8[index] = label;
      }
    }
    break;
  case GREY16:
    label = (label - 1) % 65535 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ssize_t index = Stack_Util_Offset(obj->voxels[i][0] + dx,
          obj->voxels[i][1] + dy,
          obj->voxels[i][2] + dz, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array16[index] = label;
      }
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

void ZObject3d::labelStack(Stack *stack, int label, int dx, int dy, int dz,
                           int xIntv, int yIntv, int zIntv) const
{
  Object_3d *obj = c_obj();

  Image_Array ima;
  ima.array = stack->array;

  int rx = xIntv + 1;
  int ry = yIntv + 1;
  int rz = zIntv + 1;

  ZIntPoint offset(dx, dy, dz);

  switch (Stack_Kind(stack)) {
  case GREY:
    label = (label - 1) % 255 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      int x = (obj->voxels[i][0] + offset[0]) / rx;
      int y = (obj->voxels[i][1] + offset[1]) / ry;
      int z = (obj->voxels[i][2] + offset[2]) / rz;
      ssize_t index = Stack_Util_Offset(x, y, z, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array8[index] = label;
      }
    }
    break;
  case GREY16:
    label = (label - 1) % 65535 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      int x = (obj->voxels[i][0] + offset[0]) / rx;
      int y = (obj->voxels[i][1] + offset[1]) / ry;
      int z = (obj->voxels[i][2] + offset[2]) / rz;
      ssize_t index = Stack_Util_Offset(x, y, z, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array16[index] = label;
      }
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

ZPoint ZObject3d::computeCentroid(FMatrix *matrix)
{
  ZPoint center(0.0, 0.0, 0.0);
  double totalWeight = 0.0;

  vector<size_t> indexArray = toIndexArray<size_t>(matrix->dim[0], matrix->dim[1],
                                           matrix->dim[2]);
  for (size_t i = 0; i < size(); i++) {
    /*
    double weight = matrix->array[
        Stack_Util_Offset(x(i) - 1, y(i) - 1, z(i) - 1,
                          )];
                          */
    double weight = matrix->array[indexArray[i]];
    center += ZPoint(getX(i) * weight, getY(i) * weight, getZ(i) * weight);
    totalWeight += weight;
  }

  center /= totalWeight;

  return center;
}

void ZObject3d::translate(int dx, int dy, int dz)
{
  for (size_t i = 0; i < size(); i++) {
    set(i, getX(i) + dx, getY(i) + dy, getZ(i) + dz);
  }
}

void ZObject3d::translate(const ZIntPoint &pt)
{
  translate(pt.getX(), pt.getY(), pt.getZ());
}

void ZObject3d::exportSwcFile(string filePath)
{
  ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); i++) {
    stream << i + 1 << " " << 2 << " " << getX(i) << " " << getY(i) << " " << getZ(i)
           << " " << 3.0 << " " << -1 << endl;
  }

  stream.close();
}

void ZObject3d::exportCsvFile(string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "w");

  Object_3d_Csv_Fprint(c_obj(), fp);

  fclose(fp);
}

ZObject3d* ZObject3d::clone() const
{
  ZObject3d *obj = new ZObject3d();

  *obj = *this;

  return obj;
}

double ZObject3d::averageIntensity(const Stack *stack) const
{
  double mu = 0.0;

  for (size_t i = 0; i < size(); ++i) {
    mu += Get_Stack_Pixel(const_cast<Stack*>(stack), getX(i), getY(i), getZ(i), 0);
    //mu += stack->value(x(i), y(i), z(i));
  }

  mu /= size();

  return mu;
}

void ZObject3d::print()
{
  cout << "3d object (" << size() << " voxels):" << endl;
  for (size_t i = 0; i < size(); ++i) {
    cout << "  " << getX(i) << " " << getY(i) << " " << getZ(i) << endl;
  }
}

ZStack* ZObject3d::toStackObject() const
{
  int offset[3];
  Stack *stack = toStack(offset);
  ZStack *stackObject = NULL;

  if (stack != NULL) {
    stackObject = new ZStack;
    stackObject->consume(stack);
    stackObject->setOffset(offset[0], offset[1], offset[2]);
  }

  return stackObject;
}

Stack* ZObject3d::toStack(int *offset) const
{
  Object_3d *obj = c_obj();

  int corners[6];
  Object_3d_Range(obj, corners);

  if (offset != NULL) {
    memcpy(offset, corners, sizeof(int) * 3);
  }

  int width = corners[3] - corners[0] + 1;
  int height = corners[4] - corners[1] + 1;
  int depth = corners[5] - corners[2] + 1;

  Stack *stack = C_Stack::make(GREY, width, height, depth);
  C_Stack::setZero(stack);

  for (size_t i = 0; i < obj->size; ++i) {
    int x = obj->voxels[i][0] - corners[0];
    int y = obj->voxels[i][1] - corners[1];
    int z = obj->voxels[i][2] - corners[2];

    stack->array[Stack_Util_Offset(x, y, z, width, height, depth)] = 1;
  }

  return stack;
}

void ZObject3d::drawStack(ZStack *stack) const
{
  Object_3d *obj = c_obj();

  uint8_t color[3] = {255, 255, 255};
#if _QT_GUI_USED_
  color[0] = m_color.red();
  color[1] = m_color.green();
  color[2] = m_color.blue();
#endif

  ZIntPoint origin = stack->getOffset();
  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();

  for (int c = 0;  c < 3; ++c) {
    Stack *cstack = stack->c_stack(c);
    for (size_t i = 0; i < obj->size; ++i) {
      int x = obj->voxels[i][0] - origin[0];
      int y = obj->voxels[i][1] - origin[1];
      int z = obj->voxels[i][2] - origin[2];
      ssize_t offset = Stack_Util_Offset(x, y, z, width, height, depth);
      if (offset >= 0) {
        cstack->array[offset] = color[c];
      }
    }
  }
}

void ZObject3d::drawStack(ZStack *stack, int xIntv, int yIntv, int zIntv) const
{
  Object_3d *obj = c_obj();

  uint8_t color[3] = {255, 255, 255};
#ifdef _QT_GUI_USED_
  color[0] = m_color.red();
  color[1] = m_color.green();
  color[2] = m_color.blue();
#endif
  ZIntPoint origin = stack->getOffset();
  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();

  int rx = xIntv + 1;
  int ry = yIntv + 1;
  int rz = zIntv + 1;

  for (int c = 0;  c < 3; ++c) {
    Stack *cstack = stack->c_stack(c);
    for (size_t i = 0; i < obj->size; ++i) {
      int x = obj->voxels[i][0] - origin[0];
      int y = obj->voxels[i][1] - origin[1];
      int z = obj->voxels[i][2] - origin[2];
      ssize_t offset = Stack_Util_Offset(
            x / rx, y / ry, z / rz, width, height, depth);
      if (offset >= 0) {
        cstack->array[offset] = color[c];
      }
    }
  }
}

void ZObject3d::drawStack(const std::vector<Stack*> &stackArray,
                          const int *offset,
                          int xIntv, int yIntv, int zIntv) const
{
  Object_3d *obj = c_obj();

  uint8_t color[3] = {255, 255, 255};
#ifdef _QT_GUI_USED_
  color[0] = m_color.red();
  color[1] = m_color.green();
  color[2] = m_color.blue();
#endif

  ZIntPoint origin(0, 0, 0);
  if (offset != NULL) {
    origin.set(offset[0], offset[1], offset[2]);
  }
  int width = C_Stack::width(stackArray[0]);
  int height = C_Stack::height(stackArray[0]);
  int depth = C_Stack::depth(stackArray[0]);

  int rx = xIntv + 1;
  int ry = yIntv + 1;
  int rz = zIntv + 1;

  for (size_t c = 0; c < stackArray.size(); ++c) {
    Stack *cstack = stackArray[c];
    for (size_t i = 0; i < obj->size; ++i) {
      int x = obj->voxels[i][0] / rx - origin[0];
      int y = obj->voxels[i][1] / ry - origin[1];
      int z = obj->voxels[i][2] / rz - origin[2];
      ssize_t offset = Stack_Util_Offset(x, y, z, width, height, depth);
      if (offset >= 0) {
        cstack->array[offset] = color[c];
      }
    }
  }
}

ZStack* ZObject3d::toLabelStack() const
{
  Object_3d *obj = c_obj();

  int corners[6];
  Object_3d_Range(obj, corners);

  int offset[3];
  memcpy(offset, corners, sizeof(int) * 3);

  int width = corners[3] - corners[0] + 1;
  int height = corners[4] - corners[1] + 1;
  int depth = corners[5] - corners[2] + 1;

  Stack *stack = C_Stack::make(GREY, width, height, depth);
  C_Stack::setZero(stack);

  int label = m_label;
  CLIP_VALUE(label, 0, 255);
  for (size_t i = 0; i < obj->size; ++i) {
    int x = obj->voxels[i][0] - corners[0];
    int y = obj->voxels[i][1] - corners[1];
    int z = obj->voxels[i][2] - corners[2];

    stack->array[Stack_Util_Offset(x, y, z, width, height, depth)] = label;
  }

  ZStack *stackObject = NULL;

  if (stack != NULL) {
    stackObject = new ZStack;
    stackObject->consume(stack);
    stackObject->setOffset(offset[0], offset[1], offset[2]);
  }

  return stackObject;
}

ZObject3dArray* ZObject3d::growLabel(const ZObject3d &seed, int growLevel)
{
  int offset[3];

  Stack *stack = toStack(offset);
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  vector<size_t> currentSeed;

  for (size_t i = 0; i < seed.size(); ++i) {
    int x = seed.getX(i) - offset[0];
    int y = seed.getY(i) - offset[1];
    int z = seed.getZ(i) - offset[2];

    ssize_t index = Stack_Util_Offset(x, y, z, width, height, depth);

    if (index >= 0) {
      currentSeed.push_back(index);
    }
  }

  ZObject3dArray *objArray = new ZObject3dArray;

  //While the current seed is not empty
  while (!currentSeed.empty()) {
    //Update the label
    ZObject3d *obj =
        new ZObject3d(currentSeed, width, height, offset[0], offset[1], offset[2]);

#ifdef _DEBUG_2
    Print_Object_3d(obj->c_obj());
#endif

    objArray->push_back(obj);

    if (growLevel >= 0) {
      if (objArray->size() > (size_t) growLevel) {
        break;
      }
    }

    C_Stack::setStackValue(stack, currentSeed, 0);

    //Update the seed
    currentSeed = C_Stack::getNeighborIndices(stack, currentSeed, 26, 1);
  }

  C_Stack::kill(stack);

  return objArray;
}

void ZObject3d::getRange(int *corner) const
{
  Object_3d_Range(c_obj(), corner);
}

ZPoint ZObject3d::getCenter() const
{
  ZPoint center;

  double pos[3];
  Object_3d_Centroid(c_obj(), pos);

  center.set(pos[0], pos[1], pos[2]);

  return center;
}

double ZObject3d::getRadius() const
{
  double radius = 0.0;

  if (size() > 1) {
    ZPoint center = getCenter();
    Object_3d *obj = c_obj();
    for (size_t i = 0; i < obj->size; ++i) {
      radius += center.distanceTo(obj->voxels[i][0], obj->voxels[i][1],
          obj->voxels[i][2]);
    }
    radius /= size();
  }

  return radius;
}

bool ZObject3d::isEmpty() const
{
  return m_voxelArray.empty();
}

bool ZObject3d::loadStack(const Stack *stack, int threshold)
{
  clear();

  if (stack == NULL) {
    return false;
  }

  if (C_Stack::kind(stack) != GREY) {
    return false;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  size_t offset = 0;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        if (stack->array[offset] > threshold) {
          append(x, y, z);
        }
        ++offset;
      }
    }
  }

  if (isEmpty()) {
    return false;
  }

  return true;
}

bool ZObject3d::loadStack(const ZStack *stack, int threshold)
{
  clear();

  if (stack == NULL) {
    return false;
  }

  if (stack->kind() != GREY) {
    return false;
  }

  ZIntPoint origin = stack->getOffset();

  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();

  size_t offset = 0;
  const Stack* stackData = stack->c_stack();
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        if (stackData->array[offset] > threshold) {
          append(x + origin.getX(), y + origin.getY(), z + origin.getZ());
        }
        ++offset;
      }
    }
  }

  if (isEmpty()) {
    return false;
  }

  return true;
}

void ZObject3d::duplicateAcrossZ(int depth)
{
  size_t originalSize = m_voxelArray.size();


  m_voxelArray.resize(m_voxelArray.size() * depth);

  for (int i = 1; i < depth; ++i) {
    copy(m_voxelArray.begin(), m_voxelArray.begin() + originalSize - 1,
         m_voxelArray.begin() + originalSize * i);
  }

  for (size_t i = 2; i < m_voxelArray.size(); i += 3) {
    m_voxelArray[i] = i / originalSize;
  }
}

void ZObject3d::reverse()
{
  size_t voxelNumber = size();
  if (voxelNumber >= 2) {
    size_t halfSize = voxelNumber / 2;
    int tmp;
    for (size_t i = 0; i < halfSize; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        SWAP2(m_voxelArray[i * 3 + j],
            m_voxelArray[(voxelNumber - i - 1) * 3 + j], tmp);
      }
    }
  }
}

void ZObject3d::upSample(int xIntv, int yIntv, int zIntv)
{
  int rx = xIntv + 1;
  int ry = yIntv + 1;
  int rz = zIntv + 1;
  size_t originalSize = m_voxelArray.size();
  std::vector<int> voxelArray = m_voxelArray;
  m_voxelArray.resize(originalSize * rx * ry * rz);

  size_t offset = 0;
  for (int dz = 0; dz <= zIntv; ++dz) {
    for (int dy = 0; dy <= yIntv; ++dy) {
      for (int dx = 0; dx <= xIntv; ++dx) {
        for (size_t i = 0; i < voxelArray.size(); i += 3) {
          m_voxelArray[offset] = voxelArray[i] * rx + dx;
          m_voxelArray[offset + 1] = voxelArray[i + 1] * ry + dy;
          m_voxelArray[offset + 2] = voxelArray[i + 2] * rz + dz;
          offset += 3;
        }
      }
    }
  }
}

ZJsonObject ZObject3d::toJsonObject() const
{
  ZJsonObject jsonObj;
  jsonObj.setEntry("label", m_label);
  ZJsonArray jsonArray;
  for (size_t i = 0; i < m_voxelArray.size(); ++i) {
    jsonArray.append(m_voxelArray[i]);
  }
  jsonObj.setEntry("obj3d", jsonArray);

  return jsonObj;
}

void ZObject3d::loadJsonObject(const ZJsonObject &jsonObj)
{
  clear();
  if (jsonObj.hasKey("label")) {
    setLabel(ZJsonParser::integerValue(jsonObj["label"]));
  }

  if (jsonObj.hasKey("obj3d")) {
    ZJsonArray voxelArray(jsonObj["obj3d"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < voxelArray.size(); ++i) {
      m_voxelArray.push_back(ZJsonParser::integerValue(voxelArray.at(i)));
    }
  }
}

bool ZObject3d::hit(double x, double y)
{
  m_hitVoxelIndex = -1;

  if (!isHittable()) {
    return false;
  }

  int ix = iround(x);
  int iy = iround(y);

  for (size_t i = 0; i < size(); ++i) {
    if (ix == getX(i) && iy == getY(i)) {
      m_hitVoxelIndex = i;
      return true;
    }
  }

  return false;
}

bool ZObject3d::hit(double x, double y, double z)
{
  m_hitVoxelIndex = -1;

  if (!isHittable()) {
    return false;
  }

  int ix = iround(x);
  int iy = iround(y);
  int iz = iround(z);

  for (size_t i = 0; i < size(); ++i) {
    if (ix == getX(i) && iy == getY(i) && iz == getZ(i)) {
      m_hitVoxelIndex = i;
      return true;
    }
  }

  return false;
}

bool ZObject3d::hasHitVoxel() const
{
  return m_hitVoxelIndex >= 0 && m_hitVoxelIndex < (int) size();
}

ZIntPoint ZObject3d::getHitVoxel() const
{
  ZIntPoint pt;
  if (hasHitVoxel()) {
    pt.set(getX(m_hitVoxelIndex), getY(m_hitVoxelIndex), getZ(m_hitVoxelIndex));
  }

  return pt;
}

void ZObject3d::getBoundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    if (!isEmpty()) {
      box->setFirstCorner(getX(0), getY(0), getZ(0));
      box->setLastCorner(getX(0), getY(0), getZ(0));
      box->set(getX(0), getY(0), getZ(0), getX(0), getY(0), getZ(0));
    }
    for (size_t i = 1; i < size(); i++) {
      box->joinX(getX(i));
      box->joinY(getY(i));
      box->joinZ(getZ(i));
    }
  }
}

ZIntPoint ZObject3d::getCentralVoxel() const
{
  Voxel_t center;
  Object_3d_Central_Voxel(c_obj(), center);

  return ZIntPoint(center[0], center[1], center[2]);
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZObject3d)
