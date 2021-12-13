#include "flyembodyannotationio.h"

#include "zjsonobject.h"

FlyEmBodyAnnotationIO::FlyEmBodyAnnotationIO()
{

}

std::vector<ZJsonObject> FlyEmBodyAnnotationIO::readBodyAnnotations(
      const std::vector<uint64_t> &bodyIds)
{
  std::vector<ZJsonObject>  result;
  for (uint64_t bodyId : bodyIds) {
    result.push_back(readBodyAnnotation(bodyId));
  }

  return result;
}

void FlyEmBodyAnnotationIO::writeBodyAnnotations(
    const std::vector<std::pair<uint64_t, ZJsonObject>> &annotations,
    std::function<void(uint64_t, const std::string&)> handleError)
{
  for (const auto &annotation : annotations) {
    try {
      writeBodyAnnotation(annotation.first, annotation.second);
    } catch (std::exception &e) {
      if (handleError) {
        handleError(annotation.first, e.what());
      }
    }
  }
}
