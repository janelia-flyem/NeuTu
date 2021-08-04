#ifndef ZSEGMENTANNOTATIONBUILDER_H
#define ZSEGMENTANNOTATIONBUILDER_H

#include "zjsonobject.h"

class ZSegmentAnnotationBuilder
{
public:
  ZSegmentAnnotationBuilder();

  ZSegmentAnnotationBuilder& copy(const ZJsonObject &obj);
  virtual ZSegmentAnnotationBuilder& fromOldAnnotation(const ZJsonObject &obj);

  operator ZJsonObject() const;

protected:
  ZJsonObject m_annotation;
};

#endif // ZSEGMENTANNOTATIONBUILDER_H
