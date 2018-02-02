#include "zobject3dscanarray.h"
#include "zfilelist.h"
#include "zstack.hxx"
#include "zintcuboid.h"
#include "zstackfactory.h"

ZObject3dScanArray::ZObject3dScanArray()
{
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

#ifdef _DEBUG_
  std::cout << cuboid.toJsonArray().dumpString() << std::endl;
#endif

  return cuboid;
}

ZStack* ZObject3dScanArray::toLabelField() const
{
  ZStack *stack = NULL;

  if (!empty()) {
    ZIntCuboid cuboid = getBoundBox();
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

ZStack* ZObject3dScanArray::toColorField() const
{
  ZStack *stack = NULL;

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
