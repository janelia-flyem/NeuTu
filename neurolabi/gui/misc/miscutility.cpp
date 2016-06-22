#include "miscutility.h"
#include <iostream>
#include <cmath>
#include "zerror.h"
#include "c_stack.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"
#include "flyem/zflyemqualityanalyzer.h"
#include "zfiletype.h"
#include "zgraph.h"
#include "tz_stack_neighborhood.h"
#include "tz_utilities.h"
#include "zswctree.h"
#include "zclosedcurve.h"


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

  Stack *out = C_Stack::make(GREY, outWidth, outHeight, 1);

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
    const ZIntCuboidArray &blockArray, int margin)
{
  FlyEm::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(margin, margin, 0);

  ZIntCuboidArray calibratedBlockArray = blockArray;

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

double misc::computeConfidence(double v, double median, double p95)
{
  double alpha = median;
  double beta = -(p95 - median) / 2.9444;

  return 1.0 / (1.0 + exp((v - alpha)/beta));
}

std::vector<std::string> misc::parseHdf5Path(const string &path)
{
  std::vector<std::string> tokens = ZString(path).tokenize(':');
  std::vector<std::string> pathArray;
  if (tokens.size() > 0) {
    if (ZFileType::fileType(tokens[0]) == ZFileType::HDF5_FILE) {
      pathArray = tokens;
    }
  }

  return pathArray;
}

#if 0
// partial specialization optimization for 32-bit numbers
template<>
int numDigits(int32_t x)
{
  if (x == std::numeric_limits<int>::min()) return 10 + 1;
  if (x < 0) return numDigits(-x) + 1;

  if (x >= 10000) {
    if (x >= 10000000) {
      if (x >= 100000000) {
        if (x >= 1000000000)
          return 10;
        return 9;
      }
      return 8;
    }
    if (x >= 100000) {
      if (x >= 1000000)
        return 7;
      return 6;
    }
    return 5;
  }
  if (x >= 100) {
    if (x >= 1000)
      return 4;
    return 3;
  }
  if (x >= 10)
    return 2;
  return 1;
}
#endif

ZGraph* misc::makeCoOccurGraph(const Stack *stack, int nnbr)
{
  ZGraph *graph = NULL;
  if (stack != NULL) {
    graph = new ZGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);
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
    for (index = 0; index < voxelNumber; ++index) {
      int v1 = stack->array[index];
      if (v1 > 0) {
        int n_in_bound = Stack_Neighbor_Bound_Test_I(nnbr, width, height, depth,
                                                     index, is_in_bound);
        int j;
        for (j = 0; j < nnbr; ++j) {
          if (n_in_bound == nnbr || is_in_bound[j]) {
            size_t neighbor_index = index + neighbor[j];
            int v2 = stack->array[neighbor_index];

            if (v1 < v2) {
              graph->addEdge(v1, v2);
            }
          }
        }
      }
    }
  }

  return graph;
}

ZTree<int>* misc::buildSegmentationTree(const Stack *stack)
{
  ZGraph *graph = misc::makeCoOccurGraph(stack, 6);

  Graph *rawGraph = graph->getRawGraph();
  double maxWeight = 0.0;
  int root = 0;
  int rootBuddy = 0;
  for (int i = 0; i < rawGraph->nedge; ++i) {
    if (maxWeight < rawGraph->weights[i]) {
      maxWeight = rawGraph->weights[i];
      root = graph->getEdgeBegin(i);
      rootBuddy = graph->getEdgeEnd(i);
    }
    rawGraph->weights[i] = 1.0 / (1 + rawGraph->weights[i]);
  }
  graph->toMst();

#ifdef _DEBUG_
  std::cout << "Root: " << root << std::endl;
#endif

  graph->traverseDirect(root);
  graph->print();
//  const int *index = graph->topologicalSort();
//  iarray_print2(index, graph->getVertexNumber(), 1);

  //Build real tree
  std::vector<ZTreeNode<int>*> treeNodeArray(graph->getVertexNumber() + 1);
  treeNodeArray[0] = new ZTreeNode<int>; //root
  treeNodeArray[0]->setData(0);
  for (int i = 0; i < graph->getEdgeNumber(); ++i) {
    int v1 = graph->getEdgeBegin(i);
    int v2 = graph->getEdgeEnd(i);

    //v1 is the parent of v2
    if (treeNodeArray[v1] == NULL) {
      treeNodeArray[v1] = new ZTreeNode<int>;
      treeNodeArray[v1]->setData(v1);
    }
    if (treeNodeArray[v2] == NULL) {
      treeNodeArray[v2] = new ZTreeNode<int>;
      treeNodeArray[v2]->setData(v2);
    }
    if (v1 == root) {
      treeNodeArray[v1]->setParent(treeNodeArray[0]);
      if (v2 == rootBuddy) {
        treeNodeArray[v2]->setParent(treeNodeArray[0]);
      }
    }

    if (v2 != rootBuddy) {
      treeNodeArray[v2]->setParent(treeNodeArray[v1]);
    }
  }

//  return treeNodeArray;

  ZTree<int> *tree = new ZTree<int>;

  tree->setRoot(treeNodeArray[0]);

#ifdef _DEBUG_
  ZTreeIterator<int> iterator(*tree);
  int count = 0;
  while (iterator.hasNext()) {
    ++count;
    std::cout << iterator.next() << std::endl;
  }
  std::cout << count << " nodes" << std::endl;
#endif

  return tree;
}

ZIntPoint misc::getDsIntvFor3DVolume(const ZIntCuboid &box)
{
  static const size_t maxVolume = 512 * 512 * 256;
  ZIntPoint dsIntv;
  int s = 0;
  if (box.getVolume() > maxVolume) {
    s =  iround(Cube_Root(((double)  box.getVolume()) / maxVolume - 1));
  }
  dsIntv.set(s, s, s);

  return dsIntv;
}

ZIntPoint misc::getDsIntvFor3DVolume(double dsRatio)
{
  ZIntPoint dsIntv;

  if (dsRatio > 128) {
    int s = iround(Cube_Root(dsRatio));
    int k, m;
    pow2decomp(s, &k, &m);
    s = iround(std::pow((double) 2, k + 1)) - 1;
    dsIntv.set(s, s, s);
  } else if (dsRatio > 64) {
    dsIntv.set(7, 7, 1);
  } else if (dsRatio > 32) {
//    int s =  iround(Cube_Root(dsRatio)) - 1;
    dsIntv.set(3, 3, 3);
  } else if (dsRatio > 16) {
    dsIntv.set(3, 3, 1);
  } else if (dsRatio > 8 ) {
    dsIntv.set(3, 3, 0);
  } else if (dsRatio > 4) {
    dsIntv.set(1, 1, 1);
  } else if (dsRatio > 1) {
    dsIntv.set(1, 1, 0);
  }

  return dsIntv;
}

ZClosedCurve misc::convertSwcToClosedCurve(const ZSwcTree &tree)
{
  ZClosedCurve curve;
  if (!tree.isEmpty()) {
    Swc_Tree_Node *tn = tree.firstRegularRoot();
    while (tn != NULL) {
      curve.append(SwcTreeNode::center(tn));
      tn = SwcTreeNode::firstChild(tn);
    }
  }

  return curve;
}

double misc::SampleStack(
    const Stack *stack, double x, double y, double z, ESampleStackOption option)
{
  if (stack == NULL) {
    return NAN;
  }

  if (C_Stack::kind(stack) != GREY) {
    return NAN;
  }

  double v = NAN;

  if (IS_IN_CLOSE_RANGE3(x, y, z, 0, C_Stack::width(stack) - 1,
                         0, C_Stack::height(stack) - 1,
                         0, C_Stack::depth(stack) - 1)) {
    double nbr[8];

    if (option == SAMPLE_STACK_AVERAGE) {
      for (size_t i = 0; i < 8; ++i) {
        nbr[i] = 0.0;
      }
    } else {
      for (size_t i = 0; i < 8; ++i) {
        nbr[i] = NAN;
      }
    }


    double wx = x - std::floor(x);
    double wy = y - std::floor(y);
    double wz = z - std::floor(z);

    int width = C_Stack::width(stack);

    uint8_t *stackArray = C_Stack::array8(stack);
    size_t area = C_Stack::area(stack);
    size_t offset =
        area * int(z) + width * int(y) + int(x);
    nbr[0] = stackArray[offset];

    if (wx > 0.0) {
      nbr[1] = stackArray[offset + 1];
    }
    if (wy > 0.0) {
      nbr[2] = stackArray[offset + width];
    }
    if (wz > 0.0) {
      nbr[4] = stackArray[offset + area];
    }

    if (wx > 0.0 && wy > 0.0) {
      nbr[3] = stackArray[offset + width + 1];
    }

    if (wx > 0.0 && wz > 0.0) {
      nbr[5] = stackArray[offset + area + 1];
    }

    if (wy > 0.0 && wz > 0.0) {
      nbr[6] = stackArray[offset + area + width];
    }

    if (wx > 0.0 && wy > 0.0 && wz > 0.0) {
      nbr[7] = stackArray[offset + area + width + 1];
    }

    switch (option) {
    case SAMPLE_STACK_NN:
    {
      int index = 4 * (wz > 0.5) + 2 * (wy > 0.5) + (wx > 0.5);
      v = nbr[index];
    }
      break;
    case SAMPLE_STACK_AVERAGE:
      v = ((nbr[0] * (1.0 - wx) + nbr[1] * wx) * (1.0 - wy) +
          (nbr[2] * (1.0 - wx) + nbr[3] * wx) * wy) * (1.0 - wz) +
          ((nbr[4] * (1.0 - wx) + nbr[5] * wx) * (1.0 - wy) +
          (nbr[6] * (1.0 - wx) + nbr[7] * wx) * wy) * wz;
      break;
    case SAMPLE_STACK_UNIFORM:
      v = nbr[0];
      for (size_t i = 1; i < 8; ++i) {
        if (!std::isnan(nbr[i])) {
          if (nbr[0] != nbr[i]) {
            v = NAN;
            break;
          }
        }
      }
      break;
    }
  }

  return v;
}
