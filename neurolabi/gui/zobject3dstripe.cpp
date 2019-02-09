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
       int &lastStart = m_segmentArray[m_segmentArray.size() - 2];
       if (x1 >= lastStart) {
         m_segmentArray.back() = imax2(x2, m_segmentArray.back());
       } else if (x2 >= lastStart - 1){
         lastStart = x1;
         m_segmentArray.back() = imax2(x2, m_segmentArray.back());
         if (m_segmentArray.size() > 2) {
           if (lastStart - m_segmentArray[m_segmentArray.size() - 3] <= 1) {
             m_isCanonized = false;
           }
         }
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

size_t ZObject3dStripe::getByteCount() const
{
  return sizeof(int) * (2 + m_segmentArray.size());
}

bool ZObject3dStripe::hasVoxel() const
{
  return !m_segmentArray.empty();
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

void ZObject3dStripe::write(std::ostream &stream) const
{
  stream.write((const char*)(&(m_z)), sizeof(int));
  stream.write((const char*)(&(m_y)), sizeof(int));
  int n = m_segmentArray.size();
  stream.write((const char*)(&(n)), sizeof(int));
  stream.write((const char*)(&(m_segmentArray[0])), sizeof(int) * n);
}

/*
std::ostream& operator<<(
      std::ostream &stream, const ZObject3dStripe &stripe)
{
  stripe.write(stream);

  return stream;
}

std::istream& operator>>(
    std::istream &stream, ZObject3dStripe &stripe)
{
  stripe.read(stream);

  return stream;
}
*/

void ZObject3dStripe::read(FILE *fp)
{
  if (fp != NULL) {
    fread(&(m_z), sizeof(int), 1, fp);
    fread(&(m_y), sizeof(int), 1, fp);
    int nseg = 0;
    fread(&(nseg), sizeof(int), 1, fp);

    if (nseg > 0) {
      m_segmentArray.resize(nseg * 2);
      fread(&(m_segmentArray[0]), sizeof(int), m_segmentArray.size(), fp);
      m_isCanonized = false;
    } else {
      m_segmentArray.clear();
    }
  }
}

void ZObject3dStripe::read(std::istream &stream)
{
  stream.read((char*)(&m_z), sizeof(int));
  stream.read((char*)(&m_y), sizeof(int));
  int n = 0;
  stream.read((char*)(&n), sizeof(int));
  if (n > 0) {
    m_segmentArray.resize(n);
    stream.read((char*)(&(m_segmentArray[0])), sizeof(int) * m_segmentArray.size());
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


template<typename T>
void ZObject3dStripe::addArray(
    T *array, int v, int minV, int maxV, int width, const int *offset) const
{
//  v = (v < minV) ? minV : ((v > maxV) ? maxV : v);
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    if (x0 < width) {
      x1 = std::min(x1, width - 1);
      for (int x = x0; x <= x1; ++x) {
        int tmpV = v + array[x];
        array[x] = (tmpV < minV) ? minV : ((tmpV > maxV) ? maxV : tmpV);
      }
    }
  }
}

void ZObject3dStripe::addStackValue(Stack *stack, int v, const int *offset) const
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
    addArray(ima.array8, v, 0, 255, C_Stack::width(stack), offset);
    break;
  case GREY16:
    ima.array16 += arrayOffset;
    addArray(ima.array16, v, 0, 65535, C_Stack::width(stack), offset);
    break;
  default:
    break;
  }
}

template<typename T>
void ZObject3dStripe::drawArray(
    T *array, int v, int minV, int maxV, int width, const int *offset) const
{
  v = (v < minV) ? minV : ((v > maxV) ? maxV : v);
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    if (x0 < width) {
      x1 = std::min(x1, width - 1);
      for (int x = x0; x <= x1; ++x) {
        array[x] = v;
      }
    }
  }
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

  if (y < 0 || y >= C_Stack::height(stack)) {
    return;
  }

  if (z < 0 || z >= C_Stack::depth(stack)) {
    return;
  }

  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t arrayOffset = area * z + C_Stack::width(stack) * y;

  switch (C_Stack::kind(stack)) {
  case GREY:
    ima.array8 += arrayOffset;
    drawArray(ima.array8, v, 0, 255, C_Stack::width(stack), offset);
    break;
  case GREY16:
    ima.array16 += arrayOffset;
    drawArray(ima.array16, v, 0, 65535, C_Stack::width(stack), offset);
    break;
  default:
    break;
  }
}

void ZObject3dStripe::drawStack(Stack *stack, int v, neutu::EAxis axis,
                                const int *offset) const
{
  switch (axis) {
  case neutu::EAxis::Z:
  case neutu::EAxis::ARB:
    drawStack(stack, v, offset);
    break;
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
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
        zgeom::shiftSliceAxis(dx, dy, dz, axis);
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
      zgeom::shiftSliceAxis(shiftedWidth, shiftedHeight, shiftedDepth, axis);
      if (y >= shiftedHeight) {
        return;
      }

      if (z >= shiftedDepth) {
        return;
      }

      size_t area = C_Stack::width(stack) * C_Stack::height(stack);
      size_t arrayOffset = 0;

      int stride = 1;
      if (axis == neutu::EAxis::Y) {
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

  if (y < 0 || z < 0) {
    return;
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
    if (x0 < 0) {
      x0 = 0;
    }
    if (x1 >= C_Stack::width(stack)) {
      x1 = C_Stack::width(stack) - 1;
    }

//    TZ_ASSERT(x0 < C_Stack::width(stack), "x too large");
//    TZ_ASSERT(x1 < C_Stack::width(stack), "x too large");
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
  if (!m_segmentArray.empty() && !m_isCanonized) {
    qsort(&m_segmentArray[0], m_segmentArray.size() / 2, sizeof(int) * 2,
        ZObject3dSegmentCompare);
  }
}

void ZObject3dStripe::sortedCanonize()
{
  if (!m_isCanonized) {
    if (!m_segmentArray.empty()) {
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

void ZObject3dStripe::canonize()
{
  if (!m_isCanonized) {
    if (!m_segmentArray.empty()) {
      sort();
      sortedCanonize();
    }

    m_isCanonized = true;
  }
}

bool ZObject3dStripe::unify(const ZObject3dStripe &stripe, bool canonizing)
{
  if (isEmpty()) {
    *this = stripe;
    if (canonizing) {
      canonize();
    }
    return true;
  } else {
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

void ZObject3dStripe::remove(int bx0, int bx1)
{
  sort();

  if (getMinX() >= bx0 && getMaxX() <= bx1) {
    m_segmentArray.clear();
  } else if (getMinX() <= bx1 && getMaxX() >= bx0) {
    std::vector<int> newSegmentArray;
    newSegmentArray.resize(m_segmentArray.size() + 2);
    size_t currentIndex = 0;
    size_t remainingIndex = 0;

    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i + 1];
      remainingIndex = i;
      if (bx0 > x1) { //||[]
        newSegmentArray[currentIndex] = x0;
        newSegmentArray[currentIndex + 1] = x1;
        currentIndex += 2;
        remainingIndex = i + 2;
      } else {
        if (bx0 <= x0) { //[|
          if (bx1 < x0) { // []||
            break;
          } else if (bx1 < x1) { // [|]|
            newSegmentArray[currentIndex] =bx1 + 1;
            newSegmentArray[currentIndex + 1] = x1;
            currentIndex += 2;
            remainingIndex = i + 2;
            break;
          } else { // [||]
            remainingIndex = i + 2;
          }
        } else if (bx0 <= x1) { // |[|
          newSegmentArray[currentIndex] = x0;
          newSegmentArray[currentIndex + 1] = bx0 - 1;
          currentIndex += 2;
          remainingIndex = i + 2;
          if (bx1 < x1) { //|[]|
            newSegmentArray[currentIndex] = bx1 + 1;
            newSegmentArray[currentIndex + 1] = x1;
            currentIndex += 2;
          }/* else { //|[|]
        }*/
        }
      }
    }


    for (size_t i = remainingIndex; i < m_segmentArray.size(); i++) {
      newSegmentArray[currentIndex++] = m_segmentArray[i];
    }

    newSegmentArray.resize(currentIndex);

    m_segmentArray.swap(newSegmentArray);
  }
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

ZObject3dStripe ZObject3dStripe::getComplement(int x0, int x1)
{
  ZObject3dStripe stripe;
  stripe.setY(getY());
  stripe.setZ(getZ());

  stripe.addSegment(x0, x1);

  return stripe - (*this);
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

namespace {
inline bool HasPotentialOverlap(
    const ZObject3dStripe &s1, const ZObject3dStripe &s2)
{
  return ((s1.getY() == s2.getY()) && (s1.getZ() == s2.getZ()) &&
          !s1.isEmpty() && !s2.isEmpty() &&
          s1.getSegmentArray().back() >= s2.getSegmentArray().front() &&
          s2.getSegmentArray().back() >= s1.getSegmentArray().front());
}

inline bool HasPotentialAdjacency(
    const ZObject3dStripe &s1, const ZObject3dStripe &s2,int diffYZ,
    neutu::EStackNeighborhood nbr)
{
  bool goodDiffYZ = true;
  switch (nbr) {
  case neutu::EStackNeighborhood::D1:
    goodDiffYZ = (diffYZ <= 1);
    break;
  case neutu::EStackNeighborhood::D2:
  case neutu::EStackNeighborhood::D3:
    goodDiffYZ = (diffYZ <= 2);
    break;
  }


  return (goodDiffYZ &&
          !s1.isEmpty() && !s2.isEmpty() &&
          s1.getSegmentArray().back() + 1 >= s2.getSegmentArray().front() &&
          s2.getSegmentArray().back() + 1 >= s1.getSegmentArray().front());
}

}

//6-conn neighborhood
bool ZObject3dStripe::isAdjacentTo(
    const ZObject3dStripe &stripe, neutu::EStackNeighborhood nbr) const
{
  ZObject3dStripe &s1 = const_cast<ZObject3dStripe&>(*this);
  ZObject3dStripe &s2 = const_cast<ZObject3dStripe&>(stripe);

  bool adjacent = false;

  int diffY = std::abs(s1.getY() - s2.getY());
  int diffZ = std::abs(s1.getZ() - s2.getZ());
  if (diffY > 1 || diffZ > 1) {
    return false;
  }

   int diffYZ = diffY + diffZ;
  if (HasPotentialAdjacency(s1, s2, diffYZ, nbr)) {
    int dg = 1;

    switch (nbr) {
    case neutu::EStackNeighborhood::D1:
      dg = 1 - diffYZ;
      break;
    case neutu::EStackNeighborhood::D2:
      if (diffYZ == 2) { //X must overlap
        dg = 0;
      } else { //X can be adjacent
        dg = 1;
      }
      break;
    case neutu::EStackNeighborhood::D3:
      break;
    }

    int seg1 = 0;
    int seg2 = 0;

    int nseg1 = s1.getSegmentNumber();
    int nseg2 = s2.getSegmentNumber();

    int s1Start = s1.getSegmentStart(0);
    int s1End = s1.getSegmentEnd(0);
    int s2Start = s2.getSegmentStart(0);
    int s2End = s2.getSegmentEnd(0);

    while (seg1 < nseg1 && seg2 < nseg2) {
      if (s2End + dg < s1Start) { //t2 - s1
        MOVE_SECOND_SEGMENT;
      } else if (s1End + dg < s2Start) { // t1 - t2
        MOVE_FIRST_SEGMENT;
      } else {
        adjacent = true;
        break;
      }
    }
  }

  return adjacent;
}

bool ZObject3dStripe::isAdjacentOnPlaneTo(const ZObject3dStripe &stripe) const
{
  ZObject3dStripe &s1 = const_cast<ZObject3dStripe&>(*this);
  ZObject3dStripe &s2 = const_cast<ZObject3dStripe&>(stripe);

  bool adjacent = false;

  if (HasPotentialOverlap(s1, s2)) {
    int seg1 = 0;
    int seg2 = 0;

    int nseg1 = s1.getSegmentNumber();
    int nseg2 = s2.getSegmentNumber();

    int s1Start = s1.getSegmentStart(0);
    int s1End = s1.getSegmentEnd(0);
    int s2Start = s2.getSegmentStart(0);
    int s2End = s2.getSegmentEnd(0);

    while (seg1 < nseg1 && seg2 < nseg2) {
      if (s2End + 1 < s1Start) { //t2 - s1
        MOVE_SECOND_SEGMENT;
      } else if (s1End + 1 < s2Start) { // t1 - t2
        MOVE_FIRST_SEGMENT;
      } else {
        adjacent = true;
        break;
      }
    }
  }

  return adjacent;
}

bool ZObject3dStripe::hasOverlap(const ZObject3dStripe &stripe) const
{
  ZObject3dStripe &s1 = const_cast<ZObject3dStripe&>(*this);
  ZObject3dStripe &s2 = const_cast<ZObject3dStripe&>(stripe);

  bool overlapping = false;

  if (HasPotentialOverlap(s1, s2)) {
    int seg1 = 0;
    int seg2 = 0;

    int nseg1 = s1.getSegmentNumber();
    int nseg2 = s2.getSegmentNumber();

    int s1Start = s1.getSegmentStart(0);
    int s1End = s1.getSegmentEnd(0);
    int s2Start = s2.getSegmentStart(0);
    int s2End = s2.getSegmentEnd(0);

    while (seg1 < nseg1 && seg2 < nseg2) {
      if (s2End < s1Start) { //t2 - s1
        MOVE_SECOND_SEGMENT;
      } else if (s1End < s2Start) { // t1 - t2
        MOVE_FIRST_SEGMENT;
      } else {
        overlapping = true;
        break;
      }
    }
  }

  return overlapping;
}

