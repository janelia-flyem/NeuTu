#ifndef Z3DVIEW_H
#define Z3DVIEW_H

#include "z3dglobalparameters.h"
#include "zbbox.h"
#include "zstackdoc.h"
#include <QDir>
#include <QObject>
#include <QAction>

class QMainWindow;

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

class Z3DView : public QObject
{
Q_OBJECT
public:
  enum class InitMode {
    NORMAL, EXCLUDE_VOLUME, FULL_RES_VOLUME
  };

  Z3DView(ZStackDoc* doc, InitMode initMode, bool stereo, QMainWindow* parent = nullptr);

  ~Z3DView();

  inline QAction* zoomInAction()
  { return m_zoomInAction; }

  inline QAction* zoomOutAction()
  { return m_zoomOutAction; }

  inline QAction* resetCameraAction()
  { return m_resetCameraAction; }

  Z3DCameraParameter& camera()
  { return m_globalParas.camera; }

  const Z3DCameraParameter& camera() const
  { return m_globalParas.camera; }

  Z3DTrackballInteractionHandler& interactionHandler()
  { return m_globalParas.interactionHandler; }

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
  { return m_globalParas; }

  inline Z3DVolumeFilter& volumeFilter()
  { return *m_volumeFilter; }

  inline Z3DPunctaFilter& punctaFilter()
  { return *m_punctaFilter; }

  inline Z3DSwcFilter& swcFilter()
  { return *m_swcFilter; }

  inline Z3DMeshFilter& meshFilter()
  { return *m_meshFilter; }

  inline Z3DGraphFilter& graphFilter()
  { return *m_graphFilter; }

  inline Z3DSurfaceFilter& surfaceFilter()
  { return *m_surfaceFilter; }

  inline ZFlyEmTodoListFilter& todoFilter()
  { return *m_todoFilter; }

  QWidget* globalParasWidget();

  QWidget* captureWidget();

  QWidget* backgroundWidget();

  QWidget* axisWidget();

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

  void gotoPosition(double x, double y, double z, double radius = 64);

  void gotoPosition(const ZBBox<glm::dvec3>& bound, double minRadius = 64);

  void flipView(); //Look from the oppsite side
  void setXZView();
  void setYZView();

  const ZBBox<glm::dvec3>& boundBox() const
  { return m_boundBox; }

signals:

  void objViewReady(size_t id);

private:
  void zoomIn();

  void zoomOut();

  bool takeFixedSizeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                                     bool clockWise, int numFrame, int width, int height,
                                     Z3DScreenShotType sst);

  bool takeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                            bool clockWise, int numFrame, Z3DScreenShotType sst);

  void init(InitMode initMode);

  void createActions();

  void volumeDataChanged();
  void punctaDataChanged();
  void swcDataChanged();
  void meshDataChanged();
  void swcNetworkDataChanged();
  void graph3DDataChanged();
  void todoDataChanged();

  void objectSelectionChanged(const QList<ZStackObject*>& selected,
                              const QList<ZStackObject*>& deselected);

private:
  ZStackDoc* m_doc;
  bool m_isStereoView;
  QMainWindow* m_mainWin;

  //
  QAction* m_zoomInAction;
  QAction* m_zoomOutAction;
  QAction* m_resetCameraAction;

  std::unique_ptr<Z3DNetworkEvaluator> m_networkEvaluator;
  Z3DGlobalParameters m_globalParas;
  Z3DCanvas* m_canvas;
  std::unique_ptr<Z3DCanvasPainter> m_canvasPainter;
  std::unique_ptr<Z3DCompositor> m_compositor;

  std::unique_ptr<Z3DVolumeFilter> m_volumeFilter;
  std::unique_ptr<Z3DPunctaFilter> m_punctaFilter;
  std::unique_ptr<Z3DSwcFilter> m_swcFilter;
  std::unique_ptr<Z3DMeshFilter> m_meshFilter;
  std::unique_ptr<Z3DGraphFilter> m_graphFilter;
  std::unique_ptr<Z3DSurfaceFilter> m_surfaceFilter;
  std::unique_ptr<ZFlyEmTodoListFilter> m_todoFilter;
//  std::unique_ptr<Z3DGraphFilter> m_decorationFilter;

  ZBBox<glm::dvec3> m_boundBox;
  std::vector<Z3DBoundedFilter*> m_allFilters;

  bool m_lock;
  InitMode m_initMode;
};

#endif // Z3DVIEW_H
