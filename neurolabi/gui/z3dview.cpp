#include "z3dview.h"

#include "z3dcanvas.h"
#include "z3dcompositor.h"
#include "z3dcanvaspainter.h"
#include "z3dcameraparameter.h"
#include "z3dinteractionhandler.h"
#include "z3dnetworkevaluator.h"

#include "z3dvolumefilter.h"
#include "z3dpunctafilter.h"
#include "z3dswcfilter.h"
#include "z3dmeshfilter.h"
#include "z3dgraphfilter.h"
#include "z3dsurfacefilter.h"
#include "flyem/zflyemtodolistfilter.h"

#include "zfiletype.h"
#include "flyem/zflyembody3ddoc.h"
#include "neutubeconfig.h"
#include "zswcnetwork.h"

#include "zsysteminfo.h"
#include "zwidgetsgroup.h"
#include "ztakescreenshotwidget.h"
#include "z3dgpuinfo.h"
#include "z3dmainwindow.h"
#include <QMessageBox>
#include <QProgressDialog>
#include <QMainWindow>
#include <QScrollArea>
#include <boost/math/constants/constants.hpp>

Z3DView::Z3DView(ZStackDoc* doc, InitMode initMode, bool stereo, QMainWindow* parent)
  : QObject(parent)
  , m_doc(doc)
  , m_isStereoView(stereo)
  , m_mainWin(parent)
  , m_lock(false)
  , m_initMode(initMode)
{
  CHECK(m_doc);
  m_canvas = new Z3DCanvas("", 512, 512);
  init(initMode);

  createActions();
}

Z3DView::~Z3DView()
{
  m_canvas->setNetworkEvaluator(nullptr);
}

QWidget* Z3DView::globalParasWidget()
{
  return m_globalParas.widgetsGroup(true)->createWidget(false);
}

QWidget* Z3DView::captureWidget()
{
  auto res = new QScrollArea();
  auto m_screenShotWidget = new ZTakeScreenShotWidget(false, false, nullptr);
  m_screenShotWidget->setCaptureStereoImage(m_isStereoView);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::take3DScreenShot,
          this, &Z3DView::takeScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeFixedSize3DScreenShot,
          this, &Z3DView::takeFixedSizeScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeSeries3DScreenShot,
          this, &Z3DView::takeSeriesScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeSeriesFixedSize3DScreenShot,
          this, &Z3DView::takeFixedSizeSeriesScreenShot);
  res->setWidget(m_screenShotWidget);

  return res;
}

QWidget* Z3DView::backgroundWidget()
{
  return m_compositor->backgroundWidgetsGroup()->createWidget(false);
}

QWidget* Z3DView::axisWidget()
{
  return m_compositor->axisWidgetsGroup()->createWidget(false);
}

std::shared_ptr<ZWidgetsGroup> Z3DView::globalParasWidgetsGroup()
{
  return m_globalParas.widgetsGroup(true);
}

std::shared_ptr<ZWidgetsGroup> Z3DView::captureWidgetsGroup()
{
  auto res = new QScrollArea();
  auto m_screenShotWidget = new ZTakeScreenShotWidget(false, false, nullptr);
  m_screenShotWidget->setCaptureStereoImage(m_isStereoView);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::take3DScreenShot,
          this, &Z3DView::takeScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeFixedSize3DScreenShot,
          this, &Z3DView::takeFixedSizeScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeSeries3DScreenShot,
          this, &Z3DView::takeSeriesScreenShot);
  connect(m_screenShotWidget, &ZTakeScreenShotWidget::takeSeriesFixedSize3DScreenShot,
          this, &Z3DView::takeFixedSizeSeriesScreenShot);
  res->setWidget(m_screenShotWidget);

  auto capture = std::make_shared<ZWidgetsGroup>("Capture", 1);
  capture->addChild(*res, 1);
  return capture;
}

std::shared_ptr<ZWidgetsGroup> Z3DView::backgroundWidgetsGroup()
{
  return m_compositor->backgroundWidgetsGroup();
}

std::shared_ptr<ZWidgetsGroup> Z3DView::axisWidgetsGroup()
{
  return m_compositor->axisWidgetsGroup();
}

void Z3DView::updateBoundBox()
{
  m_boundBox.reset();
  for (auto flt : m_allFilters) {
    if (flt->isVisible()) {
      m_boundBox.expand(flt->axisAlignedBoundBox());
    }
  }
  if (m_boundBox.empty()) {
    // nothing visible
    m_boundBox.setMinCorner(glm::dvec3(0.0));
    m_boundBox.setMaxCorner(glm::dvec3(1.0));
  }
  m_boundBox.setMaxCorner(glm::max(m_boundBox.maxCorner(), m_boundBox.minCorner() + 1.0));
  resetCameraClippingRange();
}

void Z3DView::zoomIn()
{
  camera().dolly(1.1);
  //resetCameraClippingRange();
}

void Z3DView::zoomOut()
{
  camera().dolly(0.9);
  //resetCameraClippingRange();
}

void Z3DView::resetCamera()
{
  camera().resetCamera(m_boundBox, Z3DCamera::ResetOption::ResetAll);
}

void Z3DView::resetCameraCenter()
{
  camera().resetCamera(m_boundBox, Z3DCamera::ResetOption::PreserveViewVector);
}

void Z3DView::resetCameraClippingRange()
{
  if (m_lock)
    return;
  m_lock = true;
  camera().resetCameraNearFarPlane(m_boundBox);
  m_lock = false;
}

bool Z3DView::takeFixedSizeScreenShot(const QString& filename, int width, int height, Z3DScreenShotType sst)
{
  bool res = true;
  m_lock = true;
  if (!m_canvasPainter->renderToImage(filename, width, height, sst, compositor())) {
    res = false;
    QMessageBox::critical(m_mainWin, qApp->applicationName(), m_canvasPainter->renderToImageError());
  }
  m_lock = false;
  return res;
}

bool Z3DView::takeScreenShot(const QString& filename, Z3DScreenShotType sst)
{
  int h = m_canvas->height();
  if (h % 2 == 1) {
    ++h;
  }
  int w = m_canvas->width();
  if (w % 2 == 1) {
    ++w;
  }
  if (m_canvas->width() % 2 == 1 || m_canvas->height() % 2 == 1) {
    LOG(INFO) << "Resize canvas size from (" << m_canvas->width() << ", " << m_canvas->height() << ") to (" << w << ", "
              << h << ").";
    m_canvas->resize(w, h);
  }
  bool res = true;
  if (!m_canvasPainter->renderToImage(filename, sst)) {
    res = false;
    QMessageBox::critical(m_mainWin, qApp->applicationName(), m_canvasPainter->renderToImageError());
  }
  return res;
}

void Z3DView::gotoPosition(double x, double y, double z, double radius)
{
  ZBBox<glm::dvec3> bound(glm::dvec3(x, y, z) - radius, glm::dvec3(x, y, z) + radius);
  camera().resetCamera(bound, Z3DCamera::ResetOption::ResetAll);
}

void Z3DView::gotoPosition(const ZBBox<glm::dvec3>& bound, double minRadius)
{
  glm::dvec3 cent = (bound.minCorner() + bound.maxCorner()) / 2.;
  auto bd = bound;
  bd.expand(ZBBox<glm::dvec3>(cent - minRadius, cent + minRadius));
  camera().resetCamera(bd, Z3DCamera::ResetOption::PreserveViewVector);
}

void Z3DView::flipView()
{
  camera().flipViewDirection();
}

void Z3DView::setXZView()
{
  resetCamera();
  camera().rotate90X();
  resetCameraClippingRange();
}

void Z3DView::setYZView()
{
  resetCamera();
  camera().rotate90XZ();
  resetCameraClippingRange();
}

bool Z3DView::takeFixedSizeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                                            bool clockWise, int numFrame, int width, int height, Z3DScreenShotType sst)
{
  using namespace boost::math::double_constants;
  QString title = "Capturing Images...";
  if (sst == Z3DScreenShotType::HalfSideBySideStereoView) {
    title = "Capturing Half Side-By-Side Stereo Images...";
  } else if (sst == Z3DScreenShotType::FullSideBySideStereoView) {
    title = "Capturing Full Side-By-Side Stereo Images...";
  }
  QProgressDialog progress(title, "Cancel", 0, numFrame, m_mainWin);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = two_pi / numFrame;
  bool res = true;
  for (int i = 0; i < numFrame; ++i) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      camera().rotate(rAngle, camera().get().vectorEyeToWorld(axis), camera().get().center());
    else
      camera().rotate(-rAngle, camera().get().vectorEyeToWorld(axis), camera().get().center());
    //resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    if (!takeFixedSizeScreenShot(filepath, width, height, sst)) {
      res = false;
      break;
    }
  }
  progress.setValue(numFrame);
  return res;
}

bool Z3DView::takeSeriesScreenShot(const QDir& dir, const QString& namePrefix, const glm::vec3& axis,
                                   bool clockWise, int numFrame, Z3DScreenShotType sst)
{
  using namespace boost::math::double_constants;
  QString title = "Capturing Images...";
  if (sst == Z3DScreenShotType::HalfSideBySideStereoView) {
    title = "Capturing Half Side-By-Side Stereo Images...";
  } else if (sst == Z3DScreenShotType::FullSideBySideStereoView) {
    title = "Capturing Full Side-By-Side Stereo Images...";
  }
  QProgressDialog progress(title, "Cancel", 0, numFrame, m_mainWin);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();
  double rAngle = two_pi / numFrame;
  bool res = true;
  for (int i = 0; i < numFrame; ++i) {
    progress.setValue(i);
    if (progress.wasCanceled())
      break;

    if (clockWise)
      camera().rotate(rAngle, camera().get().vectorEyeToWorld(axis), camera().get().center());
    else
      camera().rotate(-rAngle, camera().get().vectorEyeToWorld(axis), camera().get().center());
    //resetCameraClippingRange();
    int fieldWidth = numDigits(numFrame);
    QString filename = QString("%1%2.tif").arg(namePrefix).arg(i, fieldWidth, 10, QChar('0'));
    QString filepath = dir.filePath(filename);
    if (!takeScreenShot(filepath, sst)) {
      res = false;
      break;
    }
  }
  progress.setValue(numFrame);
  return res;
}

void Z3DView::init(InitMode initMode)
{
  try {
    m_globalParas.setCanvas(m_canvas);

    // filters
    m_compositor.reset(new Z3DCompositor(m_globalParas));

    m_canvasPainter.reset(new Z3DCanvasPainter(m_globalParas));
    m_canvasPainter->setCanvas(m_canvas);

    m_canvas->addEventListenerToBack(m_compositor.get());  // for interaction

    m_compositor->outputPort("Image")->connect(m_canvasPainter->inputPort("Image"));
    m_compositor->outputPort("LeftEyeImage")->connect(m_canvasPainter->inputPort("LeftEyeImage"));
    m_compositor->outputPort("RightEyeImage")->connect(m_canvasPainter->inputPort("RightEyeImage"));

    // connection: canvas <-----> networkevaluator <-----> canvasrender
    m_networkEvaluator.reset(new Z3DNetworkEvaluator());
    m_canvas->setNetworkEvaluator(m_networkEvaluator.get());

    // build network
    m_volumeFilter.reset(new Z3DVolumeFilter(m_globalParas));
    m_volumeFilter->outputPort("Image")->connect(m_compositor->inputPort("Image"));
    m_volumeFilter->outputPort("LeftEyeImage")->connect(m_compositor->inputPort("LeftEyeImage"));
    m_volumeFilter->outputPort("RightEyeImage")->connect(m_compositor->inputPort("RightEyeImage"));
    m_volumeFilter->outputPort("VolumeFilter")->connect(m_compositor->inputPort("VolumeFilters"));
    connect(m_volumeFilter.get(), &Z3DVolumeFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_volumeFilter.get(), &Z3DVolumeFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_volumeFilter.get());
    m_allFilters.push_back(m_volumeFilter.get());

    m_punctaFilter.reset(new Z3DPunctaFilter(m_globalParas));
    m_punctaFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
    connect(m_punctaFilter.get(), &Z3DPunctaFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_punctaFilter.get(), &Z3DPunctaFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_punctaFilter.get());
    m_allFilters.push_back(m_punctaFilter.get());

    m_swcFilter = new Z3DSwcFilter(m_globalParas, this);
    m_swcFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
    connect(m_swcFilter, &Z3DSwcFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_swcFilter, &Z3DSwcFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_swcFilter);
    m_allFilters.push_back(m_swcFilter);

    m_meshFilter.reset(new Z3DMeshFilter(m_globalParas));
    m_meshFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
    connect(m_meshFilter.get(), &Z3DMeshFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_meshFilter.get(), &Z3DMeshFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_meshFilter.get());
    m_allFilters.push_back(m_meshFilter.get());

    m_graphFilter.reset(new Z3DGraphFilter(m_globalParas));
    m_graphFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
    connect(m_graphFilter.get(), &Z3DGraphFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_graphFilter.get(), &Z3DGraphFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_graphFilter.get());
    m_allFilters.push_back(m_graphFilter.get());

#if defined _FLYEM_
    m_todoFilter = new ZFlyEmTodoListFilter(m_globalParas, this);
    m_todoFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
    connect(m_todoFilter, &ZFlyEmTodoListFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
    connect(m_todoFilter, &ZFlyEmTodoListFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
    m_canvas->addEventListenerToBack(m_todoFilter);
    m_allFilters.push_back(m_todoFilter);
#endif

    // get data from doc and add to network
    // volume
    if (initMode != InitMode::EXCLUDE_VOLUME) {
      volumeDataChanged();
      connect(m_doc, &ZStackDoc::volumeModified, this, &Z3DView::volumeDataChanged);
    }
    // puncta
    punctaDataChanged();
    connect(m_doc, &ZStackDoc::punctaModified, this, &Z3DView::punctaDataChanged);
    // swc
    swcDataChanged();
    connect(m_doc, &ZStackDoc::swcModified, this, &Z3DView::swcDataChanged);
    // mesh
    meshDataChanged();
    connect(m_doc, &ZStackDoc::meshModified, this, &Z3DView::meshDataChanged);
    // graph
    swcNetworkDataChanged();
    graph3DDataChanged();
    connect(m_doc, &ZStackDoc::swcNetworkModified, this, &Z3DView::swcNetworkDataChanged);
    connect(m_doc, &ZStackDoc::graph3dModified, this, &Z3DView::graph3DDataChanged);
    // tododata
    todoDataChanged();
    connect(m_doc, &ZStackDoc::todoModified, this, &Z3DView::todoDataChanged);

    connect(m_doc, &ZStackDoc::objectSelectionChanged, this, &Z3DView::objectSelectionChanged);
    connect(m_doc, &ZStackDoc::punctaSelectionChanged, m_punctaFilter.get(), &Z3DPunctaFilter::invalidateResult);
    connect(m_doc, &ZStackDoc::punctumVisibleStateChanged, m_punctaFilter.get(), &Z3DPunctaFilter::updatePunctumVisibleState);
    connect(m_doc, &ZStackDoc::meshSelectionChanged, m_meshFilter.get(), &Z3DMeshFilter::invalidateResult);
    connect(m_doc, &ZStackDoc::meshVisibleStateChanged, m_meshFilter.get(), &Z3DMeshFilter::updateMeshVisibleState);
    connect(m_doc, &ZStackDoc::swcSelectionChanged, m_swcFilter, &Z3DSwcFilter::invalidateResult);
    connect(m_doc, &ZStackDoc::swcVisibleStateChanged, m_swcFilter, &Z3DSwcFilter::updateSwcVisibleState);
    connect(m_doc, QOverload<QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>>::of(&ZStackDoc::swcTreeNodeSelectionChanged),
            m_swcFilter, &Z3DSwcFilter::invalidateResult);
    connect(m_doc, &ZStackDoc::graphVisibleStateChanged, this, &Z3DView::graph3DDataChanged); // todo: fix this?

    if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isAxisOn()) {
      m_compositor->setShowAxis(false);
    }
#if defined(REMOTE_WORKSTATION)
    m_compositor->setShowBackground(false);
#endif

    // pass the canvasrender to the network evaluator and build network
    m_networkEvaluator->setNetworkSink(m_canvasPainter.get());
    // initializes all connected filters
    m_networkEvaluator->initializeNetwork();

    updateBoundBox();

    // adjust camera
    resetCamera();

    connect(&camera(), &Z3DCameraParameter::valueChanged, this, &Z3DView::resetCameraClippingRange);
  }
  catch (const ZException& e) {
    LOG(ERROR) << "Failed to init 3DView: " << e.what();
    QMessageBox::critical(m_canvas, qApp->applicationName(), "Failed to init 3DView.\n" + e.what());
  }
}

void Z3DView::createActions()
{
  m_zoomInAction = new QAction(QIcon(":/icons/zoom_in-512.png"), tr("Zoom &In"), this);
  QList<QKeySequence> zoomInKey;
  zoomInKey << QKeySequence::ZoomIn << QKeySequence(Qt::Key_Plus) << QKeySequence(Qt::Key_Equal);
  m_zoomInAction->setShortcuts(zoomInKey);
  m_zoomInAction->setStatusTip(tr("Zoom in"));
  connect(m_zoomInAction, &QAction::triggered, this, &Z3DView::zoomIn);

  m_zoomOutAction = new QAction(QIcon(":/icons/zoom_out-512.png"), tr("Zoom &Out"), this);
  QList<QKeySequence> zoomOutKey;
  zoomOutKey << QKeySequence::ZoomOut << QKeySequence(Qt::Key_Minus);
  m_zoomOutAction->setShortcuts(zoomOutKey);
  m_zoomOutAction->setStatusTip(tr("Zoom out"));
  connect(m_zoomOutAction, &QAction::triggered, this, &Z3DView::zoomOut);

  m_resetCameraAction = new QAction(tr("&Reset Camera"), this);
  m_resetCameraAction->setStatusTip(tr("Reset camera to show all objects in scene"));
  connect(m_resetCameraAction, &QAction::triggered, this, &Z3DView::resetCamera);
}

void Z3DView::volumeDataChanged()
{
  if (m_initMode == InitMode::NORMAL) {
    m_volumeFilter->setData(m_doc);
  } else if (m_initMode == InitMode::FULL_RES_VOLUME) {
    m_volumeFilter->setData(m_doc, std::numeric_limits<int>::max() / 2);
  }
}

void Z3DView::punctaDataChanged()
{
  m_punctaFilter->setData(m_doc->getPunctumList());
}

void Z3DView::swcDataChanged()
{
  m_swcFilter->setData(m_doc->getSwcList());
}

void Z3DView::meshDataChanged()
{
  m_meshFilter->setData(m_doc->getMeshList());
}

void Z3DView::swcNetworkDataChanged()
{
  if (m_doc->swcNetwork()) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    m_graphFilter->setData(*network, NULL);
    delete network;
  } else if (ZFileType::FileType(m_doc->additionalSource()) == ZFileType::FILE_JSON) {
    Z3DGraph graph;
    graph.importJsonFile(m_doc->additionalSource());
    m_graphFilter->setData(graph);
  }
}

void Z3DView::graph3DDataChanged()
{
  m_graphFilter->setData(m_doc->get3DGraphDecoration());
  TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
  for (TStackObjectList::const_iterator iter = objList.begin(); iter != objList.end(); ++iter) {
    Z3DGraph *graph = dynamic_cast<Z3DGraph*>(*iter);
    if (graph->isVisible()) {
      m_graphFilter->addData(*graph);
    }
  }
}

void Z3DView::todoDataChanged()
{
#if defined(_FLYEM_)
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(m_doc);
  if (doc) {
    QList<ZFlyEmToDoItem*> objList = doc->getObjectList<ZFlyEmToDoItem>();
    m_todoFilter->setData(objList);
/*
    const TStackObjectList& objList =
        doc->getObjectList(ZStackObject::TYPE_FLYEM_TODO_ITEM);
//        doc->getDataDocument()->getObjectList(ZStackObject::TYPE_FLYEM_TODO_LIST);
    if (!objList.isEmpty()) {
      m_todoFilter->setData(objList.front());
    }
    */
  }
#endif
}

void Z3DView::objectSelectionChanged(const QList<ZStackObject*>& selected,
                                     const QList<ZStackObject*>& deselected)
{
  QSet<ZStackObject::EType> typeSet;
  for (QList<ZStackObject*>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    ZStackObject *obj = *iter;
    typeSet.insert(obj->getType());
  }

  for (QList<ZStackObject*>::const_iterator iter = deselected.begin();
       iter != deselected.end(); ++iter) {
    ZStackObject *obj = *iter;
    typeSet.insert(obj->getType());
  }

  for (QSet<ZStackObject::EType>::const_iterator iter = typeSet.begin();
       iter != typeSet.end(); ++iter) {
    ZStackObject::EType  type = *iter;
    switch (type) {
    case ZStackObject::TYPE_SWC:
      m_swcFilter->invalidate();
      break;
    case ZStackObject::TYPE_PUNCTA:
      m_punctaFilter->invalidate();
      break;
    case ZStackObject::TYPE_FLYEM_TODO_ITEM:
      m_todoFilter->invalidate();
      break;
    case ZStackObject::TYPE_MESH:
      m_meshFilter->invalidate();
      break;
    default:
      break;
    }
  }
}

int Z3DView::getDevicePixelRatio() const
{
  return m_mainWin->devicePixelRatio();
}

void Z3DView::updateNetwork()
{
  getNetworkEvaluator().process();
}
