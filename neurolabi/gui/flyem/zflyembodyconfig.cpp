#include "zflyembodyconfig.h"

ZFlyEmBodyConfig::ZFlyEmBodyConfig()
{

}

ZFlyEmBodyConfig::ZFlyEmBodyConfig(uint64_t bodyId) : m_bodyId(bodyId)
{

}

void ZFlyEmBodyConfig::setBodyColor(const QColor &color)
{
  m_bodyColor = color;
}

void ZFlyEmBodyConfig::setRange(const ZIntCuboid &range)
{
  m_range = range;
}

QColor ZFlyEmBodyConfig::getBodyColor() const
{
  return m_bodyColor;
}

flyem::EBodyType ZFlyEmBodyConfig::getBodyType() const
{
  return m_bodyType;
}

void ZFlyEmBodyConfig::setDsLevel(int level)
{
  m_dsLevel = level;
//  m_dsLevel = CLIP_VALUE(level, m_minDsLevel, m_maxDsLevel);
}

void ZFlyEmBodyConfig::setLocalDsLevel(int level)
{
  m_localDsLevel = level;
}

bool ZFlyEmBodyConfig::isHybrid() const
{
  return (m_dsLevel != m_localDsLevel) && !getRange().isEmpty();
}


void ZFlyEmBodyConfig::decDsLevel()
{
//  if (m_dsLevel <= m_minDsLevel) {
//    return false;
//  }

  --m_dsLevel;
}

ZIntCuboid ZFlyEmBodyConfig::getRange() const
{
  return m_range;
}

/*
void ZFlyEmBodyConfig::setMaxDsLevel(int level)
{
  m_maxDsLevel = level;
  if (m_dsLevel > m_maxDsLevel) {
    m_dsLevel = m_maxDsLevel;
  }
}

void ZFlyEmBodyConfig::setMinDsLevel(int level)
{
  m_minDsLevel = level;
  if (m_dsLevel < m_minDsLevel) {
    m_dsLevel = m_minDsLevel;
  }
}
*/

/*
bool ZFlyEmBodyConfig::usingCoarseSource() const
{
  return (getBodyType() == flyem::BODY_COARSE) ||
      (getDsLevel() > 0 && getDsLevel() == m_maxDsLevel);
}
*/

void ZFlyEmBodyConfig::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
}
