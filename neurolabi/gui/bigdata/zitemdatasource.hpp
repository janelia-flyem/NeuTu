#ifndef ZITEMDATASOURCE_HPP
#define ZITEMDATASOURCE_HPP

#include <vector>

#include "geometry/zintpoint.h"

class ZIntCuboid;

template<typename TItem, typename TKey>
class ZItemDataSource
{
public:
  ZItemDataSource() {}
  virtual ~ZItemDataSource() {}

  using KeyType = TKey;

  virtual std::vector<TItem> getData(const ZIntCuboid &box) const = 0;
  virtual ZIntCuboid getRange() const = 0;
  virtual ZIntPoint getBlockSize() const
  {
    return ZIntPoint(32, 32, 32);
  }

  virtual void saveItem(const TItem &item) = 0;
  virtual void removeItem(const TKey &pos) = 0;
  virtual TItem getItem(const TKey &pos) const = 0;
};

#endif // ZITEMDATASOURCE_HPP
