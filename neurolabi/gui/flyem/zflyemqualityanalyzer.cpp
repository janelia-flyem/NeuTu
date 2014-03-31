#include "zflyemqualityanalyzer.h"
#include "neutubeconfig.h"
#include "zswctree.h"
#include "tz_math.h"
#include "swctreenode.h"
#include "zpointarray.h"
#include "flyem/zhotspotfactory.h"

ZFlyEmQualityAnalyzer::ZFlyEmQualityAnalyzer()
{
}

bool ZFlyEmQualityAnalyzer::isExitingOrphanBody(const ZObject3dScan &obj)
{
  //Expand bottom blocks
  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  //Test if the body touches the boundary
  ZObject3dScan tmpObj = obj;
  tmpObj.dilate();
  for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
    ZObject3dStripe stripe = tmpObj.getStripe(i);
    if (stripe.getZ() > objBox.ce[2]) {
      return false;
    }

    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }

      if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmQualityAnalyzer::isOrphanBody(const ZObject3dScan &obj)
{
  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  int boxIndex = m_substackRegion.hitInternalTest(
        objBox.cb[0], objBox.cb[1], objBox.cb[2]);
  if (boxIndex >= 0) {
    if (boxIndex == m_substackRegion.hitInternalTest(
          objBox.ce[0], objBox.ce[1], objBox.ce[2])) {
      return true;
    }
  }

  return false;
}

bool ZFlyEmQualityAnalyzer::isStitchedOrphanBody(const ZObject3dScan &obj)
{
  if (obj.isEmpty()) {
    return false;
  }

  Cuboid_I objBox;
  obj.getBoundBox(&objBox);

  FlyEm::ZIntCuboidArray roi = m_substackRegion;
  roi.intersect(objBox);

  if (roi.size() == 1) {
    return false;
  } else {
#if 0

    FlyEm::ZIntCuboidArray innerFace = m_substackRegion.getInnerFace();
    FlyEm::ZIntCuboidArray face = m_substackRegion.getFace();

#ifdef _DEBUG_2
    innerFace.exportSwc(GET_DATA_DIR + "/test.swc");
    face.exportSwc(GET_DATA_DIR + "/test2.swc");
#endif


    if (face.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_
      std::cout << face.hitTest(objBox.cb[0], objBox.cb[1], objBox.cb[2]) << std::endl;
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.cb[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.cb[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.cb[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.cb[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.ce[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.ce[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.ce[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.ce[1], objBox.cb[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.ce[1], objBox.cb[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.ce[1] << ' '
                << objBox.cb[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.cb[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.cb[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.cb[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.cb[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.cb[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.cb[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.cb[0], objBox.ce[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.cb[0], objBox.ce[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.cb[0] << ' ' << objBox.ce[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
    if (face.hitTest(objBox.ce[0], objBox.ce[1], objBox.ce[2]) >= 0 &&
        innerFace.hitTest(objBox.ce[0], objBox.ce[1], objBox.ce[2]) < 0) {
#ifdef _DEBUG_2
      std::cout << "Boundary: " << objBox.ce[0] << ' ' << objBox.ce[1] << ' '
                << objBox.ce[2] << std::endl;
#endif
      return false;
    }
#endif

    ZObject3dScan tmpObj = obj;
    tmpObj.dilate();
    for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = tmpObj.getStripe(i);

      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                     stripe.getZ()) < 0) {
          return false;
        }

        if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                     stripe.getZ()) < 0) {
          return false;
        }
      }
    }
  }

  return true;
}

void ZFlyEmQualityAnalyzer::setSubstackRegion(const FlyEm::ZIntCuboidArray &roi)
{
  m_substackRegion = roi;
}

void ZFlyEmQualityAnalyzer::setSubstackRegion(
    const FlyEm::ZIntCuboidArray &roi, const SubstackRegionCalbration &calbr)
{
  m_substackRegion = roi;
  calbr.calibrate(m_substackRegion);
}

void ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(
    const ZFlyEmNeuron &neuron, const ZFlyEmNeuronRange &range, int label)
{
  ZSwcTree *tree = neuron.getModel();

  if (tree != NULL) {
    ZFlyEmNeuronAxis axis = neuron.getAxis();
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      ZPoint pt = SwcTreeNode::pos(tn);
      ZPoint axisCenter = axis.getCenter(pt.z());
      pt.setX(pt.x() - axisCenter.x());
      pt.setY(pt.y() - axisCenter.y());
      if (!range.contains(pt)) {
        SwcTreeNode::setLabel(tn, label);
      }
    }
  }
}

bool ZFlyEmQualityAnalyzer::touchingGlobalBoundary(const ZObject3dScan &obj)
{
  //Expand bottom blocks
  /*
  Cuboid_I objBox;
  obj.getBoundBox(&objBox);
  */

  Cuboid_I boundBox = m_substackRegion.getBoundBox();

  //Test if the body touches the boundary
  ZObject3dScan tmpObj = obj;
  tmpObj.dilate();
  for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = tmpObj.getStripe(i);

    if (stripe.getZ() < boundBox.cb[2]) {
      return true;
    }

    if (stripe.getZ() > boundBox.ce[2]) {
      return true;
    }

    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }

      if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmQualityAnalyzer::touchingSideBoundary(const ZObject3dScan &obj)
{
  Cuboid_I boundBox = m_substackRegion.getBoundBox();

  //Test if the body touches the side boundary
  ZObject3dScan tmpObj = obj;
  tmpObj.dilate();
  for (size_t i = 0; i < tmpObj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = tmpObj.getStripe(i);

    if (stripe.getZ() < boundBox.cb[2]) { //Skip the stripe if it's above the top
      continue;
    }

    if (stripe.getZ() > boundBox.ce[2]) { //Skpe the stripe if it's below the bottom
      continue;
    }

    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      if (m_substackRegion.hitTest(stripe.getSegmentStart(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }

      if (m_substackRegion.hitTest(stripe.getSegmentEnd(j), stripe.getY(),
                                   stripe.getZ()) < 0) {
        return true;
      }
    }
  }

  return false;
}

ZFlyEmQualityAnalyzer::SubstackRegionCalbration::SubstackRegionCalbration()
{
  for (int i = 0; i < 3; i++) {
    m_margin[i] = 0;
    m_bounding[i] = true;
  }
}

void ZFlyEmQualityAnalyzer::SubstackRegionCalbration::setMargin(
    int x, int y, int z)
{
  m_margin[0] = x;
  m_margin[1] = y;
  m_margin[2] = z;
}

void ZFlyEmQualityAnalyzer::SubstackRegionCalbration::setBounding(
    bool x, bool y, bool z)
{
  m_bounding[0] = x;
  m_bounding[1] = y;
  m_bounding[2] = z;
}

void ZFlyEmQualityAnalyzer::SubstackRegionCalbration::calibrate(
    FlyEm::ZIntCuboidArray &roi) const
{
  Cuboid_I boundBox = roi.getBoundBox();

  int m_offset[3] = {0, 0, 0};
  for (int i = 0; i < 3; i++) {
    m_offset[i] = m_margin[i];
    if (m_bounding[i]) {
      m_offset[i] -= boundBox.cb[i];
    }
  }

  roi.translate(m_offset[0], m_offset[1], m_offset[2]);
}

FlyEm::ZHotSpotArray
ZFlyEmQualityAnalyzer::computeHotSpot(const ZFlyEmNeuron *neuron)
{
  return computeHotSpot(*neuron);
}

FlyEm::ZHotSpotArray
ZFlyEmQualityAnalyzer::computeHotSpot(const ZFlyEmNeuron &neuron)
{
  neuron.print();
  ZSwcTree *tree = neuron.getModel();
  //ZPointArray pointArray;
  FlyEm::ZHotSpotArray hotSpotArray;

  const double *resolution = neuron.getSwcResolution();

#ifdef _DEBUG_
  std::cout << "Computing hot spot ..." << std::endl;
#endif

  if (tree != NULL) {
    ZSwcTree *tmpTree = tree->clone();
    Swc_Tree_Remove_Terminal_Branch(tmpTree->data(), 100 * resolution[0]);
    const std::vector<Swc_Tree_Node *> terminalArray =
        tmpTree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);
    for (std::vector<Swc_Tree_Node *>::const_iterator iter =
         terminalArray.begin(); iter != terminalArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;

      ZObject3dScan *obj = neuron.getBody();
      while (SwcTreeNode::isRegular(tn)) {
        int x = iround(SwcTreeNode::x(tn) / resolution[0]);
        int y = iround(SwcTreeNode::y(tn) / resolution[1]);
        int z = iround(SwcTreeNode::z(tn) / resolution[2]);

        if (!m_substackRegion.empty()) {
          if (m_substackRegion.hitTest(x, y, z) < 0) {
            break;
          }
        }
        FlyEm::ZHotSpot *hotSpot = NULL;

        if (obj != NULL) {
          if (obj->contains(x, y, z)) {
            //pointArray.append(x, y, z);
            //hotSpotArray.append(hotSpot);
            hotSpot = FlyEm::ZHotSpotFactory::createPointHotSpot(x, y, z);
          }
        } else {
          //hotSpotArray.append(hotSpot);
          //pointArray.append(x, y, z);
          hotSpot = FlyEm::ZHotSpotFactory::createPointHotSpot(x, y, z);
        }
        if (hotSpot != NULL) {
          hotSpotArray.append(hotSpot);
          break;
        }

        tn = SwcTreeNode::parent(tn);
      }
    }
  } else {
#ifdef _DEBUG_
    std::cout << "Null tree. Failed." << std::endl;
#endif
  }

  //pointArray.print();

  return hotSpotArray;
}
