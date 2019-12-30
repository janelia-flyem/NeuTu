#ifndef ZFLYEMBODYCONFIG_H
#define ZFLYEMBODYCONFIG_H

#include <cstdint>

#include <QColor>

#include "common/neutudefs.h"
#include "flyemdef.h"
#include "geometry/zintcuboid.h"

/*!
 * \brief The class of configuring a body, mainly for visualization.
 *
 * Each configuration has a body ID and the downsampling level to specifiy what
 * body data should be obtained. The body can also be a hybrid of parts from
 * different downsampling levels.
 *
 * Example of configuring a hybrid body of ID 1:
 *
 *   ZFlyEmBodyConfig config(1);
 *   config.setRange(ZIntCuboid(ZIntPoint(10, 20, 30), ZIntPoint(100, 200, 300));
 *   config.setLocalDsLevel(0); //High res in the box (10, 20, 30)->(100, 200, 300)
 *   config.setDsLevel(5); //Low res for the rest part
 */
class ZFlyEmBodyConfig
{
public:
  ZFlyEmBodyConfig();
  explicit ZFlyEmBodyConfig(uint64_t bodyId);

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
  uint64_t getDecodedBodyId() const;

  void setBodyId(uint64_t bodyId);

  QColor getBodyColor() const;
  flyem::EBodyType getBodyType() const;

  bool isHybrid() const;

  bool isTar() const;
  bool isBodyTar() const;
  bool isSupervoxelTar() const;
  int getBodyEncodeLevel() const;

  neutu::EBodyLabelType getLabelType() const;

//  void setMaxDsLevel(int level);
//  void setMinDsLevel(int level);

//  bool usingCoarseSource() const;

  // Indicates that the body meshes should be buffered (for prefetching).
  void setAddBuffer(bool addBuffer = true);
  bool getAddBuffer() const;

private:
  uint64_t m_bodyId = 0;
  QColor m_bodyColor;
  flyem::EBodyType m_bodyType = flyem::EBodyType::DEFAULT;
  neutu::EBodyLabelType m_labelType = neutu::EBodyLabelType::BODY;

  ZIntCuboid m_range;
  int m_dsLevel = 0;
  int m_localDsLevel = 0;

  bool m_addBuffer = false;
};

#endif // ZFLYEMBODYCONFIG_H
