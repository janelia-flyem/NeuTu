#include "z3dmeshfilter.h"

#include "z3drenderport.h"
#include "zmesh.h"
#include "zrandom.h"
#include <QFileInfo>
#include <QPushButton>

Z3DMeshFilter::Z3DMeshFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_monoEyeOutport("Image", this)
  , m_leftEyeOutport("LeftEyeImage", this)
  , m_rightEyeOutport("RightEyeImage", this)
  , m_monoEyeOutport2("Image2", this)
  , m_leftEyeOutport2("LeftEyeImage2", this)
  , m_rightEyeOutport2("RightEyeImage2", this)
  , m_triangleListRenderer(m_rendererBase)
  , m_colorMode("Color Mode")
  , m_singleColorForAllMesh("Mesh Color", glm::vec4(ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    1.f))
  , m_textureGlowRenderer(m_rendererBase)
  , m_glow("Glow", false)
  , m_textureCopyRenderer(m_rendererBase)
  , m_selectMeshEvent("Select Mesh", false)
  , m_pressedMesh(nullptr)
  , m_selectedMeshes(nullptr)
  , m_dataIsInvalid(false)
{
  addPrivateRenderPort(m_monoEyeOutport);
  addPrivateRenderPort(m_leftEyeOutport);
  addPrivateRenderPort(m_rightEyeOutport);
  addPrivateRenderPort(m_monoEyeOutport2);
  addPrivateRenderPort(m_leftEyeOutport2);
  addPrivateRenderPort(m_rightEyeOutport2);

  m_textureCopyRenderer.setDiscardTransparent(true);

  m_singleColorForAllMesh.setStyle("COLOR");
  connect(&m_singleColorForAllMesh, &ZVec4Parameter::valueChanged, this, &Z3DMeshFilter::prepareColor);

  // Color Mode
  m_colorMode.addOption("Single Color");
  m_colorMode.select("Single Color");

  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshFilter::prepareColor);
  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshFilter::adjustWidgets);

  addParameter(m_colorMode);

  addParameter(m_singleColorForAllMesh);

  connect(&m_glow, &ZBoolParameter::valueChanged, this, &Z3DMeshFilter::adjustWidgets);
  addParameter(m_glow);
  addParameter(m_textureGlowRenderer.glowModePara());
  addParameter(m_textureGlowRenderer.blurRadiusPara());
  addParameter(m_textureGlowRenderer.blurScalePara());
  addParameter(m_textureGlowRenderer.blurStrengthPara());

  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo("append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo("append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease);
  connect(&m_selectMeshEvent, &ZEventListenerParameter::mouseEventTriggered, this, &Z3DMeshFilter::selectMesh);
  addEventListener(m_selectMeshEvent);

  adjustWidgets();

  addParameter(m_triangleListRenderer.wireframeModePara());
  addParameter(m_triangleListRenderer.wireframeColorPara());
  m_triangleListRenderer.setColorSource("CustomColor");
}

void Z3DMeshFilter::process(Z3DEye eye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
  if (m_glow.get()) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    Z3DRenderOutputPort& currentOutport = (eye == Z3DEye::Mono) ?
                                          m_monoEyeOutport : (eye == Z3DEye::Left) ? m_leftEyeOutport
                                                                                   : m_rightEyeOutport;

    currentOutport.bindTarget();
    currentOutport.clearTarget();
    m_rendererBase.setViewport(currentOutport.size());
    m_rendererBase.render(eye, m_triangleListRenderer);
    CHECK_GL_ERROR
    currentOutport.releaseTarget();

    Z3DRenderOutputPort& currentOutport2 = (eye == Z3DEye::Mono) ?
                                           m_monoEyeOutport2 : (eye == Z3DEye::Left) ? m_leftEyeOutport2
                                                                                     : m_rightEyeOutport2;
    currentOutport2.bindTarget();
    currentOutport2.clearTarget();
    m_rendererBase.setViewport(currentOutport2.size());
    m_textureGlowRenderer.setColorTexture(currentOutport.colorTexture());
    m_textureGlowRenderer.setDepthTexture(currentOutport.depthTexture());
    m_rendererBase.render(eye, m_textureGlowRenderer);
    CHECK_GL_ERROR
    currentOutport2.releaseTarget();

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
  }
}

void Z3DMeshFilter::setData(std::vector<ZMesh*>* meshList)
{
  m_origMeshList.clear();
  if (meshList) {
    m_origMeshList = *meshList;
    LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DMeshFilter::setData(QList<ZMesh*>* meshList)
{
  m_origMeshList.clear();
  if (meshList) {
    for (auto mesh : *meshList)
      m_origMeshList.push_back(mesh);
    LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  }
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

bool Z3DMeshFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origMeshList.empty();
}

//namespace {

//bool compareParameterName(const ZParameter *p1, const ZParameter *p2)
//{
//  QString n1 = p1->getName().mid(5); // "Mesh "
//  QString n2 = p2->getName().mid(5);
//  n1.remove(n1.size()-6, 6); //" Color"
//  n2.remove(n2.size()-6, 6);
//  return n1.toInt() < n2.toInt();
//}

//}

std::shared_ptr<ZWidgetsGroup> Z3DMeshFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Mesh", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_colorMode, 1);
    m_widgetsGroup->addChild(m_singleColorForAllMesh, 1);
    m_widgetsGroup->addChild(m_triangleListRenderer.wireframeModePara(), 1);
    m_widgetsGroup->addChild(m_triangleListRenderer.wireframeColorPara(), 1);

    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform") {
        m_widgetsGroup->addChild(*para, 2);
        //        QPushButton *pb = new QPushButton("Apply Transform");
        //        connect(pb, &QPushButton::clicked, this, &Z3DMeshFilter::onApplyTransform);
        //        m_widgetsGroup->addChild(*pb, 2);
      }
        //else if (para->name() == "Size Scale")
        //m_widgetsGroup->addChild(para, 3);
        //else if (para->name() == "Rendering Method")
        //m_widgetsGroup->addChild(para, 4);
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 5);
      else if (para->name() != "Size Scale")
        m_widgetsGroup->addChild(*para, 7);
    }

    m_widgetsGroup->addChild(m_glow, 5);
    m_widgetsGroup->addChild(m_textureGlowRenderer.glowModePara(), 5);
    m_widgetsGroup->addChild(m_textureGlowRenderer.blurRadiusPara(), 5);
    m_widgetsGroup->addChild(m_textureGlowRenderer.blurScalePara(), 5);
    m_widgetsGroup->addChild(m_textureGlowRenderer.blurStrengthPara(), 5);

    m_widgetsGroup->addChild(m_xCut, 5);
    m_widgetsGroup->addChild(m_yCut, 5);
    m_widgetsGroup->addChild(m_zCut, 5);
    m_widgetsGroup->addChild(m_boundBoxMode, 5);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 5);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 5);
    m_widgetsGroup->addChild(m_selectionLineWidth, 7);
    m_widgetsGroup->addChild(m_selectionLineColor, 7);
    m_widgetsGroup->addChild(m_manipulatorSize, 7);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

std::shared_ptr<ZWidgetsGroup> Z3DMeshFilter::widgetsGroupForAnnotationFilter()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Mesh", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_singleColorForAllMesh, 1);
    m_widgetsGroup->addChild(m_triangleListRenderer.wireframeModePara(), 1);
    m_widgetsGroup->addChild(m_triangleListRenderer.wireframeColorPara(), 1);

    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name().contains("Opacity") || para->name().contains("Material"))
        m_widgetsGroup->addChild(*para, 5);
    }
    m_widgetsGroup->addChild(m_xCut, 5);
    m_widgetsGroup->addChild(m_yCut, 5);
    m_widgetsGroup->addChild(m_zCut, 5);
    m_widgetsGroup->addChild(m_boundBoxMode, 5);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 5);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 5);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DMeshFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_triangleListRenderer);
  renderBoundBox(eye);
}

void Z3DMeshFilter::renderTransparent(Z3DEye eye)
{
  if (m_glow.get()) {
    Z3DRenderOutputPort& currentOutport2 = (eye == Z3DEye::Mono) ?
                                           m_monoEyeOutport2 : (eye == Z3DEye::Left) ? m_leftEyeOutport2
                                                                                     : m_rightEyeOutport2;
    m_textureCopyRenderer.setColorTexture(currentOutport2.colorTexture());
    m_textureCopyRenderer.setDepthTexture(currentOutport2.depthTexture());
    m_rendererBase.render(eye, m_textureCopyRenderer);
    renderBoundBox(eye);
  } else {
    m_rendererBase.render(eye, m_triangleListRenderer);
    renderBoundBox(eye);
  }
}

void Z3DMeshFilter::renderPicking(Z3DEye eye)
{
  if (!m_pickingObjectsRegistered)
    registerPickingObjects();
  m_rendererBase.renderPicking(eye, m_triangleListRenderer);
}

void Z3DMeshFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects();

  initializeCutRange();
  initializeRotationCenter();

  m_triangleListRenderer.setData(&m_meshList);
  prepareColor();
  adjustWidgets();
  m_dataIsInvalid = false;
}

void Z3DMeshFilter::registerPickingObjects()
{
  if (!m_pickingObjectsRegistered) {
    for (auto mesh : m_meshList) {
      pickingManager().registerObject(mesh);
    }
    m_registeredMeshList = m_meshList;
    m_meshPickingColors.clear();
    for (auto mesh : m_meshList) {
      glm::col4 pickingColor = pickingManager().colorOfObject(mesh);
      glm::vec4 fPickingColor(pickingColor[0] / 255.f, pickingColor[1] / 255.f, pickingColor[2] / 255.f,
                              pickingColor[3] / 255.f);
      m_meshPickingColors.push_back(fPickingColor);
    }
    m_triangleListRenderer.setDataPickingColors(&m_meshPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void Z3DMeshFilter::deregisterPickingObjects()
{
  if (m_pickingObjectsRegistered) {
    for (auto mesh : m_registeredMeshList) {
      pickingManager().deregisterObject(mesh);
    }
    m_registeredMeshList.clear();
  }

  m_pickingObjectsRegistered = false;
}

ZBBox<glm::dvec3> Z3DMeshFilter::meshBound(ZMesh* p)
{
  std::map<ZMesh*, ZBBox<glm::dvec3>>::const_iterator it = m_meshBoundboxMapper.find(p);
  if (it != m_meshBoundboxMapper.end()) {
    ZBBox<glm::dvec3> result = it->second;
    //    result[0] *= getCoordTransform().x;
    //    result[1] *= getCoordTransform().x;
    //    result[2] *= getCoordTransform().y;
    //    result[3] *= getCoordTransform().y;
    //    result[4] *= getCoordTransform().z;
    //    result[5] *= getCoordTransform().z;
    return result;
  } else {
    ZBBox<glm::dvec3> result = p->boundBox(coordTransform());
    m_meshBoundboxMapper[p] = result;
    //    result[0] *= getCoordTransform().x;
    //    result[1] *= getCoordTransform().x;
    //    result[2] *= getCoordTransform().y;
    //    result[3] *= getCoordTransform().y;
    //    result[4] *= getCoordTransform().z;
    //    result[5] *= getCoordTransform().z;
    return result;
  }
}

void Z3DMeshFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  for (size_t i = 0; i < m_origMeshList.size(); ++i) {
    m_notTransformedBoundBox.expand(m_origMeshList[i]->boundBox());
  }
}

void Z3DMeshFilter::prepareColor()
{
  m_meshColors.clear();

  if (m_colorMode.isSelected("Single Color")) {
    for (size_t i = 0; i < m_meshList.size(); ++i) {
      m_meshColors.push_back(m_singleColorForAllMesh.get());
    }
  }

  m_triangleListRenderer.setDataColors(&m_meshColors);
}

void Z3DMeshFilter::adjustWidgets()
{
  m_singleColorForAllMesh.setVisible(m_colorMode.isSelected("Single Color"));

  m_textureGlowRenderer.glowModePara().setVisible(m_glow.get());
  m_textureGlowRenderer.blurRadiusPara().setVisible(m_glow.get());
  m_textureGlowRenderer.blurScalePara().setVisible(m_glow.get());
  m_textureGlowRenderer.blurStrengthPara().setVisible(m_glow.get());
}

void Z3DMeshFilter::selectMesh(QMouseEvent* e, int, int h)
{
  Q_UNUSED(h)
  if (m_meshList.empty())
    return;

  e->ignore();
  if (e->type() == QEvent::MouseButtonDblClick) {
    const void* obj = pickingManager().objectAtWidgetPos(
      glm::ivec2(e->x(), e->y()));
    bool appending = (e->modifiers() == Qt::ControlModifier);
    if (!obj && !appending && m_isSelected) {
      emit objDeselected();
      return;
    }
    bool hit = std::find(m_meshList.begin(), m_meshList.end(), static_cast<const ZMesh*>(obj)) != m_meshList.end();
    if (hit) {
      emit objSelected(appending);
      e->accept();
    }
    return;
  }

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(e->x(), e->y()));
    if (!obj) {
      return;
    }

    // Check if any point was selected...
    for (auto m : m_meshList)
      if (m == obj) {
        m_pressedMesh = m;
        break;
      }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      if (e->modifiers() == Qt::ControlModifier)
        emit meshSelected(m_pressedMesh, true);
      else
        emit meshSelected(m_pressedMesh, false);
      if (m_pressedMesh)
        e->accept();
    }
    m_pressedMesh = nullptr;
  }
}

void Z3DMeshFilter::onApplyTransform()
{
  LOG(INFO) << m_rendererBase.coordTransform();
}

void Z3DMeshFilter::updateMeshVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DMeshFilter::getVisibleData()
{
  m_meshList = m_origMeshList;
}
