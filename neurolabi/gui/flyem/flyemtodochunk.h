#ifndef FLYEMTODOCHUNK_H
#define FLYEMTODOCHUNK_H

#include <unordered_map>
#include <vector>

#include "zflyemtodoitem.h"

class FlyEmTodoChunk
{
public:
  FlyEmTodoChunk();

  void setReady(bool ready);
  bool isReady() const;

  bool hasItem(int x, int y, int z) const;
  ZFlyEmToDoItem getItem(int x, int y, int z) const;
  std::vector<ZFlyEmToDoItem> getItemList() const;

private:
  static std::string GetItemKey(int x, int y, int z);

private:
  bool m_isReady = false;
  std::unordered_map<std::string, ZFlyEmToDoItem> m_itemMap;
};

#endif // FLYEMTODOCHUNK_H
