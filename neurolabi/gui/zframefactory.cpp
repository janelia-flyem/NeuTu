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
    neutu::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
  case neutu::Document::ETag::FLYEM_MERGE:
    frame = ZFlyEmBodyMergeFrame::Make(NULL);
    frame->document()->setStackBackground(neutu::EImageBackground::BRIGHT);
    frame->setObjectDisplayStyle(ZStackObject::EDisplayStyle::SOLID);
    break;
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
    neutu::Document::ETag tag, ZStackFrame *parentFrame)
{
  ZStackFrame *frame = NULL;
  switch (tag) {
  case neutu::Document::ETag::FLYEM_MERGE:
    frame = ZFlyEmBodyMergeFrame::Make(NULL);
    frame->document()->setStackBackground(neutu::EImageBackground::BRIGHT);
    break;
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
