#ifndef FLYEMTODOSOURCE_H
#define FLYEMTODOSOURCE_H

//#include <vector>
//#include <functional>
//#include <string>

#include "bigdata/zintpointannotationsource.hpp"

#include "zflyemtodoitem.h"

class FlyEmTodoSource : public ZIntPointAnnotationSource<ZFlyEmToDoItem>
{
public:
  FlyEmTodoSource();

  ZIntPoint getBlockSize() const override;

//  virtual std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const = 0;
//  virtual ZIntCuboid getRange() const = 0;
//  virtual ZIntPoint getBlockSize() const;

//  virtual void saveItem(const ZFlyEmToDoItem &item) = 0;
//  virtual void removeItem(const ZIntPoint &pos) = 0;
//  void removeItem(int x, int y, int z);

};

#endif // FLYEMTODOSOURCE_H
