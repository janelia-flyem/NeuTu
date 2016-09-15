#include "c_stack.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <cmath>

#include "tz_error.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_stack_io.h"
#include "tz_stack_stat.h"
#include "zfiletype.h"
#include "zobject3dscan.h"
#include "zobject3d.h"
#include "neutubeconfig.h"
#include "zerror.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"
#include "tz_int_histogram.h"
#include "tz_iarray.h"


#if _QT_GUI_USED_
#include <QMutex>
#include <QMutexLocker>

static QMutex stackMutex;
#  define STACK_MUTEX_GUARD QMutexLocker locker(&stackMutex);
#else
#  define STACK_MUTEX_GUARD
#endif

using namespace std;

void C_Stack::copyPlaneValue(Stack *stack, const void *array, int slice)
{
  PROCESS_ERROR(stack == NULL, "Null stack", return);
  PROCESS_ERROR(slice < 0 || slice >= C_Stack::depth(stack), "Invalid slice",
                return);

  if (array != NULL) {
    size_t byteNumber = C_Stack::planeByteNumber(stack);
    memcpy(stack->array + byteNumber * slice, array, byteNumber);
  }
}

void C_Stack::copyPlaneValue(
    Mc_Stack *stack, const void *array, int channel, int slice)
{
  PROCESS_ERROR(stack == NULL, "Null stack", return);
  PROCESS_ERROR(slice < 0 || slice >= C_Stack::depth(stack), "Invalid slice",
                return);
  PROCESS_ERROR(channel < 0 || channel >= C_Stack::channelNumber(stack),
                "Invalid channel", return);

  //TZ_ASSERT(stack != NULL, "Null stack.");
  //TZ_ASSERT(channel >= 0 && channel < C_Stack::channelNumber(stack),
   //         "Invalide channel");
  //TZ_ASSERT(slice >= 0 && slice < C_Stack::depth(stack), "Invalide slice");

  if (array != NULL) {
    size_t byteNumber = C_Stack::planeByteNumber(stack);
    size_t channelByteNumber = byteNumber * C_Stack::depth(stack);

    memcpy(stack->array + channelByteNumber * channel + byteNumber * slice, array,
           byteNumber);
  }
}

void C_Stack::copyChannelValue(Mc_Stack *mc_stack, int chan, const Stack *stack)
{
  Mc_Stack_Copy_Channel(mc_stack, chan, stack);
}

void C_Stack::setAttribute(
    Mc_Stack *stack, int kind, int width, int height, int depth, int channel)
{
  TZ_ASSERT(stack != NULL, "Null stack.");

  stack->kind = kind;
  stack->width = width;
  stack->depth = depth;
  stack->height = height;
  stack->nchannel = channel;
}

void C_Stack::setAttribute(
    Stack *stack, int kind, int width, int height, int depth)
{
  TZ_ASSERT(stack != NULL, "Null stack.");

  stack->kind = kind;
  stack->width = width;
  stack->depth = depth;
  stack->height = height;
}

Stack* C_Stack::crop(const Stack* stack, int left, int top, int front,
            int width, int height, int depth, Stack *desstack)
{
  STACK_MUTEX_GUARD

  return Crop_Stack(stack, left, top, front, width, height, depth, desstack);
}

Stack* C_Stack::crop(const Stack *stack, const Cuboid_I &box, Stack *desstack)
{
  return crop(stack, box.cb[0], box.cb[1], box.cb[2], box.ce[0] - box.cb[0] + 1,
      box.ce[1] - box.cb[1] + 1, box.ce[2] - box.cb[2] + 1, desstack);
}

Stack* C_Stack::boundCrop(const Stack *stack, int margin)
{
  STACK_MUTEX_GUARD

  return Stack_Bound_Crop(stack, margin);
}

Stack* C_Stack::boundCrop(const Stack *stack, int margin, int *offset)
{
  Cuboid_I bound_box;
  Stack_Bound_Box(stack, &bound_box);

  int width = 0;
  int height = 0;
  int depth = 0;
  Cuboid_I_Size(&bound_box, &width, &height, &depth);

  Stack *out = crop(stack, bound_box.cb[0] - margin,
      bound_box.cb[1] - margin, bound_box.cb[2] - margin,
      width + margin * 2, height + margin * 2, depth + margin * 2, NULL);
  /*
  STACK_MUTEX_GUARD
  Stack *out =  Crop_Stack(stack, bound_box.cb[0] - margin,
      bound_box.cb[1] - margin, bound_box.cb[2] - margin,
      width + margin * 2, height + margin * 2, depth + margin * 2, NULL);
      */

  if (offset != NULL) {
    for (int i = 0; i < 3; ++i) {
      offset[i] = bound_box.cb[i] - margin;
    }
  }

  return out;
}

int* C_Stack::hist(const Stack* stack)
{
  if (voxelNumber(stack) > MAX_INT32) {
    double ratio = (double) voxelNumber(stack) / MAX_INT32;
    int intv[3] = {0, 0, 0};
    int i = 0;
    while ((intv[0] + 1) * (intv[1] + 1) * (intv[2] + 1) < ratio) {
      intv[i++] += 1;
      if (i > 2) {
        i = 0;
      }
    }

    Stack *ds = NULL;

    {
      STACK_MUTEX_GUARD
      ds = Downsample_Stack(stack, intv[0], intv[1], intv[2]);
    }

    int *hist = Stack_Hist(ds);

    kill(ds);
    return hist;
  }

  return Stack_Hist(stack);
}

ZIntHistogram* C_Stack::hist(const Stack *stack, ZIntHistogram *out)
{
  if (out == NULL) {
    out = new ZIntHistogram;
  }

  int *h = hist(stack);

  out->setData(h);

  return out;
}

void C_Stack::setZero(Stack *stack)
{
  Zero_Stack(stack);
}

void C_Stack::setOne(Stack *stack)
{
  One_Stack(stack);
}

void C_Stack::setZero(Mc_Stack *stack)
{
  if (stack != NULL) {
    size_t length = allByteNumber(stack);
    bzero(stack->array, length);
  }
}

void C_Stack::setOne(Mc_Stack *stack)
{
  Stack sstack;
  for (int i = 0; i < channelNumber(stack); ++i) {
    view(stack, &sstack, i);
    One_Stack(&sstack);
  }
}



ssize_t C_Stack::offset(int x, int y, int z, int width, int height, int depth)
{
  return Stack_Util_Offset(x, y, z, width, height, depth);
}

void C_Stack::setPixel(Stack *stack, int x, int y, int z, int c, double v)
{
  Set_Stack_Pixel(stack, x, y, z, c, v);
}

size_t C_Stack::voxelNumber(const Stack *stack)
{
  return Stack_Voxel_Number(stack);
}

uint16_t* C_Stack::guardedArray16(const Stack *stack)
{
  uint16_t *array = NULL;
  if (stack != NULL) {
    if (kind(stack) == GREY16) {
      array = (uint16_t*) stack->array;
    }
  }

  return array;
}

float* C_Stack::guardedArrayFloat32(const Stack *stack)
{
  float *array = NULL;
  if (stack != NULL) {
    if (kind(stack) == FLOAT32) {
      array = (float*) stack->array;
    }
  }

  return array;
}

double C_Stack::value(const Stack *stack, size_t index)
{
  return Stack_Array_Value(stack, index);
}

double C_Stack::value(const Stack *stack, int x, int y, int z, int c)
{
  return Stack_Pixel(stack, x, y, z, c);
}

void C_Stack::print(const Stack *stack)
{
  Print_Stack(stack);
}

void C_Stack::printValue(const Stack *stack)
{
  Print_Stack_Value(stack);
}

void C_Stack::printValue(const Mc_Stack *stack)
{
  int nc = channelNumber(stack);
  Stack sstack;
  for (int c = 0; c < nc; ++c) {
    view(stack, &sstack, c);
    std::cout << "Channel " << c << ":" << std::endl;
    printValue(&sstack);
  }
}

Stack* C_Stack::channelExtraction(const Stack *stack, int channel)
{
  STACK_MUTEX_GUARD

  return Stack_Channel_Extraction(stack, channel, NULL);
}

bool C_Stack::setValue(
    Stack *stack, size_t offset, const void *buffer, size_t length)
{
  if (stack != NULL) {
    uint8_t *stackBuffer = array8(stack);
    if (stackBuffer != NULL && allByteNumber(stack) >= offset + length) {
      memcpy(stackBuffer + offset, buffer, length);

      return true;
    }
  }

  return false;
}

bool C_Stack::setValue(
    Mc_Stack *stack, size_t offset, const void *buffer, size_t length)
{
  if (stack != NULL) {
    uint8_t *stackBuffer = array8(stack);
    if (stackBuffer != NULL && allByteNumber(stack) >= offset + length) {
      memcpy(stackBuffer + offset, buffer, length);

      return true;
    }
  }

  return false;
}

Stack* C_Stack::resize(const Stack *stack, int width, int height, int depth)
{
  STACK_MUTEX_GUARD

  return Resize_Stack(stack, width, height, depth);
}

#define DOWNSAMPLE_GENERAL(srcArray, dstArray, compare) \
  for (int z = 0; z < d; ++z) {\
    for (int y = 0; y < h; ++y) {\
      for (int x = 0; x < w; ++x) {\
        if (compare || (xCounter + yCounter + zCounter) == 0) {\
          *dstArray = *srcArray;\
        }\
        ++srcArray;\
        if (++xCounter > xintv) {\
          xCounter = 0;\
          ++dstArray;\
        }\
      }\
      if (xCounter != 0) {\
        ++dstArray;\
      }\
      if (++yCounter > yintv) {\
        yCounter = 0;\
      } else {\
        dstArray -= swidth;\
      }\
      xCounter = 0;\
    }\
    if (yCounter != 0) {\
      dstArray += swidth;\
    }\
    if (++zCounter > zintv) {\
      zCounter = 0;\
    } else {\
      dstArray -= outArea;\
    }\
    xCounter = 0;\
    yCounter = 0;\
  }

#define DOWNSAMPLE_MAX(srcArray, dstArray) \
  DOWNSAMPLE_GENERAL(srcArray, dstArray, (*srcArray > *dstArray))

#define DOWNSAMPLE_MIN(srcArray, dstArray) \
  DOWNSAMPLE_GENERAL(srcArray, dstArray, (*srcArray < *dstArray))

Stack* C_Stack::downsampleMax(
    const Stack *stack, int xintv, int yintv, int zintv, Stack *out)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    if (out == NULL) {
      return clone(stack);
    } else {
      C_Stack::copyValue(stack, out);
      return out;
    }
  }

  int xCounter = 0;
  int yCounter = 0;
  int zCounter = 0;

  int w = width(stack);
  int h = height(stack);
  int d = depth(stack);
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  if (out == NULL) {
    out = make(kind(stack), swidth, sheight, sdepth);
  } else {
    out->kind = kind(stack);
    out->width = swidth;
    out->height = sheight;
    out->depth = sdepth;
  }
  //setZero(out);

  size_t outArea = area(out);

  Image_Array srcIma;
  Image_Array dstIma;
  srcIma.array = stack->array;
  dstIma.array = out->array;

  switch (kind(stack)) {
  case GREY:
    DOWNSAMPLE_MAX(srcIma.array8, dstIma.array8);
    break;
  case GREY16:
    DOWNSAMPLE_MAX(srcIma.array16, dstIma.array16);
    break;
  case GREY32:
    DOWNSAMPLE_MAX(srcIma.array32, dstIma.array32);
    break;
  case GREY64:
    DOWNSAMPLE_MAX(srcIma.array64, dstIma.array64);
    break;
  default:
    break;
  }

  return out;
}

Stack* C_Stack::downsampleMin(
    const Stack *stack, int xintv, int yintv, int zintv, Stack *out)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return clone(stack);
  }

  int xCounter = 0;
  int yCounter = 0;
  int zCounter = 0;

  int w = width(stack);
  int h = height(stack);
  int d = depth(stack);
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  if (out == NULL) {
    out = make(kind(stack), swidth, sheight, sdepth);
  } else {
    out->kind = kind(stack);
    out->width = swidth;
    out->height = sheight;
    out->depth = sdepth;
  }
  setZero(out);

  size_t outArea = area(out);

  Image_Array srcIma;
  Image_Array dstIma;
  srcIma.array = stack->array;
  dstIma.array = out->array;

  switch (kind(stack)) {
  case GREY:
    DOWNSAMPLE_MIN(srcIma.array8, dstIma.array8);
    break;
  case GREY16:
    DOWNSAMPLE_MIN(srcIma.array16, dstIma.array16);
    break;
  case GREY32:
    DOWNSAMPLE_MIN(srcIma.array32, dstIma.array32);
    break;
  case GREY64:
    DOWNSAMPLE_MIN(srcIma.array64, dstIma.array64);
    break;
  default:
    break;
  }

  return out;
}

/*
Stack* C_Stack::copy(const Stack *stack)
{
  return Copy_Stack(const_cast<Stack*>(stack));
}
*/

void C_Stack::copyValue(const Stack *src, Stack *dst)
{
  Copy_Stack_Array(dst, src);
}

double C_Stack::sum(const Stack *stack)
{
  return Stack_Sum(stack);
}

double C_Stack::mean(const Stack *stack)
{
  return Stack_Mean(stack);
}

double C_Stack::mode(const Stack *stack)
{
  int *hist = Stack_Hist(stack);
  int comm = Int_Histogram_Mode(hist, Int_Histogram_Min(hist),
                                Int_Histogram_Max(hist));
  free(hist);

  return comm;
}

Stack* C_Stack::translate(Stack *stack, int kind, int in_place)
{
  STACK_MUTEX_GUARD

  return Translate_Stack(stack, kind, in_place);
}

Stack* C_Stack::make(int kind, int width, int height, int depth)
{
  STACK_MUTEX_GUARD

  return Make_Stack(kind, width,height, depth);
}

Stack* C_Stack::make(float *data, int kind, int width, int height, int depth)
{
  STACK_MUTEX_GUARD

  return Scale_Float_Stack(data, width, height, depth, kind);
}

Stack* C_Stack::make(double *data, int kind, int width, int height, int depth)
{
  STACK_MUTEX_GUARD

  return Scale_Double_Stack(data, width, height, depth, kind);
}


Mc_Stack* C_Stack::make(int kind, int width, int height, int depth, int channelNumber)
{
  STACK_MUTEX_GUARD

  return Make_Mc_Stack(kind, width,height, depth, channelNumber);
}

void C_Stack::setChannelNumber(Mc_Stack *stack, int nchannel)
{
  TZ_ASSERT(stack != NULL, "Null stack.");

  stack->nchannel = nchannel;
}

void C_Stack::systemKill(Mc_Stack *stack)
{
  if (stack != NULL) {
    free(stack->array);
    free(stack);
  }
}

void C_Stack::systemKill(Stack *stack)
{
  if (stack != NULL) {
    free(stack->array);
    free(stack);
  }
}

void C_Stack::freePointer(Mc_Stack *stack)
{
  if (stack != NULL) {
    free(stack);
  }
}

void C_Stack::kill(Mc_Stack *stack)
{
  if (stack != NULL) {
    STACK_MUTEX_GUARD

    Kill_Mc_Stack(stack);
  }
}

void C_Stack::kill(Stack *stack)
{
  if (stack != NULL) {
    STACK_MUTEX_GUARD

    Kill_Stack(stack);
  }
}

int C_Stack::stackUsage()
{
  STACK_MUTEX_GUARD

  return Stack_Usage();
}

void C_Stack::view(const Stack *src, Mc_Stack *dst)
{
  TZ_ASSERT(src != NULL && dst != NULL, "Null pointer");

  dst->array = src->array;
  setAttribute(dst, C_Stack::kind(src), C_Stack::width(src),
               C_Stack::height(src), C_Stack::depth(src), 1);
}

void C_Stack::view(const Mc_Stack *src, Stack *dst, int channel)
{
  TZ_ASSERT(src != NULL && dst != NULL, "Null pointer");
  TZ_ASSERT(channel >= 0 && channel < C_Stack::channelNumber(src),
            "Invalide channel");

  dst->array = src->array + volumeByteNumber(src) * channel;
  setAttribute(dst, C_Stack::kind(src), C_Stack::width(src),
               C_Stack::height(src), C_Stack::depth(src));
}

void C_Stack::view(const Mc_Stack *src, Mc_Stack *dst, int channel)
{
  TZ_ASSERT(src != NULL && dst != NULL, "Null pointer");
  TZ_ASSERT(channel >= 0 && channel < C_Stack::channelNumber(src),
            "Invalide channel");

  dst->array = src->array + volumeByteNumber(src) * channel;
  setAttribute(dst, C_Stack::kind(src), C_Stack::width(src),
               C_Stack::height(src), C_Stack::depth(src), 1);
}

void C_Stack::view(const Stack *src, Image_Array *dst)
{
  TZ_ASSERT(src != NULL && dst != NULL, "Null pointer");
  dst->array = src->array;
}

bool C_Stack::hasSameValue(Mc_Stack *stack, size_t index1, size_t index2,
                           size_t channelOffset)
{ 
  size_t byteNumber = kind(stack);

  bool isEqual = true;

  for (int c = 0; c < channelNumber(stack); ++c) {
    size_t voxelOffset1 = channelOffset * c + index1;
    size_t voxelOffset2 = channelOffset * c + index2;
    for (size_t i = 0; i < byteNumber; ++i) {
      if (stack->array[voxelOffset1 + i] !=
          stack->array[voxelOffset2 + i]) {
        isEqual = false;
        break;
      }
    }
  }

  return isEqual;
}

bool C_Stack::hasSameValue(Mc_Stack *stack, size_t index1, size_t index2)
{
  size_t channelOffset = C_Stack::volumeByteNumber(stack);

  return hasSameValue(stack, index1, index2, channelOffset);
}

bool C_Stack::hasValue(Mc_Stack *stack, size_t index, uint8_t *buffer)
{
  size_t channelOffset = C_Stack::volumeByteNumber(stack);

  return hasValue(stack, index, buffer, channelOffset);
}

bool C_Stack::hasValue(
    Mc_Stack *stack, size_t index, uint8_t *buffer, size_t channelOffset)
{
  size_t byteNumber = kind(stack);

  bool isEqual = true;

  size_t voxelOffset2 = 0;
  for (int c = 0; c < channelNumber(stack); ++c) {
    size_t voxelOffset1 = channelOffset * c + index;
    for (size_t i = 0; i < byteNumber; ++i) {
      if (stack->array[voxelOffset1 + i] != buffer[voxelOffset2++]) {
        isEqual = false;
        break;
      }
    }
  }

  return isEqual;
}

int C_Stack::neighborTest(int conn, int width, int height, int depth,
                          size_t index, int *isInBound)
{
  TZ_ASSERT(isInBound != NULL, "null pointer");

  int nnbr = Stack_Neighbor_Bound_Test_I(conn, width, height, depth,
                                         index, isInBound);
  if (nnbr == 0) {
    for (int i = 0; i < conn; ++i) {
      isInBound[i] = 0;
    }
  } else if (nnbr == conn) {
    for (int i = 0; i < conn; ++i) {
      isInBound[i] = 1;
    }
  }

  return nnbr;
}

void C_Stack::neighborOffset(int conn, int width, int height, int neighbor[])
{
  Stack_Neighbor_Offset(conn, width, height, neighbor);
}

int C_Stack::neighborTest(int conn, int width, int height, int depth,
                          int x, int y, int z, int *isInBound)
{
  TZ_ASSERT(isInBound != NULL, "null pointer");

  int nnbr = Stack_Neighbor_Bound_Test_S(conn, width - 1, height - 1, depth - 1,
                                         x, y, z, isInBound);
  if (nnbr == 0) {
    for (int i = 0; i < conn; ++i) {
      isInBound[i] = 0;
    }
  } else if (nnbr == conn) {
    for (int i = 0; i < conn; ++i) {
      isInBound[i] = 1;
    }
  }

  return nnbr;
}


#define MRAW_MAGIC_NUMBER 1836212599

void C_Stack::write(
    const std::string &filePath, const Mc_Stack *stack, const char *meta)
{
  if (stack == NULL) {
    return;
  }

  ZFileType::EFileType fileType = ZFileType::fileType(filePath) ;

  switch (fileType) {
  case ZFileType::MC_STACK_RAW_FILE:
  {
    FILE *fp = fopen(filePath.c_str(), "w");
    if (fp != NULL) {
      int magicNumber = MRAW_MAGIC_NUMBER;
      int reserved = 0;
      fwrite(&magicNumber, 4, 1, fp);
      fwrite(&(stack->kind), 4, 1, fp);
      fwrite(&(stack->width), 4, 1, fp);
      fwrite(&(stack->height), 4, 1, fp);
      fwrite(&(stack->depth), 4, 1, fp);
      fwrite(&(stack->nchannel), 4, 1, fp);
      fwrite(&reserved, 4, 1, fp);
      size_t byteNumber = C_Stack::volumeByteNumber(stack);
      fwrite(stack->array, 1, byteNumber, fp);
      fclose(fp);
    }
  }
    break;
  default:
    Write_Mc_Stack(filePath.c_str(), stack, meta);
    break;
  }
}

void C_Stack::write(const std::string &filePath, const Stack *stack)
{
  Write_Stack_U(filePath.c_str(), stack, NULL);
}

void C_Stack::readStackOffset(const string &filePath, int *x, int *y, int *z)
{
  *x = 0;
  *y = 0;
  *z = 0;

  Read_Stack_Offset(filePath.c_str(), x, y, z);
}

char *C_Stack::toMrawBuffer(const Mc_Stack *stack, size_t *length)
{
  *length = 0;
  char *buffer = NULL;

  if (stack != NULL) {
    *length = 24 + C_Stack::allByteNumber(stack);
    buffer = (char*) malloc(*length);

    int *intBuffer = (int*) buffer;
    intBuffer[0] = MRAW_MAGIC_NUMBER;
    intBuffer[1] = C_Stack::kind(stack);
    intBuffer[2] = C_Stack::width(stack);
    intBuffer[3] = C_Stack::height(stack);
    intBuffer[4] = C_Stack::depth(stack);
    intBuffer[5] = C_Stack::channelNumber(stack);

    memcpy(buffer + 24, C_Stack::array8(stack), C_Stack::allByteNumber(stack));
  }

  return buffer;
}

Mc_Stack* C_Stack::readMrawFromBuffer(const char *buffer, int channel)
{
  Mc_Stack *stack = NULL;
  if (buffer != NULL) {
    int magicNumber;
    int kind;
    int width;
    int height;
    int depth;
    int nchannel;

    int *intBuffer = (int*) buffer;

    magicNumber = *(intBuffer++);
    if (magicNumber != MRAW_MAGIC_NUMBER) {
      return NULL;
    }

    kind = *(intBuffer++);
    if (kind <= 0 || kind > 8) {
      return NULL;
    }

    width = *(intBuffer++);
    if (width <= 0) {
      return NULL;
    }

    height = *(intBuffer++);
    if (height <= 0) {
      return NULL;
    }

    depth = *(intBuffer++);
    if (depth <= 0) {
      return NULL;
    }

    nchannel = *(intBuffer++);
    if (nchannel <= 0) {
      return NULL;
    }

    intBuffer++; //reserved field


    if (channel >= 0) {
      if (channel >= nchannel) {
        return NULL;
      } else {
        nchannel = 1;
      }
    }

    stack = make(kind, width, height, depth, nchannel);
    size_t byteNumber = C_Stack::volumeByteNumber(stack);

    uint8_t *currentBuffer = (uint8_t*) intBuffer;
    if (channel >= 0) {
      currentBuffer += byteNumber * channel;
    }

    if (setValue(stack, 0, currentBuffer, byteNumber) == false) {
      kill(stack);
      stack = NULL;
    }
  }

  return stack;
}

void readStackOffset(const std::string &filePath, int *x, int *y, int *z)
{
  C_Stack::readStackOffset(filePath, x, y, z);
}

Mc_Stack* C_Stack::read(const std::string &filePath, int channel)
{
  if (!fexist(filePath.c_str())) {
    return NULL;
  }

  Mc_Stack *stack = NULL;
  ZFileType::EFileType fileType = ZFileType::fileType(filePath) ;

  switch (fileType) {
  case ZFileType::OBJECT_SCAN_FILE:
  {
    ZObject3dScan obj;
    if (obj.load(filePath)) {
      ZObject3d *obj3d = obj.toObject3d();
      Stack *tmpstack = obj3d->toStack(NULL);
      stack = C_Stack::make(
            GREY, width(tmpstack), height(tmpstack), depth(tmpstack), 1);
      C_Stack::copyChannelValue(stack, 0, tmpstack);
      C_Stack::kill(tmpstack);
      delete obj3d;
    }
  }
    break;
  case ZFileType::MC_STACK_RAW_FILE:
  {
    FILE *fp = fopen(filePath.c_str(), "r");
    if (fp != NULL) {
      int magicNumber = 0;
      int kind = 0;
      int width = 0;
      int height = 0;
      int depth = 0;
      int nchannel = 0;
      int reserved = 0;
      size_t byteNumber = 0;

      if (fread(&magicNumber, 4, 1, fp) != 1) {
        goto _read_failed;
      }

      if (magicNumber != MRAW_MAGIC_NUMBER) {
        goto _read_failed;
      }

      if (fread(&kind, 4, 1, fp) != 1) {
        goto _read_failed;
      }
      if (fread(&width, 4, 1, fp) != 1) {
        goto _read_failed;
      }

      if (fread(&height, 4, 1, fp) != 1) {
        goto _read_failed;
      }
      if (fread(&depth, 4, 1, fp) != 1) {
        goto _read_failed;
      }
      if (fread(&nchannel, 4, 1, fp) != 1) {
        goto _read_failed;
      }
      if (fread(&reserved, 4, 1, fp) != 1) {
        goto _read_failed;
      }
      if (kind <= 0 || kind > 8 || width <= 0 || height <= 0 || depth <= 0) {
        goto _read_failed;
      }

      if (channel >= 0) {
        if (channel >= nchannel) {
          goto _read_failed;
        } else {
          nchannel = 1;
        }
      }

      stack = make(kind, width, height, depth, nchannel);
      byteNumber = C_Stack::volumeByteNumber(stack);

      if (channel >= 0) {
        fseek(fp, byteNumber * channel, SEEK_CUR);
      }

      if (fread(stack->array, 1, byteNumber, fp) != byteNumber) {
        kill(stack);
        stack = NULL;
      }

_read_failed:
      fclose(fp);
    }
  }
    break;
  default:
  {
    STACK_MUTEX_GUARD
    stack = Read_Mc_Stack(filePath.c_str(), channel);
  }
    if ((size_t)stack->width * stack->height * 2 >= (size_t)1024*1024*1024) {
      double scale =
          (1024.0*1024*1024) / ((double)stack->width * stack->height * 2);
      int newWidth = static_cast<int>(std::floor(stack->width * scale));
      int newHeight = static_cast<int>(std::floor(stack->height * scale));
      Mc_Stack *stack2 = C_Stack::resize(stack, newWidth, newHeight, stack->depth);
      C_Stack::kill(stack);
      stack = stack2;
    }
  }

  return stack;
}

Stack* C_Stack::readSc(const string &filePath)
{
  Stack *stack = NULL;

  if (ZFileType::fileType(filePath) == ZFileType::OBJECT_SCAN_FILE) {
    ZObject3dScan obj;
    if (obj.load(filePath)) {
      ZObject3d *obj3d = obj.toObject3d();
      stack = obj3d->toStack(NULL);
      delete obj3d;
    }
  } else {
    STACK_MUTEX_GUARD
    stack = Read_Stack_U(filePath.c_str());
  }

  return stack;
}

Stack* C_Stack::extractChannel(const Stack *stack, int c)
{
  TZ_ASSERT(kind(stack) == COLOR, "unsupported format");

  Stack *out = make(GREY, width(stack), height(stack), depth(stack));

  color_t *arrayc = (color_t*) stack->array;
  size_t nvoxel = Stack_Voxel_Number(stack);

  for (size_t i = 0; i < nvoxel; ++i) {
    out->array[i] = arrayc[i][c];
  }

  return out;
}

void C_Stack::setStackValue(Stack *stack, const std::vector<size_t> &indexArray,
                            double value)
{
  TZ_ASSERT(kind(stack) == GREY, "Unsupported stack kind");

  for (std::vector<size_t>::const_iterator iter = indexArray.begin();
       iter != indexArray.end(); ++iter) {
    stack->array[*iter] = value;
  }
}

Stack* C_Stack::clone(const Stack *stack)
{
  STACK_MUTEX_GUARD

  if (stack == NULL) {
    return NULL;
  }

  bool hasNullText = false;
  if (stack->text == NULL) {
    const_cast<Stack*>(stack)->text = const_cast<char*>("\0");
    hasNullText = true;
  }

  Stack *cloned = Copy_Stack(const_cast<Stack*>(stack));

  if (hasNullText) {
    const_cast<Stack*>(stack)->text = NULL;
  }

  return cloned;
}

Mc_Stack* C_Stack::clone(const Mc_Stack *stack)
{
  STACK_MUTEX_GUARD

  return Copy_Mc_Stack(stack);
}

size_t C_Stack::countForegoundNeighbor(const Stack *stack, size_t index, int conn)
{
  int inBound[26];
  int neighborOffset[26];
  Stack_Neighbor_Offset(conn, width(stack), height(stack), neighborOffset);

  int nnbr = Stack_Neighbor_Bound_Test_I(
        conn, width(stack), height(stack), depth(stack),
        index, inBound);

  int count = 0;

  for (int i = 0; i < conn; ++i) {
    if (nnbr == conn || inBound[i]) {
      int neighborIndex = index + neighborOffset[i];
      if (stack->array[neighborIndex] > 0) {
        ++count;
      }
    }
  }

  return count;
}

vector<size_t> C_Stack::getNeighborIndices(const Stack *stack,
                                           const vector<size_t> &indexArray,
                                           int conn, double value)
{
  TZ_ASSERT(kind(stack) == GREY, "Unsupported stack kind");

  int inBound[26];
  int neighborOffset[26];
  Stack_Neighbor_Offset(conn, width(stack), height(stack), neighborOffset);

  Stack *mask = clone(stack);

  vector<size_t> neighborList;
#ifdef _DEBUG_2
  cout << neighborList.size() << endl;
#endif

  for (std::vector<size_t>::const_iterator iter = indexArray.begin();
       iter != indexArray.end(); ++iter) {
    int nnbr = Stack_Neighbor_Bound_Test_I(
          conn, width(stack), height(stack), depth(stack),
          *iter, inBound);

    for (int i = 0; i < conn; ++i) {
      if (nnbr == 26 || inBound[i]) {
        int neighborIndex = *iter + neighborOffset[i];
        if (mask->array[neighborIndex] == (uint8) value) {
          neighborList.push_back(neighborIndex);
          mask->array[neighborIndex] = 0;
        }
      }
    }
  }

  kill(mask);

#ifdef _DEBUG_2
  cout << neighborList.size() << endl;
#endif

  return neighborList;
}

static double normalize(double value, double s0, double s1, double t0, double t1,
                        double min, double max)
{
  value -= s0;
  value *= (t1 - t0) / (s1 - s0);
  value += t0;
  if (value < min) {
    value = min;
  } else if (value > max) {
    value = max;
  }

  return value;
}

Mc_Stack* C_Stack::translate(const Mc_Stack *stack, int targetKind)
{
  Mc_Stack *newStack = NULL;

  if (targetKind == kind(stack)) {
    newStack = clone(stack);
  } else {
    newStack = make(targetKind, width(stack), height(stack), depth(stack),
                    channelNumber(stack));
    size_t size = elementNumber(stack);

    Image_Array ima;
    ima.array = stack->array;
    Image_Array dstima;
    dstima.array = newStack->array;

    double minValue = min(stack);
    double maxValue = max(stack);

    switch (targetKind) {
    case GREY:
      switch (kind(stack)) {
      case GREY16:
        for (size_t index = 0; index < size; ++index) {
          dstima.array8[index] =
              normalize(ima.array16[index], minValue, maxValue, 0, 255, 0, 255);
        }
        break;
      case FLOAT32:
        for (size_t index = 0; index < size; ++index) {
          dstima.array8[index] =
              normalize(ima.array32[index], minValue, maxValue, 0, 255, 0, 255);
        }
        break;
      case FLOAT64:
        for (size_t index = 0; index < size; ++index) {
          dstima.array8[index] =
              normalize(ima.array64[index], minValue, maxValue, 0, 255, 0, 255);
        }
        break;
      default:
        TZ_ERROR(ERROR_DATA_TYPE);
      }
      break;
    case GREY16:
      switch (kind(stack)) {
      case GREY8:
        for (size_t index = 0; index < size; ++index) {
          dstima.array16[index] = ima.array8[index];
        }
        break;
      case FLOAT32:
        for (size_t index = 0; index < size; ++index) {
          dstima.array16[index] =
              normalize(ima.array32[index], minValue, maxValue, 0, 65535, 0, 65535);
        }
        break;
      case FLOAT64:
        for (size_t index = 0; index < size; ++index) {
          dstima.array16[index] = normalize(
                ima.array64[index], minValue, maxValue, 0, 65535, 0, 65535);
        }
        break;
      default:
        TZ_ERROR(ERROR_DATA_TYPE);
      }
      break;
    case FLOAT32:
      switch (kind(stack)) {
      case GREY8:
        for (size_t index = 0; index < size; ++index) {
          dstima.array32[index] = ima.array8[index];
        }
        break;
      case GREY16:
        for (size_t index = 0; index < size; ++index) {
          dstima.array32[index] = ima.array16[index];
        }
        break;
      case FLOAT64:
        for (size_t index = 0; index < size; ++index) {
          dstima.array32[index] = ima.array64[index];
        }
        break;
      default:
        TZ_ERROR(ERROR_DATA_TYPE);
      }
      break;
    case FLOAT64:
      switch (kind(stack)) {
      case GREY8:
        for (size_t index = 0; index < size; ++index) {
          dstima.array64[index] = ima.array8[index];
        }
        break;
      case GREY16:
        for (size_t index = 0; index < size; ++index) {
          dstima.array64[index] = ima.array16[index];
        }
        break;
      case FLOAT32:
        for (size_t index = 0; index < size; ++index) {
          dstima.array64[index] = ima.array32[index];
        }
        break;
      default:
        TZ_ERROR(ERROR_DATA_TYPE);
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
    }
  }

  return newStack;
}

double C_Stack::min(const Stack *stack)
{
  return Stack_Min(stack, NULL);
}

double C_Stack::max(const Stack *stack)
{
  return Stack_Max(stack, NULL);
}

double C_Stack::min(const Mc_Stack *stack)
{
  Stack stackView;
  view(stack, &stackView, 0);
  double minValue = min(&stackView);

  for (int i = 1; i < channelNumber(stack); ++i) {
    view(stack, &stackView, i);
    double tmpMinValue = min(&stackView);
    if (tmpMinValue < minValue) {
      minValue = tmpMinValue;
    }
  }

  return minValue;
}

double C_Stack::max(const Mc_Stack *stack)
{
  Stack stackView;
  view(stack, &stackView, 0);
  double maxValue = max(&stackView);

  for (int i = 1; i < channelNumber(stack); ++i) {
    view(stack, &stackView, i);
    double tmpMaxValue = max(&stackView);
    if (tmpMaxValue < maxValue) {
      maxValue = tmpMaxValue;
    }
  }

  return maxValue;
}

void C_Stack::drawPatch(Stack *canvas, const Stack *patch,
               int dx, int dy, int dz, int transparentValue)
{
  TZ_ASSERT(canvas->kind == GREY, "unsupported kind");
  TZ_ASSERT(patch->kind == GREY, "unsupported kind");
  TZ_ASSERT(canvas->depth == 1, "unsupported depth");
  TZ_ASSERT(patch->depth == 1, "unsupported depth");

  size_t offset1 = 0;
  size_t offset2 = 0;

  Stack canvasSlice = sliceView(canvas, dz);
  Stack patchSlice = sliceView(patch, dz);

  for (int y = 0; y < C_Stack::height(patch); ++y) {
    if (y + dy >= C_Stack::height(canvas)) {
      break;
    }
    offset1 = C_Stack::width(canvas) * (y + dy) + dx;
    offset2 = C_Stack::width(patch) * y;
    for (int x = 0; x < C_Stack::width(patch); ++x) {
      if (x + dx >= C_Stack::width(canvas)) {
        break;
      }
      if (patchSlice.array[offset2] != transparentValue) {
        canvasSlice.array[offset1] = patchSlice.array[offset2];
      }
      ++offset1;
      ++offset2;
    }
  }
}

int C_Stack::digitWidth(int n)
{
  ostringstream stream;
#if defined(_NEUTUBE_)
  const NeutubeConfig &config = NeutubeConfig::getInstance();
  stream << config.getPath(NeutubeConfig::DATA) << "/benchmark/digit" << n
         << ".tif";
#else
  stream << "../data" << "/benchmark/digit" << n << ".tif";
#endif

  /*
  const NeutubeConfig &config = NeutubeConfig::getInstance();
  ostringstream stream;
  stream << config.getPath(NeutubeConfig::DATA) << "/benchmark/digit" << n
         << ".tif";
         */
  Stack *digitPatch = readSc(stream.str().c_str());

  int width = digitPatch->width;

  C_Stack::kill(digitPatch);

  return width;
}

int C_Stack::integerWidth(int n, int interval)
{
  ostringstream stream;
  stream << n;

  int width = 0;
  for (size_t i = 0; i < stream.str().length(); ++i) {
    width += digitWidth(stream.str().at(i) - '0');
    if (i > 0) {
      width += interval;
    }
  }

  return width;
}


int C_Stack::drawDigit(Stack *canvas, int n, int dx, int dy, int dz)
{
  ostringstream stream;
#if defined(_NEUTUBE_)
  const NeutubeConfig &config = NeutubeConfig::getInstance();
  stream << config.getPath(NeutubeConfig::DATA) << "/benchmark/digit" << n
         << ".tif";
#else
  stream << "../data" << "/benchmark/digit" << n << ".tif";
#endif

  Stack *digitPatch = readSc(stream.str().c_str());
  drawPatch(canvas, digitPatch, dx, dy, dz, 0);

  int width = digitPatch->width;

  C_Stack::kill(digitPatch);

  return width;
}

void C_Stack::drawInteger(Stack *canvas, int n, int dx, int dy, int dz,
                          int interval)
{
  ostringstream stream;
  stream << n;

  for (size_t i = 0; i < stream.str().length(); ++i) {
    int width = drawDigit(canvas, stream.str().at(i) - '0', dx, dy, dz);
    dx += width + interval;
  }
}

Stack C_Stack::sliceView(const Mc_Stack *stack, int slice, int channel)
{
  Stack scStack;
  C_Stack::view(stack, &scStack, channel);

  return sliceView(&scStack, slice);
}

Stack C_Stack::sliceView(const Stack *stack, int slice)
{
  TZ_ASSERT(slice >= 0 && slice < C_Stack::depth(stack), "Invalide slice");
  Stack stackSlice = *stack;
  stackSlice.array = array8(stack) + C_Stack::planeByteNumber(stack) * slice;
  setDepth(&stackSlice, 1);

  return stackSlice;
}

Stack C_Stack::sliceView(const Stack *stack, int startPlane, int endPlane)
{
  TZ_ASSERT(startPlane <= endPlane, "Invalide slice");
  TZ_ASSERT(endPlane >= 0, "Invalide slice");
  TZ_ASSERT(startPlane < C_Stack::depth(stack), "Invalide slice");

  if (startPlane < 0) {
    startPlane = 0;
  }

  if (endPlane >= C_Stack::depth(stack)) {
    endPlane = C_Stack::depth(stack) - 1;
  }

  Stack stackSlice = *stack;
  stackSlice.array = array8(stack) +
      C_Stack::planeByteNumber(stack) * startPlane;

  setDepth(&stackSlice, endPlane - startPlane + 1);

  return stackSlice;
}

ssize_t C_Stack::indexFromCoord(
    int x, int y, int z, int width, int height, int depth)
{
  return Stack_Util_Offset(x, y, z, width, height, depth);
}

void C_Stack::indexToCoord(
    size_t index, int width, int height, int *x, int *y, int *z)
{
  Stack_Util_Coord(index, width, height, x, y, z);
}

size_t C_Stack::closestForegroundPixel(const Stack *stack,
                                       double cx, double cy, double cz)
{
  size_t targetIndex = 0;

  double minDist2 = cx * cx + cy * cy + cz * cz;

  size_t index = 0;
  for (int z = 0; z < depth(stack); ++z) {
    double dz = cz - z;
    for (int y = 0; y < height(stack); ++y) {
      double dy = cy - y;
      for (int x = 0; x < width(stack); ++x) {
        if (value(stack, index) > 0) {
          double dx = cx - x;
          double dist2 = dx * dx + dy * dy + dz * dz;
          if (dist2 < minDist2) {
            targetIndex = index;
            minDist2 = dist2;
          }
        }
        ++index;
      }
    }
  }

  return targetIndex;
}


Mc_Stack *C_Stack::resize(const Mc_Stack *stack, int width, int height, int depth)
{
  Mc_Stack *res = C_Stack::make(stack->kind, width, height, depth, stack->nchannel);
  size_t byteNumber = C_Stack::planeByteNumber(res);
  size_t channelByteNumber = byteNumber * C_Stack::depth(res);
  for (int i=0; i<stack->nchannel; ++i) {
    Stack stackView;
    view(stack, &stackView, i);
    Stack *stk = resize(&stackView, width, height, depth);
    memcpy(res->array + channelByteNumber * i, stk->array, channelByteNumber);
    C_Stack::kill(stk);
  }
  return res;
}

#define MC_STACK_SET_ZERO(array) \
  for (int c = 0; c < nc; ++c) { \
    size_t offset = s4 * c + s3 * z0 + s2 * y0 + x0;\
    for (int z = 0; z < sd; ++z) {\
      for (int y = 0; y < sh; ++y) {\
        for (int x = 0; x < sw; ++x) {\
          array[offset] = 0;\
          offset += p1;\
        }\
        offset += p2;\
      }\
      offset += p3;\
    }\
  }

void C_Stack::setZero(
    Mc_Stack *stack, int x0, int y0, int z0, int sw, int sh, int sd)
{
  if (sw < 0 || sh < 0 || sd < 0) {
    return;
  }

  //Readjust the size to avoid out-of-bound operation
  Cuboid_I stackBox;
  Cuboid_I_Set_S(&stackBox, 0, 0, 0, width(stack), height(stack), depth(stack));

  Cuboid_I subBox;
  Cuboid_I_Set_S(&subBox, x0, y0, z0, sw, sh, sd);

  Cuboid_I_Intersect(&stackBox, &subBox, &subBox);

  Image_Array ima;
  ima.array = stack->array;

  if (Cuboid_I_Is_Valid(&subBox)) {
    x0 = subBox.cb[0];
    y0 = subBox.cb[1];
    z0 = subBox.cb[2];
    sw = Cuboid_I_Width(&subBox);
    sh = Cuboid_I_Height(&subBox);
    sd = Cuboid_I_Depth(&subBox);

    const size_t s2 = width(stack);
    const size_t s3 = area(stack);
    const size_t p1 = 1;
    const size_t p2 = s2 - sw + 1 - p1;
    const size_t p3 = p2 + s3 - sh * s2 - p2;
    const size_t s4 = voxelNumber(stack);

    int nc = channelNumber(stack);
    switch (kind(stack)) {
    case GREY:
      MC_STACK_SET_ZERO(ima.array8);
      break;
    case GREY16:
      MC_STACK_SET_ZERO(ima.array16);
      break;
    case FLOAT32:
      MC_STACK_SET_ZERO(ima.array32);
      break;
    case FLOAT64:
      MC_STACK_SET_ZERO(ima.array64);
      break;
    default:
      break;
    }
  }
}

#define STACK_SET_BLOCK_VALUE(dstArray, srcArray) \
  if (alpha == 1.0) {\
    for (int z = 0; z < sd; ++z) {\
      for (int y = 0; y < sh; ++y) {\
        for (int x = 0; x < sw; ++x) {\
          if ((int) srcArray[offset2] != srcValueIgnored && \
              (int) dstArray[offset] != dstValueIgnored) {\
            dstArray[offset] = srcArray[offset2];\
          }\
          offset += p1;\
          offset2 += bp1;\
        }\
        offset += p2;\
        offset2 += bp2;\
      }\
      offset += p3;\
      offset2 += bp3;\
    }\
  } else {\
    for (int z = 0; z < sd; ++z) {\
      for (int y = 0; y < sh; ++y) {\
        for (int x = 0; x < sw; ++x) {\
          if ((int) srcArray[offset2] != srcValueIgnored && \
              (int) dstArray[offset] != dstValueIgnored) {\
            double v = alpha * srcArray[offset2] + (1 - alpha) * dstArray[offset];\
            dstArray[offset] = iround(v);\
          }\
          offset += p1;\
          offset2 += bp1;\
        }\
        offset += p2;\
        offset2 += bp2;\
      }\
      offset += p3;\
      offset2 += bp3;\
    }\
  }

void C_Stack::setBlockValue(Stack *stack, const Stack *block, int x0, int y0, int z0,
    int srcValueIgnored, int dstValueIgnored, double alpha)
{
  if (kind(stack) != kind(block) || stack == NULL || block == NULL) {
    return;
  }

  /*
  if (x0 < 0 || y0 < 0 || z0 < 0) { //negative offset not supported yet
    return;
  }
  */

  int sw = width(block);
  int sh = height(block);
  int sd = depth(block);

  //Readjust the size to avoid out-of-bound operation
  Cuboid_I stackBox;
  Cuboid_I_Set_S(&stackBox, 0, 0, 0, width(stack), height(stack), depth(stack));

  Cuboid_I subBox;
  Cuboid_I_Set_S(&subBox, x0, y0, z0, sw, sh, sd);

  Cuboid_I_Intersect(&stackBox, &subBox, &subBox);

  Image_Array ima;
  ima.array = stack->array;
  Image_Array blockIma;
  blockIma.array = block->array;

  if (Cuboid_I_Is_Valid(&subBox)) {
    int nx0 = subBox.cb[0];
    int ny0 = subBox.cb[1];
    int nz0 = subBox.cb[2];
    sw = Cuboid_I_Width(&subBox);
    sh = Cuboid_I_Height(&subBox);
    sd = Cuboid_I_Depth(&subBox);

    const size_t s2 = width(stack);
    const size_t s3 = area(stack);
    const size_t p1 = 1;
    const size_t p2 = s2 - sw + 1 - p1;
    const size_t p3 = p2 + s3 - sh * s2 - p2;

    const size_t bs2 = width(block);
    const size_t bs3 = area(block);
    const size_t bp1 = 1;
    const size_t bp2 = bs2 - sw + 1 - p1;
    const size_t bp3 = bp2 + bs3 - sh * bs2 - bp2;


    size_t offset = s3 * nz0 + s2 * ny0 + nx0;
    size_t offset2 = bs3 * (nz0 - z0) + bs2 * (ny0 - y0) + nx0 - x0;

    switch (kind(stack)) {
    case GREY:
      STACK_SET_BLOCK_VALUE(ima.array8, blockIma.array8);
      break;
    case GREY16:
      STACK_SET_BLOCK_VALUE(ima.array16, blockIma.array16);
      break;
    case FLOAT32:
      STACK_SET_BLOCK_VALUE(ima.array32, blockIma.array32);
      break;
    case FLOAT64:
      STACK_SET_BLOCK_VALUE(ima.array64, blockIma.array64);
      break;
    default:
      break;
    }

  }
}

bool C_Stack::isBinary(const Stack *stack)
{
  if (kind(stack) != GREY) {
    return false;
  }

  bool state = false;
  size_t v = voxelNumber(stack);
  for (size_t i = 0; i < v; ++i) {
    if (stack->array[i] == 1) {
      state = true;
    } else if (stack->array[i] > 1) {
      state = false;
      break;
    }
  }

  return state;
}

Stack* C_Stack::computeGradient(const Stack *stack)
{
  Stack *out = NULL;
  if (stack != NULL) {
    out = make(GREY, C_Stack::width(stack), C_Stack::height(stack),
               C_Stack::depth(stack));
    Zero_Stack(out);

    size_t offset = 0;
    int width = C_Stack::width(stack);
    int area = C_Stack::area(stack);
    int cwidth = C_Stack::width(stack) - 1;
    int cheight = C_Stack::height(stack) - 1;
    int cdepth = C_Stack::depth(stack) - 1;

    if (cdepth == 0) {
      offset = width;
      for (int y = 1; y < cheight; ++y) {
        ++offset;
        for (int x = 1; x < cwidth; ++x) {
          int dx = ((int) stack->array[offset + 1] -
              (int) stack->array[offset - 1]);
          int dy = ((int) stack->array[offset + width] -
              (int) stack->array[offset - width]);
          int v = iround(sqrt((dx * dx + dy * dy) / 8.0));
          out->array[offset++] = v;
        }
        ++offset;
      }
    } else {
      offset = area;
      for (int z = 1; z < cdepth; ++z) {
        offset += width;
        for (int y = 1; y < cheight; ++y) {
          ++offset;
          for (int x = 1; x < cwidth; ++x) {
            int dx = ((int) stack->array[offset + 1] -
                (int) stack->array[offset - 1]);
            int dy = ((int) stack->array[offset + width] -
                (int) stack->array[offset - width]);
            int dz = ((int) stack->array[offset + area] -
                (int) stack->array[offset - area]);
            int v = iround(sqrt((dx * dx + dy * dy + dz * dz) / 12.0));
            out->array[offset++] = v;
          }
          ++offset;
        }
        offset += width;
      }
    }
  }

  return out;
}

void C_Stack::shrinkBorder(const Stack *stack, int r, int nnbr)
{
  if (r == 0) {
    return;
  }

  int is_in_bound[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  /* alloc <mask> */
  Stack *mask = C_Stack::make(GREY, width, height, depth);
  Zero_Stack(mask);

  int neighbor[26];
  Stack_Neighbor_Offset(nnbr, width, height, neighbor);
  size_t index;
  size_t voxelNumber = Stack_Voxel_Number(stack);
  size_t count = 0;
  for (index = 0; index < voxelNumber; ++index) {
    if (stack->array[index] > 0) {
      int n_in_bound = Stack_Neighbor_Bound_Test_I(nnbr, width, height, depth,
          index, is_in_bound);
      int j;
      for (j = 0; j < nnbr; ++j) {
        if (n_in_bound == nnbr || is_in_bound[j]) {
          size_t neighbor_index = index + neighbor[j];
          if (stack->array[neighbor_index] != stack->array[index]) {
            mask->array[index] = 1;
            ++count;
            break;
          }
        }
      }
    }
  }

  Stack_Not(mask, mask);

  STACK_MUTEX_GUARD

  Stack *dist = Stack_Bwdist_L_U16(mask, NULL, 0);
  uint16* distArray = C_Stack::guardedArray16(dist);
  //Generate new mask
  int r2 = r * r;
  for (index = 0; index < voxelNumber; ++index) {
    if (distArray[index] <= r2) {
      stack->array[index] = 0;
    }
  }

  Kill_Stack(dist);
}

#if 0
#define STACK_WATERSHED_IS_SEED(mask, i)			\
  (((mask)->array[i] >= 1) && ((mask)->array[i] <= STACK_WATERSHED_MAX_SEED))

#define STACK_WATERSHED_UNLABELED(i) (out->array[i] == 0)

#define STACK_WATERSHED_ENQUEUE(level, i)	\
  if (queue_head[level] == -1) {		\
    queue_head[level] = i;			\
    queue_tail[level] = i;			\
  } else {					\
    level_queue[queue_tail[level]] = i;		\
    queue_tail[level] = i;			\
  }

#define STACK_WATERSHED_DEQUEUE(level, v)	\
  v = queue_head[level];			\
  if (v >= 0) {					\
    queue_head[level] = level_queue[v];		\
    if (queue_head[level] == -1) {		\
      queue_tail[level] = -1;			\
    }						\
  }

Stack* C_Stack::watershed(
    const Stack *stack, Stack_Watershed_Workspace *ws, Stack *out)
{
#ifdef _DEBUG_2
  Zero_Stack(out);
  return out;
#endif

  if (ws->mask == NULL) {
    return NULL;
  }

  int width = stack->width;
  int height = stack->height;
  int depth = stack->depth;

  if (width == 1 && height == 1 && depth == 1) {
    return NULL;
  }

  if (out == NULL) {
    out = C_Stack::make(GREY, width, height, depth);
  }
  Zero_Stack(out);

  size_t nvoxel = voxelNumber(stack);

  int is_in_bound[26];
  int neighbors[26];

  Stack_Neighbor_Offset(ws->conn, Stack_Width(stack), Stack_Height(stack),
            neighbors);

  int i;

  int max_level = 65535;

  #if 1
  int *queue_head = iarray_malloc(max_level + 1);
  int *queue_tail = iarray_malloc(max_level + 1);
  //int queue_head[65536];
  //int queue_tail[65536];

  for (i = 0; i <= max_level; i++) {
    queue_head[i] = -1;
    queue_tail[i] = -1;
  }

  int j;

  for (i = 0; i < nvoxel; i++) {
    ws->array[i] = -1;
  }

  int water_level = ws->start_level;
  int nseed = 0;
  int *level_queue = ws->array;

  for (i = 0; i < nvoxel; i++) {
    /* If the voxel is an aviable seed */
    if (STACK_WATERSHED_IS_SEED(ws->mask, i)) {
      int basin = ws->mask->array[i];
      if (nseed < basin) {
    nseed = basin;
      }
      out->array[i] = basin;
      int level = (int) Stack_Array_Value(stack, i);
      if (level >= water_level) {
    STACK_WATERSHED_ENQUEUE(water_level, i);
      } else {
    STACK_WATERSHED_ENQUEUE(level, i);
      }
    }
  }


  /* Now start watershed. */
  while (water_level >= ws->min_level) {
#ifdef _DEBUG_2
    printf("water level: %d\n", water_level);
#endif
    int cur_index;
    STACK_WATERSHED_DEQUEUE(water_level, cur_index);

    while (cur_index >= 0) {
      int basin = out->array[cur_index];
      int nbound = Stack_Neighbor_Bound_Test_I(ws->conn,
                           Stack_Width(stack),
                           Stack_Height(stack),
                           Stack_Depth(stack),
                           cur_index,
                           is_in_bound);

#define STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j)			\
      int nbr = cur_index + neighbors[j];				\
      if (STACK_WATERSHED_UNLABELED(nbr) && (ws->mask->array[nbr] == 0)) { \
    int level;							\
    if (ws->weights == NULL) {					\
      level = (int) Stack_Array_Value(stack, nbr);			\
    } else {							\
      level = iround(Stack_Array_Value(stack, nbr) * ws->weights[j]); \
    }								\
    if (level >= ws->min_level) {					\
      if (level >= water_level) {					\
        STACK_WATERSHED_ENQUEUE(water_level, nbr);			\
        out->array[nbr] = basin;					\
      } else {							\
        STACK_WATERSHED_ENQUEUE(level, nbr);			\
        out->array[nbr] = basin;					\
      }								\
    }								\
      }

      /* Enqueue the neighbors. */
      if (nbound == ws->conn) {
    for (j = 0; j < ws->conn; j++) {
      STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j);
    }
      } else {
    for (j = 0; j < ws->conn; j++) {
      if (is_in_bound[j]) {
        STACK_WATERSHED_CHECK_NEIGHBOR(cur_index, j);
      }
    }
      }

      STACK_WATERSHED_DEQUEUE(water_level, cur_index);
    }

    water_level--;
  }

  free(queue_head);
  free(queue_tail);

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", out);
#endif

  return out;
#endif
}
#endif

#define PROJ_STACK_ZMIN(stack_array, image_array)           \
  for(z = startZ;z < endZ;z++) {                                         \
    image_offset = 0;                                                   \
    for(y=0;y<stack->height;y++)                                        \
      for(x=0;x<stack->width;x++) {                                     \
        if(image_array[image_offset] > stack_array[stack_offset])       \
          image_array[image_offset] = stack_array[stack_offset];        \
        image_offset++;                                                 \
        stack_offset++;                                                 \
    }                                                                   \
  }

#define PROJ_STACK_ZMAX(stack_array, image_array)           \
  for(z = startZ;z < endZ;z++) {                                         \
    image_offset = 0;                                                   \
    for(y=0;y<stack->height;y++)                                        \
      for(x=0;x<stack->width;x++) {                                     \
        if(image_array[image_offset] < stack_array[stack_offset])       \
          image_array[image_offset] = stack_array[stack_offset];        \
        image_offset++;                                                 \
        stack_offset++;                                                 \
    }                                                                   \
  }

Image* C_Stack::makeMinProjZ(const Stack* stack, int minZ, int maxZ)
{
  Image *image = NULL;

  if(stack && minZ <= maxZ) {
    if (minZ < 0) {
      minZ = 0;
    }
    if (maxZ >= depth(stack)) {
      maxZ = depth(stack) - 1;
    }

    int x,y,z;

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;

    Image_Array ima;
    image = Make_Image(stack->kind,stack->width,stack->height);
    ima.array = image->array;

    size_t area = stack->width * stack->height;
    stack_offset = area * minZ;
    memcpy(image->array, stack->array + stack_offset * stack->kind,
           area * stack->kind);
    stack_offset += area;

    int startZ = minZ + 1;
    int endZ = maxZ;

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_ZMIN(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_ZMIN(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_ZMIN(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_ZMIN(sta.array64, ima.array64);
      break;
    case COLOR:
      for(z=startZ;z<=endZ;z++) {
        image_offset = 0;
        for(y=0;y<stack->height;y++)
          for(x=0;x<stack->width;x++) {
            int c;
            for (c = 0; c < 3; c++) {
              if(ima.arrayc[image_offset][c]<sta.arrayc[stack_offset][c]) {
                ima.arrayc[image_offset][c] = sta.arrayc[stack_offset][c];
              }
            }
            image_offset++;
            stack_offset++;
          }
      }
      break;
    default:
      fprintf(stderr,"Unsupported image kind in C_Stack::makeMinProjZ()\n");
      break;
    }
  }

  return image;
}

Image* C_Stack::makeMaxProjZ(const Stack* stack, int minZ, int maxZ)
{
  Image *image = NULL;

  if(stack && minZ <= maxZ) {
    if (minZ < 0) {
      minZ = 0;
    }
    if (maxZ >= depth(stack)) {
      maxZ = depth(stack) - 1;
    }

    int x,y,z;

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;

    Image_Array ima;
    image = Make_Image(stack->kind,stack->width,stack->height);
    ima.array = image->array;

    size_t area = stack->width * stack->height;
    stack_offset = area * minZ;
    memcpy(image->array, stack->array + stack_offset * stack->kind,
           area * stack->kind);
    stack_offset += area;

    int startZ = minZ + 1;
    int endZ = maxZ;

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_ZMAX(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_ZMAX(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_ZMAX(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_ZMAX(sta.array64, ima.array64);
      break;
    case COLOR:
      for(z=startZ;z<=endZ;z++) {
        image_offset = 0;
        for(y=0;y<stack->height;y++)
          for(x=0;x<stack->width;x++) {
            int c;
            for (c = 0; c < 3; c++) {
              if(ima.arrayc[image_offset][c] > sta.arrayc[stack_offset][c]) {
                ima.arrayc[image_offset][c] = sta.arrayc[stack_offset][c];
              }
            }
            image_offset++;
            stack_offset++;
          }
      }
      break;
    default:
      fprintf(stderr,"Unsupported image kind in C_Stack::makeMaxProjZ()\n");
      break;
    }
  }

  return image;
}

