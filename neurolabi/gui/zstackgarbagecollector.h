#ifndef ZSTACKGARBAGECOLLECTOR_H
#define ZSTACKGARBAGECOLLECTOR_H

#include <vector>

class ZStackObject;
class ZStack;
class ZSparseStack;

class ZStackGarbageCollector
{
public:
  ZStackGarbageCollector();
  ~ZStackGarbageCollector();

  void registerObject(ZStackObject *obj);
  void registerObject(ZStack *obj);
  void registerObject(ZSparseStack *obj);

  void dispose();

private:
  template <typename T>
  static void RegisterObject(std::vector<T*> &container, T *obj);

private:
  std::vector<ZStackObject*> m_objList;
  std::vector<ZStack*> m_stackList;
  std::vector<ZSparseStack*> m_spStackList;
};

#endif // ZSTACKGARBAGECOLLECTOR_H
