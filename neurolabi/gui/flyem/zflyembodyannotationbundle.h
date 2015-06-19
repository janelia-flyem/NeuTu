#ifndef ZFLYEMBODYANNOTATIONBUNDLE_H
#define ZFLYEMBODYANNOTATIONBUNDLE_H

#include <vector>
#include "zflyembodyannotation.h"

class ZFlyEmBodyAnnotationBundle
{
public:
  ZFlyEmBodyAnnotationBundle();

  void addBodyAnnotation(const ZFlyEmBodyAnnotation &annotation);

private:
  std::vector<ZFlyEmBodyAnnotation> m_annotationArray;
  std::string m_userName;
  std::string m_software;
  std::string m_description;
};

#endif // ZFLYEMBODYANNOTATIONBUNDLE_H
