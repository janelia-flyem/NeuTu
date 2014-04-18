#include "miscutility.h"
#include <iostream>
#include <cmath>
#include "zerror.h"
#include "c_stack.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"
#include "flyem/zflyemqualityanalyzer.h"

using namespace std;

void misc::paintRadialHistogram(
    const ZHistogram hist, double cx, double cy, int z, Stack *stack)
{
  PROCESS_WARNING(stack == NULL, "null stack", return);
  PROCESS_WARNING(C_Stack::kind(stack) != GREY, "GREY kind only", return);

  if (z < 0 || z >= C_Stack::depth(stack)) {
    return;
  }

  ZHistogram histForPaint = hist;
  histForPaint.normalize();

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);

  uint8_t *array = C_Stack::array8(stack);

  size_t offset = width * height;
  offset *= z;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      double dx = x - cx;
      double dy = y - cy;
      double dist = sqrt(dx * dx + dy * dy);
#ifdef _DEBUG_2
      std::cout << dist << ": " << histForPaint.getDensity(dist) << std::endl;
#endif
      int v = iround(histForPaint.getDensity(dist) * 255.0);
      array[offset++] = CLIP_VALUE(v, 0, 255);
    }
  }
}

void misc::paintRadialHistogram2D(
    const vector<ZHistogram> hist, double cx, int startZ, Stack *stack)
{
  PROCESS_WARNING(stack == NULL, "null stack", return);
  PROCESS_WARNING(C_Stack::kind(stack) != GREY, "GREY kind only", return);

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);

  uint8_t *array = C_Stack::array8(stack);

  int y = startZ;

  for (vector<ZHistogram>::const_iterator histIter = hist.begin();
       histIter != hist.end(); ++histIter, ++y) {

    if (y >= height) {
      break;
    }

    ZHistogram histForPaint = *histIter;
    histForPaint.normalize();

    size_t offset = y *  width;
    for (int x = 0; x < width; ++x) {
      double dist = fabs(x - cx);
#ifdef _DEBUG_2
      std::cout << dist << ": " << histForPaint.getDensity(dist) << std::endl;
#endif
      int v = iround(histForPaint.getDensity(dist) * 255.0);
      array[offset++] = CLIP_VALUE(v, 0, 255);
    }
  }
}

double RetrieveDist(
    const Stack *stack, const Stack *innerDist, const Stack *outerDist,
    int x, int y, int z)
{
  size_t offset = C_Stack::offset(
        x, y, z, C_Stack::width(stack),
        C_Stack::height(stack), C_Stack::depth(stack));
  double v = 0.0;
  if (stack->array[offset] > 0) { //inner, negative
    v = -sqrt(C_Stack::value(innerDist, offset)) + 1.0;
  } else {
    v = sqrt(C_Stack::value(outerDist, offset));
  }

  return v;
}

static void ComputeGradient(
    const Stack *stack, const Stack *innerDist, const Stack *outerDist,
    int x, int y, int z, double *dx, double *dy, double *dz)
{
  *dx = 0.0;
  *dy = 0.0;
  *dz = 0.0;

  if (x == 0) {
    *dx = RetrieveDist(stack, innerDist, outerDist, x + 1, y, z) -
        RetrieveDist(stack, innerDist, outerDist, x, y, z);
  } else if (x == C_Stack::width(stack) - 1) {
    *dx = RetrieveDist(stack, innerDist, outerDist, x, y, z) -
        RetrieveDist(stack, innerDist, outerDist, x - 1, y, z);
  } else  if (x > 0 && x < C_Stack::width(stack) - 1) {
    *dx = (RetrieveDist(stack, innerDist, outerDist, x + 1, y, z) -
           RetrieveDist(stack, innerDist, outerDist, x - 1, y, z)) / 2.0;
  }

  if (y == 0) {
    *dy = RetrieveDist(stack, innerDist, outerDist, x, y + 1, z) -
        RetrieveDist(stack, innerDist, outerDist, x, y, z);
  } else if (y == C_Stack::height(stack) - 1) {
    *dy = RetrieveDist(stack, innerDist, outerDist, x, y, z) -
        RetrieveDist(stack, innerDist, outerDist, x, y - 1, z);
  } else  if (y > 0 && y < C_Stack::height(stack) - 1) {
    *dy = (RetrieveDist(stack, innerDist, outerDist, x, y + 1, z) -
        RetrieveDist(stack, innerDist, outerDist, x, y - 1, z)) / 2.0;
  }


  if (z == 0) {
    *dz = RetrieveDist(stack, innerDist, outerDist, x, y, z + 1) -
        RetrieveDist(stack, innerDist, outerDist, x, y, z);
  } else if (z == C_Stack::depth(stack) - 1) {
    *dz = RetrieveDist(stack, innerDist, outerDist, x, y, z) -
        RetrieveDist(stack, innerDist, outerDist, x, y, z - 1);
  } else  if (z > 0 && z < C_Stack::depth(stack) - 1) {
    *dz = (RetrieveDist(stack, innerDist, outerDist, x, y, z + 1) -
        RetrieveDist(stack, innerDist, outerDist, x, y, z - 1)) / 2.0;
  }
}

static int ComputeLightIntensity(
    const Stack *stack, const Stack *innerDist, const Stack *outerDist,
    int x, int y, int z, NeuTube::EAxis axis)
{
  double dx, dy, dz;
  ComputeGradient(stack, innerDist, outerDist, x, y, z, &dx, &dy, &dz);
  double norm = 1.0;
  if (dx != 0.0 || dy != 0.0 || dz != 0.0) {
    switch (axis) {
    case NeuTube::X_AXIS:
      norm = fabs(dx / sqrt(dx * dx + dy * dy + dz * dz));
      break;
    case NeuTube::Y_AXIS:
      norm = fabs(dy / sqrt(dx * dx + dy * dy + dz * dz));
      break;
    case NeuTube::Z_AXIS:
      norm = fabs(dz / sqrt(dx * dx + dy * dy + dz * dz));
      break;
    }
  }

  return Clip_Value(iround(norm * 255.0), 0, 255);
  //return Clip_Value(1.0 / (1.0 + exp((0.5 - norm) * 5.0)) * 255.0, 0, 255);
}

Stack* misc::computeNormal(const Stack *stack, NeuTube::EAxis axis)
{
  Stack *tmpStack = C_Stack::clone(stack);
  Stack *innerDist = Stack_Bwdist_L_U16P(tmpStack, NULL, 0);

  Stack_Not(const_cast<Stack*>(stack), tmpStack);
  Stack *outerDist = Stack_Bwdist_L_U16P(tmpStack, NULL, 1);
  C_Stack::kill(tmpStack);

  size_t area = C_Stack::area(stack);
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  int outWidth = 0;
  int outHeight = 0;

  switch (axis) {
  case NeuTube::X_AXIS:
    outWidth = height;
    outHeight = depth;
    break;
  case NeuTube::Y_AXIS:
    outWidth = width;
    outHeight = depth;
    break;
  case NeuTube::Z_AXIS:
    outWidth = height;
    outHeight = depth;
    break;
  }

  Stack *out = C_Stack::make(GREY, width, height, 1);

  C_Stack::setZero(out);
  size_t offset = 0;
  size_t offset2 = 0;

  switch (axis) {
  case NeuTube::X_AXIS:
  for (int z = 0; z < C_Stack::depth(stack); ++z) {
    for (int y = 0; y < C_Stack::height(stack); ++y) {
      bool hit = false;
      offset = area * z + y * width;
      int x = 0;
      for (; x < C_Stack::width(stack); ++x) {
        if (stack->array[offset] > 0) {
          hit = true;
          break;
        }
        ++offset;
      }
      if (hit) {
        out->array[offset2] =
            ComputeLightIntensity(stack, innerDist, outerDist, x, y, z, axis);
      }
      ++offset2;
    }
  }
  break;
  case NeuTube::Y_AXIS:
  for (int z = 0; z < C_Stack::depth(stack); ++z) {
    for (int x = 0; x < C_Stack::width(stack); ++x) {
      bool hit = false;
      offset = area * z + x;
      int y = 0;
      for (; y < C_Stack::height(stack); ++y) {
        if (stack->array[offset] > 0) {
          hit = true;
          break;
        }
        offset += width;
      }
      if (hit) {
        out->array[offset2] =
            ComputeLightIntensity(stack, innerDist, outerDist, x, y, z, axis);
      }
      ++offset2;
    }
  }
  break;
  case NeuTube::Z_AXIS:
  for (int y = 0; y < C_Stack::height(stack); ++y) {
    for (int x = 0; x < C_Stack::width(stack); ++x) {
      bool hit = false;
      offset = width * y + x;
      int z = 0;
      for (; z < C_Stack::depth(stack); ++z) {
        if (stack->array[offset] > 0) {
          hit = true;
          break;
        }
        offset += area;
      }
      if (hit) {
        out->array[offset2] =
            ComputeLightIntensity(stack, innerDist, outerDist, x, y, z, axis);
      }
      ++offset2;
    }
  }
  break;
  }

  C_Stack::kill(innerDist);
  C_Stack::kill(outerDist);

  return out;
}

int misc::computeRavelerHeight(
    const FlyEm::ZIntCuboidArray &blockArray, int margin)
{
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(margin, margin, 0);

  FlyEm::ZIntCuboidArray calibratedBlockArray = blockArray;

  calbr.calibrate(calibratedBlockArray);

  Cuboid_I boundBox = calibratedBlockArray.getBoundBox();

  return Cuboid_I_Height(&boundBox) + margin;
}

bool misc::exportPointList(
    const string &filePath, const ZPointArray &pointArray)
{
  if (!pointArray.empty()) {
    //alloc rootObj
    json_t *rootObj = json_object();
    json_t *pointListObj = json_array();
    json_object_set_new(rootObj, "point-list", pointListObj);
    for (ZPointArray::const_iterator iter = pointArray.begin();
         iter != pointArray.end(); ++iter) {
      const ZPoint &pt = *iter;
      json_t *pointObj = json_array();
      json_array_append_new(pointObj, json_integer(iround(pt.x())));
      json_array_append_new(pointObj, json_integer(iround(pt.y())));
      json_array_append_new(pointObj, json_integer(iround(pt.z())));

      json_array_append_new(pointListObj, pointObj);
    }
    json_dump_file(rootObj, filePath.c_str(), JSON_INDENT(2));

    //free rootObj
    json_decref(rootObj);

#ifdef _DEBUG_
    std::cout << filePath << " saved." << std::endl;
#endif

    return true;
  }

  return false;
}

std::string misc::num2str(int n)
{
  std::ostringstream stream;
  stream << n;

  return stream.str();
}

double misc::computeConfidence(double v, double median, double p95)
{
  double alpha = median;
  double beta = -(p95 - median) / 2.9444;

  return 1.0 / (1.0 + exp((v - alpha)/beta));
}
