#ifndef ZSEGMENTANNOTATIONSTORE_H
#define ZSEGMENTANNOTATIONSTORE_H

#include <vector>
#include <utility>
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
   * Each element of the returned array is a (id, annotation) pair that represents
   * the id and annotation correspondence. The length of the returned array
   * is less than or equal to \a ids.
   */
  virtual std::vector<std::pair<uint64_t, ZJsonObject>> getAnnotations(
      const std::vector<uint64_t> &ids,
      neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST);

  /*!
   * \brief Save a group of annotations.
   *
   * It saves a group of annotations in \a annotations, in which each element is
   * a pair of a segment ID and its annotation. When an error happened for a
   * certain body ID, \a handleError will be called by passing the the ID and
   * error message if \a handleError is not NULL. If the function does not know
   * which body causes the error, it will pass ID 0.
   */
  virtual void saveAnnotations(
      const std::vector<std::pair<uint64_t, ZJsonObject>> &annotations,
      std::function<void(uint64_t, const std::string&)> handleError);

protected:
  QMap<uint64_t, ZJsonObject> m_annotationCache;
};

#endif // ZSEGMENTANNOTATIONSTORE_H
