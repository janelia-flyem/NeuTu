#include "zstackgarbagecollector.h"

#include "zstackobject.h"
#include "zstack.hxx"
#include "zsparsestack.h"

ZStackGarbageCollector::ZStackGarbageCollector()
{

}

ZStackGarbageCollector::~ZStackGarbageCollector()
{
  dispose();
}

void ZStackGarbageCollector::dispose()
{
  for (std::vector<ZStackObject*>::iterator iter = m_objList.begin();
       iter != m_objList.end(); ++iter) {
    delete *iter;
  }

  for (std::vector<ZStack*>::iterator iter = m_stackList.begin();
       iter != m_stackList.end(); ++iter) {
    delete *iter;
  }

  for (std::vector<ZSparseStack*>::iterator iter = m_spStackList.begin();
       iter != m_spStackList.end(); ++iter) {
    delete *iter;
  }
}

template <typename T>
void ZStackGarbageCollector::RegisterObject(std::vector<T*> &container, T *obj)
{
  if (obj != NULL) {
    container.push_back(obj);
  }
}

void ZStackGarbageCollector::registerObject(ZStackObject *obj)
{
  RegisterObject(m_objList, obj);
}

void ZStackGarbageCollector::registerObject(ZStack *obj)
{
  RegisterObject(m_stackList, obj);
}

void ZStackGarbageCollector::registerObject(ZSparseStack *obj)
{
  RegisterObject(m_spStackList, obj);
}
