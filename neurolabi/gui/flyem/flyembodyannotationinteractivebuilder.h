#ifndef FLYEMBODYANNOTATIONINTERACTIVEBUILDER_H
#define FLYEMBODYANNOTATIONINTERACTIVEBUILDER_H

#include "flyembodyannotationbuilder.h"

class QWidget;

class FlyEmBodyAnnotationInteractiveBuilder : public FlyEmBodyAnnotationBuilder
{
public:
  FlyEmBodyAnnotationInteractiveBuilder();

  FlyEmBodyAnnotationInteractiveBuilder& withParent(QWidget *parent);

protected:
  QWidget *m_parent;
};

#endif // FLYEMBODYANNOTATIONINTERACTIVEBUILDER_H
