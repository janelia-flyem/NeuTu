#ifndef FLYEMTODOBLOCKGRID_H
#define FLYEMTODOBLOCKGRID_H

#include <unordered_map>
#include <memory>
#include <mutex>

#include "bigdata/zblockgrid.h"

#include "zflyemtodoitem.h"
#include "flyemtodochunk.h"

class FlyEmTodoSource;
class ZAffineRect;

class FlyEmTodoBlockGrid : public ZBlockGrid
{
public:
  FlyEmTodoBlockGrid();

  FlyEmTodoChunk getTodoChunk(int i, int j, int k) const;

  void setSource(std::shared_ptr<FlyEmTodoSource> source);

  std::vector<ZFlyEmToDoItem> getIntersectTodoList(
      const ZAffineRect &plane) const;

private:
  static std::string GetChunkKey(int i, int j, int k);

private:
  mutable std::mutex m_dataMutex;
  mutable std::unordered_map<std::string, FlyEmTodoChunk> m_chunkMap;
  std::shared_ptr<FlyEmTodoSource> m_source;
};

#endif // FLYEMTODOBLOCKGRID_H
