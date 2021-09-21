#include "flyembodyannotationbuilder.h"

FlyEmBodyAnnotationBuilder::FlyEmBodyAnnotationBuilder()
{

}

FlyEmBodyAnnotationBuilder& FlyEmBodyAnnotationBuilder::forBody(uint64_t bodyId)
{
  m_bodyId = bodyId;

  return *this;
}
