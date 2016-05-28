#include "zswcgenerator.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "swc/zswcresampler.h"
#if defined(_FLYEM_)
#include "flyem/zflyemneuronrangecompare.h"
#endif
#include "zdoublevector.h"
#include "zpointarray.h"
#include "zlinesegmentarray.h"
#include "zintcuboidface.h"
#if _QT_GUI_USED_
#include "zstroke2d.h"
#endif
#include "zobject3d.h"
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "tz_stack_bwmorph.h"
#include "neutubeconfig.h"
#include "tz_math.h"
#include "zclosedcurve.h"
#include "tz_stack_neighborhood.h"

ZSwcGenerator::ZSwcGenerator()
{
}

ZSwcTree* ZSwcGenerator::createVirtualRootSwc()
{
  ZSwcTree *tree = new ZSwcTree;
  tree->forceVirtualRoot();

  return tree;
}

ZSwcTree* ZSwcGenerator::createCircleSwc(double cx, double cy, double cz, double r)
{
  if (r < 0.0) {
    return NULL;
  }

  ZSwcTree *tree = createVirtualRootSwc();

  Swc_Tree_Node *parent = tree->root();

  double nodeRadius = r * 0.05;

  for (double angle = 0.0; angle < 6.0; angle += 0.314) {
    double x, y, z;

    x = r * cos(angle) + cx;
    y = r * sin(angle) + cy;
    z = cz;

    Swc_Tree_Node *tn = SwcTreeNode::makePointer(x, y, z, nodeRadius);
    SwcTreeNode::setParent(tn, parent);
    parent = tn;
  }

  Swc_Tree_Node *tn = SwcTreeNode::makePointer();
  SwcTreeNode::copyProperty(SwcTreeNode::firstChild(tree->root()), tn);
  SwcTreeNode::setParent(tn, parent);

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createBoxSwc(const ZCuboid &box, double radius)
{
  return ZSwcTree::CreateCuboidSwc(box, radius);
}

ZSwcTree* ZSwcGenerator::createBoxSwc(const ZIntCuboid &box, double radius)
{
  ZCuboid cuboid;
  cuboid.setFirstCorner(ZPoint(box.getFirstCorner().toPoint()));
  cuboid.setSize(box.getWidth(), box.getHeight(), box.getDepth());

  return createBoxSwc(cuboid, radius);
}
#if defined (_FLYEM_)
ZSwcTree* ZSwcGenerator::createSwc(const ZFlyEmNeuronRange &range)
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

  ZSwcTree *tree = createVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = createCircleSwc(0, 0, z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(
    const ZFlyEmNeuronRange &range, const ZFlyEmNeuronAxis &axis)
{
  double minZ = range.getMinZ();
  double maxZ = range.getMaxZ();

  double dz = (maxZ - minZ) / 50.0;

  ZSwcTree *tree = createVirtualRootSwc();

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZPoint pt = axis.getCenter(z);
    ZSwcTree *subtree = createCircleSwc(pt.x(), pt.y(), z, r);
    tree->merge(subtree, true);
  }

  tree->resortId();

  return tree;
}
#endif

#if defined (_FLYEM_)
ZSwcTree* ZSwcGenerator::createRangeCompareSwc(
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

  ZSwcTree *tree = createVirtualRootSwc();

  double minReferenceZ = reference.getMinZ();
  double maxReferenceZ = reference.getMaxZ();

  for (double z = minReferenceZ; z < minZ; z += dz) {
    ZSwcTree *subtree = createCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = maxZ + dz; z <= maxReferenceZ; z += dz) {
    ZSwcTree *subtree = createCircleSwc(0, 0, z, reference.getRadius(z));
    subtree->setType(5);
    tree->merge(subtree, true);
  }

  for (double z = minZ; z <= maxZ; z += dz) {
    double r = range.getRadius(z);
    ZSwcTree *subtree = createCircleSwc(0, 0, z, r);
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
#endif
ZSwcTree* ZSwcGenerator::createSwcByRegionSampling(
    const ZVoxelArray &voxelArray, double radiusAdjustment)
{
#ifdef _DEBUG_2
  voxelArray.print();
#endif

  ZDoubleVector voxelSizeArray(voxelArray.size());

  const std::vector<ZVoxel> &voxelData = voxelArray.getInternalData();
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
      Swc_Tree_Node *tn = SwcTreeNode::makePointer();
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

ZSwcTree* ZSwcGenerator::createSwc(
    const ZVoxelArray &voxelArray, ZSwcGenerator::EPostProcess option)
{
  if (option == REGION_SAMPLING) {
    return createSwcByRegionSampling(voxelArray);
  }

  size_t startIndex = 0;
  size_t endIndex = voxelArray.size() - 1;

  Swc_Tree *tree = New_Swc_Tree();

  const std::vector<ZVoxel> &voxelData = voxelArray.getInternalData();
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

ZSwcTree* ZSwcGenerator::createSwc(
    const ZPointArray &pointArray, double radius, bool isConnected)
{
  ZSwcTree *tree = new ZSwcTree;
  tree->useCosmeticPen(true);

  Swc_Tree_Node *root = tree->forceVirtualRoot();
  Swc_Tree_Node *parent = root;

  for (ZPointArray::const_iterator iter = pointArray.begin();
       iter != pointArray.end(); ++iter) {
    const ZPoint &pt = *iter;
    Swc_Tree_Node *tn = New_Swc_Tree_Node();

    SwcTreeNode::setPos(tn, pt.x(), pt.y(), pt.z());
    SwcTreeNode::setRadius(tn, radius);
    SwcTreeNode::setParent(tn, parent);
    if (isConnected) {
      parent = tn;
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(
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

ZSwcTree* ZSwcGenerator::createSwc(const ZIntCuboidFace &face, double radius)
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

  return createSwc(ptArray, radius, true);
}

ZSwcTree* ZSwcGenerator::createSwc(
    const ZIntCuboidFaceArray &faceArray, double radius)
{
  if (faceArray.empty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree;

  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
       iter != faceArray.end(); ++iter) {
    ZSwcTree *subtree = createSwc(*iter, radius);
    tree->merge(subtree, true);
  }

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(const ZStroke2d &stroke)
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
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(x, y, z, r);
    SwcTreeNode::setParent(tn, parent);
    parent = tn;
  }

  tree->resortId();
  return tree;
#else
  return NULL;
#endif
}

ZSwcTree* ZSwcGenerator::createSwc(
    const ZObject3d &obj, double radius, int sampleStep)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZSwcTree *tree = new ZSwcTree();
  tree->forceVirtualRoot();
  Swc_Tree_Node *parent = tree->root();
  for (size_t i = 0; i < obj.size(); i += sampleStep) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(
          obj.getX(i), obj.getY(i), obj.getZ(i), radius);
    SwcTreeNode::setId(tn, i + 1);
    SwcTreeNode::setFirstChild(parent, tn);
//    SwcTreeNode::setParent(tn, parent);
  }

//  tree->resortId();
  tree->setColor(obj.getColor());

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(const ZObject3dScan &obj)
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
          SwcTreeNode::makePointer(stripe.getSegmentStart(j), y, z, 2.0);
      SwcTreeNode::setFirstChild(root, tn);
      Swc_Tree_Node *tn2 =
          SwcTreeNode::makePointer(stripe.getSegmentEnd(j), y, z, 2.0);
      SwcTreeNode::setFirstChild(tn, tn2);
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcGenerator::createSurfaceSwc(
    const ZObject3dScan &obj, int sparseLevel)
{
  size_t volume = obj.getBoundBox().getVolume();

  int intv = 0;
  if (volume > MAX_INT32) {
    intv = iround(Cube_Root((double) volume / MAX_INT32));
  }

  ZStack *stack = NULL;
  std::cout << "Creating object mask ..." << "ds: " << intv <<  std::endl;
  if (intv > 0) {
    ZObject3dScan obj2 = obj;
    obj2.downsampleMax(intv, intv, intv);
    stack = obj2.toStackObject();
  } else {
    stack = obj.toStackObject();
  }

  ZSwcTree *tree = NULL;
  if (stack != NULL) {
    tree = createSurfaceSwc(*stack, sparseLevel);
    tree->setColor(obj.getColor());
    tree->rescale(intv + 1, intv + 1, intv + 1);
    delete stack;
  }

  return tree;
}

ZSwcTree* ZSwcGenerator::createSurfaceSwc(const ZStack &stack, int sparseLevel)
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
    int is_in_bound[26];
    int n_in_bound;
    int count = 0;

    tree = new ZSwcTree();
    tree->forceVirtualRoot();
    Swc_Tree_Node *root = tree->root();
    Stack_Neighbor_Offset(conn, width, height, neighbor);

    for (int k = 0; k <= cdepth; k++) {
      for (int j = 0; j <= cheight; j++) {
        for (int i = 0; i <= cwidth; i++) {
//          out_array[offset] = 0;
          if (in_array[offset] > 0) {
            n_in_bound = Stack_Neighbor_Bound_Test_S(
                  conn, cwidth, cheight, cdepth, i, j, k, is_in_bound);
            bool isSurface = false;
            if (n_in_bound == conn) {
              for (int n = 0; n < n_in_bound; n++) {
                if (in_array[offset + neighbor[n]] != in_array[offset]) {
                  isSurface = true;
                  break;
                }
              }
            } else {
              isSurface = true;
            }

            if (isSurface) {
              if (count++ % sparseLevel == 0) {
                SwcTreeNode::makePointer(i + stack.getOffset().getX(),
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

ZSwcTree* ZSwcGenerator::createSwc(const ZClosedCurve &curve, double radius)
{
  ZSwcTree *tree = NULL;
  if (!curve.isEmpty()) {
    tree = new ZSwcTree();
    tree->setStructrualMode(ZSwcTree::STRUCT_CLOSED_CURVE);
    Swc_Tree_Node *parent =
        SwcTreeNode::makePointer(curve.getLandmark(0), radius);
    tree->addRegularRoot(parent);
    for (size_t i = 1; i < curve.getLandmarkNumber(); ++i) {
      Swc_Tree_Node *tn =
          SwcTreeNode::makePointer(curve.getLandmark(i), radius);
      SwcTreeNode::setParent(tn, parent);
      parent = tn;
    }
  }

  return tree;
}

ZSwcTree* ZSwcGenerator::createSwc(
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
        tree->merge(createBoxSwc(cuboid));
      }
    }
  }

  return tree;
#else
  UNUSED_PARAMETER(&blockObj);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(&dvidInfo);
  return NULL;
#endif
}

ZSwcTree* ZSwcGenerator::createSwcProjection(const ZSwcTree *tree)
{
  ZSwcTree *proj = NULL;
  if (tree != NULL) {
    proj = tree->clone();
    ZSwcTree::DepthFirstIterator iter(proj);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      if (SwcTreeNode::isRegular(tn)) {
        SwcTreeNode::setZ(tn, 0);
      }
    }
  }

  return proj;
}
