#include "sharedresourcepool.h"


template<typename T>
neutu::SharedResourcePool<T>::SharedResourcePool()
{
}

template<typename T>
std::shared_ptr<T> neutu::SharedResourcePool<T>::take()
{
  std::lock_guard<std::mutex> guard(m_mutex);
  std::shared_ptr<T> res = m_resourceQueue.pop();

  return res;
}

template<typename T>
void neutu::SharedResourcePool<T>::add(const std::shared_ptr<T> &res)
{
  std::lock_guard<std::mutex> guard(m_mutex);
  m_resourceQueue.push(res);
}
