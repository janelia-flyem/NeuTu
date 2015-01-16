#include "zframefactory.h"
#include "flyem/zflyemdataframe.h"
#include "zstackdocreader.h"
#include "zstackframe.h"
#include "zstackdocreader.h"
#include "flyem/zflyembodymergeframe.h"

ZFrameFactory::ZFrameFactory()
{
}

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

ZStackFrame*
ZFrameFactory::MakeStackFrame(ZStackDocReader &reader,
    NeuTube::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
  case NeuTube::Document::FLYEM_MERGE:
    frame = new ZFlyEmBodyMergeFrame;
    frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
    break;
  default:
    frame = new ZStackFrame;
    break;
  }

  frame->setParentFrame(parentFrame);
  frame->document()->setTag(tag);
  frame->document()->addData(reader);
  if (parentFrame != NULL) {
    frame->document()->setStackBackground(
          parentFrame->document()->getStackBackground());
  }

  return frame;
}

ZStackFrame*
ZFrameFactory::MakeStackFrame(
    NeuTube::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
  case NeuTube::Document::FLYEM_MERGE:
    frame = new ZFlyEmBodyMergeFrame;
    frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
    break;
  default:
    frame = new ZStackFrame;
    break;
  }

  frame->setParentFrame(parentFrame);
  frame->document()->setTag(tag);
  if (parentFrame != NULL) {
    frame->document()->setStackBackground(
          parentFrame->document()->getStackBackground());
  }

  return frame;
}
