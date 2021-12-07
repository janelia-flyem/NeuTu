#ifndef ZSEGMENTANNOTATIONSTORE_H
#define ZSEGMENTANNOTATIONSTORE_H

#include <vector>
#include <QMap>

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

  /*!
   * \brief Get annotation in batch
   *
   * The returned array has one-to-one correspondence with \a ids.
   */
  virtual std::vector<ZJsonObject> getAnnotations(
      const std::vector<uint64_t> ids,
      neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST);

protected:
  QMap<uint64_t, ZJsonObject> m_annotationCache;
};

#endif // ZSEGMENTANNOTATIONSTORE_H
