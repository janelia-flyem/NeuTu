#include "zstackdocutil.h"

#include "common/math.h"
#include "zstackdoc.h"
#include "zstack.hxx"

ZStackDocUtil::ZStackDocUtil()
{

}

std::string ZStackDocUtil::SaveStack(
    const ZStackDoc *doc, const std::string &path)
{
  std::string  resultPath;
  if (doc->hasStackData()) {
    resultPath = doc->getStack()->save(path);
  }

  return resultPath;
}

ZIntCuboid ZStackDocUtil::GetStackSpaceRange(
    const ZStackDoc *doc, neutu::EAxis sliceAxis)
{
  ZIntCuboid box;
  if (doc != NULL) {
    box = GetStackSpaceRange(*doc, sliceAxis);
  }

  return box;
}

ZIntCuboid ZStackDocUtil::GetStackSpaceRange(
    const ZStackDoc &doc, neutu::EAxis sliceAxis)
{
  ZIntCuboid box;

  if (doc.hasStack()) {
    box = doc.getStack()->getBoundBox();
    if (sliceAxis == neutu::EAxis::ARB) {
      ZIntPoint center = box.getCenter();
      int length = neutu::iround(box.getDiagonalLength());
      box.setSize(length, length, length);
      box.setCenter(center);
    } else {
      box.shiftSliceAxis(sliceAxis);
    }
  }

  return box;
}

ZIntCuboid ZStackDocUtil::GetDataSpaceRange(const ZStackDoc *doc)
{
  ZIntCuboid box;
  if (doc != NULL) {
    box = GetDataSpaceRange(*doc);
  }

  return box;
}

ZIntCuboid ZStackDocUtil::GetDataSpaceRange(const ZStackDoc &doc)
{
  ZIntCuboid box;

  if (doc.hasStack()) {
    box = doc.getStack()->getBoundBox();
  }

  return box;
}
