#include "zframefactory.h"

#include "zstackdocreader.h"
#include "zstackframe.h"
#include "zstackdocreader.h"
#if defined (_FLYEM_)
#include "flyem/zflyembodymergeframe.h"
#include "flyem/zflyemdataframe.h"
#endif
ZFrameFactory::ZFrameFactory()
{
}
#if defined (_FLYEM_)
ZFlyEmDataFrame*
ZFrameFactory::MakeFlyEmDataFrame(const std::string &bundlePath)
{
  ZFlyEmDataFrame *frame = new ZFlyEmDataFrame;
  if (!frame->load(bundlePath)) {
    delete frame;
    frame = NULL;
  }

  return frame;
}

ZFlyEmDataFrame*
ZFrameFactory::MakeFlyEmDataFrame(const QString &bundlePath)
{
  return MakeFlyEmDataFrame(bundlePath.toStdString());
}
#endif
ZStackFrame*
ZFrameFactory::MakeStackFrame(ZStackDocReader &reader,
    NeuTube::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
#if defined (_FLYEM_)
  case NeuTube::Document::FLYEM_MERGE:
    frame = ZFlyEmBodyMergeFrame::Make(NULL);
    frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
    frame->setObjectDisplayStyle(ZStackObject::SOLID);
    break;
#endif
  default:
    frame = ZStackFrame::Make(NULL);
    break;
  }

  frame->setParentFrame(parentFrame);
  frame->document()->setTag(tag);
  frame->document()->addData(reader);
  if (parentFrame != NULL) {
    frame->document()->setStackBackground(
          parentFrame->document()->getStackBackground());
  }
  frame->customizeWidget();

  return frame;
}

ZStackFrame*
ZFrameFactory::MakeStackFrame(
    NeuTube::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
#if defined (_FLYEM_)
  case NeuTube::Document::FLYEM_MERGE:
    frame = ZFlyEmBodyMergeFrame::Make(NULL);
    frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
    break;
#endif
  default:
    frame = ZStackFrame::Make(NULL);
    break;
  }

  frame->setParentFrame(parentFrame);
  frame->document()->setTag(tag);
  if (parentFrame != NULL) {
    frame->document()->setStackBackground(
          parentFrame->document()->getStackBackground());
  }

  frame->enableMessageManager();

  return frame;
}
