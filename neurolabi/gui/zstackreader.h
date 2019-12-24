#ifndef ZSTACKREADER_H
#define ZSTACKREADER_H

#include <string>

class ZStack;
class QUrl;
class ZIntCuboid;
class ZJsonObject;

class ZStackReader
{
public:
  ZStackReader();

public:
  static ZStack* Read(const std::string &path);

private:
  static ZStack* ReadDvid(const QUrl &url);
  static ZStack* ReadSeries(const QUrl &url);
  static ZStack* ReadJson(const ZJsonObject &obj);

private:
  static ZIntCuboid GetRange(const QUrl &url);
};

#endif // ZSTACKREADER_H
