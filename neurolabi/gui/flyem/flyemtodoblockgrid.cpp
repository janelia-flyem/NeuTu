#include "flyemtodoblockgrid.h"

#include "neulib/core/stringbuilder.h"

#include "flyemtodosource.h"

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
  std::lock_guard<std::mutex> guard(m_dataMutex);
  std::string key = GetChunkKey(i, j, k);
  if (m_chunkMap.count(key) > 0) {
    return m_chunkMap.at(key);
  } else {

    if (m_source) {
      std::vector<ZFlyEmToDoItem> todoList = m_source->getData(
            getBlockBox(ZIntPoint(i, j, k)));
      FlyEmTodoChunk chunk;
      chunk.addItem(todoList);
      chunk.setReady(true);
      m_chunkMap[key] = chunk;
      return chunk;
    }
  }

  return FlyEmTodoChunk();
}

void FlyEmTodoBlockGrid::setSource(std::shared_ptr<FlyEmTodoSource> source)
{
  m_source = source;
  setBlockSize(source->getBlockSize());
  setGridByRange(source->getRange());
}

/*
std::vector<ZFlyEmToDoItem> FlyEmTodoBlockGrid::getIntersectTodoList(
    const ZAffineRect &plane) const
{

}
*/
