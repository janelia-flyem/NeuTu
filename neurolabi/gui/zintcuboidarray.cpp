#include "zintcuboidarray.h"

#include <iostream>
#include <math.h>
#include <algorithm>
#include "zstring.h"
#include "zcuboid.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

using namespace std;

ZIntCuboidArray::ZIntCuboidArray()
{
}

void ZIntCuboidArray::append(
    int x, int y, int z, int width, int height, int depth)
{
  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, x, y, z, width, height, depth);
  push_back(cuboid);
#ifdef _DEBUG_2
  std::cout << depth << std::endl;
  Print_Cuboid_I(&cuboid);
#endif

  deprecate(ALL_COMPONENT);
}

int ZIntCuboidArray::hitTest(const ZPoint &pt) const
{
  return hitTest(pt.x(), pt.y(), pt.z());
}

int ZIntCuboidArray::hitTest(double x, double y, double z) const
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  for (size_t i = 0; i < size(); ++i) {
    if (Cuboid_I_Hit(&((*this)[i]), ix, iy, iz)) {
#ifdef _DEBUG_2
      std::cout << ix << ' ' << iy << ' ' << iz << std::endl;
      Print_Cuboid_I(&((*this)[i]));
#endif
      return i;
    }
  }

  return -1;
}

int ZIntCuboidArray::hitInternalTest(double x, double y, double z) const
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  for (size_t i = 0; i < size(); ++i) {
    if (Cuboid_I_Hit_Internal(&((*this)[i]), ix, iy, iz)) {
#ifdef _DEBUG_2
      std::cout << ix << ' ' << iy << ' ' << iz << std::endl;
      Print_Cuboid_I(&((*this)[i]));
#endif
      return i;
    }
  }

  return -1;
}

std::vector<int> ZIntCuboidArray::loadSubstackList(
    const std::string filePath)
{
  std::vector<int> stackIdArray;

  clear();

  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    while (str.readLine(fp)) {
      std::vector<int> valueArray = str.toIntegerArray();
      if (valueArray.size() == 7) {
        int id = valueArray[0];
        stackIdArray.push_back(id);

        append(valueArray[1], valueArray[3], valueArray[5],
            abs(valueArray[2]) - valueArray[1] + 1,
            abs(valueArray[4]) - valueArray[3] + 1,
            abs(valueArray[6]) - valueArray[5] + 1);
      }
    }
  } else {
    cerr << "Cannot open " << filePath << endl;
  }

  deprecate(ALL_COMPONENT);

  return stackIdArray;
}

void ZIntCuboidArray::translate(int x, int y, int z)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    iter->cb[0] += x;
    iter->ce[0] += x;
    iter->cb[1] += y;
    iter->ce[1] += y;
    iter->cb[2] += z;
    iter->ce[2] += z;
  }

  deprecate(ALL_COMPONENT);
}

void ZIntCuboidArray::rescale(double factor)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    iter->cb[0] *= factor;
    iter->ce[0] *= factor;
    iter->cb[1] *= factor;
    iter->ce[1] *= factor;
    iter->cb[2] *= factor;
    iter->ce[2] *= factor;
  }

  deprecate(ALL_COMPONENT);
}

ZSwcTree* ZIntCuboidArray::toSwc() const
{
  ZSwcTree *tree = NULL;
  if (!empty()) {
    tree = new ZSwcTree;
    int index = 0;
    for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
         ++iter, ++index) {
      ZCuboid cuboid;
      cuboid.set(iter->cb[0], iter->cb[1], iter->cb[2], iter->ce[0], iter->ce[1],
          iter->ce[2]);
      ZSwcTree *subtree = ZSwcTree::CreateCuboidSwc(cuboid);
      subtree->setType(index);
      tree->merge(subtree, true);
    }

    tree->resortId();
  }

  return tree;
}

void ZIntCuboidArray::exportSwc(const string &filePath) const
{
  if (!empty()) {
    ZSwcTree *tree = new ZSwcTree;
    int index = 0;
    for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
         ++iter, ++index) {
      ZCuboid cuboid;
      cuboid.set(iter->cb[0], iter->cb[1], iter->cb[2], iter->ce[0], iter->ce[1],
          iter->ce[2]);
      ZSwcTree *subtree = ZSwcTree::CreateCuboidSwc(cuboid);
      subtree->setType(index);
      tree->merge(subtree, true);
    }

    tree->resortId();
    tree->save(filePath);

    delete tree;
  }
}

bool ZIntCuboidArray::isInvalid(const Cuboid_I &cuboid)
{
  return !Cuboid_I_Is_Valid(&cuboid);
}

void ZIntCuboidArray::removeInvalidCuboid()
{
  iterator newEnd = std::remove_if(begin(), end(), isInvalid);
  if (newEnd != end()) {
    resize(newEnd - begin());
  }
}

void ZIntCuboidArray::intersect(const Cuboid_I &cuboid)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    Cuboid_I_Intersect(&(*iter), &cuboid, &(*iter));
  }

  removeInvalidCuboid();

  deprecate(ALL_COMPONENT);
}


Cuboid_I ZIntCuboidArray::getBoundBox() const
{
  Cuboid_I box;
  Cuboid_I_Set_S(&box, 0, 0, 0, 0, 0, 0);

#ifdef _DEBUG_2
  Print_Cuboid_I(&box);
#endif

  if (!empty()) {
    ZIntCuboidArray::const_iterator iter = begin();
    box = *iter;
    for (++iter; iter != end(); ++iter) {
      Cuboid_I_Union(&(*iter), &box, &box);
#ifdef _DEBUG_2
  Print_Cuboid_I(&box);
#endif
    }
  }

  return box;
}

ZIntCuboidArray ZIntCuboidArray::getFace() const
{
  ZIntCuboidArray face;

  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end(); ++iter) {
    if (Cuboid_I_Is_Valid(&(*iter))) {
      for (int i = 0; i < 3; ++i) {
        Cuboid_I tmpFace = *iter;
        tmpFace.ce[i] = tmpFace.cb[i];
        face.push_back(tmpFace);
        tmpFace = *iter;
        tmpFace.cb[i] = tmpFace.ce[i];
        face.push_back(tmpFace);
      }
    }
  }

  return face;
}

ZIntCuboidArray ZIntCuboidArray::getInnerFace() const
{
  ZIntCuboidArray face;

  int index1 = 0;

  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter, ++index1) {
    Cuboid_I box1 = *iter;
    Cuboid_I_Expand_X(&box1, 1);
    Cuboid_I_Expand_Y(&box1, 1);
    Cuboid_I_Expand_Z(&box1, 1);

    int index2 = 0;
    for (ZIntCuboidArray::const_iterator iter2 = begin(); iter2 != end();
         ++iter2, ++index2) {
      if (index1 != index2) {
        Cuboid_I box2 = *iter2;
#ifdef _DEBUG_2
        std::cout << "Intersecting " << std::endl;
          Print_Cuboid_I(&box1);
          Print_Cuboid_I(&box2);
#endif
        Cuboid_I_Intersect(&box1, &box2, &box2);

        int width, height, depth;
        Cuboid_I_Size(&box2, &width, &height, &depth);
        if (width > 0 && height > 0 && depth > 0 &&
            (width > 1) + (height > 1) + (depth > 1) > 1) {
          face.push_back(box2);
#ifdef _DEBUG_2
          Print_Cuboid_I(&box2);
#endif
        }
      }
    }
  }

  return face;
}

void ZIntCuboidArray::print() const
{
  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    cout << "(" << iter->cb[0] << ", " << iter->cb[1] << ", " << iter->cb[2]
         << ") -> (" << iter->ce[0] << ", " << iter->ce[1] << ", "
         << iter->ce[2] << ")" << endl;
  }
}

size_t ZIntCuboidArray::getVolume() const
{
  size_t volume = 0;
  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    volume += Cuboid_I_Volume(&(*iter));
  }

  return volume;
}

ZIntCuboidFaceArray ZIntCuboidArray::getBorderFace() const
{
  if (isDeprecated(BORDER_FACE)) {
    ZIntCuboidFaceArray faceArray;
    for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      faceArray.append(&(*iter));
    }

    ZIntCuboidFaceArray cropFaceArray = faceArray;
    cropFaceArray.moveBackward(1);

    m_borderFace = faceArray.cropBy(cropFaceArray);
  }

  return m_borderFace;
}

ZIntCuboidFaceArray ZIntCuboidArray::getSideBorderFace() const
{
  ZIntCuboidFaceArray borderFaceArray = getBorderFace();

  ZIntCuboidFaceArray sideBorderFaceArray;
  for (ZIntCuboidFaceArray::const_iterator iter = borderFaceArray.begin();
       iter != borderFaceArray.end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    if (face.getAxis() != NeuTube::Z_AXIS) {
      sideBorderFaceArray.append(face);
    }
  }

  return sideBorderFaceArray;
}


void ZIntCuboidArray::deprecate(EComponent component) const
{
  deprecateDependent(component);

  switch (component) {
  case BORDER_FACE:
    m_borderFace.clear();
    break;
  case ALL_COMPONENT:
    break;
  }
}

void ZIntCuboidArray::deprecateDependent(EComponent component) const
{
  switch (component) {
  case ALL_COMPONENT:
    deprecate(BORDER_FACE);
    break;
  case BORDER_FACE:
    break;
  }
}

bool ZIntCuboidArray::isDeprecated(EComponent component) const
{
  switch (component) {
  case ALL_COMPONENT: //Is any of the component deprecated?
    return isDeprecated(BORDER_FACE);
  case BORDER_FACE:
    return m_borderFace.empty();
  }

  return false;
}

FlyEm::ZIntCuboidCutter::ZIntCuboidCutter()
{
  Cuboid_I_Set_S(&m_cuboid, 0, 0, 0, 0, 0, 0);
}

bool FlyEm::ZIntCuboidCutter::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey("start") && obj.hasKey("size")) {
    ZJsonArray start(obj["start"], false);
    ZJsonArray size(obj["size"], false);

    std::vector<int> startCoord = start.toIntegerArray();
    std::vector<int> blockSize = size.toIntegerArray();

    if (start.size() == 3 && size.size() == 3) {
      Cuboid_I_Set_S(&m_cuboid, startCoord[0], startCoord[1], startCoord[2],
          blockSize[0], blockSize[1], blockSize[2]);
      return true;
    }
  }

  return false;
}

void FlyEm::ZIntCuboidCutter::cut(Cuboid_I *cuboid)
{
  if (cuboid == NULL) {
    return;
  }

  if (Cuboid_I_Is_Valid(&m_cuboid)) {
    Cuboid_I tmpCuboid;
    tmpCuboid = m_cuboid;
    for (int i = 0; i < 3; ++i) {
      tmpCuboid.cb[i] += cuboid->cb[i];
      tmpCuboid.ce[i] += cuboid->ce[i];
    }
    Cuboid_I_Intersect(&tmpCuboid, cuboid, &tmpCuboid);
    if (Cuboid_I_Is_Valid(&tmpCuboid)) {
      *cuboid = tmpCuboid;
    }
  }
}

FlyEm::SubstackRegionCalbration::SubstackRegionCalbration()
{
  for (int i = 0; i < 3; i++) {
    m_margin[i] = 0;
    m_bounding[i] = true;
  }
}

void FlyEm::SubstackRegionCalbration::setMargin(
    int x, int y, int z)
{
  m_margin[0] = x;
  m_margin[1] = y;
  m_margin[2] = z;
}

void FlyEm::SubstackRegionCalbration::setBounding(
    bool x, bool y, bool z)
{
  m_bounding[0] = x;
  m_bounding[1] = y;
  m_bounding[2] = z;
}

bool FlyEm::SubstackRegionCalbration::importJsonObject(const ZJsonObject &obj)
{
  std::vector<int> margin = ZJsonArray(obj["margin"], false).toIntegerArray();
  std::vector<bool> bounding = ZJsonArray(obj["bounding"], false).toBoolArray();

  if (margin.size() == 3 && bounding.size() == 3) {
    for (int i = 0; i < 3; ++i) {
      m_margin[i] = margin[i];
      m_bounding[i] = bounding[i];
    }
    return true;
  }

  return false;
}

void FlyEm::SubstackRegionCalbration::calibrate(
    ZIntCuboidArray &roi) const
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

void FlyEm::ZSubstackRoi::clear()
{
  m_idArray.clear();
  m_cuboidArray.clear();
}

const char* FlyEm::ZSubstackRoi::m_blockFileKey = "block_file";
const char* FlyEm::ZSubstackRoi::m_calbrationKey = "calibration";
const char* FlyEm::ZSubstackRoi::m_cutterKey = "cutter";


void FlyEm::ZSubstackRoi::importJsonFile(const std::string &filePath)
{
  clear();

  ZJsonObject obj;
  obj.load(filePath);

  std::string blockFile = ZJsonParser::stringValue(obj[m_blockFileKey]);
  if (!blockFile.empty()) {
    m_idArray = m_cuboidArray.loadSubstackList(blockFile);
  }

  FlyEm::SubstackRegionCalbration calbr;
  calbr.importJsonObject(ZJsonObject(obj[m_calbrationKey], false));
  calbr.calibrate(m_cuboidArray);

  if (obj.hasKey(m_cutterKey)) {
    ZJsonArray cutterArray(obj[m_cutterKey], false);
    for (size_t i = 0; i < cutterArray.size(); ++i) {
      ZJsonObject cutterObj(cutterArray.at(i), false);
      if (!cutterObj.isEmpty()) {
        ZIntCuboidCutter cutter;
        int id = ZJsonParser::integerValue(cutterObj["id"]);
        cutter.loadJsonObject(cutterObj);
        cutter.cut(getCuboidFromId(id));
      }
    }
  }
}

Cuboid_I* FlyEm::ZSubstackRoi::getCuboidFromId(int id)
{
  Cuboid_I *cuboid = NULL;
  for (size_t i = 0; i < m_idArray.size(); ++i) {
    if (m_idArray[i] == id) {
      cuboid = &(m_cuboidArray[i]);
    }
  }

  return cuboid;
}

void FlyEm::ZSubstackRoi::exportSwc(const string &filePath)
{
  if (!m_cuboidArray.empty()) {
    ZSwcTree *tree = new ZSwcTree;
    int index = 0;
    for (ZIntCuboidArray::const_iterator iter = m_cuboidArray.begin();
         iter != m_cuboidArray.end(); ++iter, ++index) {
      ZCuboid cuboid;
      cuboid.set(iter->cb[0], iter->cb[1], iter->cb[2], iter->ce[0], iter->ce[1],
          iter->ce[2]);
      ZSwcTree *subtree = ZSwcTree::CreateCuboidSwc(cuboid);
      if (!m_idArray.empty()) {
        subtree->setType(m_idArray[index]);
      } else {
        subtree->setType(index);
      }
      tree->merge(subtree, true);
    }

    tree->resortId();
    tree->save(filePath);

    delete tree;
  }
}
