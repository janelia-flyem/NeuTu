#include "z3dmeshfilter.h"

#include "zmesh.h"
#include "zrandom.h"
#include <QFileInfo>
#include <QPushButton>

Z3DMeshFilter::Z3DMeshFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_meshRenderer(m_rendererBase)
  , m_colorMode("Color Mode")
  , m_singleColorForAllMesh("Mesh Color", glm::vec4(ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    1.f))
  , m_selectMeshEvent("Select Mesh", false)
  , m_pressedMesh(nullptr)
  , m_dataIsInvalid(false)
{
  m_singleColorForAllMesh.setStyle("COLOR");
  connect(&m_singleColorForAllMesh, &ZVec4Parameter::valueChanged, this, &Z3DMeshFilter::prepareColor);

  // Color Mode
  m_colorMode.addOptions("Mesh Color", "Single Color", "Mesh Source");
  m_colorMode.select("Mesh Source");

  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshFilter::prepareColor);
  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshFilter::adjustWidgets);

  addParameter(m_colorMode);

  addParameter(m_singleColorForAllMesh);

  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo("select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo("append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo("append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease);
  connect(&m_selectMeshEvent, &ZEventListenerParameter::mouseEventTriggered, this, &Z3DMeshFilter::selectMesh);
  addEventListener(m_selectMeshEvent);

  adjustWidgets();

  addParameter(m_meshRenderer.wireframeModePara());
  addParameter(m_meshRenderer.wireframeColorPara());

  addParameter(m_meshRenderer.useTwoSidedLightingPara());
}

void Z3DMeshFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DMeshFilter::setColorMode(const std::string &mode)
{
  m_colorMode.select(mode.c_str());
}

void Z3DMeshFilter::setData(const std::vector<ZMesh*>& meshList)
{
  m_origMeshList = meshList;
  //LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DMeshFilter::setData(const QList<ZMesh*>& meshList)
{
  m_origMeshList.clear();
  for (auto mesh : meshList)
    m_origMeshList.push_back(mesh);
  //LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

bool Z3DMeshFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origMeshList.empty();
}

std::shared_ptr<ZWidgetsGroup> Z3DMeshFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Mesh", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_meshRenderer.useTwoSidedLightingPara(), 1);
    m_widgetsGroup->addChild(m_colorMode, 2);
    m_widgetsGroup->addChild(m_singleColorForAllMesh, 2);

    for (const auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }

    m_widgetsGroup->addChild(m_meshRenderer.wireframeModePara(), 3);
    m_widgetsGroup->addChild(m_meshRenderer.wireframeColorPara(), 3);

    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform") {
        m_widgetsGroup->addChild(*para, 4);
        //        QPushButton *pb = new QPushButton("Apply Transform");
        //        connect(pb, &QPushButton::clicked, this, &Z3DMeshFilter::onApplyTransform);
        //        m_widgetsGroup->addChild(*pb, 2);
      }
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 1);
      else if (para->name() != "Size Scale")
        m_widgetsGroup->addChild(*para, 7);
    }

    m_widgetsGroup->addChild(m_xCut, 5);
    m_widgetsGroup->addChild(m_yCut, 5);
    m_widgetsGroup->addChild(m_zCut, 5);
    m_widgetsGroup->addChild(m_boundBoxMode, 5);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 5);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 5);
    m_widgetsGroup->addChild(m_selectionLineWidth, 7);
    m_widgetsGroup->addChild(m_selectionLineColor, 7);
    //m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DMeshFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DMeshFilter::renderTransparent(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DMeshFilter::renderPicking(Z3DEye eye)
{
  if (!m_pickingObjectsRegistered)
    registerPickingObjects();
  m_rendererBase.renderPicking(eye, m_meshRenderer);
}

void Z3DMeshFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects();

  initializeCutRange();
  initializeRotationCenter();

  std::set<QString, QStringNaturalCompare> allSources;
  for (auto mesh : m_origMeshList) {
    allSources.insert(mesh->getSource().c_str());
  }
  // do nothing if sources don't change
  if (m_sourceColorMapper.size() != allSources.size() ||
      !std::equal(m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                  allSources.begin(), _KeyEqual())) {
    // remove old source color parameters from widget, will add new ones later
    if (m_widgetsGroup) {
      for (auto& kv : m_sourceColorMapper) {
        m_widgetsGroup->removeChild(*kv.second);
      }
    }

    // remove not in use sources
    for (auto it = m_sourceColorMapper.begin(); it != m_sourceColorMapper.end(); ) {
      if (allSources.find(it->first) == allSources.end()) {
        removeParameter(*it->second);
        it = m_sourceColorMapper.erase(it);
      } else {
        ++it;
      }
    }

    // create color parameters for new sources
    std::set<QString, QStringNaturalCompare> newSources;
    std::set_difference(allSources.begin(), allSources.end(),
                        m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                        std::inserter(newSources, newSources.end()),
                        QStringKeyNaturalLess());
    for (const auto& kv : newSources) {
      QString guiname = QString("Source: %1").arg(kv);
      m_sourceColorMapper.insert(std::make_pair(kv,
                                                std::make_unique<ZVec4Parameter>(guiname,
                                                                                 glm::vec4(ZRandom::instance().randReal<float>(),
                                                                                           ZRandom::instance().randReal<float>(),
                                                                                           ZRandom::instance().randReal<float>(),
                                                                                           1.f))));

      m_sourceColorMapper[kv]->setStyle("COLOR");
      connect(m_sourceColorMapper[kv].get(), &ZVec4Parameter::valueChanged,
              this, &Z3DMeshFilter::prepareColor);
      addParameter(*m_sourceColorMapper[kv]);
    }

    // update widget group
    if (m_widgetsGroup) {
      for (const auto& kv : m_sourceColorMapper) {
        m_widgetsGroup->addChild(*kv.second, 2);
      }
      m_widgetsGroup->emitWidgetsGroupChangedSignal();
    }
  }

  m_meshRenderer.setData(&m_meshList);
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
    m_meshRenderer.setDataPickingColors(&m_meshPickingColors);
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
    return result;
  } else {
    ZBBox<glm::dvec3> result = p->boundBox(coordTransform());
    m_meshBoundboxMapper[p] = result;
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

void Z3DMeshFilter::addSelectionLines()
{
  for (const auto& p : m_meshList) {
    if (p->isVisible() && p->isSelected()) {
      appendBoundboxLines(meshBound(p), m_selectionLines);
    }
  }
}

void Z3DMeshFilter::prepareColor()
{
  m_meshColors.clear();

  if (m_colorMode.isSelected("Single Color")) {
    for (size_t i = 0; i < m_meshList.size(); ++i) {
      m_meshColors.push_back(m_singleColorForAllMesh.get());
    }
    m_meshRenderer.setDataColors(&m_meshColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("Mesh Source")) {
    for (size_t i=0; i<m_meshList.size(); i++) {
      //LOG(INFO) << m_meshList[i]->getSource().c_str() << m_sourceColorMapper[m_meshList[i]->getSource().c_str()].get();
      glm::vec4 color = m_sourceColorMapper[m_meshList[i]->getSource().c_str()]->get();
      m_meshColors.push_back(color);
    }
    m_meshRenderer.setDataColors(&m_meshColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("Mesh Color")) {
    m_meshRenderer.setColorSource("MeshColor");
  }
}

void Z3DMeshFilter::adjustWidgets()
{
  m_singleColorForAllMesh.setVisible(m_colorMode.isSelected("Single Color"));

  for (auto& kv : m_sourceColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("Mesh Source"));
  }
}

void Z3DMeshFilter::selectMesh(QMouseEvent* e, int, int)
{
  if (m_meshList.empty())
    return;

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
  m_meshList.clear();
  for (size_t i=0; i<m_origMeshList.size(); ++i) {
    if (m_origMeshList[i]->isVisible())
      m_meshList.push_back(m_origMeshList[i]);
  }
}
