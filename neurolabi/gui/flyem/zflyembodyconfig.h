#ifndef ZFLYEMBODYCONFIG_H
#define ZFLYEMBODYCONFIG_H

#include <QColor>
#include "tz_stdint.h"
#include "neutube_def.h"
#include "zintcuboid.h"

class ZFlyEmBodyConfig
{
public:
  ZFlyEmBodyConfig();
  ZFlyEmBodyConfig(uint64_t bodyId);

  int getDsLevel() const {
    return m_dsLevel;
  }
  int getLocalDsLevel() const {
    return m_localDsLevel;
  }

  ZIntCuboid getRange() const;

  void setDsLevel(int level);
  void setLocalDsLevel(int level);

  /*!
   * \brief Decrement downsample level.
   *
   */
  void decDsLevel();

  void setBodyColor(const QColor &color);
  void setRange(const ZIntCuboid &range);

  uint64_t getBodyId() const {
    return m_bodyId;
  }

  void setBodyId(uint64_t bodyId);

  QColor getBodyColor() const;
  flyem::EBodyType getBodyType() const;

  bool isHybrid() const;

//  void setMaxDsLevel(int level);
//  void setMinDsLevel(int level);

//  bool usingCoarseSource() const;

private:
  uint64_t m_bodyId = 0;
  QColor m_bodyColor;
  flyem::EBodyType m_bodyType = flyem::BODY_DEFAULT;

  ZIntCuboid m_range;
  int m_dsLevel = 0;
  int m_localDsLevel = 0;
//  int m_minDsLevel = 0;
//  int m_maxDsLevel = 10;
};

#endif // ZFLYEMBODYCONFIG_H
