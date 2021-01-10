#include "zswcfactory.h"

#include <cmath>

#include "tz_stack_bwmorph.h"
#include "tz_stack_neighborhood.h"

#include "common/math.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "swc/zswcresampler.h"
#include "flyem/zflyemneuronrangecompare.h"
#include "neurolabi/zdoublevector.h"
#include "geometry/zpointarray.h"
#include "geometry/zlinesegmentarray.h"
#include "geometry/zintcuboidface.h"
#if _QT_GUI_USED_
#include "zstroke2d.h"
#endif
#include "zobject3d.h"
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "neutubeconfig.h"
#include "zclosedcurve.h"
#include "zstackfactory.h"
#include "geometry/zintcuboid.h"
#include "zlocsegchain.h"

ZSwcFactory::ZSwcFactory()
{
}

ZSwcTree* ZSwcFactory::CreateVirtualRootSwc()
{
  ZSwcTree *tree = new ZSwcTree;
  tree->forceVirtualRoot();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateCircleSwc(double cx, double cy, double cz, double r)
{
  if (r < 0.0) {
    return NULL;
  }

  ZSwcTree *tree = CreateVirtualRootSwc();

  Swc_Tree_Node *parent = tree->root();

  double nodeRadius = r * 0.05;

  for (double angle = 0.0; angle < 6.0; angle += 0.314) {
    double x, y, z;

    x = r * cos(angle) + cx;
    y = r * sin(angle) + cy;
    z = cz;

    Swc_Tree_Node *tn = SwcTreeNode::MakePointer(x, y, z, nodeRadius);
    SwcTreeNode::setParent(tn, parent);
    parent = tn;
  }

  Swc_Tree_Node *tn = SwcTreeNode::MakePointer();
  SwcTreeNode::copyProperty(SwcTreeNode::firstChild(tree->root()), tn);
  SwcTreeNode::setParent(tn, parent);

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateBoxSwc(const ZCuboid &box, double radius)
{
  return ZSwcTree::CreateCuboidSwc(box, radius);
}

ZSwcTree* ZSwcFactory::CreateBoxSwc(const ZIntCuboid &box, double radius)
{
  ZCuboid cuboid;
  cuboid.setMinCorner(ZPoint(box.getMinCorner().toPoint()));
  cuboid.setSize(box.getWidth(), box.getHeight(), box.getDepth());

  return CreateBoxSwc(cuboid, radius);
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZFlyEmNeuronRange &range)
{
  if (range.isEmpty()) {
    return NULL;
  }

  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  if (dz == 0.0) { //Avoid dead loop
    dz = 1.0;
  }

  ZSwcTree *tree = CreateVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = CreateCircleSwc(0, 0, z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZFlyEmNeuronRange &range, const ZFlyEmNeuronAxis &axis)
{
  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  ZSwcTree *tree = CreateVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZPoint pt = axis.getCenter(z);
    ZSwcTree *subtree = CreateCircleSwc(pt.x(), pt.y(), z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateRangeCompareSwc(
    const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference)
{
  if (range.isEmpty()) {
    return NULL;
  }

  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  if (dz == 0.0) { //Avoid dead loop
    dz = 1.0;
  }

  ZSwcTree *tree = CreateVirtualRootSwc();

  double minReferenceZ = reference.getMinZ();
  double maxReferenceZ = reference.getMaxZ();

  for (double z = minReferenceZ; z < minZ; z += dz) {
    ZSwcTree *subtree = CreateCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = maxZ + dz; z <= maxReferenceZ; z += dz) {
    ZSwcTree *subtree = CreateCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = CreateCircleSwc(0, 0, z, r);
    ZFlyEmNeuronRangeCompare comp;
    ZFlyEmNeuronRangeCompare::EMatchStatus status =
        comp.compare(range, reference, z);
    switch (status) {
    case ZFlyEmNeuronRangeCompare::MISSING_BRANCH:
      subtree->setType(2);
      break;
    case ZFlyEmNeuronRangeCompare::EXTRA_BRANCH:
      subtree->setType(3);
      break;
    case ZFlyEmNeuronRangeCompare::GOOD_MATCH:
      subtree->setType(0);
      break;
    default:
      subtree->setType(0);
    }

    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwcByRegionSampling(
    const ZVoxelArray &voxelArray, double radiusAdjustment)
{
#ifdef _DEBUG_2
  voxelArray.print();
#endif

  ZDoubleVector voxelSizeArray(voxelArray.size());

  const ZVoxelArray::TVector &voxelData = voxelArray.getInternalData();
  //Retrieve voxel size
  for (size_t i = 0; i < voxelSizeArray.size(); ++i) {
    voxelSizeArray[i] = -voxelData[i].value();
  }

  std::vector<int> indexArray;
  voxelSizeArray.sort(indexArray);

  std::vector<bool> sampled(voxelArray.size(), true);

  for (size_t i = 1; i < voxelArray.size(); ++i) {
    size_t currentVoxelIndex = indexArray[i];
    const ZVoxel &currentVoxel = voxelData[currentVoxelIndex];
    for (size_t j = 0; j < i; ++j) {
      size_t prevVoxelIndex = indexArray[j];
      if (sampled[prevVoxelIndex]) {
        const ZVoxel &prevVoxel = voxelData[prevVoxelIndex];
        double dist = currentVoxel.distanceTo(prevVoxel);
        if (dist < prevVoxel.value()) {
          sampled[currentVoxelIndex] = false;
          break;
        }
      }
    }
  }

  Swc_Tree_Node *prevTn = NULL;

  for (size_t i = 0; i < voxelArray.size(); ++i) {
    if (sampled[i]) {
      Swc_Tree_Node *tn = SwcTreeNode::MakePointer();
      SwcTreeNode::setPos(
            tn, voxelData[i].x(), voxelData[i].y(), voxelData[i].z());
      SwcTreeNode::setRadius(
            tn, voxelData[i].value() + radiusAdjustment);
      Swc_Tree_Node_Set_Parent(tn, prevTn);
      prevTn = tn;
    }
  }

  ZSwcTree *tree = new ZSwcTree;
  tree->setDataFromNodeRoot(prevTn);

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZVoxelArray &voxelArray, ZSwcFactory::EPostProcess option)
{
  if (option == REGION_SAMPLING) {
    return CreateSwcByRegionSampling(voxelArray);
  }

  size_t startIndex = 0;
  size_t endIndex = voxelArray.size() - 1;

  Swc_Tree *tree = New_Swc_Tree();


  const ZVoxelArray::TVector &voxelData = voxelArray.getInternalData();
  ZVoxel prevVoxel = voxelData[startIndex];

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  SwcTreeNode::setPos(tn, prevVoxel.x(), prevVoxel.y(), prevVoxel.z());
  SwcTreeNode::setRadius(tn, prevVoxel.value());

  Swc_Tree_Node *prevTn = tn;

  for (size_t i = startIndex + 1; i < endIndex; i++) {
    double dist = voxelData[i].distanceTo(prevVoxel);
    bool sampling = true;

    if (option == SPARSE_SAMPLING) {
      if (dist < prevVoxel.value()) {
        sampling = false;
      }
    }

    if (sampling) {
      tn = New_Swc_Tree_Node();

      SwcTreeNode::setPos(tn, voxelData[i].x(), voxelData[i].y(), voxelData[i].z());
      SwcTreeNode::setRadius(tn, voxelData[i].value());
      Swc_Tree_Node_Set_Parent(prevTn, tn);
      prevTn = tn;
      prevVoxel = voxelData[i];
    }
  }

  if (endIndex - startIndex > 0) { //last node
    tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, voxelData[endIndex].x(), voxelData[endIndex].y(),
                        voxelData[endIndex].z());
    SwcTreeNode::setRadius(tn, voxelData[endIndex].value());
    Swc_Tree_Node_Set_Parent(prevTn, tn);
    /*
    if (SwcTreeNode::hasOverlap(prevTn, tn) && SwcTreeNode::hasChild(prevTn)) {
      SwcTreeNode::mergeToParent(prevTn);
    }
    */
  }

  tree->root = tn;

  ZSwcTree *treeWrapper = new ZSwcTree;
  treeWrapper->setData(tree);

  if (option == OPTIMAL_SAMPLING) {
    ZSwcResampler resampler;
    resampler.optimalDownsample(treeWrapper);
  }

  return treeWrapper;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const std::vector<ZPoint> &pointArray, double radius, bool isConnected)
{
  ZSwcTree *tree = new ZSwcTree;
  tree->useCosmeticPen(true);

  Swc_Tree_Node *root = tree->forceVirtualRoot();
  Swc_Tree_Node *parent = root;

  for (std::vector<ZPoint>::const_iterator iter = pointArray.begin();
       iter != pointArray.end(); ++iter) {
    const ZPoint &pt = *iter;
    Swc_Tree_Node *tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, pt.x(), pt.y(), pt.z());
    SwcTreeNode::setRadius(tn, radius);
    SwcTreeNode::setFirstChild(parent, tn);
//    SwcTreeNode::setParent(tn, parent);
    if (isConnected) {
      parent = tn;
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const std::vector<ZWeightedPoint> &pointArray, bool isConnected)
{
  ZSwcTree *tree = new ZSwcTree;
  tree->useCosmeticPen(true);

  Swc_Tree_Node *root = tree->forceVirtualRoot();
  Swc_Tree_Node *parent = root;

  for (std::vector<ZWeightedPoint>::const_iterator iter = pointArray.begin();
       iter != pointArray.end(); ++iter) {
    const ZWeightedPoint &pt = *iter;
    Swc_Tree_Node *tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, pt.x(), pt.y(), pt.z());
    SwcTreeNode::setRadius(tn, pt.weight());
    SwcTreeNode::setFirstChild(parent, tn);
//    SwcTreeNode::setParent(tn, parent);
    if (isConnected) {
      parent = tn;
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZLineSegmentArray &lineArray, double radius)
{
  ZSwcTree *tree = new ZSwcTree;

  Swc_Tree_Node *root = tree->forceVirtualRoot();

  for (ZLineSegmentArray::const_iterator iter = lineArray.begin();
       iter != lineArray.end(); ++iter) {
    const ZLineSegment &seg = *iter;
    Swc_Tree_Node *tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, seg.getStartPoint());
    SwcTreeNode::setRadius(tn, radius);
    SwcTreeNode::setParent(tn, root);

    Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
    SwcTreeNode::setPos(tn2, seg.getEndPoint());
    SwcTreeNode::setRadius(tn2, radius);
    SwcTreeNode::setParent(tn2, tn);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZIntCuboidFace &face, double radius)
{
  if (!face.isValid()) {
    return NULL;
  }

  ZPointArray ptArray;
  for (int i = 0; i < 4; ++i) {
    ZIntPoint pt = face.getCornerCoordinates(i);
    ptArray.append(ZPoint(pt.getX(), pt.getY(), pt.getZ()));
  }
  ptArray.append(ptArray[0]);

  return CreateSwc(ptArray, radius, true);
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZIntCuboidFaceArray &faceArray, double radius)
{
  if (faceArray.empty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree;

  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
       iter != faceArray.end(); ++iter) {
    ZSwcTree *subtree = CreateSwc(*iter, radius);
    tree->merge(subtree, true);
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZStroke2d &stroke)
{
#if _QT_GUI_USED_
  if (stroke.isEmpty()) {
    return NULL;
  }

  double z = stroke.getZ();
  double r = stroke.getWidth() / 2.0;
  ZSwcTree *tree = new ZSwcTree();
  tree->forceVirtualRoot();
  Swc_Tree_Node *parent = tree->root();
  for (size_t i = 0; i < stroke.getPointNumber(); ++i) {
    double x, y;
    stroke.getPoint(&x, &y, i);
    Swc_Tree_Node *tn = SwcTreeNode::MakePointer(x, y, z, r);
    SwcTreeNode::setParent(tn, parent);
    parent = tn;
  }

  tree->resortId();
  return tree;
#else
  return NULL;
#endif
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZObject3d &obj, double radius, int sampleStep)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree();
  tree->forceVirtualRoot();
  Swc_Tree_Node *parent = tree->root();
  for (size_t i = 0; i < obj.size(); i += sampleStep) {
    Swc_Tree_Node *tn = SwcTreeNode::MakePointer(
          obj.getX(i), obj.getY(i), obj.getZ(i), radius);
    SwcTreeNode::setId(tn, i + 1);
    SwcTreeNode::setFirstChild(parent, tn);
//    SwcTreeNode::setParent(tn, parent);
  }

//  tree->resortId();
  tree->setColor(obj.getColor());

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZObject3dScan &obj)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree();
  tree->forceVirtualRoot();
  Swc_Tree_Node *root = tree->root();

  size_t stripeNumber = obj.getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int segNumber = stripe.getSegmentNumber();
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < segNumber; ++j) {
      Swc_Tree_Node *tn =
          SwcTreeNode::MakePointer(stripe.getSegmentStart(j), y, z, 2.0);
      SwcTreeNode::setFirstChild(root, tn);
      Swc_Tree_Node *tn2 =
          SwcTreeNode::MakePointer(stripe.getSegmentEnd(j), y, z, 2.0);
      SwcTreeNode::setFirstChild(tn, tn2);
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwc(
    const ZObject3dScan &obj, int sparseLevel)
{
  ZIntCuboid box = obj.getIntBoundBox();

  int intv = neutu::iround(std::cbrt(round(double(obj.getSegmentNumber()) /
                                    ZObject3dScan::MAX_SPAN_HINT)));

  size_t volume = size_t((box.getWidth() + 2)) * ((box.getHeight() + 2)) *
      ((box.getDepth() + 2))/ (intv + 1) / (intv + 1) / (intv + 1);


  ZSwcTree *tree = NULL;
  if (volume > MAX_INT32) {
    //Separate into two parts
    int minZ = obj.getMinZ();
    int maxZ = obj.getMaxZ();
    int medZ = (minZ + maxZ) / 2;

    ZObject3dScan obj1 = obj.getSlice(minZ, medZ);
    ZObject3dScan obj2 = obj.getSlice(medZ + 1, maxZ);
    tree = CreateSurfaceSwcNoPartition(obj1, sparseLevel, NULL);
    tree = CreateSurfaceSwcNoPartition(obj2, sparseLevel, tree);
    tree->setSource("oversize"); //ad hoc flag for stating that the object is big enough
  } else {
    tree = CreateSurfaceSwcNoPartition(obj, sparseLevel, NULL);
  }

  if (tree != NULL) {
    if (intv > 0 && intv < 7) {
      tree->setSource("oversize");
    }

    tree->setColor(obj.getColor());
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwc(
    const ZObject3dScan &obj, int slicePartition, int sparseLevel)
{
  ZSwcTree *tree = NULL;
  if (slicePartition == 0) {
    tree = CreateSurfaceSwc(obj, sparseLevel);
  } else {
    if (tree == NULL) {
      tree = new ZSwcTree();
    }
    tree->forceVirtualRoot();
    Swc_Tree_Node *root = tree->root();

    int minZ = obj.getMinZ();
    int maxZ = obj.getMaxZ();
    for (int z = minZ; z < maxZ; ++z) {
      ZObject3dScan slice = obj.getSlice(z - slicePartition, z + slicePartition);
      ZStack *stack = slice.toStackObjectWithMargin(1, 1);
      if (stack != NULL) {
        size_t area = stack->width() * stack->height();
        const uint8_t* in_array =
            stack->array8() + area * (z - stack->getOffset().getZ());

        int conn = 6;
        size_t offset = 0;
        int neighbor[26];
        int n_in_bound = conn;
        int count = 0;
        double radius = sparseLevel;
        int width = stack->width();
        int height = stack->height();

        Stack_Neighbor_Offset(conn, width, height, neighbor);

        int cwidth = width - 1;
        int cheight = height - 1;

        for (int j = 0; j <= cheight; j++) {
          for (int i = 0; i <= cwidth; i++) {
            uint8_t in_value = in_array[offset];
            if (in_value > 0) {
              bool isSurface = false;
              for (int n = 0; n < n_in_bound; n++) {
                if (in_array[offset + neighbor[n]] != in_value) {
                  isSurface = true;
                  break;
                }
              }

              if (isSurface) {
                if ((count++ % sparseLevel == 0) /*|| nbrCount < 3*/) {
                  SwcTreeNode::MakePointer(
                        i + stack->getOffset().getX(),
                        j + stack->getOffset().getY(),
                        z, radius, root);
                }
              }
            }
            offset++;
          }
        }
        delete stack;
      }
    }
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwcNoPartition(
    const ZObject3dScan &obj, int sparseLevel, ZSwcTree *tree)
{
  ZIntCuboid box = obj.getIntBoundBox();
//  size_t voxelNumber = obj.getVoxelNumber();

  int intv = neutu::iround(std::cbrt(round(double(obj.getSegmentNumber()) /
                                           ZObject3dScan::MAX_SPAN_HINT)));

  size_t volume = size_t((box.getWidth() + 2)) * ((box.getHeight() + 2)) *
      ((box.getDepth() + 2))/ (intv + 1) / (intv + 1) / (intv + 1);

  if (volume > MAX_INT32) {
    intv = (neutu::iround(std::cbrt(double(volume) / MAX_INT32)) + 1) * (intv + 1) - 1;
    /*
    if (intv < 0) {
      intv = 0;
    }
    */
  }

  ZStack *stack = NULL;
  std::cout << "Creating object mask ..." << "ds: " << intv <<  std::endl;
  tic();

  ZIntPoint dsIntv = obj.getDsIntv();
  if (intv > 0) {
    ZObject3dScan obj2 = obj;
    obj2.downsampleMax(intv, intv, intv);
    intv = neutu::iround(std::cbrt(round(double(obj2.getSegmentNumber()) /
                                         ZObject3dScan::MAX_SPAN_HINT)));
    if (intv > 0) {
      obj2.downsampleMax(intv, intv, intv);
    }

    dsIntv = obj2.getDsIntv();
    stack = obj2.toStackObjectWithMargin(1, 1);
  } else {
    intv = neutu::iround(std::cbrt(round(double(obj.getSegmentNumber()) /
                                  ZObject3dScan::MAX_SPAN_HINT)));
    if (intv > 0) {
      ZObject3dScan obj2 = obj;
      obj2.downsampleMax(intv, intv, intv);
      dsIntv = obj2.getDsIntv();
      stack = obj2.toStackObjectWithMargin(1, 1);
    } else {
      stack = obj.toStackObjectWithMargin(1, 1);
    }
  }
  std::cout << "Preparing for stack time: " << toc() << std::endl;

//  ZSwcTree *tree = NULL;
  if (stack != NULL) {
    tree = CreateSurfaceSwc(*stack, sparseLevel, dsIntv + 1, tree);
    if (tree != NULL) {
//      tree->setColor(obj.getColor());
//      tree->rescale(dsIntv.getX() + 1,
//                    dsIntv.getY() + 1,
//                    dsIntv.getZ() + 1);
    } else {
#ifdef _QT_GUI_USED_
      LWARN() << "Failed to generate body surface for sparsevol: "
              << obj.getVoxelNumber() << " voxels";
#endif
    }
    delete stack;
  }

  return tree;
}

std::vector<ZSwcTree*> ZSwcFactory::CreateDiffSurfaceSwc(
      const ZObject3dScan &obj1, const ZObject3dScan &obj2)
{
  ZIntCuboid box = obj1.getIntBoundBox();
  box.join(obj2.getIntBoundBox());

  int segNumber = obj1.getSegmentNumber() + obj2.getSegmentNumber();

  int intv = GetIntv(segNumber, box);

  ZObject3dScan obj3 = obj1;
  obj3.downsampleMax(intv, intv, intv);

  ZObject3dScan obj4 = obj2;
  obj4.downsampleMax(intv, intv, intv);

  box = obj3.getIntBoundBox();
  box.join(obj4.getIntBoundBox());
  box.expand(1, 1, 1);

  ZStack *stack = ZStackFactory::MakeZeroStack(GREY, box);

  obj3.drawStack(stack, 1);
  obj4.addStack(stack, 2);

#ifdef _DEBUG_2
  C_Stack::printValue(stack->data());
#endif

  std::vector<ZSwcTree*> treeArray = CreateLevelSurfaceSwc(
        *stack, 1, ZIntPoint(intv + 1, intv + 1, intv + 1));

  for (size_t i = 0; i < treeArray.size(); ++i) {
    ZSwcTree *tree = treeArray[i];
    if (tree != NULL) {
      if (i == 1) { //deleted
        tree->setSource("split");
        tree->setColor(255, 0, 0);
      } else if (i == 2) { //unchanged
        tree->setSource("same");
        tree->setColor(128, 128, 128);
      } else if (i == 0) { //added
        tree->setSource("merge");
        tree->setColor(0, 255, 0);
      }
    }
  }

  delete stack;

  return treeArray;
}

int ZSwcFactory::GetIntv(int segNumber, const ZIntCuboid &box)
{
  int intv = neutu::iround(std::cbrt(
                             round(segNumber / ZObject3dScan::MAX_SPAN_HINT)));

  size_t volume = size_t((box.getWidth() + 2)) * ((box.getHeight() + 2)) *
      ((box.getDepth() + 2))/ (intv + 1) / (intv + 1) / (intv + 1);

  if (volume > MAX_INT32) {
    intv = (neutu::iround(std::cbrt(double(volume) / MAX_INT32)) + 1) * (intv + 1) - 1;
  }

  return intv;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwc(const ZStack &stack, int sparseLevel)
{
  return CreateSurfaceSwc(stack, sparseLevel, ZIntPoint(0, 0, 0), NULL);
}

std::vector<ZSwcTree*> ZSwcFactory::CreateLevelSurfaceSwc(
    const ZStack &stack, int sparseLevel, const ZIntPoint &scale)
{
  std::vector<ZSwcTree*> result;

  if (stack.kind() == GREY && stack.hasData()) {
    int width = stack.width();
    int height = stack.height();
    int depth = stack.depth();

    int cwidth = width - 1;
    int cheight = height - 1;
    int cdepth = depth - 1;

    const uint8_t* in_array = stack.array8();

    int conn = 6;
    size_t offset = 0;
    int neighbor[26];
//    int is_in_bound[26];
    int n_in_bound = conn;
    int count = 0;

    result.resize(255);
    Stack_Neighbor_Offset(conn, width, height, neighbor);

    double radius = sparseLevel * 0.7 * sqrt(scale.getX() * scale.getY());
    tic();
    for (int k = 0; k <= cdepth; k++) {
      for (int j = 0; j <= cheight; j++) {
        for (int i = 0; i <= cwidth; i++) {
          uint8_t in_value = in_array[offset];
          if (in_value > 0) {
            bool isSurface = false;
            for (int n = 0; n < n_in_bound; n++) {
              if (in_array[offset + neighbor[n]] != in_value) {
                isSurface = true;
                break;
              }
            }

            if (isSurface) {
              if (count++ % sparseLevel == 0) {
                ZSwcTree *tree = result[in_value - 1];
                if (tree == NULL) {
                  tree = new ZSwcTree();
                  tree->forceVirtualRoot();
                  result[in_value - 1] = tree;
                }

                SwcTreeNode::MakePointer(
                      (i + stack.getOffset().getX()) * scale.getX(),
                      (j + stack.getOffset().getY()) * scale.getY(),
                      (k + stack.getOffset().getZ()) * scale.getZ(),
                      radius, tree->root());
              }
            }
          }
          offset++;
        }
      }
    }
    std::cout << "Surface extracting time:" << toc() << std::endl;
  }

  if (!result.empty()) {
    int index = ((int) result.size()) - 1;
    for (; index >= 0; --index) {
      if (result[index] != NULL) {
        break;
      }
    }

    result.resize(index + 1);
  }

  return result;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwc(
    const ZStack &stack, int sparseLevel, const ZIntPoint &scale, ZSwcTree *tree)
{
  if (stack.kind() != GREY) {
    return NULL;
  }

//  ZSwcTree *tree = NULL;

  if (stack.hasData()) {
    int width = stack.width();
    int height = stack.height();
    int depth = stack.depth();

    int cwidth = width - 1;
    int cheight = height - 1;
    int cdepth = depth - 1;

    const uint8_t* in_array = stack.array8();

    int conn = 6;
    size_t offset = 0;
    int neighbor[26];
//    int is_in_bound[26];
    int n_in_bound = conn;
    int count = 0;

    if (tree == NULL) {
      tree = new ZSwcTree();
    }
    tree->forceVirtualRoot();
    Swc_Tree_Node *root = tree->root();
    Stack_Neighbor_Offset(conn, width, height, neighbor);

    double radius = sparseLevel * 0.7 * sqrt(scale.getX() * scale.getY());
    tic();
    for (int k = 0; k <= cdepth; k++) {
      for (int j = 0; j <= cheight; j++) {
        for (int i = 0; i <= cwidth; i++) {
//          out_array[offset] = 0;
          uint8_t in_value = in_array[offset];
          if (in_value > 0) {
//            n_in_bound = Stack_Neighbor_Bound_Test_S(
//                  conn, cwidth, cheight, cdepth, i, j, k, is_in_bound);
            bool isSurface = false;
//            int nbrCount = 0;
            //            if (n_in_bound == conn) {
            for (int n = 0; n < n_in_bound; n++) {
              if (in_array[offset + neighbor[n]] != in_value) {
                isSurface = true;
                break;

              }/* else {
                ++nbrCount;
              }*/
            }


//            } else {
//              isSurface = true;
//            }

            if (isSurface) {
              if ((count++ % sparseLevel == 0) /*|| nbrCount < 3*/) {
                SwcTreeNode::MakePointer(
                      (i + stack.getOffset().getX()) * scale.getX(),
                      (j + stack.getOffset().getY()) * scale.getY(),
                      (k + stack.getOffset().getZ()) * scale.getZ(),
                      radius, root);
              }
            }
          }
          offset++;
        }
      }
    }
    std::cout << "Surface extracting time:" << toc() << std::endl;

  } else {
#ifdef _QT_GUI_USED_
    LWARN() << "Failed to generate surface for empty stack";
#endif
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSurfaceSwcFast(const ZStack &stack, int sparseLevel)
{
  if (stack.kind() != GREY) {
    return NULL;
  }

  ZSwcTree *tree = NULL;

  if (stack.hasData()) {
    int width = stack.width();
    int height = stack.height();
    int depth = stack.depth();

    int cwidth = width - 1;
    int cheight = height - 1;
    int cdepth = depth - 1;

    const uint8_t* in_array = stack.array8();

    int conn = 6;
    size_t offset = 0;
    int neighbor[26];
//    int is_in_bound[26];
//    int n_in_bound;
    int count = 0;

    tree = new ZSwcTree();
    tree->forceVirtualRoot();
    Swc_Tree_Node *root = tree->root();
    Stack_Neighbor_Offset(conn, width, height, neighbor);

    for (int k = 0; k <= cdepth; k++) {
      for (int j = 0; j <= cheight; j++) {
        for (int i = 0; i <= cwidth; i++) {
//          out_array[offset] = 0;
          uint8_t in_value = in_array[offset];
          if (in_value > 0) {
            bool isSurface = false;
            for (int n = 0; n < conn; n++) {
              if (in_array[offset + neighbor[n]] != in_value) {
                isSurface = true;
                break;
              }
            }

            if (isSurface) {
              if (count++ % sparseLevel == 0) {
                SwcTreeNode::MakePointer(i + stack.getOffset().getX(),
                                         j + stack.getOffset().getY(),
                                         k + stack.getOffset().getZ(),
                                         sparseLevel * 0.7, root);
              }
            }
          }
          offset++;
        }
      }
    }

  }

#if 0
  Stack *surface = Stack_Perimeter(stack.c_stack(), NULL, 6);

#ifdef _DEBUG_2
  C_Stack::write(GET_DATA_DIR + "/test.tif", surface);
#endif


  if (surface != NULL) {
    tree = new ZSwcTree();
    tree->forceVirtualRoot();
    Swc_Tree_Node *root = tree->root();

    int width = C_Stack::width(surface);
    int height = C_Stack::height(surface);
    int depth = C_Stack::depth(surface);

    size_t offset = 0;
    int count = 0;
    for (int z = 0; z < depth; ++z) {
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          if ((surface->array[offset++]) > 0) {
            if (count++ % sparseLevel == 0) {
              SwcTreeNode::makePointer(x + stack.getOffset().getX(),
                                       y + stack.getOffset().getY(),
                                       z + stack.getOffset().getZ(),
                                       sparseLevel * 0.7, root);
            }
          }
        }
      }
    }

    C_Stack::kill(surface);
  }
#endif

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZClosedCurve &curve, double radius)
{
  ZSwcTree *tree = NULL;
  if (!curve.isEmpty()) {
    tree = new ZSwcTree();
    tree->setStructrualMode(ZSwcTree::STRUCT_CLOSED_CURVE);
    Swc_Tree_Node *parent =
        SwcTreeNode::MakePointer(curve.getLandmark(0), radius);
    tree->addRegularRoot(parent);
    for (size_t i = 1; i < curve.getLandmarkNumber(); ++i) {
      Swc_Tree_Node *tn =
          SwcTreeNode::MakePointer(curve.getLandmark(i), radius);
      SwcTreeNode::setParent(tn, parent);
      parent = tn;
    }
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(
    const ZObject3dScan &blockObj, int z, const ZDvidInfo &dvidInfo)
{
#ifdef _FLYEM_
  ZObject3dScan slice = blockObj.getSlice(z);
  size_t stripeNumber = slice.getStripeNumber();

  ZSwcTree *tree = new ZSwcTree;
  for (size_t s = 0; s < stripeNumber; ++s) {
    const ZObject3dStripe &stripe = slice.getStripe(s);
    int nseg = stripe.getSegmentNumber();
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int i = 0; i < nseg; ++i) {
      int x0 = stripe.getSegmentStart(i);
      int x1 = stripe.getSegmentEnd(i);
      for (int x = x0; x <= x1; ++x) {
        ZIntCuboid cuboid = dvidInfo.getBlockBox(x, y, z);
        tree->merge(CreateBoxSwc(cuboid));
      }
    }
  }

  return tree;
#else
  UNUSED_PARAMETER(blockObj);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(dvidInfo);
  return NULL;
#endif
}

/*
void ZSwcFactory::Attach(const ZLocsegChain &chain, Swc_Tree_Node *parent)
{
  if (parent) {
    int n;
    Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array(
          chain.data(), NULL, &n);

    if (n > 0) {
      for (int i = 0; i < n; ++i) {
        parent = SwcTreeNode::makePointer(
              circles[i].center[0], circles[i].center[1], circles[i].center[2],
            circles[i].radius, parent);
      }
    }

    if (circles) {
      free(circles);
    }
  }
}
*/

ZSwcTree* ZSwcFactory::CreateSwc(Locseg_Chain *chain, ZSwcTree *tree)
{
  if (chain) {
    int n;
    Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array(chain, NULL, &n);

    if (n > 0) {
      if (tree == nullptr) {
        tree = new ZSwcTree;
      }

      tree->forceVirtualRoot();
      Swc_Tree_Node *parent = tree->root();
      for (int i = 0; i < n; ++i) {
        parent = SwcTreeNode::MakePointer(
              circles[i].center[0], circles[i].center[1], circles[i].center[2],
            circles[i].radius, parent);
      }
    }

    if (circles) {
      free(circles);
    }
  }

  return tree;
}

ZSwcTree* ZSwcFactory::CreateSwc(const ZLocsegChain &chain, ZSwcTree *tree)
{
  return CreateSwc(chain.data(), tree);
}

ZSwcTree* ZSwcFactory::CreateSwc(
      const std::vector<Locseg_Chain*> &chainArray, ZSwcTree *host)
{
  for (Locseg_Chain *chain : chainArray) {
    host = CreateSwc(chain, host);
  }

  return host;
}
