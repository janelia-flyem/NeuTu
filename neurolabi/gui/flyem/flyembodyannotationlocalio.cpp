#include "flyembodyannotationlocalio.h"

#include "zjsonobject.h"

FlyEmBodyAnnotationLocalIO::FlyEmBodyAnnotationLocalIO()
{

}

ZJsonObject FlyEmBodyAnnotationLocalIO::readBodyAnnotation(uint64_t bodyId)
{
  if (m_store.count(bodyId) > 0) {
    return m_store.at(bodyId).clone();
  }

  return ZJsonObject();
}

void FlyEmBodyAnnotationLocalIO::deleteBodyAnnotation(uint64_t bodyId)
{
  m_store.erase(bodyId);
}

void FlyEmBodyAnnotationLocalIO::writeBodyAnnotation(
    uint64_t bodyId, const ZJsonObject &obj)
{
  m_store[bodyId] = obj.clone();
}

bool FlyEmBodyAnnotationLocalIO::hasBodyAnnotation(uint64_t bodyId)
{
  return m_store.count(bodyId) > 0;
}
