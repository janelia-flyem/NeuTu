#include "flyembodyannotationproofdocio.h"

#include <exception>

#include "zflyemproofdoc.h"

FlyEmBodyAnnotationProofDocIO::FlyEmBodyAnnotationProofDocIO()
{

}

void FlyEmBodyAnnotationProofDocIO::setDocument(ZFlyEmProofDoc *doc)
{
  m_doc = doc;
}

ZDvidWriter& FlyEmBodyAnnotationProofDocIO::getValidWriter()
{
  if (m_doc) {
    ZDvidWriter &writer = m_doc->getDvidWriter();
    if (writer.good()) {
      return writer;
    } else {
      throw std::runtime_error("Invalid DVID reader.");
    }
  }

  throw std::runtime_error("Document not initialized.");
}

ZJsonObject FlyEmBodyAnnotationProofDocIO::readBodyAnnotation(uint64_t bodyId)
{
  try {
    ZDvidWriter &writer = getValidWriter();

    ZJsonObject obj = writer.getDvidReader().readBodyAnnotationJson(bodyId);
    obj.setEntry(ZFlyEmBodyAnnotation::KEY_BODY_ID, bodyId);
    return obj;
  } catch (...) {
    return ZJsonObject();
  }
}

std::vector<ZJsonObject> FlyEmBodyAnnotationProofDocIO::readBodyAnnotations(
      const std::vector<uint64_t> &bodyIds)
{
  std::vector<ZJsonObject> result;
  try {
    ZDvidWriter &writer = getValidWriter();

    result = writer.getDvidReader().readBodyAnnotationJsons(bodyIds);
    for (size_t i = 0; i < result.size(); ++i) {
      result[i].setEntry(ZFlyEmBodyAnnotation::KEY_BODY_ID, bodyIds[i]);
    }
    return result;
  } catch (...) {
  }

  return result;
}

void FlyEmBodyAnnotationProofDocIO::deleteBodyAnnotation(uint64_t bodyId)
{
  ZDvidWriter &writer = getValidWriter();
  writer.deleteBodyAnnotation(bodyId);
  if (writer.getStatusCode() != 200) {
    throw std::runtime_error(
          "Failed to delete body annotation with status code " +
          std::to_string(writer.getStatusCode()));
  }
}

void FlyEmBodyAnnotationProofDocIO::writeBodyAnnotation(
    uint64_t bodyId, const ZJsonObject &obj)
{
  if (ZFlyEmBodyAnnotation::IsEmptyAnnotation(obj)) {
    deleteBodyAnnotation(bodyId);
  } else {
    ZDvidWriter &writer = getValidWriter();
    writer.writeAnnotation(bodyId, obj);
    if (writer.getStatusCode() != 200) {
      throw std::runtime_error(
            "Failed to update body annotation for " + std::to_string(bodyId) +
            ". Status code: " + std::to_string(writer.getStatusCode()));
    }
  }
}

bool FlyEmBodyAnnotationProofDocIO::hasBodyAnnotation(uint64_t bodyId)
{
  try {
    return getValidWriter().getDvidReader().hasBodyAnnotation(bodyId);
  } catch (...) {
    return false;
  }
}
