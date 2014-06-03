#include "zobject3dscanarray.h"
#include "zfilelist.h"
#include "zstack.hxx"

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
