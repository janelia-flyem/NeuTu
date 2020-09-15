#ifndef ZSTACKSOURCE_H
#define ZSTACKSOURCE_H

#include <memory>

class ZStack;
class ZIntCuboid;

class ZStackSource
{
public:
  ZStackSource();
  virtual ~ZStackSource() {}

  virtual int getMaxZoom() const = 0;
  virtual std::shared_ptr<ZStack> getStack(
      const ZIntCuboid &box, int zoom) const = 0;

};

#endif // ZSTACKSOURCE_H
