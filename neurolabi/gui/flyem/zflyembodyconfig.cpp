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

  if (m_nextDsLevel >= 0) {
    m_dsLevel = m_nextDsLevel;
    m_nextDsLevel = -1;
  } else if (m_nextDsLevel == -1) {
    --m_dsLevel;
  }
}

void ZFlyEmBodyConfig::disableNextDsLevel()
{
  setNextDsLevel(-2);
}

void ZFlyEmBodyConfig::setNextDsLevel(int level)
{
  m_nextDsLevel = level;
}

bool ZFlyEmBodyConfig::hasNextDsLevel(int minLevel) const
{
  return (m_dsLevel > minLevel) && (m_nextDsLevel >= -1);
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

neutu::EBodyLabelType ZFlyEmBodyConfig::getLabelType() const
{
  return m_labelType;
}

void ZFlyEmBodyConfig::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  if (ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
    m_labelType = neutu::EBodyLabelType::SUPERVOXEL;
  }
}

void ZFlyEmBodyConfig::setAddBuffer(bool addBuffer)
{
  m_addBuffer = addBuffer;
}

bool ZFlyEmBodyConfig::getAddBuffer() const
{
  return m_addBuffer;
}
