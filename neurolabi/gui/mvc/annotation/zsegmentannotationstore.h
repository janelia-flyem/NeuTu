#ifndef ZSEGMENTANNOTATIONSTORE_H
#define ZSEGMENTANNOTATIONSTORE_H

#include "common/neutudefs.h"
#include "zjsonobject.h"

class ZSegmentAnnotationStore
{
public:
  ZSegmentAnnotationStore();

  virtual ZJsonObject getAnnotation(
      uint64_t sid, neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST) = 0;
  virtual void saveAnnotation(uint64_t sid, const ZJsonObject &obj) = 0;
  virtual void removeAnnotation(uint64_t sid) = 0;
};

#endif // ZSEGMENTANNOTATIONSTORE_H
