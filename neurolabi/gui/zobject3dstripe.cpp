#include "zobject3dstripe.h"

#include <cstring>

#include "tz_error.h"
#include "zerror.h"
#include "tz_math.h"
#include "geometry/zgeometry.h"

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
        x1 = std::min(x1, C_Stack::width(stack) - 1);
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
        x1 = std::min(x1, C_Stack::width(stack) - 1);
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

void ZObject3dStripe::drawStack(Stack *stack, int v, NeuTube::EAxis axis,
                                const int *offset) const
{
  switch (axis) {
  case NeuTube::Z_AXIS:
    drawStack(stack, v, offset);
    break;
  case NeuTube::X_AXIS:
  case NeuTube::Y_AXIS:
    if (C_Stack::kind(stack) == GREY || C_Stack::kind(stack) != GREY16) {
      Image_Array ima;
      ima.array = stack->array;
      int dx = 0;
      int dy = 0;
      int dz = 0;
      if (offset != NULL) {
        dx = offset[0];
        dy = offset[1];
        dz = offset[2];
        ZGeometry::shiftSliceAxis(dx, dy, dz, axis);
      }

      int y = getY();
      int z = getZ();

      if (offset != NULL) {
        y += dy;
        z += dz;
      }

      int shiftedWidth = C_Stack::width(stack);
      int shiftedHeight = C_Stack::height(stack);
      int shiftedDepth = C_Stack::depth(stack);
      ZGeometry::shiftSliceAxis(shiftedWidth, shiftedHeight, shiftedDepth, axis);
      if (y >= shiftedHeight) {
        return;
      }

      if (z >= shiftedDepth) {
        return;
      }

      size_t area = C_Stack::width(stack) * C_Stack::height(stack);
      size_t arrayOffset = 0;

      int stride = 1;
      if (axis == NeuTube::Y_AXIS) {
        arrayOffset = C_Stack::width(stack) * y + area * z;
      } else {
        arrayOffset = area * y + z;
        stride = C_Stack::width(stack);
      }

      switch (C_Stack::kind(stack)) {
      case GREY:
        ima.array8 += arrayOffset;
        v = (v < 0) ? 0 : ((v > 255) ? 255 : v);
        for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
          int x0 = m_segmentArray[i];
          int x1 = m_segmentArray[i + 1];
          if (offset != NULL) {
            x0 += dx;
            x1 += dx;
          }
          if (x0 < shiftedWidth) {
            x1 = std::min(x1, shiftedWidth - 1);
            for (int x = x0; x <= x1; ++x) {
              ima.array8[x * stride] = v;
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
            x0 += dx;
            x1 += dx;
          }
          if (x0 < shiftedWidth) {
            x1 = std::min(x1, shiftedWidth - 1);
            for (int x = x0; x <= x1; ++x) {
              ima.array16[x * stride] = v;
            }
          }
        }
        break;
      default:
        break;
      }
    } else {
      RECORD_ERROR(true, "Unsupported kind");
      return;
    }

    break;
  }
}

void ZObject3dStripe::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
    const int *offset) const
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

void ZObject3dStripe::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, double alpha,
    const int *offset) const
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
  uint8_t color[3];
  color[0] = red;
  color[1] = green;
  color[2] = blue;
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
      for (int c = 0; c < 3; ++c) {
//        if (color[c] != 0) {
          double v = ima.arrayc[x][c] * (1.0 - alpha) + color[c] * alpha;
          ima.arrayc[x][c] = iround(v);
//        }
      }
    }
  }
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
      std::vector<int> newSegmentArray(m_segmentArray.size());
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
    std::vector<int> newSegmentArray;
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
  std::cout << "Segments at " << "." << m_z << "|" << m_y << ": " << std::endl;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    for (int k = 0; k < indent; ++k) {
      std::cout << " ";
    }
    std::cout << "  " << m_segmentArray[i] << " - " << m_segmentArray[i+1] << std::endl;
  }
}

#define MOVE_SEGMENT(seg, nseg, s, start, end) \
  ++seg;\
  if (seg >= nseg) {\
    break;\
  }\
  start = s.getSegmentStart(seg);\
  end = s.getSegmentEnd(seg);

#define MOVE_FIRST_SEGMENT \
  ++seg1;\
  if (seg1 >= nseg1) {\
    break;\
  }\
  s1Start = s1.getSegmentStart(seg1);\
  s1End = s1.getSegmentEnd(seg1);

#define MOVE_SECOND_SEGMENT \
  ++seg2;\
  if (seg2 >= nseg2) {\
    break;\
  }\
  s2Start = s2.getSegmentStart(seg2);\
  s2End = s2.getSegmentEnd(seg2);


ZObject3dStripe operator - (
    const ZObject3dStripe &s1, const ZObject3dStripe &s2)
{
  ZObject3dStripe result;

  if ((s1.getY() == s2.getY()) && (s1.getZ() == s2.getZ()) &&
      !s1.m_segmentArray.empty() && !s2.m_segmentArray.empty()) {
    result.setY(s1.getY());
    result.setZ(s2.getZ());

    const_cast<ZObject3dStripe&>(s1).canonize();
    const_cast<ZObject3dStripe&>(s2).canonize();

    int seg1 = 0;
    int seg2 = 0;

    int nseg1 = s1.getSegmentNumber();
    int nseg2 = s2.getSegmentNumber();

    int s1Start = s1.getSegmentStart(0);
    int s1End = s1.getSegmentEnd(0);
    int s2Start = s2.getSegmentStart(0);
    int s2End = s2.getSegmentEnd(0);
//    int currentStart = s1Start;
//    int currentEnd = s1End;

    while (seg1 < nseg1 && seg2 < nseg2) {
      if (s2End < s1Start) {
        MOVE_SECOND_SEGMENT;
      } else if (s2Start <= s1Start) {
        if (s2End >= s1End) {
          MOVE_FIRST_SEGMENT;
        } else {
          s1Start = s2End + 1;
          MOVE_SECOND_SEGMENT;
        }
      } else if (s2Start <= s1End) {
        result.addSegment(s1Start, s2Start - 1, false);
        if (s2End < s1End) {
          s1Start = s2End + 1;
          MOVE_SECOND_SEGMENT;
        } else {
          MOVE_FIRST_SEGMENT;
        }
      } else {
        result.addSegment(s1Start, s1End, false);
        MOVE_FIRST_SEGMENT;
      }
    }

    while (seg1 < nseg1) {
      result.addSegment(s1Start, s1End, false);
      MOVE_FIRST_SEGMENT;
    }

    result.setCanonized(true);
  } else {
    result = s1;
  }

  return result;
}
