#include "zsegmentannotationstore.h"

ZSegmentAnnotationStore::ZSegmentAnnotationStore()
{

}

std::vector<ZJsonObject> ZSegmentAnnotationStore::getAnnotations(
      const std::vector<uint64_t> ids, neutu::ECacheOption option)
{
  std::vector<ZJsonObject> annotations(ids.size());
  for (size_t i = 0; i < ids.size(); ++i) {
    annotations[i] = getAnnotation(ids[i], option);
  }

  return annotations;
}
