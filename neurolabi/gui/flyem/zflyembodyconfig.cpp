#include "zflyembodyconfig.h"

#include "zflyembodymanager.h"

ZFlyEmBodyConfig::ZFlyEmBodyConfig()
{

}

ZFlyEmBodyConfig::ZFlyEmBodyConfig(uint64_t bodyId)
{
  setBodyId(bodyId);
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

bool ZFlyEmBodyConfig::isTar() const
{
  return ZFlyEmBodyManager::encodesTar(getBodyId());
}

bool ZFlyEmBodyConfig::isBodyTar() const
{
  return isTar() && (getBodyEncodeLevel() == 1);
}

bool ZFlyEmBodyConfig::isSupervoxelTar() const
{
  return isTar() && (getBodyEncodeLevel() == 0);
}

int ZFlyEmBodyConfig::getBodyEncodeLevel() const
{
  return ZFlyEmBodyManager::encodedLevel(getBodyId());
}

uint64_t ZFlyEmBodyConfig::getDecodedBodyId() const
{
  return ZFlyEmBodyManager::decode(getBodyId());
}

flyem::EBodyLabelType ZFlyEmBodyConfig::getLabelType() const
{
  return m_labelType;
}

void ZFlyEmBodyConfig::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  if (ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
    m_labelType = flyem::EBodyLabelType::SUPERVOXEL;
  }
}
