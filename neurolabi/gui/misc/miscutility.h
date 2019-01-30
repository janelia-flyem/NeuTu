#ifndef MISCUTILITY_H
#define MISCUTILITY_H

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>

#include "zhistogram.h"
#include "zobject3dscan.h"
#include "common/neutube_def.h"
#include "geometry/zintcuboidarray.h"
#include "geometry/zpointarray.h"
#include "ztree.h"
#include "zarray.h"

class ZGraph;
class ZIntPoint;
class ZSwcTree;
class ZClosedCurve;
class ZIntCuboid;
class ZCuboid;
class ZObject3dScan;

namespace misc {

void paintRadialHistogram(const ZHistogram hist, double cx, double cy, int z,
                          Stack *stack);
void paintRadialHistogram2D(const std::vector<ZHistogram> hist,
                            double cx, int startZ, Stack *stack);

/*!
 * \brief Y normal of a binary stack
 */
Stack* computeNormal(const Stack *stack, neutube::EAxis axis);

int computeRavelerHeight(const ZIntCuboidArray &blockArray, int margin);

bool exportPointList(const std::string &filePath, const ZPointArray &pointArray);

ZGraph* makeCoOccurGraph(const Stack *stack, int nnbr);

ZIntPoint getDsIntvFor3DVolume(const ZIntCuboid &box);

ZIntPoint getDsIntvFor3DVolume(double dsRatio);
int getIsoDsIntvFor3DVolume(double dsRatio, bool powed);
int getIsoDsIntvFor3DVolume(
    const ZIntCuboid &box, size_t maxVolume, bool powed);

//int GetZoomLevel(int maxLevel, int width, int height, int zoom);

double GetExpansionScale(size_t currentVol, size_t maxVol);

size_t CountOverlap(const ZObject3dScan &obj1, const ZObject3dScan &obj2);
size_t CountNeighbor(const ZObject3dScan &obj1, const ZObject3dScan &obj2);
size_t CountNeighborOnPlane(const ZObject3dScan &obj1, const ZObject3dScan &obj2);

/*!
 * \brief A function for computing confidence
 *
 * \param median The value where confidence = 0.5
 * \param p95 The value where confidence = 0.95
 */
double computeConfidence(double v, double median, double p95);

ZTree<int> *buildSegmentationTree(const Stack *stack);

ZClosedCurve convertSwcToClosedCurve(const ZSwcTree &tree);

ZCuboid Intersect(const ZCuboid &box1, const ZIntCuboid &box2);
ZCuboid CutBox(const ZCuboid &box1, const ZIntCuboid &box2);

ZIntPoint GetFirstCorner(const ZArray *array);
ZIntCuboid GetBoundBox(const ZArray *array);

enum ESampleStackOption {
  SAMPLE_STACK_NN, SAMPLE_STACK_AVERAGE, SAMPLE_STACK_UNIFORM
};

double SampleStack(const Stack *stack, double x, double y, double z,
                   ESampleStackOption option);

/*!
 * \brief Parse hdf5 path
 *
 * "file.h5f:/group/path"
 *
 * \return empty array if the parsing failed
 */
std::vector<std::string> parseHdf5Path(const std::string &path);
}

//// partial-specialization optimization for 8-bit numbers
//template <>
//int numDigits(char n)
//{
//  // if you have the time, replace this with a static initialization to avoid
//  // the initial overhead & unnecessary branch
//  static char x[256] = {0};
//  if (x[0] == 0) {
//    for (char c = 1; c != 0; c++)
//      x[static_cast<int>(c)] = numDigits((int32_t)c);
//    x[0] = 1;
//  }
//  return x[static_cast<int>(n)];
//}


#endif // MISCUTILITY_H
