#include "flyemtodoblockgrid.h"

#include "neulib/core/stringbuilder.h"

FlyEmTodoBlockGrid::FlyEmTodoBlockGrid()
{

}

std::string FlyEmTodoBlockGrid::GetChunkKey(int i, int j, int k)
{
  return neulib::StringBuilder("").
      append(i).append("_").append(j).append("_").append(k);
}

FlyEmTodoChunk FlyEmTodoBlockGrid::getTodoChunk(int i, int j, int k) const
{
  std::string key = GetChunkKey(i, j, k);
  if (m_chunkMap.count(key) > 0) {
    return m_chunkMap.at(key);
  }

  return FlyEmTodoChunk();
}
