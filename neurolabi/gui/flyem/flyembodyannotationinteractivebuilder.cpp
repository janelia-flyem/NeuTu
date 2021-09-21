#include "flyembodyannotationinteractivebuilder.h"

FlyEmBodyAnnotationInteractiveBuilder::FlyEmBodyAnnotationInteractiveBuilder()
{

}

FlyEmBodyAnnotationInteractiveBuilder&
FlyEmBodyAnnotationInteractiveBuilder::withParent(QWidget *parent)
{
  m_parent = parent;

  return *this;
}
