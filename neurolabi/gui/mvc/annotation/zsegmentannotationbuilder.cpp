#include "zsegmentannotationbuilder.h"

ZSegmentAnnotationBuilder::ZSegmentAnnotationBuilder()
{
}

ZSegmentAnnotationBuilder& ZSegmentAnnotationBuilder::copy(
    const ZJsonObject &obj)
{
  m_annotation = obj.clone();

  return *this;
}

ZSegmentAnnotationBuilder& ZSegmentAnnotationBuilder::fromOldAnnotation(
    const ZJsonObject &obj)
{
  return copy(obj);
}

ZSegmentAnnotationBuilder::operator ZJsonObject() const
{
  return m_annotation;
}
