#include "flyembodyconfigbuilder.h"

FlyEmBodyConfigBuilder::FlyEmBodyConfigBuilder()
{

}

FlyEmBodyConfigBuilder::FlyEmBodyConfigBuilder(uint64_t bodyId)
{
  m_config.setBodyId(bodyId);
}

FlyEmBodyConfigBuilder::operator FlyEmBodyConfig() const
{
  return m_config;
}

FlyEmBodyConfigBuilder& FlyEmBodyConfigBuilder::withDsLevel(int level)
{
  m_config.setDsLevel(level);
  return *this;
}

FlyEmBodyConfigBuilder& FlyEmBodyConfigBuilder::withCoarseLevel(int level)
{
  m_config.setCoarseLevel(level);
  return *this;
}

FlyEmBodyConfigBuilder& FlyEmBodyConfigBuilder::within(const ZIntCuboid &range)
{
  m_config.setRange(range);
  return *this;
}
