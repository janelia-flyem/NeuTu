#ifndef ZFRAMEFACTORY_H
#define ZFRAMEFACTORY_H

#include <string>
#include <QString>

class ZFlyEmDataFrame;

class ZFrameFactory
{
public:
  ZFrameFactory();

  static ZFlyEmDataFrame* MakeFlyEmDataFrame(const std::string &bundlePath);
  static ZFlyEmDataFrame* MakeFlyEmDataFrame(const QString &bundlePath);
};

#endif // ZFRAMEFACTORY_H
