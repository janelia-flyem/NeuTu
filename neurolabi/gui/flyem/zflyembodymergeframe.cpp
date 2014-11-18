#include "zflyembodymergeframe.h"
#include "flyem/zflyembodymergedoc.h"

ZFlyEmBodyMergeFrame::ZFlyEmBodyMergeFrame(QWidget *parent) :
  ZStackFrame(parent, false)
{
  constructFrame();
}

void ZFlyEmBodyMergeFrame::createDocument()
{
  setDocument(ZSharedPointer<ZStackDoc>(new ZFlyEmBodyMergeDoc(NULL, NULL)));
}


ZFlyEmBodyMergeDoc* ZFlyEmBodyMergeFrame::getCompleteDocument()
{
  return dynamic_cast<ZFlyEmBodyMergeDoc*>(document().get());
}
