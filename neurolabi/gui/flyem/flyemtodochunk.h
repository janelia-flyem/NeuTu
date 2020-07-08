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

  using KeyType = ZIntPoint;

  void setReady(bool ready);
  bool isReady() const;

  bool hasItem(int x, int y, int z) const;
  ZFlyEmToDoItem getItem(int x, int y, int z) const;
  ZFlyEmToDoItem getItem(const ZIntPoint pos) const;
  std::vector<ZFlyEmToDoItem> getItemList() const;

  /*!
   * \brief Get item reference.
   *
   * These functions should be used cautiously.
   */
  ZFlyEmToDoItem& getItemRef(int x, int y, int z);
  ZFlyEmToDoItem& getItemRef(const ZIntPoint pos);

  void forEachItem(std::function<void(const ZFlyEmToDoItem &item)> f) const;

  void addItem(const ZFlyEmToDoItem &item);
  void addItem(const std::vector<ZFlyEmToDoItem> &itemList);

  bool removeItem(int x, int y, int z);
  bool removeItem(const ZIntPoint &pos);

  bool isValid() const;
  void invalidate();

  bool isEmpty() const;
  size_t countItem() const;

  /*!
   * \brief Pick an item within a range.
   *
   * When there are multiple items within the given range, whichever checked
   * first will be returned.
   */
  ZFlyEmToDoItem pickItem(double x, double y, double z, double r);

  /*!
   * \brief Pick the closest item within a range.
   */
  ZFlyEmToDoItem pickClosestItem(
      double x, double y, double z, double r);

private:
  static KeyType GetItemKey(int x, int y, int z);
  static KeyType GetItemKey(const ZIntPoint &pos);

private:
  bool m_isReady = false;
  bool m_isValid = true;
  std::unordered_map<KeyType, ZFlyEmToDoItem> m_itemMap;
  ZFlyEmToDoItem m_invalidItem;
};

#endif // FLYEMTODOCHUNK_H
