#include "zsynapselocationmetric.h"

flyem::ZSynapseLocationMetric::ZSynapseLocationMetric()
{
}

double flyem::ZSynapseLocationEuclideanMetric::distance(
    const flyem::SynapseLocation &loc1,
    const flyem::SynapseLocation &loc2)
{
  return loc1.pos().distanceTo(loc2.pos());
}

double flyem::ZSynapseLocationAngleMetric::distance(
    const flyem::SynapseLocation &loc1,
    const flyem::SynapseLocation &loc2)
{
  ZPoint vec1, vec2;
  vec1 = loc1.pos() - m_refPoint;
  vec2 = loc2.pos() - m_refPoint;

  return 1.0 - vec1.cosAngle(vec2);
}
