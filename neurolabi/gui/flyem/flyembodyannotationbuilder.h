#ifndef FLYEMBODYANNOTATIONBUILDER_H
#define FLYEMBODYANNOTATIONBUILDER_H

#include "mvc/annotation/zsegmentannotationbuilder.h"

class FlyEmBodyAnnotationBuilder : public ZSegmentAnnotationBuilder
{
public:
  FlyEmBodyAnnotationBuilder();

  FlyEmBodyAnnotationBuilder& forBody(uint64_t bodyId);

protected:
  uint64_t m_bodyId = 0;
};

#endif // FLYEMBODYANNOTATIONBUILDER_H
