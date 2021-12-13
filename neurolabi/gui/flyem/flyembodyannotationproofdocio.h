#ifndef FLYEMBODYANNOTATIONPROOFDOCIO_H
#define FLYEMBODYANNOTATIONPROOFDOCIO_H

#include "flyembodyannotationio.h"

#include <memory>

class ZFlyEmProofDoc;
class ZDvidWriter;

class FlyEmBodyAnnotationProofDocIO : public FlyEmBodyAnnotationIO
{
public:
  FlyEmBodyAnnotationProofDocIO();

  void setDocument(ZFlyEmProofDoc *doc);

  ZJsonObject readBodyAnnotation(uint64_t bodyId) override;
  void deleteBodyAnnotation(uint64_t bodyId) override;
  void writeBodyAnnotation(uint64_t bodyId, const ZJsonObject &obj) override;
  bool hasBodyAnnotation(uint64_t bodyId) override;

  std::vector<ZJsonObject> readBodyAnnotations(
        const std::vector<uint64_t> &bodyIds) override;

private:
  ZDvidWriter& getValidWriter();

private:
  ZFlyEmProofDoc *m_doc = nullptr;
};

#endif // FLYEMBODYANNOTATIONPROOFDOCIO_H
