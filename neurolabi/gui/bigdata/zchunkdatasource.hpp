#ifndef ZCHUNKDATASOURCE_HPP
#define ZCHUNKDATASOURCE_HPP

#include <vector>

#include "geometry/zintpoint.h"

class ZIntCuboid;

template<typename T, typename TKey>
class ZChunkDataSource
{
public:
  ZChunkDataSource() {}
  virtual ~ZChunkDataSource() {}

  using KeyType = TKey;

  virtual std::vector<T> getData(const ZIntCuboid &box) const = 0;
  virtual ZIntCuboid getRange() const = 0;
  virtual ZIntPoint getBlockSize() const
  {
    return ZIntPoint(32, 32, 32);
  }

  virtual void saveItem(const T &item) = 0;
  virtual void removeItem(const TKey &pos) = 0;
//  virtual void updateItem(T *item) const = 0;
  virtual T getItem(const TKey &pos) const = 0;
};

#endif // ZCHUNKDATASOURCE_HPP
