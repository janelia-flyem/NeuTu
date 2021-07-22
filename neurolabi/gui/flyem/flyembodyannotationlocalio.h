#ifndef FLYEMBODYANNOTATIONLOCALIO_H
#define FLYEMBODYANNOTATIONLOCALIO_H

#include "flyembodyannotationio.h"
#include "zjsonobject.h"

#include <unordered_map>

class FlyEmBodyAnnotationLocalIO : public FlyEmBodyAnnotationIO
{
public:
  FlyEmBodyAnnotationLocalIO();

  ZJsonObject readBodyAnnotation(uint64_t bodyId) override;
  void deleteBodyAnnotation(uint64_t bodyId) override;
  void writeBodyAnnotation(uint64_t bodyId, const ZJsonObject &obj) override;
  bool hasBodyAnnotation(uint64_t bodyId) override;

  void setNonexistingException(bool on);

private:
  std::unordered_map<uint64_t, ZJsonObject> m_store;
  bool m_nonexistingException = false;
};

#endif // FLYEMBODYANNOTATIONLOCALIO_H
