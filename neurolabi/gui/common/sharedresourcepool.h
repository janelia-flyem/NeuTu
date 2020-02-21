#ifndef SHAREDRESOURCEPOOL_H
#define SHAREDRESOURCEPOOL_H

#include <memory>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace neutu {

template<typename T>
class SharedResourcePool
{
public:
  SharedResourcePool(std::function<T*(void)> factory);
  std::shared_ptr<T> take();
  std::shared_ptr<T> take(std::function<T*(void)> factory);
  void add(const std::shared_ptr<T> &res);

private:
  std::queue<std::shared_ptr<T>> m_resourceQueue;
  std::function<T*(void)> m_factory;
  std::mutex m_mutex;
};

template<typename T>
class SharedResourcePoolMap
{
public:
  SharedResourcePoolMap(std::function<T*(const std::string&)> factory);
  std::shared_ptr<T> take(const std::string &key);
  std::shared_ptr<T> take(
      const std::string &key, std::function<T*(const std::string&)> factory);
  void add(const std::string &key, const std::shared_ptr<T> &res);

private:
  std::unordered_map<std::string, std::shared_ptr<SharedResourcePool<T>>> m_resourceMap;
  std::function<T*(std::string)> m_factory;
  std::mutex m_mutex;
};

template<typename T>
class SharedResourceRetriever
{
public:
  SharedResourceRetriever(
      const std::shared_ptr<SharedResourcePoolMap<T>> &resourceMap,
      const std::string &key);
  ~SharedResourceRetriever();

  std::shared_ptr<T> get();

private:
  std::shared_ptr<SharedResourcePoolMap<T>> m_resourceMap;
  std::pair<std::string, std::shared_ptr<T>> m_resource;
};


}

#endif // SHAREDRESOURCEPOOL_H
