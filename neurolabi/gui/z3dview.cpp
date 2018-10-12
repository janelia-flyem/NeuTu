#include "z3dview.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>

#include <boost/math/constants/constants.hpp>

#include "zqslog.h"
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
#include "z3d2dslicefilter.h"

#include "zfiletype.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembody3ddockeyprocessor.h"
#include "neutubeconfig.h"
#include "zswcnetwork.h"

#include "zsysteminfo.h"
#include "zwidgetsgroup.h"
#include "ztakescreenshotwidget.h"
#include "z3dgpuinfo.h"
#include "z3dmainwindow.h"
#include "z3dcontext.h"
#include "zstackdoc3dhelper.h"
#include "core/utilities.h"

Z3DView::Z3DView(ZStackDoc* doc, EInitMode initMode, bool stereo, QWidget* parent)
  : QObject(parent)
  , m_doc(doc)
  , m_isStereoView(stereo)
  , m_lock(false)
  , m_initMode(initMode)
{
//  CHECK(m_doc);
  m_canvas = new Z3DCanvas("", 512, 512, parent);
//  m_docHelper.attach(this);
//  ZStackDoc3dHelper::Attach(m_doc, this);

  createActions();

  m_canvas->getInteractionEngine()->setKeyProcessor(
        doc->getKeyProcessor());

  connect(m_canvas, &Z3DCanvas::openGLContextInitialized, this, &Z3DView::init);
}

Z3DView::~Z3DView()
{
  m_canvas->getGLFocus();
}

QWidget* Z3DView::globalParasWidget()
{
  return m_globalParas->widgetsGroup(true)->createWidget(false);
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


template <typename T>
std::shared_ptr<ZWidgetsGroup> Z3DView::getWidgetsGroup(T *filter)
{
  if (filter != NULL) {
    return filter->widgetsGroup();
  }

  return std::shared_ptr<ZWidgetsGroup>();
}

std::shared_ptr<ZWidgetsGroup> Z3DView::getWidgetsGroup(
    neutube3d::ERendererLayer layer)
{
  switch (layer) {
  case neutube3d::LAYER_GRAPH:
    return getWidgetsGroup(getGraphFilter());
  case neutube3d::LAYER_MESH:
    return getWidgetsGroup(getMeshFilter());
  case neutube3d::LAYER_PUNCTA:
    return getWidgetsGroup(getPunctaFilter());
  case neutube3d::LAYER_ROI:
    return getWidgetsGroup(getRoiFilter());
  case neutube3d::LAYER_SURFACE:
    return getWidgetsGroup(getSurfaceFilter());
  case neutube3d::LAYER_SWC:
    return getWidgetsGroup(getSwcFilter());
  case neutube3d::LAYER_TODO:
    return getWidgetsGroup(getTodoFilter());
  case neutube3d::LAYER_VOLUME:
    return getWidgetsGroup(getVolumeFilter());
  case neutube3d::LAYER_DECORATION:
    return getWidgetsGroup(getDecorationFilter());
    break;
  case neutube3d::LAYER_SLICE:
    return getWidgetsGroup(getSliceFilter());
    break;
  }

  return std::shared_ptr<ZWidgetsGroup>();
}

std::shared_ptr<ZWidgetsGroup> Z3DView::globalParasWidgetsGroup()
{
  return m_globalParas->widgetsGroup(true);
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
#ifdef _DEBUG_2
  std::cout << "Updating bounding box:" << std::endl;
#endif
  for (Z3DBoundedFilter* flt : m_allFilters) {
#ifdef _DEBUG_
    std::cout << "Getting bounding box of " << flt->className().toStdString() << std::endl;
#endif
    if (flt->isVisible()) {
      m_boundBox.expand(flt->axisAlignedBoundBox());
    }
  }
  if (m_boundBox.empty()) {
    // nothing visible
    m_boundBox.setMinCorner(glm::dvec3(0.0));
    m_boundBox.setMaxCorner(glm::dvec3(0.01));
  }
  m_boundBox.setMaxCorner(glm::max(m_boundBox.maxCorner(), m_boundBox.minCorner() + 0.01));
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

glm::quat Z3DView::getRotation() const
{
  return camera().getNeuroglancerRotation();
}

bool Z3DView::takeFixedSizeScreenShot(const QString& filename, int width, int height, Z3DScreenShotType sst)
{
  bool res = true;
  m_lock = true;
  if (!m_canvasPainter->renderToImage(filename, width, height, sst, compositor())) {
    res = false;
    QMessageBox::critical(m_canvas->parentWidget(), qApp->applicationName(), m_canvasPainter->renderToImageError());
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
    QMessageBox::critical(m_canvas->parentWidget(), qApp->applicationName(), m_canvasPainter->renderToImageError());
  }
  return res;
}

void Z3DView::gotoPosition(double x, double y, double z, double radius)
{
  ZBBox<glm::dvec3> bound(glm::dvec3(x, y, z) - radius, glm::dvec3(x, y, z) + radius);
  camera().resetCamera(bound, Z3DCamera::ResetOption::PreserveViewVector);
}

void Z3DView::gotoPosition(const ZBBox<glm::dvec3>& bound, double minRadius)
{
  glm::dvec3 cent = (bound.minCorner() + bound.maxCorner()) / 2.;
  auto bd = bound;
  bd.expand(ZBBox<glm::dvec3>(cent - minRadius, cent + minRadius));
  camera().resetCamera(bd, Z3DCamera::ResetOption::PreserveViewVector);
}

void Z3DView::exploreLocal(std::vector<ZPoint> &ptArray)
{
  if (ptArray.empty()) {
    emit cancelingLocalExplore();
  } else {
    ZPoint center = ptArray[0];
    if (ptArray.size() > 1) {
      center += ptArray[1];
      center *= 0.5;
    }
    emit exploringLocal(center.getX(), center.getY(), center.getZ());
  }
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
  QProgressDialog progress(title, "Cancel", 0, numFrame, m_canvas->parentWidget());
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
    int fieldWidth = neutube::numDigits(numFrame);
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
  QProgressDialog progress(title, "Cancel", 0, numFrame, m_canvas->parentWidget());
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
    int fieldWidth = neutube::numDigits(numFrame);
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

void Z3DView::init()
{
  try {
    m_canvas->getGLFocus();

    m_globalParas.reset(new Z3DGlobalParameters(*m_canvas));

    // filters
    m_canvasPainter.reset(new Z3DCanvasPainter(*m_globalParas, *m_canvas));

    m_compositor.reset(new Z3DCompositor(*m_globalParas));
    m_compositor->outputPort("Image")->connect(m_canvasPainter->inputPort("Image"));
    m_compositor->outputPort("LeftEyeImage")->connect(m_canvasPainter->inputPort("LeftEyeImage"));
    m_compositor->outputPort("RightEyeImage")->connect(m_canvasPainter->inputPort("RightEyeImage"));
    m_canvas->addEventListenerToBack(*m_compositor);

    // build network
    const NeutubeConfig &config = NeutubeConfig::getInstance();
    if (config.getZ3DWindowConfig().isVolumeOn()) {
      addFilter(neutube3d::LAYER_VOLUME);
    }
    if (config.getZ3DWindowConfig().isSwcsOn()) {
      addFilter(neutube3d::LAYER_SWC);
    }
#if !defined(_NEUTUBE_LIGHT_)
    if (config.getZ3DWindowConfig().isPunctaOn()) {
      addFilter(neutube3d::LAYER_PUNCTA);
    }
#endif
#if defined _FLYEM_
    addFilter(neutube3d::LAYER_TODO);
//    addFilter(neutube3d::LAYER_SURFACE);
#endif

    addFilter(neutube3d::LAYER_GRAPH);
    addFilter(neutube3d::LAYER_MESH);
    addFilter(neutube3d::LAYER_ROI);
    addFilter(neutube3d::LAYER_DECORATION);

//    initSurfaceFilter();

    connect(m_doc, SIGNAL(objectModified(ZStackObjectInfoSet)),
            this, SLOT(processObjectModified(ZStackObjectInfoSet)));

    // get data from doc and add to network
    // volume
    if (m_initMode != INIT_EXCLUDE_VOLUME) {
//      updateVolumeData();
      connect(m_doc, &ZStackDoc::volumeModified, this, &Z3DView::volumeDataChanged);
    }

    foreach (neutube3d::ERendererLayer layer, m_layerList) {
      updateDocData(layer);
    }

#if 0
    // puncta
    m_docHelper.updatePunctaData();
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
    // surface
    surfaceDataChanged();
    connect(m_doc, &ZStackDoc::cube3dModified, this, &Z3DView::surfaceDataChanged);
#endif

    connect(m_doc, &ZStackDoc::objectSelectionChanged, this, &Z3DView::objectSelectionChanged);
//    connect(m_doc, &ZStackDoc::graphVisibleStateChanged, this, &Z3DView::graph3DDataChanged); // todo: fix this?

    if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isAxisOn()) {
      m_compositor->setShowAxis(false);
    }
#if defined(REMOTE_WORKSTATION)
    m_compositor->setShowBackground(false);
#endif

    // build network and connect to canvas
    m_networkEvaluator.reset(new Z3DNetworkEvaluator(*m_canvasPainter));

    updateBoundBox();

    // adjust camera
    resetCamera();

    connect(&camera(), &Z3DCameraParameter::valueChanged,
            this, &Z3DView::resetCameraClippingRange);

//#ifdef _FLYEM_
//    m_canvas->getInteractionEngine()->setKeyProcessor(
//          new ZFlyEmBody3dDocKeyProcessor);
//#endif

    emit networkConstructed();

#ifdef _DEBUG_2
    m_canvas->setCustomWidget(new QLabel("test"));
#endif
  }
  catch (const ZException& e) {
    LOG(ERROR) << "Failed to init 3DView: " << e.what();
    QMessageBox::critical(m_canvas, qApp->applicationName(), "Failed to init 3DView.\n" + e.what());
  }
}

void Z3DView::addFilter(neutube3d::ERendererLayer layer)
{
  switch (layer) {
  case neutube3d::LAYER_GRAPH:
    initGraphFilter();
    break;
  case neutube3d::LAYER_MESH:
    initMeshFilter();
    break;
  case neutube3d::LAYER_PUNCTA:
    initPunctaFilter();
    break;
  case neutube3d::LAYER_ROI:
    initRoiFilter();
    break;
  case neutube3d::LAYER_SURFACE:
    initSurfaceFilter();
    break;
  case neutube3d::LAYER_SWC:
    initSwcFilter();
    break;
  case neutube3d::LAYER_TODO:
    initTodoFilter();
    break;
  case neutube3d::LAYER_VOLUME:
    initVolumeFilter();
    break;
  case neutube3d::LAYER_DECORATION:
    initDecorationFilter();
    break;
  case neutube3d::LAYER_SLICE:
    initSliceFilter();
    break;
  }
}

void Z3DView::initVolumeFilter()
{
  m_volumeFilter.reset(new Z3DVolumeFilter(*m_globalParas));
  m_volumeFilter->outputPort("Image")->connect(m_compositor->inputPort("Image"));
  m_volumeFilter->outputPort("LeftEyeImage")->connect(m_compositor->inputPort("LeftEyeImage"));
  m_volumeFilter->outputPort("RightEyeImage")->connect(m_compositor->inputPort("RightEyeImage"));
  m_volumeFilter->outputPort("VolumeFilter")->connect(m_compositor->inputPort("VolumeFilters"));
  connect(m_volumeFilter.get(), &Z3DVolumeFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_volumeFilter.get(), &Z3DVolumeFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_volumeFilter);
  m_allFilters.push_back(m_volumeFilter.get());
  m_layerList.append(neutube3d::LAYER_VOLUME);
}

void Z3DView::initSliceFilter()
{
  m_sliceFilter.reset(new Z3D2DSliceFilter(*m_globalParas));
  m_sliceFilter->outputPort("Image")->connect(m_compositor->inputPort("Image"));
  m_sliceFilter->outputPort("VolumeFilter")->connect(
        m_compositor->inputPort("VolumeFilters"));
  connect(m_sliceFilter.get(), &Z3D2DSliceFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_sliceFilter.get(), &Z3D2DSliceFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_sliceFilter);
  m_allFilters.push_back(m_sliceFilter.get());
  m_layerList.append(neutube3d::LAYER_SLICE);
}

void Z3DView::initPunctaFilter()
{
  m_punctaFilter.reset(new Z3DPunctaFilter(*m_globalParas));
  m_punctaFilter->setListenerName("Puncta filter");
  m_punctaFilter->outputPort("GeometryFilter")->connect(
        m_compositor->inputPort("GeometryFilters"));
  connect(m_punctaFilter.get(), &Z3DPunctaFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_punctaFilter.get(), &Z3DPunctaFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_punctaFilter);
  m_allFilters.push_back(m_punctaFilter.get());

  connect(m_doc, &ZStackDoc::punctaSelectionChanged,
          m_punctaFilter.get(), &Z3DPunctaFilter::invalidateResult);
  connect(m_doc, &ZStackDoc::punctumVisibleStateChanged,
          m_punctaFilter.get(), &Z3DPunctaFilter::updatePunctumVisibleState);
  m_layerList.append(neutube3d::LAYER_PUNCTA);
}

void Z3DView::initSwcFilter()
{
  m_swcFilter.reset(new Z3DSwcFilter(*m_globalParas));
  m_swcFilter->setListenerName("SWC filter");
  m_swcFilter->outputPort("GeometryFilter")->connect(
        m_compositor->inputPort("GeometryFilters"));
  connect(m_swcFilter.get(), &Z3DSwcFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_swcFilter.get(), &Z3DSwcFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_swcFilter);
  m_allFilters.push_back(m_swcFilter.get());

  connect(m_doc, &ZStackDoc::swcSelectionChanged,
          m_swcFilter.get(), &Z3DSwcFilter::invalidateResult);
  connect(m_doc, &ZStackDoc::swcVisibleStateChanged,
          m_swcFilter.get(), &Z3DSwcFilter::updateSwcVisibleState);
  connect(m_doc, QOverload<QList<Swc_Tree_Node*>,
          QList<Swc_Tree_Node*>>::of(&ZStackDoc::swcTreeNodeSelectionChanged),
          m_swcFilter.get(), &Z3DSwcFilter::invalidateResult);
  m_layerList.append(neutube3d::LAYER_SWC);
}

void Z3DView::initMeshFilter()
{
  m_meshFilter.reset(new Z3DMeshFilter(*m_globalParas));
  m_meshFilter->setListenerName("Mesh filter");
  m_meshFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
  connect(m_meshFilter.get(), &Z3DMeshFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
  connect(m_meshFilter.get(), &Z3DMeshFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_meshFilter);
  m_allFilters.push_back(m_meshFilter.get());

  connect(m_doc, &ZStackDoc::meshSelectionChanged,
          m_meshFilter.get(), &Z3DMeshFilter::invalidateResult);
  connect(m_doc, &ZStackDoc::meshVisibleStateChanged,
          m_meshFilter.get(), &Z3DMeshFilter::updateMeshVisibleState);
  m_layerList.append(neutube3d::LAYER_MESH);
}

void Z3DView::initRoiFilter()
{
  m_roiFilter.reset(new Z3DMeshFilter(*m_globalParas));
  m_roiFilter->setControlName("ROI");
  m_roiFilter->setListenerName("ROI filter");
  m_roiFilter->outputPort("GeometryFilter")->connect(
        m_compositor->inputPort("GeometryFilters"));
  m_roiFilter->setOpacity(0.2);
  m_roiFilter->setColorMode("Mesh Color");
  m_roiFilter->setMaterialSpecular(glm::vec4(0, 0, 0, 0));
  m_roiFilter->enablePicking(false);
  connect(m_roiFilter.get(), &Z3DMeshFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_roiFilter.get(), &Z3DMeshFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_roiFilter);
  m_allFilters.push_back(m_roiFilter.get());

  connect(m_doc, &ZStackDoc::meshSelectionChanged,
          m_roiFilter.get(), &Z3DMeshFilter::invalidateResult);
  connect(m_doc, &ZStackDoc::meshVisibleStateChanged,
          m_roiFilter.get(), &Z3DMeshFilter::updateMeshVisibleState);
  m_layerList.append(neutube3d::LAYER_ROI);
}

void Z3DView::initDecorationFilter()
{
  m_decorationFilter.reset(new Z3DMeshFilter(*m_globalParas));
  m_decorationFilter->setControlName("Decoration");
  m_decorationFilter->setListenerName("Decoration filter");
  m_decorationFilter->outputPort("GeometryFilter")->connect(
        m_compositor->inputPort("GeometryFilters"));
  m_decorationFilter->setOpacity(0.5);
  m_decorationFilter->setColorMode("Mesh Color");
  m_decorationFilter->setMaterialSpecular(glm::vec4(0, 0, 0, 0));
  m_decorationFilter->enablePicking(false);
  connect(m_decorationFilter.get(), &Z3DMeshFilter::boundBoxChanged,
          this, &Z3DView::updateBoundBox);
  connect(m_decorationFilter.get(), &Z3DMeshFilter::objVisibleChanged,
          this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_decorationFilter);
  m_allFilters.push_back(m_decorationFilter.get());

  connect(m_doc, &ZStackDoc::meshSelectionChanged,
          m_decorationFilter.get(), &Z3DMeshFilter::invalidateResult);
  connect(m_doc, &ZStackDoc::meshVisibleStateChanged,
          m_decorationFilter.get(), &Z3DMeshFilter::updateMeshVisibleState);
  m_layerList.append(neutube3d::LAYER_DECORATION);
}

void Z3DView::initGraphFilter()
{
  m_graphFilter.reset(new Z3DGraphFilter(*m_globalParas));
  m_graphFilter->setListenerName("Graph filter");
  m_graphFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
  connect(m_graphFilter.get(), &Z3DGraphFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
  connect(m_graphFilter.get(), &Z3DGraphFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_graphFilter);
  m_allFilters.push_back(m_graphFilter.get());
  m_layerList.append(neutube3d::LAYER_GRAPH);
}

void Z3DView::initTodoFilter()
{
  m_todoFilter.reset(new ZFlyEmTodoListFilter(*m_globalParas));
  m_todoFilter->setListenerName("Todo filter");
  m_todoFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
  connect(m_todoFilter.get(), &ZFlyEmTodoListFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
  connect(m_todoFilter.get(), &ZFlyEmTodoListFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_todoFilter);
  m_allFilters.push_back(m_todoFilter.get());
  m_layerList.append(neutube3d::LAYER_TODO);
}

void Z3DView::initSurfaceFilter()
{
  m_surfaceFilter.reset(new Z3DSurfaceFilter(*m_globalParas));
  m_surfaceFilter->outputPort("GeometryFilter")->connect(m_compositor->inputPort("GeometryFilters"));
  connect(m_surfaceFilter.get(), &Z3DSurfaceFilter::boundBoxChanged, this, &Z3DView::updateBoundBox);
  connect(m_surfaceFilter.get(), &Z3DSurfaceFilter::objVisibleChanged, this, &Z3DView::updateBoundBox);
  m_canvas->addEventListenerToBack(*m_surfaceFilter);
  m_allFilters.push_back(m_surfaceFilter.get());

  connect(m_doc, &ZStackDoc::surfaceVisibleStateChanged,
          m_surfaceFilter.get(), &Z3DSurfaceFilter::updateSurfaceVisibleState);
  m_layerList.append(neutube3d::LAYER_SURFACE);

}

void Z3DView::configureLayer(neutube3d::ERendererLayer layer, const ZJsonObject &obj)
{
  Z3DGeometryFilter *filter = getFilter(layer);
  if (filter != NULL) {
    filter->configure(obj);
  }
}

ZJsonObject Z3DView::getConfigJson(neutube3d::ERendererLayer layer) const
{
  ZJsonObject configJson;
  Z3DGeometryFilter *filter = getFilter(layer);
  if (filter != NULL) {
    configJson = filter->getConfigJson();
  }

  return configJson;
}

void Z3DView::configure(const ZJsonObject &obj)
{
  for (QList<neutube3d::ERendererLayer>::const_iterator iter = m_layerList.begin();
       iter != m_layerList.end(); ++iter) {
    neutube3d::ERendererLayer layer = *iter;
    std::string layerKey = GetLayerString(layer);
    if (obj.hasKey(layerKey.c_str())) {
      ZJsonObject layerJson(obj.value(layerKey.c_str()));
      configureLayer(layer, layerJson);
    }
  }
}

ZJsonObject Z3DView::getSettings() const
{
  ZJsonObject configJson;
  for (QList<neutube3d::ERendererLayer>::const_iterator iter = m_layerList.begin();
       iter != m_layerList.end(); ++iter) {
    neutube3d::ERendererLayer layer = *iter;
    ZJsonObject layerJson = getConfigJson(layer);
    configJson.setEntry(GetLayerString(layer).c_str(), layerJson);
  }

  return configJson;
}

void Z3DView::processObjectModified(const ZStackObjectInfoSet &objInfo)
{
  ZOUT(LTRACE(), 5) << "Processing object modification in Z3DView ...";

  ZStackDoc3dHelper helper;
//  helper.attach(this);
  helper.processObjectModified(objInfo, this);
}

void Z3DView::dump(const QString &message)
{
  m_canvas->dump(message);
}

void Z3DView::setCutBox(neutube3d::ERendererLayer layer, const ZIntCuboid &box)
{
  switch (layer) {
  case neutube3d::LAYER_SWC:
    getSwcFilter()->setCutBox(box);
    break;
  default:
    break;
  }
}

void Z3DView::resetCutBox(neutube3d::ERendererLayer layer)
{
  switch (layer) {
  case neutube3d::LAYER_SWC:
    getSwcFilter()->resetCut();
    break;
  default:
    break;
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
  updateVolumeData();
}

Z3DGeometryFilter* Z3DView::getFilter(neutube3d::ERendererLayer layer) const
{
  switch (layer) {
  case neutube3d::LAYER_SWC:
    return getSwcFilter();
  case neutube3d::LAYER_GRAPH:
    return getGraphFilter();
  case neutube3d::LAYER_PUNCTA:
    return getPunctaFilter();
  case neutube3d::LAYER_TODO:
    return getTodoFilter();
  case neutube3d::LAYER_SURFACE:
    return getSurfaceFilter();
  case neutube3d::LAYER_MESH:
    return getMeshFilter();
  case neutube3d::LAYER_ROI:
    return getRoiFilter();
  case neutube3d::LAYER_DECORATION:
    return getDecorationFilter();
  default:
    break;
  }

  return NULL;
}

Z3DBoundedFilter* Z3DView::getBoundedFilter(neutube3d::ERendererLayer layer) const
{
  switch (layer) {
  case neutube3d::LAYER_VOLUME:
    return getVolumeFilter();
  default:
    return getFilter(layer);
  }

  return NULL;
}

void Z3DView::setZScale(neutube3d::ERendererLayer layer, double scale)
{
  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    filter->setZScale(scale);
  }
}

void Z3DView::setScale(neutube3d::ERendererLayer layer, double sx, double sy, double sz)
{
  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    filter->setXScale(sx);
    filter->setYScale(sy);
    filter->setZScale(sz);
  }
}

void Z3DView::setOpacity(neutube3d::ERendererLayer layer, double opacity)
{
  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    filter->setOpacity(opacity);
  }
}

void Z3DView::setOpacityQuietly(neutube3d::ERendererLayer layer, double opacity)
{
  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    filter->setOpacityQuitely(opacity);
  }
}

void Z3DView::setLayerVisible(neutube3d::ERendererLayer layer, bool visible)
{
  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    filter->setVisible(visible);
  }
}

bool Z3DView::isLayerVisible(neutube3d::ERendererLayer layer) const
{
  bool v = false;

  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    v = filter->isVisible();
  }

  return v;
}

void Z3DView::setFront(neutube3d::ERendererLayer layer, bool on)
{
  Z3DGeometryFilter *filter = getFilter(layer);
  if (filter != NULL) {
    filter->setStayOnTop(on);
  }
}

void Z3DView::setZScale(double scale)
{
  foreach (neutube3d::ERendererLayer layer, m_layerList) {
    setZScale(layer, scale);
  }
}

void Z3DView::setScale(double sx, double sy, double sz)
{
  foreach (neutube3d::ERendererLayer layer, m_layerList) {
    setScale(layer, sx, sy, sz);
  }
}

std::string Z3DView::GetLayerString(neutube3d::ERendererLayer layer)
{
  switch (layer) {
  case neutube3d::LAYER_GRAPH:
    return "Graph";
  case neutube3d::LAYER_SWC:
    return "SWC";
  case neutube3d::LAYER_PUNCTA:
    return "Puncta";
  case neutube3d::LAYER_SURFACE:
    return "Surface";
  case neutube3d::LAYER_TODO:
    return "Todo";
  case neutube3d::LAYER_VOLUME:
    return "Volume";
  case neutube3d::LAYER_MESH:
    return "Mesh";
  case neutube3d::LAYER_ROI:
    return "ROI";
  case neutube3d::LAYER_DECORATION:
    return "Decoration";
  case neutube3d::LAYER_SLICE:
    return "Slice";
  }

  return "";
}

void Z3DView::updateDocData(neutube3d::ERendererLayer layer)
{
  if (layer == neutube3d::LAYER_VOLUME) {
    updateVolumeData();
  } else {
    ZStackDoc3dHelper::UpdateViewData(this, layer);
    /*
    ZStackDoc3dHelper *helper = ZStackDoc3dHelper::GetDocHelper(m_doc);
    if (helper) {
      helper->updateData(this, layer);
    } else {
      ZStackDoc3dHelper localHelper;
      localHelper.updateData(this, layer);
    }
    */
  }
}

void Z3DView::updateCustomCanvas(const QImage &image)
{
  canvas().paintCustomRegion(image);
}

/*
void Z3DView::surfaceDataChanged()
{
  m_docHelper.updateSurfaceData();
}
*/

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

QPointF Z3DView::getScreenProjection(
    double x, double y, double z, neutube3d::ERendererLayer layer)
{
  QPointF pt(0, 0);

  Z3DBoundedFilter *filter = getBoundedFilter(layer);
  if (filter != NULL) {
    glm::vec3 coord = filter->getViewCoord(
          x, y, z, canvas().width(), canvas().height());
    pt.setX(coord[0]);
    pt.setY(coord[1]);
  }

  return pt;
}

#if 0
///Data update
void Z3DView::updateGraphData()
{
  m_graphFilter->clear();

  if (m_doc->swcNetwork()) {
    ZPointNetwork *network = m_doc->swcNetwork()->toPointNetwork();
    m_graphFilter->setData(*network, NULL);
    delete network;
  } else if (ZFileType::FileType(m_doc->additionalSource()) == ZFileType::FILE_JSON) {
    Z3DGraph graph;
    graph.importJsonFile(m_doc->additionalSource());
    m_graphFilter->addData(graph);
  }

  m_graphFilter->addData(
        m_doc->getPlayerList(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR));

  TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
  for (TStackObjectList::const_iterator iter = objList.begin(); iter != objList.end(); ++iter) {
    Z3DGraph *graph = dynamic_cast<Z3DGraph*>(*iter);
    if (graph->isVisible()) {
      m_graphFilter->addData(graph);
    }
  }
}

void Z3DView::updateTodoData()
{
#if defined(_FLYEM_)
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(m_doc);
  if (doc) {
    QList<ZFlyEmToDoItem*> objList = doc->getObjectList<ZFlyEmToDoItem>();
    m_todoFilter->setData(objList);
  }
#endif
}

void Z3DView::updateSwcData()
{
  if (m_swcFilter) {
    m_swcFilter->setData(m_doc->getSwcList());
  }
}

void Z3DView::updateMeshData()
{
  if (m_meshFilter) {
    QList<ZMesh*> meshList = m_doc->getMeshList();

    QList<ZMesh*> filteredMeshList;
    foreach(ZMesh *mesh, meshList) {
      if (!mesh->hasRole(ZStackObjectRole::ROLE_ROI)) {
        filteredMeshList.append(mesh);
      }
    }

    m_meshFilter->setData(filteredMeshList);
  }
}

void Z3DView::updateRoiData()
{
  if (m_roiFilter) {
    QList<ZMesh*> meshList = m_doc->getMeshList();

    QList<ZMesh*> filteredMeshList;
    foreach(ZMesh *mesh, meshList) {
      if (mesh->hasRole(ZStackObjectRole::ROLE_ROI)) {
        filteredMeshList.append(mesh);
      }
    }

    m_roiFilter->setData(filteredMeshList);
  }
}

void Z3DView::updatePunctaData()
{
  if (m_punctaFilter) {
    m_punctaFilter->setData(m_doc->getPunctumList());
  }
}

void Z3DView::updateSurfaceData()
{
  if (m_surfaceFilter) {
    std::vector<ZCubeArray*> all;
    TStackObjectList objList = m_doc->getObjectList(ZStackObject::TYPE_3D_CUBE);
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      all.push_back(dynamic_cast<ZCubeArray*>(*iter));
    }
    m_surfaceFilter->setData(all);
  }
}
#endif


/*
void Z3DView::updateSliceData()
{
  //todo
}
*/

void Z3DView::updateVolumeData()
{
  if (m_volumeFilter) {
    if (m_initMode == INIT_NORMAL) {
      m_volumeFilter->setData(m_doc);
    } else if (m_initMode == INIT_FULL_RES_VOLUME) {
      m_volumeFilter->setData(m_doc, std::numeric_limits<int>::max() / 2);
    }
  }
}
