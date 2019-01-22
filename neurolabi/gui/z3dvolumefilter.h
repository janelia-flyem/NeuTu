#ifndef Z3DVOLUMEFILTER_H
#define Z3DVOLUMEFILTER_H


#include "z3dboundedfilter.h"

#include "z3dcameraparameter.h"
#include "widgets/znumericparameter.h"
#include "z3dvolume.h"
#include "z3dtransformparameter.h"
#include "zmesh.h"
#include "zwidgetsgroup.h"
#include <vector>
#include "z3dvolumeraycasterrenderer.h"
#include "z3dvolumeslicerenderer.h"
#include "z3dtextureandeyecoordinaterenderer.h"
#include "z3dimage2drenderer.h"
#include "zeventlistenerparameter.h"
#include "z3dtexturecopyrenderer.h"
#include "z3drenderport.h"
#include "geometry/zlinesegment.h"

class ZMesh;
class ZStackDoc;

class Z3DVolumeFilter : public Z3DBoundedFilter
{
Q_OBJECT

  friend class Z3DCompositor;

public:
  explicit Z3DVolumeFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  // extract vols and img from stackdoc and call the overloaded setData
  void setData(const ZStackDoc* doc = nullptr, size_t maxVoxelNumber = 0);
  // reload current stackdoc with new maxVoxelNumber
  void reloadData(size_t maxVoxelNumber);
  // will take ownership of vols
  void setData(std::vector<std::unique_ptr<Z3DVolume>>& vols,
               ZStack* img = nullptr);

  virtual bool isStayOnTop() const
  { return m_stayOnTop.get(); }

  virtual void setStayOnTop(bool s)
  { m_stayOnTop.set(s); }

  // input volPos should be in volume coordinate
  // means it is in range [0 width-1 0 height-1 0 depth-1]
  bool openZoomInView(const glm::ivec3& volPos);

  void exitZoomInView();

  const ZBBox<glm::dvec3>& zoomInBound() const
  { return m_zoomInBound; }

  bool volumeNeedDownsample() const;

  bool isVolumeDownsampled() const;

  bool isSubvolume() const
  { return m_isSubVolume.get(); }

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void enterInteractionMode() override;

  virtual void exitInteractionMode() override;

  bool isReady(Z3DEye eye) const override;

  // get salient 3d position hit by 2d point
  // check success before using the returned value
  // if first hit 3d position is in volume, success will be true,
  // otherwise don't use the returned value
  glm::vec3 get3DPosition(int x, int y, int width, int height, bool& success);

  virtual bool hasOpaque(Z3DEye eye) const override;

  virtual void renderOpaque(Z3DEye eye) override;

  virtual bool hasTransparent(Z3DEye eye) const override;

  virtual void renderTransparent(Z3DEye eye) override;

  /*!
   * \brief Get the image-space ray of a screen point
   *
   * The ray passes the screen point (\a x, \a y) with the direction into the
   * screen.
   */
  ZLineSegment getScreenRay(int x, int y, int width, int height);

  inline void setOpaque(bool opaque)
  { m_volumeRaycasterRenderer.setOpaque(opaque); }

  inline void setAlpha(double alpha)
  { m_volumeRaycasterRenderer.setAlpha(alpha); }

  inline double getAlpha()
  { return m_volumeRaycasterRenderer.getAlpha(); }

  void setCompositeMode(const QString& option)
  { m_volumeRaycasterRenderer.setCompositeMode(option); }

  void setTextureFilterMode(const QString& option)
  { m_volumeRaycasterRenderer.setTextureFilterMode(option); }

  void setChannel1Visible(bool v) { m_volumeRaycasterRenderer.setChannel1Visible(v); }
  void setChannel2Visible(bool v) { m_volumeRaycasterRenderer.setChannel2Visible(v); }
  void setChannel3Visible(bool v) { m_volumeRaycasterRenderer.setChannel3Visible(v); }
  void setChannel4Visible(bool v) { m_volumeRaycasterRenderer.setChannel4Visible(v); }
  void setChannel5Visible(bool v) { m_volumeRaycasterRenderer.setChannel5Visible(v); }

signals:
  void pointInVolumeLeftClicked(QPoint pt, glm::ivec3 pos3D, Qt::KeyboardModifiers mod);

  void pointInVolumeRightClicked(QPoint pt, glm::ivec3 pos3D, Qt::KeyboardModifiers mod);

protected:

  void changeCoordTransform();

  void changeZoomInViewSize();

  void adjustWidget();

  void leftMouseButtonPressed(QMouseEvent* e, int w, int h);

  void invalidateFRVolumeZSlice();

  void invalidateFRVolumeYSlice();

  void invalidateFRVolumeXSlice();

  void invalidateFRVolumeZSlice2();

  void invalidateFRVolumeYSlice2();

  void invalidateFRVolumeXSlice2();

  void updateCubeSerieSlices();

  virtual void setClipPlanes() override
  {}

  virtual void process(Z3DEye eye) override;

  bool hasSlices() const;

  void renderSlices(Z3DEye eye);

  const std::vector<std::unique_ptr<Z3DVolume>>& getVolumes() const;

  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void expandCutRange() override
  {}

private:
  void readVolumes();

  void readSubVolumes(int left, int right, int up, int down, int front, int back);

  // check success before using the returned value
  // if first hit 3d position is in volume, success will be true,
  // otherwise don't use the returned value
  glm::vec3 getFirstHit3DPosition(int x, int y, int width, int height, bool& success);

  // use first channel intensity
  glm::vec3 getMaxInten3DPositionUnderScreenPoint(int x, int y, int width, int height, bool& success);

  //get 3D position from 2D screen position
  glm::vec3 get3DPosition(glm::ivec2 pos2D, int width, int height, Z3DRenderOutputPort& port);

  //get 3D position from 2D screen position and depth
  glm::vec3 get3DPosition(glm::ivec2 pos2D, double depth, int width, int height);

  // based on context, prepare minimum necessary data and send to raycasterrenderer
  void prepareDataForRaycaster(Z3DVolume* volume, Z3DEye eye);

  void invalidateAllFRVolumeSlices();

  void volumeChanged();

  void readVolumes(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume>>& vols);
  void readVolumesWithObject(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume>>& vols);
  void readSparseVolume(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume>>& vols);
  void readSparseVolumeWithObject(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume>>& vols);
  void readSparseStack(const ZStackDoc* doc, std::vector<std::unique_ptr<Z3DVolume>>& vols);

  Z3DVolumeRaycasterRenderer m_volumeRaycasterRenderer;
  Z3DVolumeSliceRenderer m_volumeSliceRenderer;
  Z3DTextureAndEyeCoordinateRenderer m_textureAndEyeCoordinateRenderer;
  std::vector<std::unique_ptr<Z3DImage2DRenderer>> m_image2DRenderers;
  Z3DTextureCopyRenderer m_textureCopyRenderer;

  ZStack* m_imgPack = nullptr;
  std::vector<std::unique_ptr<Z3DVolume>> m_volumes;
  std::vector<std::unique_ptr<Z3DVolume>> m_zoomInVolumes;
  ZBoolParameter m_stayOnTop;
  ZBoolParameter m_isVolumeDownsampled;
  ZBoolParameter m_isSubVolume;
  ZIntParameter m_zoomInViewSize;
  glm::ivec3 m_zoomInPos;
  ZBBox<glm::dvec3> m_zoomInBound;

  size_t m_maxVoxelNumber = 0;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  size_t m_numParas;

  ZIntParameter m_interactionDownsample;      // screen space downsample during interaction

  Z3DRenderTarget m_entryTarget;
  Z3DRenderTarget m_exitTarget;
  Z3DRenderTarget m_layerTarget;
  Z3DTexture m_layerColorTexture;
  Z3DTexture m_layerDepthTexture;

  Z3DRenderOutputPort m_outport;
  Z3DRenderOutputPort m_leftEyeOutport;
  Z3DRenderOutputPort m_rightEyeOutport;
  Z3DFilterOutputPort<Z3DVolumeFilter> m_vPPort;
  Z3DRenderOutputPort m_opaqueOutport;
  Z3DRenderOutputPort m_opaqueLeftEyeOutport;
  Z3DRenderOutputPort m_opaqueRightEyeOutport;

  static const size_t m_maxNumOfFullResolutionVolumeSlice;
  // each channel is represented by a Z3DVolume
  std::vector<std::vector<std::unique_ptr<Z3DVolume>>> m_FRVolumeSlices;
  std::vector<bool> m_FRVolumeSlicesValidState;
  ZBoolParameter m_useFRVolumeSlice;
  ZBoolParameter m_showXSlice;
  ZIntParameter m_xSlicePosition;
  ZBoolParameter m_showYSlice;
  ZIntParameter m_ySlicePosition;
  ZBoolParameter m_showZSlice;
  ZIntParameter m_zSlicePosition;
  std::vector<std::unique_ptr<ZColorMapParameter>> m_sliceColormaps;
  ZBoolParameter m_showXSlice2;
  ZIntParameter m_xSlice2Position;
  ZBoolParameter m_showYSlice2;
  ZIntParameter m_ySlice2Position;
  ZBoolParameter m_showZSlice2;
  ZIntParameter m_zSlice2Position;

  ZEventListenerParameter m_leftMouseButtonPressEvent;
  glm::ivec2 m_startCoord;

  ZMesh m_2DImageQuad;

  std::map<std::string, ZMesh> m_cubeSerieSlices;

  double m_imgMinIntensity;
  double m_imgMaxIntensity;
};

#endif // Z3DVOLUMEFILTER_H
