#ifndef FLYEMBODYANNOTATIONGENERICDLGBUILDER_H
#define FLYEMBODYANNOTATIONGENERICDLGBUILDER_H

#include <functional>

#include "flyembodyannotationinteractivebuilder.h"

class ZGenericBodyAnnotationDialog;

class FlyEmBodyAnnotationGenericDlgBuilder : public FlyEmBodyAnnotationInteractiveBuilder
{
public:
  FlyEmBodyAnnotationGenericDlgBuilder();

  FlyEmBodyAnnotationGenericDlgBuilder& getDialogFrom(
      std::function<ZGenericBodyAnnotationDialog*()> f, bool ready);

  ZSegmentAnnotationBuilder& fromOldAnnotation(const ZJsonObject &obj) override;

protected:
  bool m_dlgReady = true;
  std::function<ZGenericBodyAnnotationDialog*()> m_dlgGetter;
};

#endif // FLYEMBODYANNOTATIONGENERICDLGBUILDER_H
