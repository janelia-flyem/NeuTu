#ifndef ZINTPOINTANNOTATIONSOURCE_H
#define ZINTPOINTANNOTATIONSOURCE_H

#include "zchunkdatasource.hpp"

template <typename T>
class ZIntPointAnnotationSource : public ZChunkDataSource<T, ZIntPoint>
{
public:
  virtual void updatePartner(T */*item*/) const {}
  virtual void updateItem(T */*item*/) const {}
  virtual void moveItem(const ZIntPoint &from, const ZIntPoint &to) = 0;
};

#endif // ZINTPOINTANNOTATIONSOURCE_H
