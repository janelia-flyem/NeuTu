#ifndef FLYEMTODOBLOCKGRID_H
#define FLYEMTODOBLOCKGRID_H

#include <unordered_map>

#include "bigdata/zblockgrid.h"

#include "zflyemtodoitem.h"
#include "flyemtodochunk.h"

class FlyEmTodoBlockGrid : public ZBlockGrid
{
public:
  FlyEmTodoBlockGrid();

  FlyEmTodoChunk getTodoChunk(int i, int j, int k) const;

private:
  static std::string GetChunkKey(int i, int j, int k);

private:
  std::unordered_map<std::string, FlyEmTodoChunk> m_chunkMap;

};

#endif // FLYEMTODOBLOCKGRID_H
