#ifndef FLYEMTODOCHUNK_H
#define FLYEMTODOCHUNK_H

#include <unordered_map>
#include <vector>
#include <functional>

#include "zflyemtodoitem.h"

class ZIntPoint;

class FlyEmTodoChunk
{
public:
  FlyEmTodoChunk();

  void setReady(bool ready);
  bool isReady() const;

  bool hasItem(int x, int y, int z) const;
  ZFlyEmToDoItem getItem(int x, int y, int z) const;
  std::vector<ZFlyEmToDoItem> getItemList() const;
  void forEachItem(std::function<void(const ZFlyEmToDoItem &item)> f);

  void addItem(const ZFlyEmToDoItem &item);
  void addItem(const std::vector<ZFlyEmToDoItem> &itemList);

private:
  static std::string GetItemKey(int x, int y, int z);
  static std::string GetItemKey(const ZIntPoint &pos);

private:
  bool m_isReady = false;
  std::unordered_map<std::string, ZFlyEmToDoItem> m_itemMap;
};

#endif // FLYEMTODOCHUNK_H
