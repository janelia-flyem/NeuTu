#ifndef SHAREDRESOURCEPOOL_H
#define SHAREDRESOURCEPOOL_H

#include <memory>
#include <queue>
#include <mutex>

namespace neutu {

template<typename T>
class SharedResourcePool
{
public:
  SharedResourcePool();
  std::shared_ptr<T> take();
  void add(const std::shared_ptr<T> &res);

private:
  std::queue<std::shared_ptr<T>> m_resourceQueue;
  std::mutex m_mutex;
};

}

#endif // SHAREDRESOURCEPOOL_H
