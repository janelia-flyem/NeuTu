#include "zobject3dscanarray.h"
#include "zfilelist.h"
#include "zstack.hxx"
#include "zintcuboid.h"
#include "zstackfactory.h"

ZObject3dScanArray::ZObject3dScanArray()
{
}

void ZObject3dScanArray::importDir(const std::string &dirPath)
{
  clear();

  ZFileList fileList;
  fileList.load(dirPath, "sobj");
  resize(fileList.size());
  for (int i = 0; i < fileList.size(); ++i) {
    ZObject3dScan &obj = (*this)[i];
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
    const ZObject3dScan &obj = *iter;
    cuboid = obj.getBoundBox();
    ++iter;
  }

  for (; iter != end(); ++iter) {
    const ZObject3dScan &obj = *iter;
    cuboid.join(obj.getBoundBox());
  }

  return cuboid;
}

ZStack* ZObject3dScanArray::toLabelField() const
{
  ZStack *stack = NULL;

  if (!empty()) {
    ZIntCuboid cuboid = getBoundBox();
    stack = ZStackFactory::makeZeroStack(GREY, cuboid);
    int offset[3];
    offset[0] = -stack->getOffset().getX();
    offset[1] = -stack->getOffset().getY();
    offset[2] = -stack->getOffset().getZ();
    int label = 1;
    for (ZObject3dScanArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      const ZObject3dScan &obj = *iter;
      obj.drawStack(stack->c_stack(), label++, offset);
    }
  }

  return stack;
}

void ZObject3dScanArray::downsample(int xintv, int yintv, int zintv)
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.downsampleMax(xintv, yintv, zintv);
  }
}

void ZObject3dScanArray::translate(int dx, int dy, int dz)
{
  for (ZObject3dScanArray::iterator iter = begin(); iter != end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.translate(dx, dy, dz);
  }
}
