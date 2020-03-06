#ifndef Z3DVIEW_H
#define Z3DVIEW_H

#include <QDir>
#include <QObject>
#include <QAction>

#include "z3dglobalparameters.h"
#include "zbbox.h"
#include "zstackobjectgroup.h"
#include "z3ddef.h"
//#include "zstackdoc3dhelper.h"

class Z3DCanvas;
class Z3DCompositor;
class Z3DCanvasPainter;
class Z3DNetworkEvaluator;

class Z3DVolumeFilter;
class Z3DPunctaFilter;
class Z3DSwcFilter;
class Z3DMeshFilter;
class Z3DGraphFilter;
class Z3DSurfaceFilter;
class ZFlyEmTodoListFilter;
class Z3DBoundedFilter;
class Z3DGeometryFilter;
class ZStackDoc;
class ZStackObjectInfoSet;
class Z3D2DSliceFilter;

class QRubberBand;

class Z3DView : public QObject
{
Q_OBJECT

public:
  enum class EInitMode {
    NORMAL, EXCLUDE_VOLUME, FULL_RES_VOLUME,
    EXCLUDE_MESH //A temporary fix for disalbing meshes in some cases
  };

  Z3DView(ZStackDoc* doc, EInitMode initMode, bool stereo, QWidget* parent = nullptr);

  ~Z3DView();

public: //utilties
  static std::string GetLayerString(neutu3d::ERendererLayer layer);

public: //properties
  void setZScale(neutu3d::ERendererLayer layer, double scale);
  void setScale(neutu3d::ERendererLayer layer, double sx, double sy, double sz);
  void setZScale(double scale);
  void setScale(double sx, double sy, double sz);
  void setOpacity(neutu3d::ERendererLayer layer, double opacity);
  void setOpacityQuietly(neutu3d::ERendererLayer layer, double opacity);
//  using QWidget::setVisible; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  void setLayerVisible(neutu3d::ERendererLayer layer, bool visible);
  bool isLayerVisible(neutu3d::ERendererLayer layer) const;
  void setFront(neutu3d::ERendererLayer layer, bool on);

  void configureLayer(neutu3d::ERendererLayer layer, const ZJsonObject &obj);
  ZJsonObject getConfigJson(neutu3d::ERendererLayer layer) const;
  void configure(const ZJsonObject &obj);

  const QList<neutu3d::ERendererLayer>& getLayerList() const {
    return m_layerList;
  }

public:
  ZStackDoc* getDocument() const {
    return m_doc;
  }

  void updateDocData(neutu3d::ERendererLayer layer);
  void updateCustomCanvas(const QImage &image);
  void updateCanvas();

public:
  inline QAction* zoomInAction()
  { return m_zoomInAction; }

  inline QAction* zoomOutAction()
  { return m_zoomOutAction; }

  inline QAction* resetCameraAction()
  { return m_resetCameraAction; }

  Z3DCameraParameter& camera()
  { return m_globalParas->camera; }

  const Z3DCameraParameter& camera() const
  { return m_globalParas->camera; }

  Z3DTrackballInteractionHandler& interactionHandler()
  { return m_globalParas->interactionHandler; }

  inline Z3DCanvas& canvas()
  { return *m_canvas; }

  inline const Z3DCanvas& canvas() const
  { return *m_canvas; }

  inline Z3DCanvasPainter& canvasPainter()
  { return *m_canvasPainter; }

  inline Z3DCompositor& compositor()
  { return *m_compositor; }

  inline Z3DNetworkEvaluator& networkEvaluator()
  { return *m_networkEvaluator; }

  inline Z3DGlobalParameters& globalParas()
  { return *m_globalParas; }

  inline Z3DVolumeFilter* getVolumeFilter() const
  { return m_volumeFilter.get(); }

  inline Z3DPunctaFilter* getPunctaFilter() const
  { return m_punctaFilter.get(); }

  inline Z3DSwcFilter* getSwcFilter() const
  { return m_swcFilter.get(); }

  inline Z3DMeshFilter* getMeshFilter() const
  { return m_meshFilter.get(); }

  inline Z3DGraphFilter* getGraphFilter() const
  { return m_graphFilter.get(); }

  inline Z3DSurfaceFilter* getSurfaceFilter() const
  { return m_surfaceFilter.get(); }

  inline ZFlyEmTodoListFilter* getTodoFilter() const
  { return m_todoFilter.get(); }

  inline Z3DMeshFilter* getRoiFilter() const
  { return m_roiFilter.get(); }

  inline Z3DMeshFilter* getDecorationFilter() const
  { return m_decorationFilter.get(); }

  inline Z3D2DSliceFilter* getSliceFilter() const
  {
    return m_sliceFilter.get();
  }

  Z3DGeometryFilter* getFilter(neutu3d::ERendererLayer layer) const;
  Z3DBoundedFilter* getBoundedFilter(neutu3d::ERendererLayer layer) const;

  inline Z3DNetworkEvaluator& getNetworkEvaluator() {
    return *m_networkEvaluator;
  }

  QWidget* globalParasWidget();

  QWidget* captureWidget();

  QWidget* backgroundWidget();

  QWidget* axisWidget();

  template <typename T>
  std::shared_ptr<ZWidgetsGroup> getWidgetsGroup(T *filter);

  std::shared_ptr<ZWidgetsGroup> getWidgetsGroup(neutu3d::ERendererLayer layer);

  std::shared_ptr<ZWidgetsGroup> globalParasWidgetsGroup();

  std::shared_ptr<ZWidgetsGroup> captureWidgetsGroup();

  std::shared_ptr<ZWidgetsGroup> backgroundWidgetsGroup();

  std::shared_ptr<ZWidgetsGroup> axisWidgetsGroup();

  void updateBoundBox();

  bool takeFixedSizeScreenShot(const QString& filename, int width, int height, Z3DScreenShotType sst);

  bool takeScreenShot(const QString& filename, Z3DScreenShotType sst);

  void resetCamera();  // set up camera based on visible objects in scene, original position
  void resetCameraCenter();
  void resetCameraClippingRange(); // Reset the camera clipping range to include this entire bounding box

//  Z3DTransformParameter getTransformPara() const;

  glm::quat getRotation() const;

  void gotoPosition(double x, double y, double z, double radius = 64);
  void gotoPosition(const ZBBox<glm::dvec3>& bound, double minRadius = 64);
  void exploreLocal(std::vector<ZPoint> &ptArray);

  void flipView(); //Look from the oppsite side
  void setXZView();
  void setYZView();

  const ZBBox<glm::dvec3>& boundBox() const
  { return m_boundBox; }

public:
  void setCutBox(neutu3d::ERendererLayer layer, const ZIntCuboid &box);
  void resetCutBox(neutu3d::ERendererLayer layer);

  QPointF getScreenProjection(
      double x, double y, double z, neutu3d::ERendererLayer layer);

  ZJsonObject getSettings() const;

public slots:
  void processObjectModified(const ZStackObjectInfoSet &objInfo);

  void dump(const QString &message);

signals:
  void networkConstructed();
  void cancelingLocalExplore();
  void exploringLocal(double x, double y, double z);

private:
  void zoomIn();

  void zoomOut();

  bool takeFixedSizeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                                     bool clockWise, int numFrame, int width, int height,
                                     Z3DScreenShotType sst);

  bool takeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                            bool clockWise, int numFrame, Z3DScreenShotType sst);

  void init();

  void createActions();

  void volumeDataChanged();
//  void punctaDataChanged();
//  void swcDataChanged();
//  void meshDataChanged();
//  void swcNetworkDataChanged();
//  void graph3DDataChanged();
//  void todoDataChanged();
//  void surfaceDataChanged();
  void objectSelectionChanged(const QList<ZStackObject*>& selected,
                              const QList<ZStackObject*>& deselected);
  void processObjectSelectionChanged(const ZStackObjectInfoSet &selected,
                                     const ZStackObjectInfoSet &deselected);

  void initVolumeFilter();
  void initPunctaFilter();
  void initSwcFilter();
  void initMeshFilter();
  void initGraphFilter();
  void initTodoFilter();
  void initSurfaceFilter();
  void initRoiFilter();
  void initDecorationFilter();
  void initSliceFilter();

  void addFilter(neutu3d::ERendererLayer layer);

  bool allowingNormalVolume() const;
  void updateVolumeData();
//  void updateSliceData();

  /*
  void updateGraphData();
  void updateSwcData();
  void updatePunctaData();
  void updateSurfaceData();
  void updateTodoData();
  void updateMeshData();
  void updateRoiData();
  */

private:
  ZStackDoc* m_doc;
  bool m_isStereoView;

  //
  QAction* m_zoomInAction;
  QAction* m_zoomOutAction;
  QAction* m_resetCameraAction;

  Z3DCanvas* m_canvas;
  std::unique_ptr<Z3DGlobalParameters> m_globalParas;
  std::unique_ptr<Z3DCanvasPainter> m_canvasPainter;
  std::unique_ptr<Z3DCompositor> m_compositor;

  std::unique_ptr<Z3DVolumeFilter> m_volumeFilter;
  std::unique_ptr<Z3DPunctaFilter> m_punctaFilter;
  std::unique_ptr<Z3DSwcFilter> m_swcFilter;
  std::unique_ptr<Z3DMeshFilter> m_meshFilter;
  std::unique_ptr<Z3DGraphFilter> m_graphFilter;
  std::unique_ptr<Z3DSurfaceFilter> m_surfaceFilter;
  std::unique_ptr<ZFlyEmTodoListFilter> m_todoFilter;
  std::unique_ptr<Z3DMeshFilter> m_roiFilter;
  std::unique_ptr<Z3DMeshFilter> m_decorationFilter;
  std::unique_ptr<Z3D2DSliceFilter> m_sliceFilter;

  std::unique_ptr<Z3DNetworkEvaluator> m_networkEvaluator;

  ZBBox<glm::dvec3> m_boundBox;
  QList<neutu3d::ERendererLayer> m_layerList;
  std::vector<Z3DBoundedFilter*> m_allFilters;

//  ZStackDoc3dHelper m_docHelper;

  bool m_lock;
  EInitMode m_initMode;

  QRubberBand* m_rubberBand;
  QPoint m_rubberBandOrigin;
};

#endif // Z3DVIEW_H
