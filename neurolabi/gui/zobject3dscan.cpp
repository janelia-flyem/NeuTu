#include "zobject3dscan.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <cstring>
#if _QT_GUI_USED_
#include <QtGui>
#endif

#include "zobject3d.h"
#include "tz_error.h"
#include "zgraph.h"
#include "tz_stack_objlabel.h"
#include "zerror.h"
#include "tz_constant.h"
#include "neutubeconfig.h"
#include "tz_stack_bwmorph.h"
#include "c_stack.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "include/tz_stdint.h"
#include "zfiletype.h"
#include "zeigensolver.h"
#include "zdoublevector.h"
#include "zstack.hxx"
#include "zstring.h"
#include "zhdf5reader.h"
#include "zstringarray.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "zstackfactory.h"
#include "tz_stack_bwmorph.h"

using namespace std;

int ZObject3dStripe::getMinX() const
{
  if (isEmpty()) {
    return 0;
  }

  return m_segmentArray.front();
}

int ZObject3dStripe::getMaxX() const
{
  if (isEmpty()) {
    return 0;
  }

  return m_segmentArray.back();
}

void ZObject3dStripe::addSegment(int x1, int x2, bool canonizing)
{
   if (x1 > x2) {
     int tmp;
     SWAP2(x1, x2, tmp);
   }

   if (!m_segmentArray.empty()) {
     if (x1 > m_segmentArray.back() + 1) {
       m_segmentArray.push_back(x1);
       m_segmentArray.push_back(x2);
     } else {
       if (x1 >= m_segmentArray[m_segmentArray.size() - 2]) {
         m_segmentArray.back() = imax2(x2, m_segmentArray.back());
       } else {
         m_segmentArray.push_back(x1);
         m_segmentArray.push_back(x2);
         m_isCanonized = false;
       }
     }
   } else {
     m_segmentArray.resize(2);
     m_segmentArray[0] = x1;
     m_segmentArray[1] = x2;
     //m_segmentArray.push_back(x1);
     //m_segmentArray.push_back(x2);
     m_isCanonized = true;
   }

   if (canonizing) {
     canonize();
   }
 }

size_t ZObject3dStripe::getVoxelNumber() const
{
  size_t voxelNumber = 0;
  size_t segmentNumber = m_segmentArray.size();
  for (size_t i = 0; i < segmentNumber; i += 2) {
    voxelNumber += m_segmentArray[i + 1] - m_segmentArray[i] + 1;
  }

  return voxelNumber;
}


const int* ZObject3dStripe::getSegment(size_t index) const
{
  if (index >= getSize()) {
    return NULL;
  }

  return &(m_segmentArray[index * 2]);
}

int ZObject3dStripe::getSegmentStart(size_t index) const
{
  return m_segmentArray[index * 2];
}

int ZObject3dStripe::getSegmentEnd(size_t index) const
{
  return m_segmentArray[index * 2 + 1];
}

void ZObject3dStripe::clearSegment()
{
  m_segmentArray.clear();
}

void ZObject3dStripe::translate(int dx, int dy, int dz)
{
  m_y += dy;
  m_z += dz;
  for (size_t i = 0; i < m_segmentArray.size(); ++i) {
    m_segmentArray[i] += dx;
  }
}

void ZObject3dStripe::fillIntArray(int *array) const
{
  if (array != NULL) {
    array[0] = getZ();
    array[1] = getY();
    array[2] = getSegmentNumber();
    memcpy(array + 3, &(m_segmentArray[0]), sizeof(int) * m_segmentArray.size());
  }
}

void ZObject3dStripe::addZ(int dz)
{
  m_z += dz;
}

bool ZObject3dStripe::isCanonizedActually()
{
  for (int i = 0; i < getSegmentNumber(); ++i) {
    //if (getSegmentStart(i) > getSegmentEnd(i)) {
#ifdef _DEBUG_2
      std::cout << "Segment start (" << getSegmentStart(i) << ") > end("
                << getSegmentEnd(i) << ")" << std::endl;
#endif
      //return false;
    //}
    if (i > 0) {
      if (getSegmentEnd(i - 1) >= getSegmentStart(i)) {
#ifdef _DEBUG_2
        std::cout << "Previous segement is greater: " << getSegmentEnd(i - 1)
                  << " " << getSegmentEnd(i) << endl;
#endif
        return false;
      }
    }
  }

  return true;
}


void ZObject3dStripe::write(FILE *fp) const
{
  if (fp != NULL) {
    fwrite(&(m_z), sizeof(int), 1, fp);
    fwrite(&(m_y), sizeof(int), 1, fp);
    int nseg = getSegmentNumber();
    fwrite(&(nseg), sizeof(int), 1, fp);
    fwrite(&(m_segmentArray[0]), sizeof(int), m_segmentArray.size(), fp);
  }
}

void ZObject3dStripe::read(FILE *fp)
{
  if (fp != NULL) {
    fread(&(m_z), sizeof(int), 1, fp);
    fread(&(m_y), sizeof(int), 1, fp);
    int nseg = 0;
    fread(&(nseg), sizeof(int), 1, fp);
    TZ_ASSERT(nseg > 0, "Invalid segment number");
    m_segmentArray.resize(nseg * 2);
    fread(&(m_segmentArray[0]), sizeof(int), m_segmentArray.size(), fp);
    m_isCanonized = false;
  }
}

size_t ZObject3dStripe::countForegroundOverlap(
    Stack *stack, const int *offset) const
{
  if (stack == NULL) {
    return 0;
  }

  if (C_Stack::kind(stack) != GREY) {
    RECORD_ERROR(true, "Unsupported kind");
    return 0;
  }

  size_t count = 0;
  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y < 0 || z < 0 || y >= C_Stack::height(stack) ||
      z >= C_Stack::depth(stack)) {
    return 0;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  size_t area = width * height;
  size_t arrayOffset = area * z + width * y;

  Image_Array ima;
  ima.array = stack->array;
  ima.array8 += arrayOffset;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    if (x1 >= 0 && x0 < width) {
      if (x0 < 0) {
        x0 = 0;
      }
      if (x1 >= width) {
        x1 = width - 1;
      }

      for (int x = x0; x <= x1; ++x) {
        if (ima.array8[x] > 0) {
          ++count;
        }
      }
    }
  }

  return count;
}
void ZObject3dStripe::drawStack(Stack *stack, int v, const int *offset) const
{
  if (C_Stack::kind(stack) != GREY && C_Stack::kind(stack) != GREY16) {
    RECORD_ERROR(true, "Unsupported kind");
    return;
  }

  Image_Array ima;
  ima.array = stack->array;

  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y >= C_Stack::height(stack)) {
    return;
  }

  if (z >= C_Stack::depth(stack)) {
    return;
  }

  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t arrayOffset = area * z + C_Stack::width(stack) * y;

  switch (C_Stack::kind(stack)) {
  case GREY:
    ima.array8 += arrayOffset;
    v = (v < 0) ? 0 : ((v > 255) ? 255 : v);
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i + 1];
      if (offset != NULL) {
        x0 += offset[0];
        x1 += offset[0];
      }
      if (x0 < C_Stack::width(stack)) {
        x1 = min(x1, C_Stack::width(stack) - 1);
        for (int x = x0; x <= x1; ++x) {
          ima.array8[x] = v;
        }
      }
    }
    break;
  case GREY16:
    ima.array16 += arrayOffset;
    v = (v < 0) ? 0 : ((v > 65535) ? 65535 : v);
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i + 1];
      if (offset != NULL) {
        x0 += offset[0];
        x1 += offset[0];
      }
      if (x0 < C_Stack::width(stack)) {
        x1 = min(x1, C_Stack::width(stack) - 1);
        for (int x = x0; x <= x1; ++x) {
          ima.array16[x] = v;
        }
      }
    }
    break;
  default:
    break;
  }
}

void ZObject3dStripe::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, const int *offset) const
{
  if (C_Stack::kind(stack) != COLOR) {
    RECORD_ERROR(true, "Unsupported kind");
    return;
  }

  Image_Array ima;
  ima.array = stack->array;

  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y >= C_Stack::height(stack)) {
    RECORD_ERROR(true, "y too large");
    return;
  }

  if (z >= C_Stack::depth(stack)) {
    RECORD_ERROR(true, "z too large");
    return;
  }
  //TZ_ASSERT(y < C_Stack::height(stack), "y too large");
  //TZ_ASSERT(z < C_Stack::depth(stack), "z too large");

  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t arrayOffset = area * z + C_Stack::width(stack) * y;

  ima.arrayc += arrayOffset;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    TZ_ASSERT(x0 < C_Stack::width(stack), "x too large");
    TZ_ASSERT(x1 < C_Stack::width(stack), "x too large");
    for (int x = x0; x <= x1; ++x) {
      ima.arrayc[x][0] = red;
      ima.arrayc[x][1] = green;
      ima.arrayc[x][2] = blue;
    }
  }
}

void ZObject3dScan::labelStack(Stack *stack, int startLabel, const int *offset)
{
  std::vector<ZObject3dScan> objArray = getConnectedComponent();

#ifdef _DEBUG_
  std::cout << "Number of components: " << objArray.size() << std::endl;
#endif

  if (objArray.size() + startLabel > 256) {
    Translate_Stack(stack, GREY16, 1);
  }
  for (size_t i = 0; i < objArray.size(); ++i) {
    objArray[i].drawStack(stack, startLabel  + i, offset);
  }
}

void ZObject3dScan::maskStack(ZStack *stack)
{
  ZStack *mask = ZStackFactory::makeZeroStack(
        stack->kind(), stack->getBoundBox());
  drawStack(mask->c_stack(), 1);
  for (int c = 0; c < stack->channelNumber(); ++c) {
    Stack_Mul(stack->c_stack(c), mask->c_stack(), stack->c_stack(c));
  }
  delete mask;
}

static int ZObject3dSegmentCompare(const void *e1, const void *e2)
{
  int *v1 = (int*) e1;
  int *v2 = (int*) e2;

  if (v1[0] < v2[0]) {
    return -1;
  } else if (v1[0] > v2[0]) {
    return 1;
  } else {
    if (v1[1] < v2[1]) {
      return -1;
    } else if (v1[1] > v2[1]) {
      return 1;
    }
  }

  return 0;
}

void ZObject3dStripe::sort()
{
  if (!m_segmentArray.empty()) {
    qsort(&m_segmentArray[0], m_segmentArray.size() / 2, sizeof(int) * 2,
        ZObject3dSegmentCompare);
  }
}

void ZObject3dStripe::canonize()
{
  if (!m_isCanonized) {
    if (!m_segmentArray.empty()) {
      sort();
      vector<int> newSegmentArray(m_segmentArray.size());
      size_t length = 0;
      //newSegmentArray.reserve(m_segmentArray.size());
      for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
        if (length == 0) {
          //newSegmentArray.push_back(m_segmentArray[i]);
          newSegmentArray[length++] = m_segmentArray[i];
          //newSegmentArray.push_back(m_segmentArray[i + 1]);
          newSegmentArray[length++] = m_segmentArray[i + 1];
        } else {
          int &lastX = newSegmentArray[length - 1];
          if (lastX + 1 >= m_segmentArray[i]) {
            if (lastX < m_segmentArray[i + 1]) {
              lastX = m_segmentArray[i + 1];
            }
          } else {
            //newSegmentArray.push_back(m_segmentArray[i]);
            newSegmentArray[length++] = m_segmentArray[i];
            //newSegmentArray.push_back(m_segmentArray[i + 1]);
            newSegmentArray[length++] = m_segmentArray[i + 1];
          }
        } 
      }
      newSegmentArray.resize(length);
      m_segmentArray.swap(newSegmentArray);
      //m_segmentArray = newSegmentArray;
    }

    m_isCanonized = true;
  }
}

bool ZObject3dStripe::unify(const ZObject3dStripe &stripe, bool canonizing)
{
  if (getY() == stripe.getY() && getZ() == stripe.getZ()) {
    if (isCanonized() && stripe.isCanonized()) {
      if (!isEmpty() && !stripe.isEmpty()) {
        if (m_segmentArray.back() + 1 >= stripe.m_segmentArray.front()) {
          m_isCanonized = false;
        }
      }
    } else {
      m_isCanonized = false;
    }

    m_segmentArray.insert(m_segmentArray.end(), stripe.m_segmentArray.begin(),
                          stripe.m_segmentArray.end());

    if (canonizing) {
      canonize();
    }

    return true;
  }

  return false;
}

bool ZObject3dStripe::equalsLiterally(const ZObject3dStripe &stripe) const
{
  if (getZ() != stripe.getZ()) {
    return false;
  }

  if (getY() != stripe.getY()) {
    return false;
  }

  if (getSegmentNumber() != stripe.getSegmentNumber()) {
    return false;
  }

  for (size_t i = 0; i < m_segmentArray.size(); ++i) {
    if (m_segmentArray[i] != stripe.m_segmentArray[i]) {
      return false;
    }
  }

  return true;
}

void ZObject3dStripe::switchYZ()
{
  int tmp;
  SWAP2(m_y, m_z, tmp);

  m_isCanonized = false;
}

bool ZObject3dStripe::containsX(int x) const
{
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    if ((x >= m_segmentArray[i]) && (x <= m_segmentArray[i + 1])) {
      return true;
    }
  }

  return false;
}

bool ZObject3dStripe::contains(int x, int y, int z) const
{
  if (getY() == y && getZ() == z) {
    return containsX(x);
  }

  return false;
}

void ZObject3dStripe::dilate()
{
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    m_segmentArray[i] -= 1;
    m_segmentArray[i + 1] += 1;
  }
  if (m_segmentArray.size() > 2) {
    setCanonized(false);
    canonize();
  }
}

//Use the fact that x0 <= x0p*t <= x1p*t <= x1 and
// (x0p - 1)*t < x0, (x1p + 1)*t > x1
void ZObject3dStripe::downsample(int xintv)
{
  if (xintv > 0) {
    vector<int> newSegmentArray;
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i+1];
      int t = xintv + 1;
      int x0p = x0 / t;
      int x1p = x1 / t;
      if (x0p * t < x0) {
        ++x0p;
      }

      if (x1p * t > x1) {
        --x1p;
      }

      if (x0p <= x1p) {
        newSegmentArray.push_back(x0p);
        newSegmentArray.push_back(x1p);
      }
    }

    m_segmentArray = newSegmentArray;
    m_isCanonized = false;

    canonize();
  }
}

//Use the fact that: x0p*t <= x0 <= x1 <= x1pt and
//  (x0p + 1)*t > x0, (x1p - 1)*t < x1
void ZObject3dStripe::downsampleMax(int xintv)
{
  if (xintv > 0) {
    //vector<int> newSegmentArray(m_segmentArray.size());
    int t = xintv + 1;
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      m_segmentArray[i] /= t;
      m_segmentArray[i + 1] /= t;
#if 0
      int &x0 = m_segmentArray[i];
      int &x1 = m_segmentArray[i+1];

      int x0p = x0 / t;
      int x1p = x1 / t;

      /*
      if (x1p % t > 0) {
        ++x1p;
      }
      */

      x0 = x0p;
      x1 = x1p;
      //newSegmentArray[i] = x0p;
      //newSegmentArray[i + 1] = x1p;
      //newSegmentArray.push_back(x0p);
      //newSegmentArray.push_back(x1p);
#endif
    }

    //m_segmentArray = newSegmentArray;
    m_isCanonized = false;
    canonize();
  }
}

void ZObject3dStripe::upSample(int xIntv)
{
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    m_segmentArray[i] *= xIntv + 1;
    m_segmentArray[i + 1] = m_segmentArray[i + 1] * (xIntv + 1) + xIntv;
  }
  setCanonized(false);
}

void ZObject3dStripe::print(int indent) const
{
  for (int k = 0; k < indent; ++k) {
    std::cout << " ";
  }
  cout << "Segments at " << "." << m_z << "|" << m_y << ": " << endl;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    for (int k = 0; k < indent; ++k) {
      std::cout << " ";
    }
    cout << "  " << m_segmentArray[i] << " - " << m_segmentArray[i+1] << endl;
  }
}

///////////////////////////////////////////////////

const ZObject3dScan::TEvent ZObject3dScan::EVENT_NULL = 0;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED = 0x1;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_MODEL_CHANGED =
    0x2 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_UNCANONIZED =
    0x4 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_CANONIZED =
    0x8 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;


ZObject3dScan::ZObject3dScan() : m_isCanonized(true), m_label(0)
{
  setTarget(OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_OBJECT3D_SCAN;
}

ZObject3dScan::~ZObject3dScan()
{
  deprecate(ALL_COMPONENT);
}

bool ZObject3dScan::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case STRIPE_INDEX_MAP:
    return m_stripeMap.empty();
  case INDEX_SEGMENT_MAP:
    return m_indexSegmentMap.empty();
  case ACCUMULATED_STRIPE_NUMBER:
    return m_accNumberArray.empty();
  case SLICEWISE_VOXEL_NUMBER:
    return m_slicewiseVoxelNumber.empty();
  default:
    break;
  }

  return false;
}

void ZObject3dScan::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case STRIPE_INDEX_MAP:
    m_stripeMap.clear();
    break;
  case INDEX_SEGMENT_MAP:
    m_indexSegmentMap.clear();
    break;
  case ACCUMULATED_STRIPE_NUMBER:
    m_accNumberArray.clear();
    break;
  case SLICEWISE_VOXEL_NUMBER:
    m_slicewiseVoxelNumber.clear();
    break;
  case ALL_COMPONENT:
    deprecate(STRIPE_INDEX_MAP);
    deprecate(INDEX_SEGMENT_MAP);
    deprecate(ACCUMULATED_STRIPE_NUMBER);
    deprecate(SLICEWISE_VOXEL_NUMBER);
    break;
  }
}

void ZObject3dScan::deprecateDependent(EComponent comp)
{
  switch (comp) {
  default:
    break;
  }
}

bool ZObject3dScan::isEmpty() const
{
  return m_stripeArray.empty();
}

size_t ZObject3dScan::getStripeNumber() const
{
  return m_stripeArray.size();
}

size_t ZObject3dScan::getVoxelNumber() const
{
  size_t voxelNumber = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    voxelNumber += m_stripeArray[i].getVoxelNumber();
  }

  return voxelNumber;
}

size_t ZObject3dScan::getVoxelNumber(int z) const
{
  size_t voxelNumber = 0;
  size_t stripeNumber = getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    if (m_stripeArray[i].getZ() == z) {
      voxelNumber += m_stripeArray[i].getVoxelNumber();
    }
  }

  return voxelNumber;
}

const std::map<int, size_t> &ZObject3dScan::getSlicewiseVoxelNumber() const
{
  if (isDeprecated(SLICEWISE_VOXEL_NUMBER)) {
    //std::vector<size_t> voxelNumber;
    size_t stripeNumber = getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      int z = m_stripeArray[i].getZ();
      if (m_slicewiseVoxelNumber.count(z) == 0) {
        m_slicewiseVoxelNumber[z] = m_stripeArray[i].getVoxelNumber();
      } else {
        m_slicewiseVoxelNumber[z] += m_stripeArray[i].getVoxelNumber();
      }
    }
  }

  return m_slicewiseVoxelNumber;
}

std::map<int, size_t> &ZObject3dScan::getSlicewiseVoxelNumber()
{
  return const_cast<std::map<int, size_t>&>(
        static_cast<const ZObject3dScan&>(*this).getSlicewiseVoxelNumber());
}

const ZObject3dStripe &ZObject3dScan::getStripe(size_t index) const
{
  return m_stripeArray[index];
}

ZObject3dStripe &ZObject3dScan::getStripe(size_t index)
{
  return m_stripeArray[index];
}

void ZObject3dScan::addStripe(int z, int y, bool canonizing)
{
  bool adding = true;
  if (!m_stripeArray.empty()) {
    if (y == m_stripeArray.back().getY() &&
        z == m_stripeArray.back().getZ()) {
      adding = false;
    }
  }

  if (adding) {
    ZObject3dStripe stripe;
    stripe.setY(y);
    stripe.setZ(z);

    addStripe(stripe, canonizing);
  }
}

void ZObject3dScan::addStripe(const ZObject3dStripe &stripe, bool canonizing)
{
  bool lastStripeMergable = false;

  TEvent event = EVENT_NULL;

  if (!m_stripeArray.empty()) {
    if (stripe.getY() != m_stripeArray.back().getY() ||
        stripe.getZ() != m_stripeArray.back().getZ()) {
      if (m_isCanonized) {
        if (stripe.getZ() < m_stripeArray.back().getZ()) {
          //m_isCanonized = false;
          event = EVENT_OBJECT_UNCANONIZED;
        } else if (stripe.getZ() == m_stripeArray.back().getZ()) {
          if (stripe.getY() < m_stripeArray.back().getY()) {
            //m_isCanonized = false;
            event |= EVENT_OBJECT_UNCANONIZED;
          }
        }
      }
    } else {
      lastStripeMergable = true;
    }
  }

  if (lastStripeMergable) {
    for (int i = 0; i < stripe.getSegmentNumber(); ++i) {
      m_stripeArray.back().addSegment(
            stripe.getSegmentStart(i), stripe.getSegmentEnd(i), canonizing);
    }
  } else {
    m_stripeArray.push_back(stripe);
  }

  event |= EVENT_OBJECT_MODEL_CHANGED;

  processEvent(event);

  if (canonizing) {
    canonize();
  }

  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::addSegment(int x1, int x2, bool canonizing)
{
  if (!isEmpty()) {
    TEvent event = EVENT_NULL;

    m_stripeArray.back().addSegment(x1, x2, canonizing);
    event |= EVENT_OBJECT_MODEL_CHANGED;

    if (!m_stripeArray.back().isCanonized()) {
      //m_isCanonized = false;
      if (isCanonized()) {
        event |= EVENT_OBJECT_UNCANONIZED;
      }
    }

    /*
    deprecate(INDEX_SEGMENT_MAP);
    deprecate(ACCUMULATED_STRIPE_NUMBER);
    */

    processEvent(event);

    if (canonizing) {
      canonize();
    }
  }
}

void ZObject3dScan::addSegment(int z, int y, int x1, int x2, bool canonizing)
{
  addStripe(z, y, false);
  addSegment(x1, x2, canonizing);
}

/*
const int* ZObject3dScan::getLastStripe() const
{
  if (isDeprecated(LAST_STRIPE)) {
    size_t numStripe = getStripeNumber();
    const int *stripe = getFirstStripe();

    for (size_t i = 0; i < numStripe - 1; i++) {
      int numScanLine = getStripeSize(stripe);
      const int *scanLine = getSegment(stripe);
      for (int j = 0; j < numScanLine; j++) {
        if (j < numScanLine - 1) {
          scanLine += 2;
        }
      }
      stripe = scanLine + 1;
    }

    m_lastStripe = const_cast<int*>(stripe);
  }

  return m_lastStripe;
}

int* ZObject3dScan::getLastStripe()
{
  return const_cast<int*>(static_cast<const ZObject3dScan&>(*this).getLastStripe());
}
*/
ZObject3d* ZObject3dScan::toObject3d() const
{
  if (isEmpty()) {
    return NULL;
  }

  ZObject3d *obj = new ZObject3d();

  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    int y = iter->getY();
    int z = iter->getZ();
    int nseg = iter->getSegmentNumber();
    for (int i = 0; i < nseg; ++i) {
      const int *seg = iter->getSegment(i);
      for (int x = seg[0]; x <= seg[1]; ++x) {
        obj->append(x, y, z);
      }
    }
  }

/*
  size_t numStripe = getStripeNumber();

  for (size_t i = 0; i < numStripe; i++) {
    int z = getY(stripe);
    int y = getZ(stripe);
    int numScanLine = getStripeSize(stripe);
    const int *scanLine = getSegment(stripe);
    for (int j = 0; j < numScanLine; j++) {
      int x1 = scanLine[0];
      int x2 = scanLine[1];
      for (int x = x1; x <= x2; x++) {
        obj->append(x, y, z);
      }
      if (j < numScanLine - 1) {
        scanLine += 2;
      }
    }
    stripe = scanLine + 1;
  }
*/
  return obj;
}

const std::map<size_t, std::pair<size_t, size_t> >&
ZObject3dScan::getIndexSegmentMap() const
{
  if (isDeprecated(INDEX_SEGMENT_MAP)) {
    m_indexSegmentMap.clear();
    size_t currentIndex = 0;
    for (size_t stripeIndex = 0; stripeIndex < getStripeNumber(); ++stripeIndex) {
      size_t segmentNumber = m_stripeArray[stripeIndex].getSegmentNumber();
      for (size_t segIndex = 0; segIndex < segmentNumber; ++segIndex) {
        m_indexSegmentMap[currentIndex++] =
            std::pair<size_t, size_t>(stripeIndex, segIndex);
      }
    }
  }

  return m_indexSegmentMap;
}

bool ZObject3dScan::getSegment(size_t index, int *z, int *y, int *x1, int *x2)
{
  const std::map<size_t, std::pair<size_t, size_t> >&segMap =
      getIndexSegmentMap();
  if (segMap.count(index) > 0) {
    std::pair<size_t, size_t> location = segMap.at(index);
    *z = m_stripeArray[location.first].getZ();
    *y = m_stripeArray[location.first].getY();
    *x1 = m_stripeArray[location.first].getSegmentStart(location.second);
    *x2 = m_stripeArray[location.first].getSegmentEnd(location.second);
    return true;
  }

  return false;
}

void ZObject3dScan::loadStack(const Stack *stack)
{
  clear();

  if (stack == NULL) {
    RECORD_ERROR(true, "null stack");
    return;
  }

  if (C_Stack::kind(stack) != GREY) {
    RECORD_ERROR(true, "unsupported kind");
    return;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  uint8_t *array = stack->array;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      int x1 = -1;
      int x2 = -1;
      int scanState = 0; //0: no ON pixel found;
                         //1: in ON region; 2: in OFF region
      for (int x = 0; x < width; ++x) {
        if (*array > 0) {
          switch (scanState) {
          case 0:
            addStripe(z, y);
            x1 = x;
            break;
          case 1:
            break;
          case 2:
            x1 = x;
            break;
          }
          scanState = 1;
        } else {
          if (scanState == 1) {
            x2 = x - 1;
            scanState = 2;
          }
        }
        if (x1 >= 0 && x2 >= 0) {
          addSegment(x1, x2, false);
          x1 = -1;
          x2 = -1;
        }
        ++array;
      }
      if (x1 >= 0) {
        addSegment(x1, width - 1, false);
      }
    }
  }
  setCanonized(true);
}

void ZObject3dScan::loadStack(const ZStack &stack)
{
  loadStack(stack.c_stack());
  translate(stack.getOffset().getX(), stack.getOffset().getY(),
            stack.getOffset().getZ());
}

void ZObject3dScan::print() const
{
  cout << getStripeNumber() << " stripes" << endl;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    //cout << "  Z:" << iter->getZ() << " Y:" << iter->getY() << " #S:"
    //     << iter->getSegmentNumber() << " #V:" << iter->getVoxelNumber() << endl;
    iter->print(2);
  }
}

bool ZObject3dScan::load(const string &filePath)
{
  bool succ = false;
  ZString filePath2(filePath);
#ifdef _DEBUG_2
    std::cout << filePath << std::endl;
#endif

  if (filePath2.contains(":")) {
    std::vector<std::string> strArray = filePath2.tokenize(':');
    if (strArray.size() >= 2) {
      if (ZFileType::fileType(strArray[0]) == ZFileType::HDF5_FILE) {
        succ = importHdf5(strArray[0], strArray[1]);
      }
    }
  } else if (ZFileType::fileType(filePath) == ZFileType::DVID_OBJECT_FILE) {
    succ = importDvidObject(filePath);
  } else if (ZFileType::fileType(filePath) == ZFileType::OBJECT_SCAN_FILE) {
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp != NULL) {
      int stripeNumber = 0;
      fread(&stripeNumber, sizeof(int), 1, fp);

      PROCESS_ERROR(stripeNumber < 0, "Invalid stripe number", return false);
      /*
    if (stripeNumber < 0) {
      RECORD_ERROR(true, "Invalid stripe number");
      return false;
    }
    */

      m_stripeArray.resize(stripeNumber);
      for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
           iter != m_stripeArray.end(); ++iter) {
        iter->read(fp);
      }
      fclose(fp);

      if (isCanonizedActually()) {
        m_isCanonized = true;
      } else {
        m_isCanonized = false;
      }

      deprecate(ALL_COMPONENT);

      succ = true;
    }
  }

  if (succ) {
    setSource(filePath);
  }

  RECORD_WARNING(!succ, "Cannont open file " + filePath);

  return succ;
}

bool ZObject3dScan::load(const char *filePath)
{
  return load(std::string(filePath));
}

void ZObject3dScan::save(const char *filePath)
{
  save(string(filePath));
}

void ZObject3dScan::save(const char *filePath) const
{
  save(string(filePath));
}

void ZObject3dScan::save(const string &filePath) const
{
  FILE *fp = fopen(filePath.c_str(), "wb");
  if (fp != NULL) {
    int stripeNumber = (int) getStripeNumber();
    fwrite(&stripeNumber, sizeof(int), 1, fp);
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->write(fp);
    }
    fclose(fp);
  } else {
    RECORD_WARNING(true, "Cannont open file " + filePath);
  }
}

size_t ZObject3dScan::countForegroundOverlap(Stack *stack, const int *offset)
{
  size_t count = 0;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    count += iter->countForegroundOverlap(stack, offset);
  }

  return count;
}

void ZObject3dScan::drawStack(Stack *stack, int v, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    iter->drawStack(stack, v, offset);
  }
}

void ZObject3dScan::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    iter->drawStack(stack, red, green, blue, offset);
  }
}

static int ZObject3dStripeCompare(const void *e1, const void *e2)
{
  ZObject3dStripe *v1 = (ZObject3dStripe*) e1;
  ZObject3dStripe *v2 = (ZObject3dStripe*) e2;

  if (v1->getZ() < v2->getZ()) {
    return -1;
  } else if (v1->getZ() > v2->getZ()) {
    return 1;
  } else {
    if (v1->getY() < v2->getY()) {
      return -1;
    } else if (v1->getY() > v2->getY()) {
      return 1;
    }
  }

  return 0;
}

void ZObject3dScan::sort()
{
  if (!isEmpty()) {
    qsort(&m_stripeArray[0], m_stripeArray.size(), sizeof(ZObject3dStripe),
        ZObject3dStripeCompare);
    //deprecate(ALL_COMPONENT);
    processEvent(EVENT_OBJECT_VIEW_CHANGED);
  }
}

void ZObject3dScan::canonize()
{
  if (!isEmpty() && !isCanonized()) {
    sort();

#ifdef _DEBUG_2
  int count = 0;
  int ncount = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = getStripe(i);
    if (stripe.isCanonized()) {
      ++count;
    } else {
      ++ncount;
    }
  }
  std::cout << "Canonized: " << count << std::endl;
  std::cout << "Uncanonized: " << ncount << std::endl;
#endif

    vector<ZObject3dStripe> newStripeArray(m_stripeArray.size());
    size_t length = 0;
    //newStripeArray.reserve(m_stripeArray.size());
    m_stripeArray[0].canonize();
    //newStripeArray.push_back(m_stripeArray[0]);
    newStripeArray[length++] = m_stripeArray[0];
    for (size_t i = 1; i < m_stripeArray.size(); ++i) {
      ZObject3dStripe &newStripe = newStripeArray[length - 1];
      ZObject3dStripe &stripe = m_stripeArray[i];
      if (!newStripe.unify(stripe)) {
        stripe.canonize();
        //newStripeArray.push_back(m_stripeArray[i]);
        newStripeArray[length++] = stripe;
      }
    }

    newStripeArray.resize(length);

    //m_stripeArray = newStripeArray;
    m_stripeArray.swap(newStripeArray);

    //m_isCanonized = true;
    processEvent(EVENT_OBJECT_CANONIZED);
  }
}

void ZObject3dScan::unify(const ZObject3dScan &obj)
{
  concat(obj);
  canonize();
}

void ZObject3dScan::concat(const ZObject3dScan &obj)
{
  m_stripeArray.insert(m_stripeArray.end(), obj.m_stripeArray.begin(),
                       obj.m_stripeArray.end());
  //m_isCanonized = false;
  processEvent(EVENT_OBJECT_MODEL_CHANGED | EVENT_OBJECT_UNCANONIZED);
  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::downsample(int xintv, int yintv, int zintv)
{
  TEvent event = EVENT_NULL;

  if (yintv > 0 || zintv > 0) {
    vector<ZObject3dStripe> newStripeArray;
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      if ((iter->getY() % (yintv + 1) == 0) &&
          (iter->getZ() % (zintv + 1) == 0)) {
        newStripeArray.push_back(*iter);
      }
    }
    m_stripeArray = newStripeArray;
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsample(xintv);
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0 || yintv > 0 || zintv > 0) {
    //deprecate(ALL_COMPONENT);
    event |= EVENT_OBJECT_MODEL_CHANGED;
  }

  processEvent(event);
}

void ZObject3dScan::downsampleMax(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  TEvent event = EVENT_NULL;

  if (yintv > 0 || zintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsampleMax(xintv);
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  processEvent(event);

  canonize();

  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::upSample(int xIntv, int yIntv, int zIntv)
{
  if (xIntv == 0 && yIntv == 0 && zIntv == 0) {
    return;
  }

  int stripeNumber = getStripeNumber();
  for (int i = 0; i < stripeNumber; ++i) {
    ZObject3dStripe &stripe = getStripe(i);
    stripe.upSample(xIntv);
  }

  ZObject3dScan grownPart;

  for (int i = 0; i < stripeNumber; ++i) {
    ZObject3dStripe &stripe = getStripe(i);

    int y = stripe.getY();
    int z = stripe.getZ();

    int z0 = z * (zIntv + 1);
    int z1 = z0 + zIntv;
    int y0 = y * (yIntv + 1);
    int y1 = y0 + yIntv;

#ifdef _DEBUG_2
    std::cout << y0 << ", " << y1 << std::endl;
#endif

    for (z = z0; z <= z1; ++z) {
      for (y = y0; y <= y1; ++y) {
        ZObject3dStripe newStripe = stripe;
        newStripe.setY(y);
        newStripe.setZ(z);
        grownPart.addStripe(newStripe, false);
      }
    }

#ifdef _DEBUG_2
        std::cout << "New Stripe: ";
        stripe.print();
        std::cout << "---" << std::endl;
#endif
  }

  *this = grownPart;

  canonize();
}

bool ZObject3dScan::isAdjacentTo(ZObject3dScan &obj)
{
  ZObject3dScan tmpObj = obj;
  tmpObj.dilate();

  return hasOverlap(tmpObj);
}

bool ZObject3dScan::hasOverlap(ZObject3dScan &obj)
{
  if (isEmpty() || obj.isEmpty()) {
    return false;
  }

  int minZ = imax2(getMinZ(), obj.getMinZ());
  int maxZ = imin2(getMaxZ(), obj.getMaxZ());

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice1 = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);
    ZStack *stack1 = slice1.toStackObject();
    int stripeNumber = obj.getStripeNumber();
    for (int i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = slice2.getStripe(i);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      for (int j = 0; j < segmentNumber; ++j) {
        int minX = stripe.getSegmentStart(j);
        int maxX = stripe.getSegmentEnd(j);
        for (int x = minX; x <= maxX; ++x) {
          if (stack1->getIntValue(x, y, z) > 0) {
            delete stack1;
            return true;
          }
        }
      }
    }
    delete stack1;
  }

  return false;
}

Stack* ZObject3dScan::toStack(int *offset, int v) const
{
  if (isEmpty()) {
    return NULL;
  }

  ZIntCuboid boundBox = getBoundBox();
  if (offset != NULL) {
    offset[0] = boundBox.getFirstCorner().getX();
    offset[1] = boundBox.getFirstCorner().getY();
    offset[2] = boundBox.getFirstCorner().getZ();
  }

  Stack *stack = C_Stack::make(GREY, boundBox.getWidth(),
                               boundBox.getHeight(),
                               boundBox.getDepth());
  C_Stack::setZero(stack);


  int drawingOffet[3];
  drawingOffet[0] = -boundBox.getFirstCorner().getX();
  drawingOffet[1] = -boundBox.getFirstCorner().getY();
  drawingOffet[2] = -boundBox.getFirstCorner().getZ();

  drawStack(stack, v, drawingOffet);

  return stack;
}

ZStack* ZObject3dScan::toStackObject(int v) const
{
  int offset[3] = {0, 0, 0};
  Stack *stack = toStack(offset, v);

  ZStack *stackObject = new ZStack;

  if (stack != NULL) {
    stackObject->load(stack);
    stackObject->setOffset(offset[0], offset[1], offset[2]);
  }

  return stackObject;
}

ZStack* ZObject3dScan::toVirtualStack() const
{
  ZIntCuboid box = getBoundBox();

  ZStack *stack = new ZStack(GREY, box.getWidth(), box.getHeight(),
                             box.getDepth(), 1, true);
  stack->setOffset(box.getFirstCorner());

  return stack;
}

ZIntCuboid ZObject3dScan::getBoundBox() const
{
  ZIntCuboid boundBox;

  bool isFirst = true;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    if (!iter->isEmpty()) {
      if (isFirst) {
        boundBox.set(iter->getMinX(), iter->getY(), iter->getZ(),
                     iter->getMaxX(), iter->getY(), iter->getZ());
        isFirst = false;
      } else {
        boundBox.joinY(iter->getY());
        boundBox.joinZ(iter->getZ());
        boundBox.joinX(iter->getMinX());
        boundBox.joinX(iter->getMaxX());
      }
#ifdef _DEBUG_2
      std::cout << iter->getMinX() << " " << iter->getMaxX() << " "
                << iter->getY() << " " << iter->getZ() << std::endl;
      std::cout << boundBox.getFirstCorner().toString() << " "
                << boundBox.getLastCorner().toString() << std::endl;
#endif
    }
  }

  return boundBox;
}

void ZObject3dScan::getBoundBox(Cuboid_I *box) const
{
  ZIntCuboid boundBox = getBoundBox();

  Cuboid_I_Set_S(box, boundBox.getFirstCorner().getX(),
                 boundBox.getFirstCorner().getY(),
                 boundBox.getFirstCorner().getZ(),
                 boundBox.getWidth(),
                 boundBox.getHeight(), boundBox.getDepth());
}

const std::vector<size_t>& ZObject3dScan::getStripeNumberAccumulation() const
{
  if (isDeprecated(ACCUMULATED_STRIPE_NUMBER)) {
    m_accNumberArray.resize(getStripeNumber() + 1);
    m_accNumberArray[0] = 0;
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_accNumberArray[i + 1] =
          m_accNumberArray[i] + m_stripeArray[i].getSegmentNumber();
    }
  }

  return m_accNumberArray;
}

const std::map<std::pair<int, int>, size_t> &ZObject3dScan::getStripeMap() const
{
  if (isDeprecated(STRIPE_INDEX_MAP)) {
    m_stripeMap.clear();
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_stripeMap[std::pair<int, int>(
            m_stripeArray[i].getZ(), m_stripeArray[i].getY())] = i;
    }
  }

  return m_stripeMap;
}

ZGraph* ZObject3dScan::buildConnectionGraph()
{
  if (isEmpty()) {
    return NULL;
  }

  ZGraph *graph = new ZGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  canonize();

  const std::vector<size_t>& stripeNumberAccumulation =
      getStripeNumberAccumulation();

  const std::map<std::pair<int, int>, size_t> &stripeMap = getStripeMap();

  for (size_t i = 0; i < getStripeNumber() - 1; ++i) {
    //Check along Y
    for (int j = 0; j < m_stripeArray[i].getSegmentNumber(); ++j) {
      const int *stripe = m_stripeArray[i].getSegment(j);
      int v1 = stripeNumberAccumulation[i] + j;

      //Check along Y
      if ((m_stripeArray[i].getZ() == m_stripeArray[i + 1].getZ()) &&
          (m_stripeArray[i].getY() + 1 == m_stripeArray[i + 1].getY())) {
        for (int k = 0; k < m_stripeArray[i + 1].getSegmentNumber(); ++k) {
          const int *downStairStripe = m_stripeArray[i + 1].getSegment(k);

          if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
              IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
            int v2 = stripeNumberAccumulation[i + 1] + k;
            graph->addEdgeFast(v1, v2);
            //break;
          }
        }
      }

      //Check along Z

      for (int dy = -1; dy <= 1; ++dy) {
        std::pair<int, int> neighborStripeZY(
              m_stripeArray[i].getZ() + 1, m_stripeArray[i].getY() + dy);
#ifdef _DEBUG_2
        std::cout << "Neighbor stripe position: Y=" << neighborStripeZY.second
                  << ", Z=" << neighborStripeZY.first << endl;
#endif
        if (stripeMap.count(neighborStripeZY) > 0) {
          size_t neighborStripeIndex = stripeMap.at(neighborStripeZY);
          for (int k = 0;
               k < m_stripeArray[neighborStripeIndex].getSegmentNumber(); ++k) {
            const int *downStairStripe =
                m_stripeArray[neighborStripeIndex].getSegment(k);

            if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
                IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
              int v2 = stripeNumberAccumulation[neighborStripeIndex] + k;
              graph->addEdgeFast(v1, v2);
              //break;
            }
          }
        }
      }
    }
  }

  return graph;
}

void ZObject3dScan::clear()
{
  m_stripeArray.clear();
  m_isCanonized = true;
  deprecate(ALL_COMPONENT);
}

std::vector<size_t> ZObject3dScan::getConnectedObjectSize()
{
  std::vector<size_t> sizeArray;
#if 0
  ZObject3d *obj3d = toObject3d();
  if (obj3d != NULL) {
    Stack *stack = obj3d->toStack();
    C_Stack::translate(stack, GREY16, 1);

    Stack_Label_Objects_N(stack, NULL, 1, 2, 26);

    std::vector<size_t> hist(65535, 0);
    uint16_t *array16 = (uint16_t*) stack->array;
    size_t voxelNumber = C_Stack::voxelNumber(stack);
    for (size_t i = 0; i < voxelNumber; ++i) {
      hist[array16[i]]++;
    }

    for (size_t i = 2; i < 65535; i++) {
      if (hist[i] > 0) {
        sizeArray.push_back(hist[i]);
      }
    }

    delete obj3d;
    C_Stack::kill(stack);
  }
#else
  if (!isEmpty()) {
    std::vector<ZObject3dScan> objArray = getConnectedComponent();

    sizeArray.resize(objArray.size());
    for (size_t i = 0; i < objArray.size(); ++i) {
      sizeArray[i] = objArray[i].getVoxelNumber();
#if _DEBUG_2
      std::cout << "Sub object:" << std::endl;
      objArray[i].print();
#endif
    }
  }
#endif

  if (!sizeArray.empty()) {
    std::sort(sizeArray.begin(), sizeArray.end());
    std::reverse(sizeArray.begin(), sizeArray.end());
  }

  return sizeArray;
}

std::vector<ZObject3dScan> ZObject3dScan::getConnectedComponent()
{
  std::vector<ZObject3dScan> objArray;

  ZGraph *graph = buildConnectionGraph();

  if (graph != NULL) {
    const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();

    const std::map<size_t, std::pair<size_t, size_t> >&segMap =
        getIndexSegmentMap();


    std::vector<bool> isAdded(segMap.size(), false);

    for (std::vector<ZGraph*>::const_iterator iter = subGraph.begin();
         iter != subGraph.end(); ++iter) {
      ZObject3dScan subobj;
      for (int edgeIndex = 0; edgeIndex < (*iter)->getEdgeNumber(); ++edgeIndex) {
        int v1 = (*iter)->edgeStart(edgeIndex);
        int v2 = (*iter)->edgeEnd(edgeIndex);
        int z, y, x1, x2;
        getSegment(v1, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2, false);
        getSegment(v2, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2, false);
        isAdded[v1] = true;
        isAdded[v2] = true;
      }
      subobj.canonize();
      objArray.push_back(subobj);
    }

    delete graph;

    for (size_t i = 0; i < isAdded.size(); ++i) {
      if (!isAdded[i]) {
        ZObject3dScan subobj;
        int z, y, x1, x2;
        getSegment(i, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2);
        objArray.push_back(subobj);
      }
    }
  }

  return objArray;
}

size_t ZObject3dScan::getSegmentNumber()
{
  const std::vector<size_t>& accArray = getStripeNumberAccumulation();

  return accArray.back();
}

void ZObject3dScan::translate(int dx, int dy, int dz)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].translate(dx, dy, dz);
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::translate(const ZIntPoint &dp)
{
  translate(dp.getX(), dp.getY(), dp.getZ());
}

void ZObject3dScan::addZ(int dz)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].addZ(dz);
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

bool ZObject3dScan::isCanonizedActually()
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    if (i > 0) {
      if (m_stripeArray[i - 1].getZ() > m_stripeArray[i].getZ()) {
#ifdef _DEBUG_
        std::cout << "Previous Z is bigger: " << m_stripeArray[i - 1].getZ()
                  << " " << m_stripeArray[i].getZ() << std::endl;
#endif
        return false;
      } else if (m_stripeArray[i - 1].getZ() == m_stripeArray[i].getZ()) {
        if (m_stripeArray[i - 1].getY() >= m_stripeArray[i].getY()) {
#ifdef _DEBUG_
          std::cout << "Previous Z/Y is bigger: " << m_stripeArray[i - 1].getY()
                    << " " << m_stripeArray[i].getY() << std::endl;
#endif
          return false;
        }
      }
    }
    if (!m_stripeArray[i].isCanonizedActually()) {
      return false;
    }
  }

  return true;
}

void ZObject3dScan::duplicateAcrossZ(int depth)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].setZ(0);
  }
  setCanonized(false);
  canonize();

  size_t orignalStripeNumber = m_stripeArray.size();

  for (int z = 1; z < depth; ++z) {
    for (size_t i = 0; i < orignalStripeNumber; ++i) {
      ZObject3dStripe stripe = m_stripeArray[i];
      stripe.setZ(z);
      addStripe(stripe, false);
    }
  }
  //deprecate(ALL_COMPONENT);
  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::display(
    ZPainter &painter, int slice, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  if (isSelected() && style == ZStackObject::SOLID) {
    return;
  }

  bool isProj = (slice == -1);

  int z = slice + iround(painter.getOffset().z());

  QPen pen(m_color);

  //QImage *targetImage = dynamic_cast<QImage*>(painter.device());

  switch (style) {
  case ZStackObject::SOLID:
  {
    painter.setPen(pen);
    size_t stripeNumber = getStripeNumber();
    std::vector<QLine> lineArray;
    int offsetX = iround(painter.getOffset().x());
    int offsetY = iround(painter.getOffset().y());
    for (size_t i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = getStripe(i);
      if (stripe.getZ() == z || isProj) {
        int nseg = stripe.getSegmentNumber();
        for (int j = 0; j < nseg; ++j) {
          int x0 = stripe.getSegmentStart(j) - offsetX;
          int x1 = stripe.getSegmentEnd(j) - offsetX;
          int y = stripe.getY() - offsetY;
          lineArray.push_back(QLine(x0, y, x1, y));
        }
      }
    }
    if (!lineArray.empty()) {
      painter.drawLines(&(lineArray[0]), lineArray.size());
    }
  }
    break;
  case ZStackObject::BOUNDARY:
  {
    QColor color = pen.color();
    color.setAlpha(255);
    pen.setColor(color);
    painter.setPen(pen);
    std::vector<QPoint> ptArray;
    ZObject3dScan slice = getSlice(z);

    if (!slice.isEmpty()) {
      ZStack *stack = slice.toStackObject();
      int conn = 4;
      if (isSelected()) {
        conn = 8;
      }
      Stack *pre = Stack_Perimeter(stack->c_stack(), NULL, conn);
      size_t offset = 0;
      for (int y = 0; y < stack->height(); ++y) {
        for (int x = 0; x < stack->width(); ++x) {
          if (pre->array[offset++] > 0) {
            ptArray.push_back(QPoint(x + stack->getOffset().getX(),
                                     y + stack->getOffset().getY()));
          }
        }
      }
    }
    if (!ptArray.empty()) {
      painter.drawPoints(&(ptArray[0]), ptArray.size());
    }
  }
    break;
  default:
    break;
  }





#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(slice);
  UNUSED_PARAMETER(style);
#endif
}

void ZObject3dScan::dilatePlane()
{
  size_t oldStripeNumber = getStripeNumber();
  for (size_t i = 0; i < oldStripeNumber; ++i) {
    ZObject3dStripe baseStripe = m_stripeArray[i];
    ZObject3dStripe stripe = baseStripe;
    stripe.setY(stripe.getY() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setY(stripe.getY() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
//    stripe = baseStripe;
//    stripe.setZ(stripe.getZ() - 1);
//    m_stripeArray.push_back(stripe);
//    //addStripe(stripe, false);
//    stripe = baseStripe;
//    stripe.setZ(stripe.getZ() + 1);
//    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    m_stripeArray[i].dilate();
  }

  setCanonized(false);
  canonize();

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::dilate()
{
  size_t oldStripeNumber = getStripeNumber();
  for (size_t i = 0; i < oldStripeNumber; ++i) {
    ZObject3dStripe baseStripe = m_stripeArray[i];
    ZObject3dStripe stripe = baseStripe;
    stripe.setY(stripe.getY() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setY(stripe.getY() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setZ(stripe.getZ() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setZ(stripe.getZ() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    m_stripeArray[i].dilate();
  }

  setCanonized(false);
  canonize();

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

ZObject3dScan ZObject3dScan::getSlice(int z) const
{
  ZObject3dScan slice;
  size_t stripeNumber = getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.getZ() == z) {
      slice.addStripe(stripe, false);
    }
  }

  return slice;
}

ZObject3dScan ZObject3dScan::getSlice(int minZ, int maxZ) const
{
  ZObject3dScan slice;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.getZ() >= minZ) {
      if (stripe.getZ() <= maxZ) {
        slice.addStripe(stripe, false);
      }
    }
  }

  if (isCanonized()) {
    slice.setCanonized(true);
  }

  return slice;
}

bool ZObject3dScan::hit(double x, double y, double z)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.contains(x, y, z)) {
      return true;
    }
  }

  return false;
}

ZPoint ZObject3dScan::getCentroid() const
{
  ZPoint center(0, 0, 0);
  int count = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      int len = x2 - x1 + 1;
      count += len;
      center.translate((x1 + x2) * len /2, m_stripeArray[i].getY() * len,
                       m_stripeArray[i].getZ() * len);
    }
  }

  if (count > 0) {
    center /= count;
  }

  return center;
}

ZObject3dScan ZObject3dScan::makeZProjection(int minZ, int maxZ)
{
  return getSlice(minZ, maxZ).makeZProjection();
}

ZHistogram ZObject3dScan::getRadialHistogram(int z) const
{
  ZHistogram hist;
  hist.setInterval(1.0);
  hist.setStart(0.0);

  ZObject3dScan slice = getSlice(z);

  ZPoint center = slice.getCentroid();
#ifdef _DEBUG_2
  std::cout << center.toString() << std::endl;
#endif
  for (size_t i = 0; i < slice.getStripeNumber(); ++i) {
    int nseg = slice.m_stripeArray[i].getSegmentNumber();
    int y = slice.m_stripeArray[i].getY();
    for (int j = 0; j < nseg; ++j) {
      int x1 = slice.m_stripeArray[i].getSegmentStart(j);
      int x2 = slice.m_stripeArray[i].getSegmentEnd(j);
      for (int x = x1; x <= x2; ++x) {
        double dist = center.distanceTo(x, y, z);
        hist.addCount(dist, 1.0 / (dist + 0.5));
      }
    }
  }

  return hist;
}

ZObject3dScan ZObject3dScan::makeZProjection() const
{
  ZObject3dScan proj;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    proj.addStripe(0, m_stripeArray[i].getY());
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      proj.addSegment(x1, x2, false);
    }
  }

  proj.canonize();

  return proj;
}

ZObject3dScan ZObject3dScan::makeYProjection() const
{
  ZObject3dScan proj;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    proj.addStripe(0, m_stripeArray[i].getZ());
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      proj.addSegment(x1, x2, false);
    }
  }

  proj.canonize();

  return proj;
}


int ZObject3dScan::getMinZ() const
{
  int minZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      minZ = m_stripeArray[0].getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        minZ = z;
      } else {
        if (z < minZ) {
          minZ = z;
        }
      }
    }
  }

  return minZ;
}

int ZObject3dScan::getMaxZ() const
{
  int maxZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      maxZ = m_stripeArray.back().getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        maxZ = z;
      } else {
        if (z > maxZ) {
          maxZ = z;
        }
      }
    }
  }

  return maxZ;
}

int ZObject3dScan::getMinY() const
{
  int minY = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int y = m_stripeArray[i].getY();
    if (i == 0) {
      minY = y;
    } else {
      if (y < minY) {
        minY = y;
      }
    }
  }

  return minY;
}

int ZObject3dScan::getMaxY() const
{
  int maxY = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int y = m_stripeArray[i].getY();
    if (i == 0) {
      maxY = y;
    } else {
      if (y > maxY) {
        maxY = y;
      }
    }
  }

  return maxY;
}

ZVoxel ZObject3dScan::getMarker() const
{
  if (isEmpty()) {
    return ZVoxel(-1, -1, -1);
  }

#if 1
  int offset[3] = {0, 0, 0};
  ZVoxel voxel;
  size_t stripeNumber = getStripeNumber();
  std::vector<ZObject3dScan> sliceArray(getMaxZ() + 1);
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = getStripe(i);
    int currentZ = stripe.getZ();
    sliceArray[currentZ].addStripe(stripe, false);
  }

  for (size_t i = 0; i < sliceArray.size(); ++i) {
    ZObject3dScan &slice = sliceArray[i];

    if (!slice.isEmpty()) {
      int currentZ = slice.getStripe(0).getZ();
      Stack *stack = slice.toStack(offset);
      Stack *dist = Stack_Bwdist_L_U16P(stack, NULL, 0);
      size_t index;
      double v = sqrt(Stack_Max(dist, &index));
      if (v > voxel.value()) {
        int x, y, tmpz;
        C_Stack::indexToCoord(index, C_Stack::width(stack), C_Stack::height(stack),
                              &x, &y, &tmpz);

        voxel.set(x + offset[0], y + offset[1], currentZ, v);
      }
      Kill_Stack(dist);
      Kill_Stack(stack);
      slice.clear();
    }
  }
#else
  int minZ = getMinZ();
  int maxZ = getMaxZ();
  int offset[3];
  ZVoxel voxel;
  int zStep = 2;
  for (int z = minZ; z < maxZ; z += zStep) {
    ZObject3dScan slice = getSlice(z);
    if (!slice.isEmpty()) {
      Stack *stack = slice.toStack(offset);
      Stack *dist = Stack_Bwdist_L_U16P(stack, NULL, 0);
      size_t index;
      double v = sqrt(Stack_Max(dist, &index));
      if (v > voxel.value()) {
        int x, y, tmpz;
        C_Stack::indexToCoord(index, C_Stack::width(stack), C_Stack::height(stack),
                              &x, &y, &tmpz);

        voxel.set(x + offset[0], y + offset[1], z, v);
      }
      Kill_Stack(dist);
      Kill_Stack(stack);
    }
  }
#endif

  return voxel;
}

bool ZObject3dScan::equalsLiterally(const ZObject3dScan &obj) const
{
  if (m_stripeArray.size() != obj.m_stripeArray.size()) {
    return false;
  }

  for (size_t i = 0; i < m_stripeArray.size(); ++i) {
    if (!m_stripeArray[i].equalsLiterally(obj.m_stripeArray[i])) {
      return false;
    }
  }

  return true;
}

ZObject3dScan ZObject3dScan::getComplementObject()
{
  ZObject3dScan compObj;

  int offset[3];
  Stack *stack = toStack(offset);
  Stack_Not(stack, stack);
  compObj.loadStack(stack);
  compObj.translate(offset[0], offset[1], offset[2]);

  C_Stack::kill(stack);

  return compObj;
}

ZObject3dScan ZObject3dScan::findHoleObject()
{
  ZObject3dScan obj;

  ZObject3dScan compObj = getComplementObject();
  std::vector<ZObject3dScan> objList = compObj.getConnectedComponent();

  Cuboid_I boundBox;
  getBoundBox(&boundBox);
  for (std::vector<ZObject3dScan>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZObject3dScan &subobj = *iter;
#ifdef _DEBUG_2
    subobj.print();
#endif
    Cuboid_I subbox;
    subobj.getBoundBox(&subbox);
    if (Cuboid_I_Hit_Internal(&boundBox, subbox.cb[0], subbox.cb[1], subbox.cb[2]) &&
        Cuboid_I_Hit_Internal(&boundBox, subbox.ce[0], subbox.ce[1], subbox.ce[2])) {
      obj.unify(subobj);
    }
  }

  return obj;
}

std::vector<ZObject3dScan> ZObject3dScan::findHoleObjectArray()
{
  std::vector<ZObject3dScan> objArray;

  ZObject3dScan compObj = getComplementObject();
  std::vector<ZObject3dScan> objList = compObj.getConnectedComponent();

  Cuboid_I boundBox;
  getBoundBox(&boundBox);
  for (std::vector<ZObject3dScan>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZObject3dScan &subobj = *iter;
    Cuboid_I subbox;
    subobj.getBoundBox(&subbox);
    if (Cuboid_I_Hit_Internal(&boundBox, subbox.cb[0], subbox.cb[1], subbox.cb[2]) &&
        Cuboid_I_Hit_Internal(&boundBox, subbox.ce[0], subbox.ce[1], subbox.ce[2])) {
      objArray.push_back(subobj);
    }
  }

  return objArray;
}

void ZObject3dScan::fillHole()
{
  ZObject3dScan holeObj = findHoleObject();
  unify(holeObj);
}

#define READ_TEST(real, expected, action) \
  if (real != expected) { \
    action; \
    fclose(fp); \
    return false; \
  }

bool ZObject3dScan::importDvidObject(const std::string &filePath)
{
  clear();

  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    tz_uint8 flag = 0;
    size_t n = fread(&flag, 1, 1, fp);
    READ_TEST(n, 1, RECORD_ERROR_UNCOND("Reading error"));

    tz_uint8 numberOfDimensions = 0;
    n = fread(&numberOfDimensions, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    if (numberOfDimensions != 3) {
      RECORD_ERROR_UNCOND("Current version only supports 3D");
      fclose(fp);
      return false;
    }

    tz_uint8 dimOfRun = 0;
    n = fread(&dimOfRun, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    if (dimOfRun != 0) {
      RECORD_ERROR_UNCOND("Unspported run dimension");
      fclose(fp);
      return false;
    }

    tz_uint8 reserved = 0;
    n = fread(&reserved, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    tz_uint32 numberOfVoxels = 0;
    n = fread(&numberOfVoxels, 4, 1, fp);

#ifdef _DEBUG_
    std::cout << "#dvid voxels: " << numberOfVoxels << std::endl;
#endif

    if (n != 1) {
      fclose(fp);
      return false;
    }

    tz_uint32 numberOfSpans = 0;
    n = fread(&numberOfSpans, 4, 1, fp);
    READ_TEST(n, 1, RECORD_ERROR_UNCOND("Failed to read number of spans"));

    tz_uint32 readSegmentNumber = 0;
    while (!feof(fp)) {
      tz_int32 coord[3];
      n = fread(coord, 4, 3, fp);
      READ_TEST(n, 3, RECORD_ERROR_UNCOND("Failed to read span"));

      if (feof(fp)) {
        RECORD_ERROR_UNCOND("Reading error. File ends prematurely")
      }

      tz_int32 runLength;
      n = fread(&runLength, 4, 1, fp);
      READ_TEST(n, 1, RECORD_ERROR_UNCOND("Failed to read run length"));
      if (runLength <= 0) {
        fclose(fp);
        RECORD_ERROR_UNCOND("Invalid run length");
        return false;
      }

      addSegment(coord[2], coord[1], coord[0], coord[0] + runLength, false);
      ++readSegmentNumber;
      if (readSegmentNumber == numberOfSpans) {
        break;
      }
    }

    if (readSegmentNumber != numberOfSpans) {
      RECORD_ERROR_UNCOND("Unmatched numbers of spans");
      fclose(fp);
      return false;
    }
  } else {
    RECORD_ERROR_UNCOND("Unable to open file");
    return false;
  }

  fclose(fp);
  return true;
}

#define READ_BYTE_BUFFER(target, type) \
  if (byteNumber < sizeof(type)) { \
    RECORD_ERROR_UNCOND("Buffer ended prematurely."); \
    return false; \
  } \
  target = *(const type*)(byteArray + currentIndex); \
  currentIndex += sizeof(type); \
  byteNumber -= sizeof(type);

bool ZObject3dScan::importDvidObjectBuffer(
    const char *byteArray, size_t byteNumber)
{
  clear();

  if (byteArray == NULL || byteNumber <= 12) {
    RECORD_ERROR_UNCOND("Invalid byte buffer");
    return false;
  }

  size_t currentIndex = 0;
  tz_uint8 flag = 0;
  READ_BYTE_BUFFER(flag, tz_uint8);


  tz_uint8 numberOfDimensions = 0;
  READ_BYTE_BUFFER(numberOfDimensions, tz_uint8);

  if (numberOfDimensions != 3) {
    RECORD_ERROR_UNCOND("Current version only supports 3D");
    return false;
  }

  tz_uint8 dimOfRun = 0;
  READ_BYTE_BUFFER(dimOfRun, tz_uint8);
  if (dimOfRun != 0) {
    RECORD_ERROR_UNCOND("Unspported run dimension");
    return false;
  }

  //tz_uint8 reserved = *static_cast<tz_uint8*>(byteArray + currentIndex);
  ++currentIndex;

  //tz_uint32 numberOfVoxels = *static_cast<tz_uint32*>(byteArray + currentIndex);
  currentIndex += 4;

  tz_uint32 numberOfSpans = 0;
  READ_BYTE_BUFFER(numberOfSpans, tz_uint32);

  for (tz_uint32 span = 0; span < numberOfSpans; ++span) {
    tz_int32 coord[3];
    for (int i = 0; i < 3; ++i) {
      READ_BYTE_BUFFER(coord[i], tz_int32);
    }

    tz_int32 runLength = 0;
    READ_BYTE_BUFFER(runLength, tz_int32);

    if (runLength <= 0) {
      RECORD_ERROR_UNCOND("Invalid run length");
      return false;
    }

    addSegment(coord[2], coord[1], coord[0], coord[0] + runLength - 1, false);
  }

  return true;
}

bool ZObject3dScan::importDvidObjectBuffer(
    const std::vector<char> &byteArray)
{
  return importDvidObjectBuffer(&(byteArray[0]), byteArray.size());
}

void ZObject3dScan::addForeground(ZStack *stack)
{
  ConstSegmentIterator iterator(this);
  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();
    int z = seg.getZ();
    int y = seg.getY();
    for (int x = seg.getStart(); x <= seg.getEnd(); ++x) {
      if (stack->getIntValue(x, y, z) > 0) {
        stack->addIntValue(x, y, z, 0, 1);
      }
    }
  }
}

ZObject3dScan ZObject3dScan::subtract(const ZObject3dScan &obj)
{
  int minZ = getMinZ();
  int maxZ = getMaxZ();

  ZObject3dScan subtracted;
  ZObject3dScan remained;

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);
    if (slice2.isEmpty()) {
      remained.concat(slice);
    } else {
      ZStack *plane = slice.toStackObject();
      slice2.addForeground(plane); //1: remained; 2: subtracted

      std::vector<ZObject3dScan*> objArray = extractAllObject(*plane);
      for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3dScan *obj = *iter;
        if (obj->getLabel() == 1) {
          remained.concat(*obj);
        } else if (obj->getLabel() == 2) {
          subtracted.concat(*obj);
        }
        delete obj;
      }
      delete plane;
    }
  }

  remained.canonize();
  subtracted.canonize();

  *this = remained;

  return subtracted;
}

void ZObject3dScan::switchYZ()
{
  for (size_t i = 0; i < m_stripeArray.size(); ++i) {
    m_stripeArray[i].switchYZ();
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED | EVENT_OBJECT_UNCANONIZED);
}

std::vector<double> ZObject3dScan::getPlaneCov() const
{
  std::vector<double> cov(3, 0.0);

  if (!isEmpty()) {
    Cuboid_I boundBox;
    getBoundBox(&boundBox);

    double xMean = 0.0;
    double yMean = 0.0;
    double xSquareMean = 0.0;
    double ySquareMean = 0.0;
    double xyCorr = 0.0;

    size_t count = 0;
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      int y = iter->getY();
      int nseg = iter->getSegmentNumber();
      for (int i = 0; i < nseg; ++i) {
        const int *seg = iter->getSegment(i);
        for (int x = seg[0]; x <= seg[1]; ++x) {
          xMean += x;
          xSquareMean += x * x;
          yMean += y;
          ySquareMean += y * y;
          xyCorr += x * y;
          ++count;
        }
      }
    }

    xMean /= count;
    yMean /= count;
    xSquareMean /= count;
    ySquareMean /= count;
    xyCorr /= count;

#ifdef _DEBUG_2
    std::cout << count << std::endl;
#endif

    double factor = 1.0;
    if (count > 1) {
      factor = static_cast<double>(count) / (count - 1);
    }

    cov[0] = (xSquareMean - xMean * xMean) * factor;
    cov[1] = (ySquareMean - yMean * yMean) * factor;
    cov[2] = (xyCorr - xMean * yMean) * factor;
  }

  return cov;
}

double ZObject3dScan::getSpread(int z) const
{
  ZObject3dScan objSlice = getSlice(z, z);
  const std::vector<double> cov = objSlice.getPlaneCov();
  ZEigenSolver solver;
  if (solver.solveCovEigen(cov)) {
    return sqrt(solver.getEigenValue(0) * solver.getEigenValue(1)) * 2.0;
  }

  return 0.0;
}

bool ZObject3dScan::contains(int x, int y, int z)
{
  canonize();

  //Binary search
  size_t lowIndex = 0;
  size_t highIndex = getStripeNumber();

  while (highIndex >= lowIndex) {
    size_t medIndex = (lowIndex + highIndex) / 2;
    ZObject3dStripe &stripe = m_stripeArray[medIndex];
    if (stripe.getZ() == z && stripe.getY() == y) {
      return stripe.containsX(x);
    } else {
      //On the left side
      if (stripe.getZ() > z || (stripe.getZ() == z && stripe.getY() > y)) {
        if (medIndex == 0) {
          if (highIndex == 1) {
            return m_stripeArray[highIndex].contains(x, y, z);
          }
          return false;
        } else {
          highIndex = medIndex - 1;
        }
      } else { //On the right side
        lowIndex = medIndex + 1;
      }
    }
  }

  return false;
}

void ZObject3dScan::processEvent(TEvent event)
{
  if (event & EVENT_OBJECT_MODEL_CHANGED & ~EVENT_OBJECT_VIEW_CHANGED) {
    deprecate(ACCUMULATED_STRIPE_NUMBER);
    deprecate(SLICEWISE_VOXEL_NUMBER);
  }

  if (event & EVENT_OBJECT_UNCANONIZED & ~EVENT_OBJECT_VIEW_CHANGED) {
    setCanonized(false);
  }

  if (event & EVENT_OBJECT_CANONIZED & ~EVENT_OBJECT_VIEW_CHANGED) {
    setCanonized(true);
  }

  if (event & EVENT_OBJECT_VIEW_CHANGED) {
    deprecate(STRIPE_INDEX_MAP);
    deprecate(INDEX_SEGMENT_MAP);
  }
}

#define VERIFY_DATA(condition) \
  if (!(condition)) { \
    clear(); \
    return false; \
  }

#define CHECK_DATA_LENGTH(s)  VERIFY_DATA(length >= (size_t) (s))

bool ZObject3dScan::load(const int *data, size_t length)
{
  clear();

  if (data != NULL) {
    m_stripeArray.resize(*(data++));
    --length;
    for (std::vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      CHECK_DATA_LENGTH(3)

      ZObject3dStripe &stripe = *iter;
      stripe.setZ(*(data++));
      stripe.setY(*(data++));
      int nseg = *(data++);

      VERIFY_DATA(nseg > 0)

      length -= 3;

      CHECK_DATA_LENGTH(nseg * 2)

      for (int i = 0; i < nseg; ++i) {
        stripe.addSegment(data[0], data[1], false);
        data += 2;
      }

      length -= nseg * 2;
    }
  }

  return true;
}

std::vector<int> ZObject3dScan::toIntArray() const
{ 
  std::vector<int> array;
  if (!isEmpty()) {
    //Count total size
    size_t arraySize = 1;
    for (std::vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      const ZObject3dStripe &stripe = *iter;
      arraySize += 3 + stripe.getSegmentNumber() * 2;
    }

    array.resize(arraySize);
    array[0] = getStripeNumber();
//    array.push_back(getStripeNumber());
    size_t index = 1;
    for (std::vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      const ZObject3dStripe &stripe = *iter;
      size_t stripeSize = 3 + stripe.getSegmentNumber() * 2;
//      array.resize(array.size() + stripeSize);
      stripe.fillIntArray(&(array[index]));
      index += stripeSize;
      /*
      array.push_back(stripe.getZ());
      array.push_back(stripe.getY());
      array.push_back(stripe.getSegmentNumber());
      for (int i = 0; i < stripe.getSegmentNumber(); ++i) {
        array.push_back(stripe.getSegmentStart(i));
        array.push_back(stripe.getSegmentEnd(i));
      }
      */
    }
  }

  return array;
}

bool ZObject3dScan::importHdf5(const string &filePath, const string &key)
{
  clear();

  ZHdf5Reader reader;
  if (reader.open(filePath)) {
    std::vector<int> array = reader.readIntArray(key);
    if (!array.empty()) {
      return load(&(array[0]), array.size());
    }
  }

  return false;
}


size_t ZObject3dScan::getSurfaceArea() const
{
  size_t area = 0;

  int minZ = getMinZ();
  int maxZ = getMaxZ();

  area += getSlice(minZ).getVoxelNumber();
  if (maxZ > minZ) {
    area += getSlice(maxZ).getVoxelNumber();
  }

  for (int z = minZ + 1; z < maxZ - 1; ++z) {
    ZObject3dScan subobj = getSlice(z - 1, z + 1);

    Stack *stack = subobj.toStack();
    Stack *edge = Stack_Perimeter(stack, NULL, 26);
    C_Stack::kill(stack);

    Stack slice = C_Stack::sliceView(edge, 1);
    area += Stack_Sum(&slice);

    C_Stack::kill(edge);
  }

  return area;
}

/////////////////////////Iterators/////////////////////////
ZObject3dScan::ConstSegmentIterator::ConstSegmentIterator(
    const ZObject3dScan *obj) : m_obj(obj), m_stripeIndex(0), m_segmentIndex(0)
{
}

const ZObject3dScan::Segment& ZObject3dScan::ConstSegmentIterator::next()
{
  if (hasNext()) {
    const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
    m_seg.set(stripe.getZ(), stripe.getY(),
              stripe.getSegmentStart(m_segmentIndex),
              stripe.getSegmentEnd(m_segmentIndex));
    advance();
  }
  return m_seg;
}

bool ZObject3dScan::ConstSegmentIterator::hasNext() const
{
  if (m_obj == NULL) {
    return false;
  }
  if (m_stripeIndex >= m_obj->getStripeNumber()) {
    return false;
  }

  const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
  if (m_segmentIndex >= stripe.getSegmentNumber()) {
    return false;
  }

  return true;
}

void ZObject3dScan::ConstSegmentIterator::advance()
{
  if (m_stripeIndex < m_obj->getStripeNumber()) {
    const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
    if (m_segmentIndex < stripe.getSegmentNumber() - 1) {
      ++m_segmentIndex;
    } else {
      m_segmentIndex = 0;
      ++m_stripeIndex;
    }
  }
}

std::vector<ZObject3dScan*> ZObject3dScan::extractAllObject(const ZStack &stack)
{
  std::vector<ZObject3dScan*> result;
  std::map<int, ZObject3dScan*> *objMap = NULL;

  switch (stack.kind()) {
  case GREY:
    objMap = extractAllForegroundObject(
          stack.array8(), stack.width(), stack.height(), stack.depth(),
          stack.getOffset().getX(), stack.getOffset().getY(),
          stack.getOffset().getZ(), NULL);
    break;
  case GREY16:
    objMap = extractAllForegroundObject(
          stack.array16(), stack.width(), stack.height(), stack.depth(),
          stack.getOffset().getX(), stack.getOffset().getY(),
          stack.getOffset().getZ(), NULL);
    break;
  default:
    break;
  }

  if (objMap != NULL) {
    for (std::map<int, ZObject3dScan*>::iterator iter = objMap->begin();
         iter != objMap->end(); ++iter) {
      result.push_back(iter->second);
    }

    delete objMap;
  }

  return result;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZObject3dScan)
