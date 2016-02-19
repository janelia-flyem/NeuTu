#include "zflyemservice.h"
#include "dvid/zdvidreader.h"
#include "zintpairmap.h"
#include "tz_math.h"
#include "tz_error.h"
#include "neutube.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "flyem/zsynapseannotation.h"
#include "flyem/zsynapseannotationarray.h"

FlyEm::Service::FaceOrphanOverlap::FaceIndex::FaceIndex() :
  m_cuboidIndex(-1), m_faceIndex(-1)
{
}

FlyEm::Service::FaceOrphanOverlap::FaceIndex::FaceIndex(int ci, int fi) :
  m_cuboidIndex(ci), m_faceIndex(fi)
{
}

FlyEm::Service::FaceOrphanOverlap::FaceOrphanOverlap() :
  m_overlapGraph(ZGraph::DIRECTED_WITH_WEIGHT),
  m_faceGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT)
{
}

void FlyEm::Service::FaceOrphanOverlap::clearFaceStackArray()
{
  for (std::vector<ZStack*>::iterator iter = m_faceStackArray.begin();
       iter != m_faceStackArray.end(); ++iter) {
    delete *iter;
  }
  m_faceStackArray.clear();
}

void FlyEm::Service::FaceOrphanOverlap::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

bool FlyEm::Service::FaceOrphanOverlap::hasTouchingFace(
    const Cuboid_I &c1, const Cuboid_I &c2)
{
  Cuboid_I largeBox;
  for (int i = 0; i < 3; ++i) {
    largeBox.cb[i] = c1.cb[i] - 1;
    largeBox.ce[i] = c1.ce[i] + 1;
  }

  return Cuboid_I_Is_Valid(Cuboid_I_Intersect(&largeBox, &c2, &largeBox));
}

std::pair<int, int> FlyEm::Service::FaceOrphanOverlap::touchTest(
    const Cuboid_I &c1, const Cuboid_I &c2)
{
  std::pair<int, int> touching(-1, -1);
  if (hasTouchingFace(c1, c2)) {
    ZIntCuboidFaceArray faceArray1;
    faceArray1.append(&c1);
    faceArray1.moveBackward(1);

    ZIntCuboidFaceArray faceArray2;
    faceArray2.append(&c2);
    for (size_t i = 0; i < faceArray1.size(); ++i) {
      ZIntCuboidFace &face1 = faceArray1[i];
      for (size_t j = 0; j < faceArray2.size(); ++j) {
        ZIntCuboidFace &face2 = faceArray2[j];
        if (face1.hasOverlap(face2)) {
          touching.first = i;
          touching.second = j;
          return touching;
        }
      }
    }
  }

  return touching;
}

void FlyEm::Service::FaceOrphanOverlap::loadFace(
    const ZIntCuboidArray &cuboidArray)
{
  m_cuboidArray = cuboidArray;
  m_faceGroup.clear();
  for (ZIntCuboidArray::const_iterator iter = cuboidArray.begin();
       iter != cuboidArray.end(); ++iter) {
    ZIntCuboidFaceArray faceArray;
    faceArray.append(&(*iter));
    m_faceGroup.push_back(faceArray);
  }

  m_faceGraph.clear();

  for (size_t i = 0; i < cuboidArray.size(); ++i) {
    const Cuboid_I &cuboid1 = cuboidArray[i];
    for (size_t j = i + 1; j < cuboidArray.size(); ++j) {
      const Cuboid_I &cuboid2 = cuboidArray[j];
      std::pair<int, int> touchingFace = touchTest(cuboid1, cuboid2);
      if (touchingFace.first >= 0) {
        FaceIndex index1(i, touchingFace.first);
        FaceIndex index2(j, touchingFace.second);
        m_faceGraph.addEdge(getStackIndex(index1), getStackIndex(index2));
      }
    }
  }

#ifdef _DEBUG_2
  m_faceGraph.print();
#endif
}

#if 0
void FlyEm::Service::FaceOrphanOverlap::markBody()
{
  clearFaceStackArray();

  ZDvidReader reader;
  reader.open(m_dvidTarget);

  for (std::vector<ZIntCuboidFaceArray>::const_iterator
       iter = m_faceGroup.begin(); iter != m_faceGroup.end(); ++iter) {
    const ZIntCuboidFaceArray &faceArray = *iter;
    for (ZIntCuboidFaceArray::const_iterator iter= faceArray.begin();
         iter != faceArray.end(); ++iter) {
      const ZIntCuboidFace &face = *iter;
      ZIntPoint firstCorner = face.getCornerCoordinates(0);
      ZIntPoint lastCorner = face.getCornerCoordinates(3);
      ZStack *stack = reader.readBodyLabel(
            firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
            lastCorner.getX() - firstCorner.getX() + 1,
            lastCorner.getY() - firstCorner.getY() + 1,
            lastCorner.getZ() - firstCorner.getZ() + 1);
      m_faceStackArray.push_back(stack);
    }
  }
}
#endif

void FlyEm::Service::FaceOrphanOverlap::markBodyMock()
{
  clearFaceStackArray();

  ZDvidReader reader;
  reader.open(m_dvidTarget);

  TBodyLabel value = 1;
  for (std::vector<ZIntCuboidFaceArray>::const_iterator
       iter = m_faceGroup.begin(); iter != m_faceGroup.end(); ++iter) {
    const ZIntCuboidFaceArray &faceArray = *iter;
    for (ZIntCuboidFaceArray::const_iterator iter= faceArray.begin();
         iter != faceArray.end(); ++iter, ++value) {
      const ZIntCuboidFace &face = *iter;
      ZIntPoint firstCorner = face.getCornerCoordinates(0);
      ZIntPoint lastCorner = face.getCornerCoordinates(3);
      int width = lastCorner.getX() - firstCorner.getX() + 1;
      int height = lastCorner.getY() - firstCorner.getY() + 1;
      int depth = lastCorner.getZ() - firstCorner.getZ() + 1;

      ZStack *stack = new ZStack(FLOAT64, width, height, depth, 1);
      TBodyLabel *array = (TBodyLabel*) stack->array8();
      size_t voxelNumber = width * height * depth;
      for (size_t i = 0; i < voxelNumber; ++i) {
        if (value == 85) {
          array[i] = 34677;
        } else if (value ==527) {
          array[i] = 236315;
        } else if (value == 163) {
          array[i] = 66038;
        } else if (value == 169) {
          array[i] = 67948;
        } else if (value == 1271) {
          array[i] = 625684;
        } else {
          array[i] = value;
        }
      }
      stack->setOffset(firstCorner.getX(), firstCorner.getY(),
                       firstCorner.getZ());
      m_faceStackArray.push_back(stack);
    }
  }
}

std::vector<FlyEm::Service::FaceOrphanOverlap::FaceIndex>
FlyEm::Service::FaceOrphanOverlap::getFaceIndexForBody(int bodyIndex) const
{
  return m_faceIndexArray[bodyIndex];
}

int FlyEm::Service::FaceOrphanOverlap::getStackIndex(
    const FaceIndex &faceIndex) const
{
  return faceIndex.getCuboidIndex() * 6 + faceIndex.getFaceIndex();
}

std::vector<ZStack*> FlyEm::Service::FaceOrphanOverlap::getNeighborStackList(
    int stackIndex) const
{
  std::set<int> neighborSet = m_faceGraph.getNeighborSet(stackIndex);
  std::vector<ZStack*> stackArray;
  for (std::set<int>::const_iterator iter = neighborSet.begin();
       iter != neighborSet.end(); ++iter) {
    stackArray.push_back(m_faceStackArray[*iter]);
  }

  return stackArray;
}

ZStack* FlyEm::Service::FaceOrphanOverlap::transformStack(ZStack *stack)
{
  Mc_Stack *stackData = (Mc_Stack *) malloc(sizeof(Mc_Stack));
  ZStack *out = new ZStack(stackData, C_Stack::freePointer);
  *stackData = *(stack->data());

  if (stack->width() == 1) {
    out->reshape(stack->height(), stack->depth(), 1);
    out->setOffset(stack->getOffset().getY(), stack->getOffset().getZ(),
                   stack->getOffset().getX());
  } else if (stack->height() == 1) {
    out->reshape(stack->width(), stack->depth(), 1);
    out->setOffset(stack->getOffset().getX(), stack->getOffset().getZ(),
                   stack->getOffset().getY());
  } else if (stack->depth() == 1) {
    out->reshape(stack->width(), stack->height(), 1);
    out->setOffset(stack->getOffset().getX(), stack->getOffset().getY(),
                   stack->getOffset().getZ());
  }

  return out;
}

void FlyEm::Service::FaceOrphanOverlap::transformCoordinates(
    ZStack *stack, int x, int y, int *cx, int *cy, int *cz)
{
  if (stack->width() == 1) {
    *cx = stack->getOffset().getX();
    *cy = stack->getOffset().getY() + x;
    *cz = stack->getOffset().getZ() + y;
  } else if (stack->height() == 1) {
    *cx = stack->getOffset().getX() + x;
    *cy = stack->getOffset().getY();
    *cz = stack->getOffset().getZ() + y;
  } else if (stack->depth() == 1) {
    *cx = stack->getOffset().getX() + x;
    *cy = stack->getOffset().getY() + y;
    *cz = stack->getOffset().getZ();
  }
}

void FlyEm::Service::FaceOrphanOverlap::printMarkerBuffer()
{
  std::cout << "Marker buffer: " << std::endl;
  for (std::map<std::pair<int, int>, ZIntPoint>::const_iterator
       iter = m_markerBuffer.begin(); iter != m_markerBuffer.end(); ++iter) {
    const std::pair<int, int> &key = iter->first;
    const ZIntPoint &value = iter->second;
    std::cout << "  " << "(" << key.first << ", " << key.second << ") -- > "
              << value.getX() << ", " << value.getY() << ", " << value.getZ()
              << std::endl;
  }
}

ZIntPairMap FlyEm::Service::FaceOrphanOverlap::countVoxelTouch(
    ZStack *stack1, ZStack *stack2, int sourceBodyId)
{
#ifdef _DEBUG_
  stack1->printInfo();
  stack2->printInfo();
#endif

  ZStack *tstack1 = transformStack(stack1);
  ZStack *tstack2 = transformStack(stack2);

  ZIntPairMap crossMap;

  int tx1 = tstack1->getOffset().getX();
  int ty1 = tstack1->getOffset().getY();
  int tx2 = tstack2->getOffset().getX();
  int ty2 = tstack2->getOffset().getY();

  size_t offset1 = 0;
  size_t offset2 = 0;

  TBodyLabel *array1 = (TBodyLabel*) tstack1->array8();
  TBodyLabel *array2 = (TBodyLabel*) tstack2->array8();

  for (int y = 0; y < tstack1->height(); ++y) {
    int y2 = y + ty1 - ty2;
    bool isYInRange = (y2 >= 0 && y2 < tstack2->height());
    for (int x = 0; x < tstack1->width(); ++x) {
      if (isYInRange) {
        int x2 = x + tx1 - tx2;
        if (x2 >= 0 && x2 < tstack2->width()) {
          offset2 = y2 * tstack2->width() + x2;
          TZ_ASSERT(array1[offset1] <= MAX_INT32, "too large label");
          TZ_ASSERT(array2[offset2] <= MAX_INT32, "too large label");
          int label1 = (int) array1[offset1];
          int label2 = (int) array2[offset2];
          if (label1 == sourceBodyId && label2 > 0) {
            crossMap.incPairCount(label1, label2);
            std::pair<int, int> key(label1, label2);
            if (m_markerBuffer.count(key) == 0) {
              int cx = 0;
              int cy = 0;
              int cz = 0;
              transformCoordinates(stack1, x, y, &cx, &cy, &cz);
              if (cx == 0 && cy == 0 && cz == 0) {
                std::cout << "Potential bug" << std::endl;
              }
              m_markerBuffer[key] = ZIntPoint(cx, cy, cz);
            }
          }
        }
      }
      ++offset1;
    }
  }

  delete tstack1;
  delete tstack2;

#ifdef _DEBUG_
  crossMap.print();
#endif

  return crossMap;
}

ZGraph* FlyEm::Service::FaceOrphanOverlap::computeOverlap()
{
  m_overlapGraph.clear();
  m_markerBuffer.clear();

  size_t bodyIndex = 0;
  for (std::vector<int>::const_iterator iter = m_bodyIdArray.begin();
       iter != m_bodyIdArray.end(); ++iter, ++bodyIndex) {
    int bodyId = *iter;
    std::vector<FaceIndex> faceIndexArray = getFaceIndexForBody(bodyIndex);

    ZIntPairMap overallHist;
    for (std::vector<FaceIndex>::const_iterator iter = faceIndexArray.begin();
         iter != faceIndexArray.end(); ++iter) {
      const FaceIndex &faceIndex = *iter;
      int stackIndex = getStackIndex(faceIndex);
      ZStack *stack = m_faceStackArray[stackIndex];

#ifdef _DEBUG_
      std::cout << "Stack index: " << stackIndex << std::endl;
#endif

#ifdef _DEBUG_2
      printLabelSet(stack);
#endif

      std::vector<ZStack*> neighborStackArray = getNeighborStackList(stackIndex);

      for (std::vector<ZStack*>::const_iterator iter = neighborStackArray.begin();
           iter != neighborStackArray.end(); ++iter) {
        ZStack *neighborStack = *iter;
        ZIntPairMap hist = countVoxelTouch(stack, neighborStack, bodyId);
        overallHist.add(hist);
      }
    }
    if (!overallHist.empty()) {
      std::pair<int, int> bodyPair = overallHist.getPeak();
      m_overlapGraph.addEdge(bodyPair.first, bodyPair.second,
                             overallHist[bodyPair]);
      m_marker.push_back(m_markerBuffer[bodyPair]);
    }
  }

#ifdef _DEBUG_
  printMarkerBuffer();
  m_overlapGraph.print();
#endif

  return &m_overlapGraph;
}

void FlyEm::Service::FaceOrphanOverlap::loadSynapse(const std::string &filePath)
{
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(filePath);
  m_synapseCount = synapseAnnotation.countSynapse();
}

void FlyEm::Service::FaceOrphanOverlap::loadFaceOrphanBody(
    const std::vector<int> &bodyIdArray)
{
  m_faceOrphanBody.clear();
  m_faceIndexArray.clear();
  m_faceIndexArray.resize(bodyIdArray.size());

  m_bodySizeArray.resize(bodyIdArray.size());
  //m_synapseCount.resize(bodyIdArray.size());

  m_bodyIdArray = bodyIdArray;

  ZDvidReader reader;
  reader.open(m_dvidTarget);
  size_t bodyIndex = 0;
  for (std::vector<int>::const_iterator iter = bodyIdArray.begin();
       iter != bodyIdArray.end(); ++iter, ++bodyIndex) {
    ZObject3dScan obj = reader.readBody(*iter);

    m_bodySizeArray[bodyIndex] = obj.getVoxelNumber();

    m_faceOrphanBody.push_back(obj);
    //Check which face it is in
    Cuboid_I box;
    obj.getBoundBox(&box);
#ifdef _DEBUG_2
    Print_Cuboid_I(&box);
#endif
    int cuboidIndex = m_cuboidArray.hitTest(box.cb[0], box.cb[1], box.cb[2]);
    ZIntCuboidFaceArray &faceArray = m_faceGroup[cuboidIndex];
    int faceIndex = 0;
    for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
         iter != faceArray.end(); ++iter, ++faceIndex) {
      const ZIntCuboidFace &face = *iter;
      if (face.contains(box.cb[0], box.cb[1], box.cb[2]) ||
          face.contains(box.ce[0], box.cb[1], box.cb[2]) ||
          face.contains(box.cb[0], box.ce[1], box.cb[2]) ||
          face.contains(box.ce[0], box.ce[1], box.cb[2]) ||
          face.contains(box.cb[0], box.cb[1], box.ce[2]) ||
          face.contains(box.ce[0], box.cb[1], box.ce[2]) ||
          face.contains(box.cb[0], box.ce[1], box.ce[2]) ||
          face.contains(box.ce[0], box.ce[1], box.ce[2])) {
        m_faceIndexArray[bodyIndex].push_back(FaceIndex(cuboidIndex, faceIndex));
#ifdef _DEBUG_
        std::cout << "Body-face: " << cuboidIndex << " " << faceIndex << std::endl;
#endif
      }
    }
  }
}

void FlyEm::Service::FaceOrphanOverlap::print() const
{
  std::cout << "Face Orphan Overlap Service: " << std::endl;
  m_dvidTarget.print();
  std::cout << m_faceGroup.size() << " blocks." << std::endl;
  std::cout << m_faceStackArray.size() << " faces." << std::endl;
  std::cout << m_faceGraph.getEdgeNumber() << " touched face pairs." << std::endl;
  std::cout << m_overlapGraph.getEdgeNumber() << " overlap regions." << std::endl;
}

void FlyEm::Service::FaceOrphanOverlap::printLabelSet(const ZStack *stack)
{
  std::cout << "Face label: " << std::endl;
  stack->printInfo();

  std::set<FlyEm::TBodyLabel> bodySet;

  size_t voxelNumber = stack->getVoxelNumber();

  FlyEm::TBodyLabel *labelArray =
      (FlyEm::TBodyLabel*) (stack->array8());
  for (size_t i = 0; i < voxelNumber; ++i) {
    bodySet.insert(labelArray[i]);
  }

  std::cout << bodySet.size() << " labels: " << std::endl;
  for (std::set<FlyEm::TBodyLabel>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    std::cout << *iter << std::endl;
  }
}

void FlyEm::Service::FaceOrphanOverlap::loadFaceOrphanBodyMock(
    const std::vector<int> &bodyIdArray)
{
  m_faceOrphanBody.clear();
  m_faceIndexArray.clear();
  m_faceIndexArray.resize(bodyIdArray.size());
  m_bodySizeArray.resize(bodyIdArray.size());

  m_bodyIdArray = bodyIdArray;

  ZDvidReader reader;
  reader.open(m_dvidTarget);
  size_t bodyIndex = 0;
  for (std::vector<int>::const_iterator iter = bodyIdArray.begin();
       iter != bodyIdArray.end(); ++iter, ++bodyIndex) {
    int bodyId = *iter;
    ZObject3dScan obj;
    std::string path = GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/0_100000/stacked.hf5:" +
        ZString::num2str(bodyId) + ".sobj";
    obj.load(path);
    m_bodySizeArray[bodyIndex] = obj.getVoxelNumber();
    m_faceOrphanBody.push_back(obj);
    //Check which face it is in
    Cuboid_I box;
    obj.getBoundBox(&box);
#ifdef _DEBUG_2
    Print_Cuboid_I(&box);
#endif
    int cuboidIndex = m_cuboidArray.hitTest(box.cb[0], box.cb[1], box.cb[2]);
    ZIntCuboidFaceArray &faceArray = m_faceGroup[cuboidIndex];
    int faceIndex = 0;
    for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
         iter != faceArray.end(); ++iter, ++faceIndex) {
      const ZIntCuboidFace &face = *iter;
      if (face.contains(box.cb[0], box.cb[1], box.cb[2]) ||
          face.contains(box.ce[0], box.cb[1], box.cb[2]) ||
          face.contains(box.cb[0], box.ce[1], box.cb[2]) ||
          face.contains(box.ce[0], box.ce[1], box.cb[2]) ||
          face.contains(box.cb[0], box.cb[1], box.ce[2]) ||
          face.contains(box.ce[0], box.cb[1], box.ce[2]) ||
          face.contains(box.cb[0], box.ce[1], box.ce[2]) ||
          face.contains(box.ce[0], box.ce[1], box.ce[2])) {
        m_faceIndexArray[bodyIndex].push_back(FaceIndex(cuboidIndex, faceIndex));
#ifdef _DEBUG_
        std::cout << "Body-face: " << cuboidIndex << " " << faceIndex << std::endl;
#endif
      }
    }
  }
}

void FlyEm::Service::FaceOrphanOverlap::setCoordinateConverter(
    const ZFlyEmCoordinateConverter &converter)
{
  m_coordConverter = converter;
}

void FlyEm::Service::FaceOrphanOverlap::exportJsonFile(
    const std::string filePath) const
{
  ZJsonObject obj(json_object(), true);
  ZJsonArray nodeArray(json_array(), true);
  ZJsonArray edgeArray(json_array(), true);

  obj.setEntry("Bodies", nodeArray);
  obj.setEntry("Overlap", edgeArray);

  std::set<int> bodySet;
  for (int i = 0; i < m_overlapGraph.getEdgeNumber(); ++i) {
    bodySet.insert(m_overlapGraph.getEdgeBegin(i));
    bodySet.insert(m_overlapGraph.getEdgeEnd(i));
  }

  ZDvidReader reader;
  reader.open(m_dvidTarget);

  //node property: id, size, #synapses,
  size_t bodyCount = bodySet.size();
  size_t currentIndex = 0;
  for (std::set<int>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter, ++currentIndex) {
    std::cout << currentIndex << "/" << bodyCount << std::endl;

    int bodyId = *iter;

    ZJsonObject obj(json_object(), true);
    json_t *value = json_integer(bodyId);
    obj.consumeEntry("id", value);

    ZObject3dScan body = reader.readBody(bodyId);

    value = json_integer(body.getVoxelNumber());
    obj.consumeEntry("size", value);

    int synapseCount = 0;
    if (bodyId < (int) m_synapseCount.size()) {
      synapseCount = m_synapseCount[bodyId];
    }
    value = json_integer(synapseCount);
    obj.consumeEntry("num_synapses", value);

    ZJsonArray pt;
    ZVoxel voxel = body.getMarker();
    double x = voxel.x();
    double y = voxel.y();
    double z = voxel.z();
    m_coordConverter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::IMAGE_SPACE,
                             ZFlyEmCoordinateConverter::RAVELER_SPACE);

    pt.append(iround(x));
    pt.append(iround(y));
    pt.append(iround(z));
    obj.setEntry("marker", pt);

    ZIntCuboid box = body.getBoundBox();
    bool isOrphan = true;
    int hitIndex1 = m_cuboidArray.hitTest(
          box.getFirstCorner().getX(), box.getFirstCorner().getY(),
          box.getFirstCorner().getZ());
    int hitIndex2 = m_cuboidArray.hitTest(
          box.getLastCorner().getX(), box.getLastCorner().getY(),
          box.getLastCorner().getZ());
    if ( hitIndex1 < 0 || hitIndex2 < 0 || hitIndex1 != hitIndex2) {
      isOrphan = false;
    }

    if (isOrphan) {
      obj.consumeEntry("cross_substack", json_false());
    } else {
      obj.consumeEntry("cross_substack", json_true());
    }

    nodeArray.append(obj);
  }

  //edge property: id1, id2, touching point
  for (int i = 0; i < m_overlapGraph.getEdgeNumber(); ++i) {
    ZJsonObject obj(json_object(), true);
    json_t *value = json_integer(m_overlapGraph.getEdgeBegin(i));
    obj.consumeEntry("id1", value);
    value = json_integer(m_overlapGraph.getEdgeEnd(i));
    obj.consumeEntry("id2", value);
    ZJsonArray pt;
    double x = m_marker[i].getX();
    double y = m_marker[i].getY();
    double z = m_marker[i].getZ();
    m_coordConverter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::IMAGE_SPACE,
                             ZFlyEmCoordinateConverter::RAVELER_SPACE);
    pt.append(iround(x));
    pt.append(iround(y));
    pt.append(iround(z));
    obj.setEntry("marker", pt);
    edgeArray.append(obj);
  }

  //std::cout << obj.dumpString() << std::endl;
  obj.dump(filePath);
}
