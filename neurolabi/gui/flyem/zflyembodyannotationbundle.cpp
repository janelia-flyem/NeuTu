#include "zflyembodyannotationbundle.h"

ZFlyEmBodyAnnotationBundle::ZFlyEmBodyAnnotationBundle()
{
}

void ZFlyEmBodyAnnotationBundle::addBodyAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  m_annotationArray.push_back(annotation);
}
