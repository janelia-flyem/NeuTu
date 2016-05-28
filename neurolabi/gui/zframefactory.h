#ifndef ZFRAMEFACTORY_H
#define ZFRAMEFACTORY_H

#include <string>
#include <QString>

#include "neutube.h"

class ZFlyEmDataFrame;
class ZStackFrame;
class ZStackDocReader;

class ZFrameFactory
{
public:
  ZFrameFactory();
#if defined (_FLYEM_)
  static ZFlyEmDataFrame* MakeFlyEmDataFrame(const std::string &bundlePath);
  static ZFlyEmDataFrame* MakeFlyEmDataFrame(const QString &bundlePath);
#endif
  static ZStackFrame* MakeStackFrame(
      ZStackDocReader &reader,
      NeuTube::Document::ETag tag = NeuTube::Document::NORMAL,
      ZStackFrame *parentFrame = NULL);
  static ZStackFrame* MakeStackFrame(
      NeuTube::Document::ETag tag = NeuTube::Document::NORMAL,
      ZStackFrame *parentFrame = NULL);

};

#endif // ZFRAMEFACTORY_H
