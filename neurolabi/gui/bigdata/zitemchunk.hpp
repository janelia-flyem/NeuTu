#ifndef ZITEMCHUNK_H
#define ZITEMCHUNK_H

#include <unordered_map>
#include <vector>

template <typename T, typename TKey>
class ZItemChunk
{
public:
  ZItemChunk() {}
  virtual ~ZItemChunk() {}

  using KeyType = TKey;

public:
  void setReady(bool ready)
  {
    m_isReady = ready;
  };
  bool isReady() const
  {
    return m_isReady;
  }

  bool isValid() const
  {
    return m_isValid;
  }
  void invalidate()
  {
    m_isValid = false;
  }

  bool isEmpty() const
  {
    return m_itemMap.size() == 0;
  }

  size_t countItem() const
  {
    return m_itemMap.size();
  }

  T& getItemRef(const KeyType &key)
  {
    if (m_itemMap.count(key) > 0) {
      return m_itemMap.at(key);
    }

    return m_invalidItem;
  }

  T getItem(const KeyType &key) const
  {
    return const_cast<ZItemChunk<T, KeyType>&>(*this).getItemRef(key);
  }

  bool removeItem(const KeyType &key)
  {
    return m_itemMap.erase(key);
  }

  bool hasItem(const KeyType &key) const
  {
    return m_itemMap.count(key) > 0;
  }

  std::vector<T> getItemList() const
  {
    std::vector<T> itemList;
    for (auto item : m_itemMap) {
      itemList.push_back(item.second);
    }

    return itemList;
  }

  void forEachItem(
      std::function<void (const T &)> f) const
  {
    for (const auto &item : m_itemMap) {
      f(item.second);
    }
  }

  void addItem(const T &item)
  {
    if (item.isValid()) {
      m_itemMap[getKey(item)] = item;
    }
  }

  void addItem(const std::vector<T> &itemList)
  {
    for (const auto &item : itemList) {
      addItem(item);
    }
  }

  void update(const std::vector<T> &itemList)
  {
    m_itemMap.clear();
    addItem(itemList);
    setReady(true);
  }

protected:
  virtual KeyType getKey(const T &item) const = 0;

protected:
  bool m_isReady = false;
  bool m_isValid = true;
  std::unordered_map<KeyType, T> m_itemMap;
  T m_invalidItem;
};

#endif // ZITEMCHUNK_H
