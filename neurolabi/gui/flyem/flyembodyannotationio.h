#ifndef FLYEMBODYANNOTATIONIO_H
#define FLYEMBODYANNOTATIONIO_H

#include <cstdint>

class ZJsonObject;

class FlyEmBodyAnnotationIO
{
public:
  FlyEmBodyAnnotationIO();
  virtual ~FlyEmBodyAnnotationIO() {}

  virtual ZJsonObject readBodyAnnotation(uint64_t bodyId) = 0;
  virtual void deleteBodyAnnotation(uint64_t bodyId) = 0;
  virtual void writeBodyAnnotation(uint64_t bodyId, const ZJsonObject &obj) = 0;
  virtual bool hasBodyAnnotation(uint64_t bodyId) = 0;
};

#endif // FLYEMBODYANNOTATIONIO_H
