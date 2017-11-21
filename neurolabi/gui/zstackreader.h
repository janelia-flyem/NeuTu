#ifndef ZSTACKREADER_H
#define ZSTACKREADER_H

#include <string>

class ZStack;
class QUrl;
class ZIntCuboid;

class ZStackReader
{
public:
  ZStackReader();

public:
  ZStack* read(const std::string &path);

private:
  static ZIntCuboid GetRange(const QUrl &url);
};

#endif // ZSTACKREADER_H
