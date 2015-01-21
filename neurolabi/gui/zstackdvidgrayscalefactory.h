#ifndef ZSTACKDVIDGRAYSCALEFACTORY_H
#define ZSTACKDVIDGRAYSCALEFACTORY_H

#include "zstackfactory.h"
#include "dvid/zdvidtarget.h"
#include "zintcuboid.h"

class ZStackDvidGrayscaleFactory : public ZStackFactory
{
public:
  ZStackDvidGrayscaleFactory();

  ZStack* makeStack(ZStack *stack = NULL) const;

private:
  ZDvidTarget m_dvidTarget;
  ZIntCuboid m_range;
};

#endif // ZSTACKDVIDGRAYSCALEFACTORY_H
