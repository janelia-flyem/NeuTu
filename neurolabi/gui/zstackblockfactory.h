#ifndef ZSTACKBLOCKFACTORY_H
#define ZSTACKBLOCKFACTORY_H

#include <vector>

class ZStack;
class ZIntPoint;

class ZStackBlockFactory
{
public:
  ZStackBlockFactory();
  virtual ~ZStackBlockFactory();

public:
  std::vector<ZStack*> make(const ZIntPoint &blockIndex, int n, int zoom) const;
  ZStack* make(const ZIntPoint &blockIndex, int zoom) const;

  virtual int getMaxZoom() const;
  virtual ZIntPoint getBlockSize() const;
  virtual ZIntPoint getGridSize() const;

protected:
  virtual std::vector<ZStack*> makeV(int i, int j, int k, int n, int zoom) const = 0;
};

#endif // ZSTACKBLOCKFACTORY_H
