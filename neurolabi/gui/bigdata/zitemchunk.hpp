#ifndef ZITEMCHUNK_H
#define ZITEMCHUNK_H

#include <unordered_map>
#include <vector>
#include <functional>

#include "zdatachunk.h"

template <typename TItem, typename TKey>
class ZItemChunk : public ZDataChunk
{
public:
  ZItemChunk() {}
  virtual ~ZItemChunk() {}

  using KeyType = TKey;

public:
  bool isEmpty() const
  {
    return m_itemMap.size() == 0;
  }

  size_t countItem() const
  {
    return m_itemMap.size();
  }

  TItem& getItemRef(const KeyType &key)
  {
    if (m_itemMap.count(key) > 0) {
      return m_itemMap.at(key);
    }

    return m_invalidItem;
  }

  TItem getItem(const KeyType &key) const
  {
    return const_cast<ZItemChunk<TItem, KeyType>&>(*this).getItemRef(key);
  }

  bool removeItem(const KeyType &key)
  {
    return m_itemMap.erase(key);
  }

  bool hasItem(const KeyType &key) const
  {
    return m_itemMap.count(key) > 0;
  }

  std::vector<TItem> getItemList() const
  {
    std::vector<TItem> itemList;
    for (auto item : m_itemMap) {
      itemList.push_back(item.second);
    }

    return itemList;
  }

  void forEachItem(
      std::function<void (const TItem &)> f) const
  {
    for (const auto &item : m_itemMap) {
      f(item.second);
    }
  }

  void addItem(const TItem &item)
  {
    if (item.isValid()) {
      m_itemMap[getKey(item)] = item;
    }
  }

  void addItem(const std::vector<TItem> &itemList)
  {
    for (const auto &item : itemList) {
      addItem(item);
    }
  }

  void update(const std::vector<TItem> &itemList)
  {
    m_itemMap.clear();
    addItem(itemList);
    setReady(true);
  }

protected:
  virtual KeyType getKey(const TItem &item) const = 0;

protected:
  std::unordered_map<KeyType, TItem> m_itemMap;
  TItem m_invalidItem;
};

#endif // ZITEMCHUNK_H
