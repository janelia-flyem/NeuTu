#include "zobject3dscanarray.h"
#include "zfilelist.h"

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
