#include "zframefactory.h"
#include "flyem/zflyemdataframe.h"

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
