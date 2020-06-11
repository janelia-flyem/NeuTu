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

  const ZDvidReader& getWorkDvidReader() const {
    return m_workReader;
  }

  bool validateSize(int *width, int *height) const;
  int updateParam(ZStackViewParam *param) const;

  int getZoom() const {
    return m_zoom;
  }

  int getLowresZoom() const;

  const ZStackViewParam& getViewParam() const {
    return m_currentViewParam;
  }
  neutu::EAxis getSliceAxis() const;

  int getMaxWidth() const {
    return m_maxWidth;
  }

  int getMaxHeight() const {
    return m_maxHeight;
  }

  void useCenterCut(bool on) {
    m_usingCenterCut = on;
  }
  bool usingCenterCut() const {
    return m_usingCenterCut;
  }

  int getMaxZoom() const;

  bool hasMaxSize(int width, int height) const;
  bool hasMaxSize() const;
  bool getMaxArea() const;

//  QRect getViewPort() const;
//  int getX() const;
//  int getY() const;
  int getZ() const;
//  void setZ(int z);
  int getWidth() const;
  int getHeight() const;
  size_t getViewPortArea() const;
  size_t getViewDataSize() const;
  static size_t GetViewDataSize(const ZStackViewParam &viewParam, int zoom);

  void closeViewPort();
  void openViewPort();

  int getCenterCutWidth() const;
  int getCenterCutHeight() const;

  static ZIntCuboid GetBoundBox(const QRect &viewPort, int z);
  ZIntCuboid getBoundBox() const;
  ZIntCuboid getDataRange() const;
//  void setBoundBox(const ZRect2d &rect);

  int getScale() const;
  void setZoom(int zoom);

  void setViewParam(const ZStackViewParam &viewParam);
  ZStackViewParam getValidViewParam(const ZStackViewParam &viewParam) const;
  bool hasNewView(const ZStackViewParam &viewParam) const;
  bool hasNewView(
      const ZStackViewParam &viewParam, int centerCutX, int centerCutY) const;
  /*
  bool containedIn(
      const ZStackViewParam &viewParam, int zoom,
      int centerCutX, int centerCutY, bool centerCut) const;
      */
  bool actualContainedIn(
      const ZStackViewParam &viewParam, int zoom,
      int centerCutX, int centerCutY, bool centerCut) const;

  bool needHighResUpdate() const;

  void setMaxSize(int maxW, int maxH);
  void setCenterCut(int width, int height);
  void setUnlimitedSize();

  void clear();

  void setMaxZoom(int maxZoom);
  void updateMaxZoom();

  void invalidateViewParam();
  void updateCenterCut();

  void setActualQuality(int zoom, int ccw, int cch, bool centerCut);
  void syncActualQuality();

  int getActualScale() const;
  int getActualZoom() const;

  neutu::EDataSliceUpdatePolicy getUpdatePolicy() const;
  void setUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);
  void inferUpdatePolicy(neutu::EAxis axis);

  void setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);
  neutu::EDataSliceUpdatePolicy getPreferredUpdatePolicy() const;

  bool hit(double x, double y, double z) const;

private:
  /*!
   * After canonizing, there is always a combination of lowres and highres areas
   * when \a centerCut is true.
   */
  static void CanonizeQuality(
      int *zoom, int *centerCutX, int *centerCutY,
      bool *centerCut, int viewWidth, int viewHeight, int maxZoom);
  static bool IsResIncreasing(
      int sourceZoom, int sourceCenterCutX, int sourceCenterCutY, bool sourceCenterCut,
      int targetZoom, int targetCenterCutX, int targetCenterCutY, bool targetCenterCut,
      int viewWidth, int viewHeight, int maxZoom);

public:
  ZStackViewParam m_currentViewParam;
  int m_zoom = 0;
  int m_maxZoom = 0;

  int m_maxWidth = 512; //0 for no limitation
  int m_maxHeight = 512; //0 for no limitation

  int m_centerCutWidth = 256;
  int m_centerCutHeight = 256;
  bool m_usingCenterCut = true;

  int m_actualZoom = 0;
  int m_actualCenterCutWidth = 256;
  int m_actualCenterCutHeight = 256;
  bool m_actualUsingCenterCut = true;

  ZDvidData::ERole m_dataRole;
  neutu::EDataSliceUpdatePolicy m_updatePolicy = neutu::EDataSliceUpdatePolicy::DIRECT;
  neutu::EDataSliceUpdatePolicy m_preferredUpdatePolicy = neutu::EDataSliceUpdatePolicy::LOWRES;

  ZDvidReader m_reader;
  ZDvidReader m_workReader; //reader for worker thread
  ZDvidInfo m_dvidInfo;
};

#endif // ZDVIDDATASLICEHELPER_H
