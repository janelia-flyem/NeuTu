#ifndef FLYEMTODOBLOCKGRID_H
#define FLYEMTODOBLOCKGRID_H

//#include <unordered_map>
//#include <memory>
//#include <mutex>
//#include <functional>

//#include "bigdata/zblockgrid.h"
#include "bigdata/zintpointannotationblockgrid.hpp"

#include "zflyemtodoitem.h"
#include "flyemtodochunk.h"

class FlyEmTodoBlockGrid :
    public ZIntPointAnnotationBlockGrid<ZFlyEmToDoItem, FlyEmTodoChunk>
{

};

#if 0
class FlyEmTodoSource;

class FlyEmTodoBlockGrid : public ZBlockGrid
{
public:
  FlyEmTodoBlockGrid();

  void setSource(std::shared_ptr<FlyEmTodoSource> source);

  void addItem(const ZFlyEmToDoItem &item);
  void removeItem(const ZIntPoint &pos);
  void removeItem(int x, int y, int z);

  ZFlyEmToDoItem getExistingItem(int x, int y, int z) const;
  ZFlyEmToDoItem getExistingItem(const ZIntPoint &pos) const;

  ZFlyEmToDoItem getItem(int x, int y, int z) const;
  ZFlyEmToDoItem getItem(const ZIntPoint &pos) const;

  ZFlyEmToDoItem pickExistingItem(double x, double y, double z) const;

  ZFlyEmToDoItem pickClosestExistingItem(
      double x, double y, double z, double r) const;

  void forEachItemInChunk(
      int i, int j, int k, std::function<void(const ZFlyEmToDoItem &item)> f);

  FlyEmTodoChunk getChunk(int i, int j, int k) const;

  bool setExistingSelection(const ZIntPoint &itemPos, bool selecting);

private:
  static std::string GetChunkKey(int i, int j, int k);
  FlyEmTodoChunk& getHostChunkRef(const ZIntPoint &pos) const;
  FlyEmTodoChunk& getTodoChunkRef(int i, int j, int k) const;
  FlyEmTodoChunk& getTodoChunkRef(const ZIntPoint &pt) const;
//  ZFlyEmToDoItem& getExistingItemRef(int x, int y, int z);

private:
  mutable std::mutex m_chunkMutex;
  mutable std::unordered_map<std::string, FlyEmTodoChunk> m_chunkMap;
  mutable FlyEmTodoChunk m_emptyChunk;
  std::shared_ptr<FlyEmTodoSource> m_source;
};
#endif

#endif // FLYEMTODOBLOCKGRID_H
