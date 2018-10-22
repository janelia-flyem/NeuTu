#include "zobject3dscanarray.h"
#include "zfilelist.h"
#include "zstack.hxx"
#include "zintcuboid.h"
#include "zstackfactory.h"
#include "misc/miscutility.h"

ZObject3dScanArray::ZObject3dScanArray()
{
}

ZObject3dScanArray::ZObject3dScanArray(const std::vector<ZObject3dScan *> &objArray)
{
  insert(end(), objArray.begin(), objArray.end());
}

ZObject3dScanArray::~ZObject3dScanArray()
{
  clearAll();
}

void ZObject3dScanArray::clearAll()
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }

  clear();
}

void ZObject3dScanArray::shallowClear()
{
  clear();
}

void ZObject3dScanArray::append(ZObject3dScan *obj)
{
  if (obj != NULL) {
    push_back(obj);
  }
}

void ZObject3dScanArray::append(const ZObject3dScan &obj)
{
  ZObject3dScan *newObj = new ZObject3dScan(obj);
  append(newObj);
}

void ZObject3dScanArray::importDir(const std::string &dirPath)
{
  clearAll();

  ZFileList fileList;
  fileList.load(dirPath, "sobj");
  resize(fileList.size());
  for (int i = 0; i < fileList.size(); ++i) {
    ZObject3dScan &obj = *((*this)[i]);
    obj.load(fileList.getFilePath(i));
  }
}

ZStack* ZObject3dScanArray::toStackObject() const
{
  ZStack *stack = NULL;

  if (!empty()) {
    int offset[3] = { 0, 0, 0 };
    Stack *rawStack = ZObject3dScan::makeStack(begin(), end(), offset);
    if (rawStack != NULL) {
      stack = new ZStack;
      stack->load(rawStack);
      stack->setOffset(offset[0], offset[1], offset[2]);
    }
  }

  return stack;
}

ZIntCuboid ZObject3dScanArray::getBoundBox() const
{
  ZIntCuboid cuboid;

  ZObject3dScanArray::const_iterator iter = begin();
  if (iter != end()) {
    const ZObject3dScan *obj = *iter;
    cuboid = obj->getBoundBox();
    ++iter;
  }

  for (; iter != end(); ++iter) {
    const ZObject3dScan &obj = **iter;
    cuboid.join(obj.getBoundBox());
  }

#ifdef _DEBUG_2
  std::cout << cuboid.toJsonArray().dumpString() << std::endl;
#endif

  return cuboid;
}

ZStack* ZObject3dScanArray::toLabelField() const
{
  ZStack *stack = NULL;

  if (!empty()) {
    ZIntCuboid cuboid = getBoundBox();

    int dsIntv = misc::getIsoDsIntvFor3DVolume(cuboid, neutube::ONEGIGA, false);

    if (dsIntv == 0) {
      stack = ZStackFactory::MakeZeroStack(GREY, cuboid);
      int offset[3];
      offset[0] = -stack->getOffset().getX();
      offset[1] = -stack->getOffset().getY();
      offset[2] = -stack->getOffset().getZ();
      int label = 1;
      for (ZObject3dScanArray::const_iterator iter = begin(); iter != end();
           ++iter) {
        const ZObject3dScan *obj = *iter;
        obj->drawStack(stack->c_stack(), label++, offset);
      }
    } else {
      ZObject3dScanArray dsObjArray = makeDownsample(dsIntv, dsIntv, dsIntv);
      stack = dsObjArray.toLabelField();
    }
  }

  return stack;
}

ZStack* ZObject3dScanArray::toColorField() const
{
  ZStack *stack = NULL;
#ifdef _QT_GUI_USED_
  if (!empty()) {
    ZIntCuboid cuboid = getBoundBox();
    stack = ZStackFactory::MakeZeroStack(COLOR, cuboid);
//    int offset[3];
//    offset[0] = -stack->getOffset().getX();
//    offset[1] = -stack->getOffset().getY();
//    offset[2] = -stack->getOffset().getZ();
//    int label = 1;
    for (ZObject3dScanArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      const ZObject3dScan *obj = *iter;
      obj->drawStack(
            stack, obj->getColor().red(), obj->getColor().green(), obj->getColor().blue());
//      obj->drawStack(stack->c_stack(), label++, offset);
    }
  }
#endif

  return stack;
}

ZStack* ZObject3dScanArray::toLabelField(const ZIntCuboid &box) const
{
  ZStack *stack = NULL;

  if (!empty()) {
    ZIntCuboid cuboid = getBoundBox();
    cuboid.intersect(box);
    stack = ZStackFactory::MakeZeroStack(GREY, cuboid);
    int offset[3];
    offset[0] = -stack->getOffset().getX();
    offset[1] = -stack->getOffset().getY();
    offset[2] = -stack->getOffset().getZ();
    int label = 1;
    for (ZObject3dScanArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      const ZObject3dScan *obj = *iter;
      obj->drawStack(stack->c_stack(), label++, offset);
    }
  }

  return stack;
}

size_t ZObject3dScanArray::getVoxelNumber() const
{
  size_t v = 0;
  for (ZObject3dScanArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZObject3dScan *obj = *iter;
    v += obj->getVoxelNumber();
  }

  return v;
}

void ZObject3dScanArray::downsample(int xintv, int yintv, int zintv)
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    ZObject3dScan *obj = *iter;
    obj->downsampleMax(xintv, yintv, zintv);
  }
}

ZObject3dScanArray ZObject3dScanArray::makeDownsample(
    int xintv, int yintv, int zintv) const
{
  ZObject3dScanArray result;
  for (ZObject3dScanArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZObject3dScan *obj = *iter;
    ZObject3dScan *dsObj = new ZObject3dScan(*obj);
    dsObj->downsampleMax(xintv, yintv, zintv);
    result.append(dsObj);
  }

  return result;
}

void ZObject3dScanArray::upsample(int xintv, int yintv, int zintv)
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    ZObject3dScan *obj = *iter;
    obj->upSample(xintv, yintv, zintv);
  }
}

void ZObject3dScanArray::translate(int dx, int dy, int dz)
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    ZObject3dScan *obj = *iter;
    obj->translate(dx, dy, dz);
  }
}

void ZObject3dScanArray::write(std::ostream &stream) const
{
  int count = size();
  int version = 1;
  stream.write((const char*)(&version), sizeof(int));
  stream.write((const char*)(&count), sizeof(int));
  for (const ZObject3dScan *obj : *this) {
    obj->write(stream);
  }
}

void ZObject3dScanArray::read(std::istream &stream)
{
  int version = 0;
  stream.read((char*)(&version), sizeof(int));

  int count = 0;
  stream.read((char*)(&count), sizeof(int));
  clearAll();
  resize(count);
  for (auto &obj : *this) {
    obj = new ZObject3dScan;
    obj->read(stream);
  }
}

void ZObject3dScanArray::load(const std::string &filePath)
{
  clearAll();
  std::ifstream stream(filePath.c_str());
  if (stream.good()) {
    read(stream);
  }
  stream.close();
}

void ZObject3dScanArray::save(const std::string &filePath)
{
  std::ofstream stream(filePath.c_str());
  if (stream.good()) {
    write(stream);
  }
  stream.close();
}
