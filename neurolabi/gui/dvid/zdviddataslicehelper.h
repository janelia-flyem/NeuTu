#ifndef ZDVIDDATASLICEHELPER_H
#define ZDVIDDATASLICEHELPER_H

#include <unordered_map>

#include "zdvidreader.h"
#include "zstackviewparam.h"
#include "zdviddata.h"

class QRect;
class ZIntCuboid;
class ZRect2d;
class ZAffineRect;

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

  const ZStackViewParam& getViewParam(int viewId) const;

  ZStackViewParam& getViewParam(int viewId) {
    return const_cast<ZStackViewParam&>(
          static_cast<const ZDvidDataSliceHelper&>(*this).getViewParam(viewId));
  }

  void forEachViewParam(std::function<void(const ZStackViewParam&)> f);

  neutu::EAxis getSliceAxis(int viewId) const;

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
  int getZ(int viewId) const;
  int getZ() const;
//  void setZ(int z);
  int getWidth(int viewId) const;
  int getHeight(int viewId) const;
  size_t getViewPortArea(int viewId) const;
  size_t getViewDataSize(int viewId) const;
  static size_t GetViewDataSize(const ZStackViewParam &viewParam, int zoom);

  void closeViewPort(int viewId);
  void openViewPort(int viewId);

  int getCenterCutWidth() const;
  int getCenterCutHeight() const;

  static ZIntCuboid GetBoundBox(const QRect &viewPort, int z);
  ZIntCuboid getBoundBox(int viewId) const;
  ZIntCuboid getDataRange() const;
//  void setBoundBox(const ZRect2d &rect);

  int getScale() const;
  void setZoom(int zoom);

  void setViewParam(const ZStackViewParam &viewParam);
  ZStackViewParam getValidViewParam(const ZStackViewParam &viewParam) const;
  bool hasNewView(const ZStackViewParam &viewParam) const;
  bool hasNewView(
      const ZStackViewParam &viewParam, const ZIntCuboid &modelRange) const;
  bool hasNewView(
      const ZStackViewParam &viewParam, int centerCutX, int centerCutY) const;
  /*
  bool containedIn(
      const ZStackViewParam &viewParam, int zoom,
      int centerCutX, int centerCutY, bool centerCut) const;
      */
  /*!
   * \brief Check if the current viewing slice is within a given slice
   *
   * It returns true iff:
   * 1. the currrent viewport does not exist; or
   * 2. the current viewport is the same as the given one, and the
   *    given viewport has a higher resolution; or
   * 3. the current viewport is within and smaller than the given one, and the
   *    given viewport does not have a lower resolution.
   */
  bool actualContainedIn(
      const ZStackViewParam &viewParam, int zoom,
      int centerCutX, int centerCutY, bool centerCut) const;

  /*
  ZSliceViewTransform getCanvasTransform(
      const ZAffinePlane &ap, int width, int height) const;
      */
  ZSliceViewTransform getCanvasTransform(
      const ZAffinePlane &ap, int width, int height, int zoom, int viewId) const;

  ZAffineRect getIntCutRect(int viewId) const;

  /*!
   * \brief Check if the actual resolution is not lower than the specified one
   */
  bool isResolutionReached(int viewId) const;

  /*!
   * \brief Check if a high resolution update is needed
   */
  bool needHighResUpdate(int viewId) const;

  void setMaxSize(int maxW, int maxH);
  void setCenterCut(int width, int height);
  void setUnlimitedSize();

  void clear();

  void setMaxZoom(int maxZoom);
  void updateMaxZoom();

  void invalidateViewParam(int viewId);
  void invalidateAllViewParam();
  void updateCenterCut();

  void setActualQuality(int zoom, int ccw, int cch, bool centerCut, int viewId);
  void syncActualQuality(int viewId);

  int getActualScale(int viewId) const;
  int getActualZoom(int viewId) const;

  neutu::EDataSliceUpdatePolicy getUpdatePolicy() const;
  void setUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);
  void inferUpdatePolicy(neutu::EAxis axis);

  void setPreferredUpdatePolicy(neutu::EDataSliceUpdatePolicy policy);
  neutu::EDataSliceUpdatePolicy getPreferredUpdatePolicy() const;

  bool hit(double x, double y, double z, int viewId) const;

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

  struct ViewParamBuffer {
    ZStackViewParam m_viewParam;

    int m_actualZoom = 0;
    int m_actualCenterCutWidth = 256;
    int m_actualCenterCutHeight = 256;
    bool m_actualUsingCenterCut = true;
  };

  bool isViewParamBuffered(int viewId) const;
  const ViewParamBuffer& getViewParamBuffer(int viewId) const;
  ViewParamBuffer& getViewParamBuffer(int viewId);

public:
  std::unordered_map<int, ViewParamBuffer> m_currentViewParamMap;
  ViewParamBuffer m_emptyViewParamBuffer;

  int m_zoom = 0;
  int m_maxZoom = 0;

  int m_maxWidth = 512; //0 for no limitation
  int m_maxHeight = 512; //0 for no limitation

  int m_centerCutWidth = 256;
  int m_centerCutHeight = 256;
  bool m_usingCenterCut = true;

//  int m_actualZoom = 0;
//  int m_actualCenterCutWidth = 256;
//  int m_actualCenterCutHeight = 256;
//  bool m_actualUsingCenterCut = true;

  ZDvidData::ERole m_dataRole;
  neutu::EDataSliceUpdatePolicy m_updatePolicy = neutu::EDataSliceUpdatePolicy::DIRECT;
  neutu::EDataSliceUpdatePolicy m_preferredUpdatePolicy = neutu::EDataSliceUpdatePolicy::LOWRES;

  ZDvidReader m_reader;
  ZDvidReader m_workReader; //reader for worker thread
  ZDvidInfo m_dvidInfo;
};

#endif // ZDVIDDATASLICEHELPER_H
