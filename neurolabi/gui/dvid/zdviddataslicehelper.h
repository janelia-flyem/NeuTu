#ifndef ZDVIDDATASLICEHELPER_H
#define ZDVIDDATASLICEHELPER_H

#include "zdvidreader.h"
#include "zstackviewparam.h"
#include "zdviddata.h"

class QRect;
class ZIntCuboid;
class ZRect2d;

class ZDvidDataSliceHelper
{
public:
  ZDvidDataSliceHelper(ZDvidData::ERole role);

  void setDvidTarget(const ZDvidTarget &target);
  inline const ZDvidTarget& getDvidTarget() const {
    return m_reader.getDvidTarget();
  }
  inline ZDvidTarget& getDvidTarget() {
    return m_reader.getDvidTarget();
  }

  const ZDvidReader& getDvidReader() const {
    return m_reader;
  }

  bool validateSize(int *width, int *height) const;
  int updateParam(ZStackViewParam *param);

  int getZoom() const {
    return m_zoom;
  }

  const ZStackViewParam& getViewParam() const {
    return m_currentViewParam;
  }

  int getMaxWidth() const {
    return m_maxWidth;
  }

  int getMaxHeight() const {
    return m_maxHeight;
  }

  int getMaxZoom() const;

  bool hasMaxSize(int width, int height) const;
  bool hasMaxSize() const;
  bool getMaxArea() const;

  QRect getViewPort() const;
  int getX() const;
  int getY() const;
  int getZ() const;
  void setZ(int z);
  int getWidth() const;
  int getHeight() const;

  int getCenterCutWidth() const;
  int getCenterCutHeight() const;

  static ZIntCuboid GetBoundBox(const QRect &viewPort, int z);
  ZIntCuboid getBoundBox() const;
  void setBoundBox(const ZRect2d &rect);

  int getScale() const;
  void setZoom(int zoom) {
    m_zoom = zoom;
  }

  void setViewParam(const ZStackViewParam &viewParam);
  ZStackViewParam getValidViewParam(const ZStackViewParam &viewParam) const;
  bool hasNewView(const ZStackViewParam &viewParam) const;
  bool hasNewView(
      const ZStackViewParam &viewParam, int centerCutX, int centerCutY) const;
  bool containedIn(
      const ZStackViewParam &viewParam, int zoom,
      int centerCutX, int centerCutY) const;

  void setMaxSize(int maxW, int maxH);
  void setCenterCut(int width, int height);
  void setUnlimitedSize();

  void clear();

  void setMaxZoom(int maxZoom);

  void invalidateViewParam();
  void updateCenterCut();

public:
  ZStackViewParam m_currentViewParam;
  int m_zoom = 0;
  int m_maxZoom = 0;

  int m_maxWidth = 512; //0 for no limitation
  int m_maxHeight = 512; //0 for no limitation

  int m_centerCutWidth = 256;
  int m_centerCutHeight = 256;

  ZDvidData::ERole m_dataRole;

  ZDvidReader m_reader;
};

#endif // ZDVIDDATASLICEHELPER_H
