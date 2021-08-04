#ifndef FLYEMBODYANNOTATIONGENERICDLGBUILDER_H
#define FLYEMBODYANNOTATIONGENERICDLGBUILDER_H

#include "flyembodyannotationinteractivebuilder.h"

class ZGenericBodyAnnotationDialog;

class FlyEmBodyAnnotationGenericDlgBuilder : public FlyEmBodyAnnotationInteractiveBuilder
{
public:
  FlyEmBodyAnnotationGenericDlgBuilder();

  FlyEmBodyAnnotationGenericDlgBuilder& getDialogFrom(
      std::function<ZGenericBodyAnnotationDialog*()> f);

  ZSegmentAnnotationBuilder& fromOldAnnotation(const ZJsonObject &obj) override;

protected:
  std::function<ZGenericBodyAnnotationDialog*()> m_dlgGetter;
};

#endif // FLYEMBODYANNOTATIONGENERICDLGBUILDER_H
