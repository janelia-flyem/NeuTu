#ifndef ZSEGMENTANNOTATIONBUILDER_H
#define ZSEGMENTANNOTATIONBUILDER_H

#include "zjsonobject.h"

class ZSegmentAnnotationBuilder
{
public:
  ZSegmentAnnotationBuilder();
  ZSegmentAnnotationBuilder(const ZJsonObject &annotation);

  ZSegmentAnnotationBuilder& copy(const ZJsonObject &obj);
  virtual ZSegmentAnnotationBuilder& fromOldAnnotation(const ZJsonObject &obj);

  /*!
   * \brief Join a json object.
   *
   * It is basically like { ...current, ...\a obj }. Note that it takes the
   * referefences of the values in \a obj.
   */
  ZSegmentAnnotationBuilder& join(const ZJsonObject &obj);

  operator ZJsonObject() const;

protected:
  ZJsonObject m_annotation;
};

#endif // ZSEGMENTANNOTATIONBUILDER_H
