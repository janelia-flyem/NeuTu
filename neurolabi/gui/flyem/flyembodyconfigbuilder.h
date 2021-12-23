#ifndef FLYEMBODYCONFIGBUILDER_H
#define FLYEMBODYCONFIGBUILDER_H

#include "flyembodyconfig.h"

class FlyEmBodyConfigBuilder
{
public:
  FlyEmBodyConfigBuilder();
  FlyEmBodyConfigBuilder(const FlyEmBodyConfig &config);
  FlyEmBodyConfigBuilder(uint64_t bodyId);

  FlyEmBodyConfigBuilder& withDsLevel(int level);
  FlyEmBodyConfigBuilder& withCoarseLevel(int level);
  FlyEmBodyConfigBuilder& within(const ZIntCuboid &range);
  FlyEmBodyConfigBuilder& withLocalDsLevel(int level);

  operator FlyEmBodyConfig() const;

private:
  FlyEmBodyConfig m_config;
};

#endif // FLYEMBODYCONFIGBUILDER_H
