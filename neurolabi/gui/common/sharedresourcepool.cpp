#include "sharedresourcepool.h"


template<typename T>
neutu::SharedResourcePool<T>::SharedResourcePool(
    std::function<T*(void)> factory)
{
  m_factory = factory;
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourcePool<T>::take()
{
  return take(m_factory);
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourcePool<T>::take(
    std::function<T*(void)> factory)
{
  std::lock_guard<std::mutex> guard(m_mutex);
  std::shared_ptr<T> res = m_resourceQueue.pop();
  if (!res) {
    if (factory) {
      return std::shared_ptr<T>(factory());
    }
  }

  return res;
}

template<typename T>
void neutu::SharedResourcePool<T>::add(const std::shared_ptr<T> &res)
{
  if (res) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_resourceQueue.push(res);
  }
}



///////SharedResourcePoolMap///////

template<typename T>
neutu::SharedResourcePoolMap<T>::SharedResourcePoolMap(
    std::function<T*(const std::string&)> factory)
{
  m_factory = factory;
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourcePoolMap<T>::take(const std::string &key)
{
  std::lock_guard<std::mutex> guard(m_mutex);
  if (!key.empty()) {
    if (m_resourceMap.count(key) > 0) {
      return m_resourceMap.at(key)->take([key, this]()->T* {
        return m_factory(key);
      });
    }
  }

  return std::shared_ptr<T>();
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourcePoolMap<T>::take(
    const std::string &key, std::function<T*(const std::string&)> factory)
{
  if (!key.empty()) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_resourceMap.count(key) == 0) {
      m_resourceMap[key] =
          std::shared_ptr<SharedResourcePool<T>>(new SharedResourcePool<T>());
    }

    return m_resourceMap.at(key)->take([key, factory]()->T* {
      return factory(key);
    });
  }

  return std::shared_ptr<T>();
}

template<typename T>
void neutu::SharedResourcePoolMap<T>::add(
    const std::string &key, const std::shared_ptr<T> &res)
{
  if (!key.empty()) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_resourceMap.count(key) == 0) {
      m_resourceMap[key] =
          std::shared_ptr<SharedResourcePool<T>>(new SharedResourcePool<T>());
    }

    m_resourceMap.at(key).add(res);
  }
}


////////SharedResourceRetriever///////////
template<typename T>
neutu::SharedResourceRetriever<T>::SharedResourceRetriever(
    const std::shared_ptr<SharedResourcePoolMap<T>> &resourceMap,
    const std::string &key)
{
  m_resourceMap = resourceMap;
  m_resource.first = key;
}


template<typename T>
neutu::SharedResourceRetriever<T>::~SharedResourceRetriever()
{
  if (m_resourceMap) {
    if (m_resource.second) {
      m_resourceMap->add(m_resource.first, m_resource.second);
    }
  }
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourceRetriever<T>::get()
{
  if (!m_resource.second) {
    m_resource.second = m_resourceMap->take();
  }

  return m_resource.second;
}


