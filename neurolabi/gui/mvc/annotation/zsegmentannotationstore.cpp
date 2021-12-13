#include "zsegmentannotationstore.h"

#include "common/debug.h"

ZSegmentAnnotationStore::ZSegmentAnnotationStore()
{
}

std::vector<std::pair<uint64_t, ZJsonObject>>
ZSegmentAnnotationStore::getAnnotations(
    const std::vector<uint64_t> &ids, neutu::ECacheOption option)
{
  std::vector<std::pair<uint64_t, ZJsonObject>> result(ids.size());
  for (size_t i = 0; i < ids.size(); ++i) {
    result[i] = {ids[i], getAnnotation(ids[i], option)};
  }

  return result;
}

void ZSegmentAnnotationStore::saveAnnotations(
      const std::vector<std::pair<uint64_t, ZJsonObject>> &annotations,
      std::function<void(uint64_t, const std::string&)> handleError)
{
  for (const auto &annotation : annotations) {
    try {
      saveAnnotation(annotation.first, annotation.second);
    } catch (std::exception &e) {
      handleError(annotation.first, e.what());
    }
  }
}
