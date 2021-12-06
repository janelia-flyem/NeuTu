#ifndef FLYEMBODYCONFIGBUILDER_H
#define FLYEMBODYCONFIGBUILDER_H

#include "flyembodyconfig.h"

class FlyEmBodyConfigBuilder
{
public:
  FlyEmBodyConfigBuilder();
  FlyEmBodyConfigBuilder(uint64_t bodyId);

  FlyEmBodyConfigBuilder& withDsLevel(int level);
  FlyEmBodyConfigBuilder& withCoarseLevel(int level);
  FlyEmBodyConfigBuilder& within(const ZIntCuboid &range);

  operator FlyEmBodyConfig() const;



private:
  FlyEmBodyConfig m_config;
};

#endif // FLYEMBODYCONFIGBUILDER_H
