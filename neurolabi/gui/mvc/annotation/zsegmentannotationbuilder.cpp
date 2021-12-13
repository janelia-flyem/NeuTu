#include "zsegmentannotationbuilder.h"

ZSegmentAnnotationBuilder::ZSegmentAnnotationBuilder()
{
}

ZSegmentAnnotationBuilder::ZSegmentAnnotationBuilder(
    const ZJsonObject &annotation)
{
  m_annotation = annotation;
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

ZSegmentAnnotationBuilder& ZSegmentAnnotationBuilder::join(
    const ZJsonObject &obj)
{
  obj.forEachValue([&](const std::string &key, ZJsonValue value) {
    m_annotation.setEntry(key, value.getData());
  });

  return *this;
}

ZSegmentAnnotationBuilder::operator ZJsonObject() const
{
  return m_annotation;
}
