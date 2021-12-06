#include "flyembodyconfig.h"

#include "zflyembodymanager.h"

FlyEmBodyConfig::FlyEmBodyConfig()
{

}

FlyEmBodyConfig::FlyEmBodyConfig(uint64_t bodyId)
{
  setBodyId(bodyId);
}

void FlyEmBodyConfig::setBodyColor(const QColor &color)
{
  m_bodyColor = color;
}

void FlyEmBodyConfig::setRange(const ZIntCuboid &range)
{
  m_range = range;
}

QColor FlyEmBodyConfig::getBodyColor() const
{
  return m_bodyColor;
}

flyem::EBodyType FlyEmBodyConfig::getBodyType() const
{
  return m_bodyType;
}

void FlyEmBodyConfig::setDsLevel(int level)
{
  m_dsLevel = level;
//  m_dsLevel = CLIP_VALUE(level, m_minDsLevel, m_maxDsLevel);
}

void FlyEmBodyConfig::setLocalDsLevel(int level)
{
  m_localDsLevel = level;
}

void FlyEmBodyConfig::setCoarseLevel(int level)
{
  m_coarseLevel = level;
}

bool FlyEmBodyConfig::isHybrid() const
{
  return (m_dsLevel != m_localDsLevel) && !getRange().isEmpty();
}


void FlyEmBodyConfig::decDsLevel()
{
//  if (m_dsLevel <= m_minDsLevel) {
//    return false;
//  }

  if (m_nextDsLevel >= 0) {
    m_dsLevel = m_nextDsLevel;
    m_nextDsLevel = -1;
  } else if (m_nextDsLevel == -1) {
    --m_dsLevel;
    /*
    if (usingCoarseLevel()) {
      m_dsLevel = m_coarseLevel - 1;
    } else {
      --m_dsLevel;
    }
    */
  }
}

void FlyEmBodyConfig::disableNextDsLevel()
{
  setNextDsLevel(-2);
}

void FlyEmBodyConfig::setNextDsLevel(int level)
{
  m_nextDsLevel = level;
}

bool FlyEmBodyConfig::hasNextDsLevel(int minLevel) const
{
  return (m_dsLevel > minLevel) && (m_nextDsLevel >= -1);
}

ZIntCuboid FlyEmBodyConfig::getRange() const
{
  return m_range;
}

bool FlyEmBodyConfig::isTar() const
{
  return ZFlyEmBodyManager::EncodesTar(getBodyId());
}

bool FlyEmBodyConfig::isBodyTar() const
{
  return isTar() && (getBodyEncodeLevel() == 1);
}

bool FlyEmBodyConfig::isSupervoxelTar() const
{
  return isTar() && (getBodyEncodeLevel() == 0);
}

int FlyEmBodyConfig::getBodyEncodeLevel() const
{
  return ZFlyEmBodyManager::EncodedLevel(getBodyId());
}

uint64_t FlyEmBodyConfig::getDecodedBodyId() const
{
  return ZFlyEmBodyManager::Decode(getBodyId());
}

neutu::EBodyLabelType FlyEmBodyConfig::getLabelType() const
{
  return ZFlyEmBodyManager::EncodingSupervoxel(m_bodyId) ?
        neutu::EBodyLabelType::SUPERVOXEL :
        neutu::EBodyLabelType::BODY;
}

void FlyEmBodyConfig::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
}

void FlyEmBodyConfig::setAddBuffer(bool addBuffer)
{
  m_addBuffer = addBuffer;
}

bool FlyEmBodyConfig::getAddBuffer() const
{
  return m_addBuffer;
}

bool FlyEmBodyConfig::usingCoarseLevel() const
{
  return ((m_coarseLevel >= 0) && (m_dsLevel >= m_coarseLevel));
}
