#include "z3dsurfacefilter.h"

#include "neutubeconfig.h"

#include "zmesh.h"
#include "zrandom.h"
#include <QFileInfo>
#include <QPushButton>

Z3DSurfaceFilter::Z3DSurfaceFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_meshRenderer(m_rendererBase)
  , m_colorMode("Color Mode")
  , m_singleColorForAllSurface("Mesh Color", glm::vec4(ZRandom::instance().randReal<float>(),
                                                       ZRandom::instance().randReal<float>(),
                                                       ZRandom::instance().randReal<float>(),
                                                       1.f))
  , m_dataIsInvalid(false)
{
  const NeutubeConfig::Z3DWindowConfig::GraphTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getGraphTabConfig();
  setVisible(config.isVisible());

  setOpacity(0.75);
  m_rendererBase.setMaterialSpecular(glm::vec4(0, 0, 0, 1));

  m_singleColorForAllSurface.setStyle("COLOR");
  connect(&m_singleColorForAllSurface, &ZVec4Parameter::valueChanged, this, &Z3DSurfaceFilter::prepareColor);

  // Color Mode
  m_colorMode.addOptions("CubeArray Color", "Single Color", "CubeArray Source");
  m_colorMode.select("CubeArray Source");

  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DSurfaceFilter::prepareColor);
  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DSurfaceFilter::adjustWidgets);

  addParameter(m_colorMode);

  addParameter(m_singleColorForAllSurface);

  adjustWidgets();

  addParameter(m_meshRenderer.wireframeModePara());
  addParameter(m_meshRenderer.wireframeColorPara());

  connect(&m_rendererBase.opacityPara(), &ZFloatParameter::floatChanged, this, &Z3DSurfaceFilter::opacityValueChanged);
}

void Z3DSurfaceFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DSurfaceFilter::setData(const std::vector<ZCubeArray*>& cubeList)
{
  m_origCubeArrayList = cubeList;
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DSurfaceFilter::setData(const QList<ZCubeArray*>& cubeList)
{
  m_origCubeArrayList.clear();
  for (auto mesh : cubeList)
    m_origCubeArrayList.push_back(mesh);
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

bool Z3DSurfaceFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origCubeArrayList.empty();
}

std::shared_ptr<ZWidgetsGroup> Z3DSurfaceFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Surface", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_colorMode, 1);
    m_widgetsGroup->addChild(m_singleColorForAllSurface, 1);

    for (const auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }

    m_widgetsGroup->addChild(m_meshRenderer.wireframeModePara(), 3);
    m_widgetsGroup->addChild(m_meshRenderer.wireframeColorPara(), 3);

    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform") {
        m_widgetsGroup->addChild(*para, 4);
      }
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 5);
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

void Z3DSurfaceFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DSurfaceFilter::renderTransparent(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DSurfaceFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  initializeCutRange();
  initializeRotationCenter();

  std::set<QString, QStringNaturalCompare> allSources;
  for (auto mesh : m_origCubeArrayList) {
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
                        _KeyLess());
    for (const auto& kv : newSources) {
      QString guiname = QString("Source: %1").arg(kv);
      m_sourceColorMapper.insert(
            std::make_pair(
              kv, std::make_unique<ZVec4Parameter>(
                guiname, glm::vec4(ZRandom::instance().randReal<float>(),
                                   ZRandom::instance().randReal<float>(),
                                   ZRandom::instance().randReal<float>(),
                                   1.f))));

      m_sourceColorMapper[kv]->setStyle("COLOR");
      connect(m_sourceColorMapper[kv].get(), &ZVec4Parameter::valueChanged,
              this, &Z3DSurfaceFilter::prepareColor);
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

  m_meshCache.clear();
  m_meshPtCache.clear();
  for (auto cap : m_cubeArrayList) {
    m_meshCache.push_back(ZMesh::FromZCubeArray(*cap));
  }
  for (auto& mesh : m_meshCache) {
    m_meshPtCache.push_back(&mesh);
  }

  m_meshRenderer.setData(&m_meshPtCache);
  prepareColor();
  adjustWidgets();
  m_dataIsInvalid = false;
}

ZBBox<glm::dvec3> Z3DSurfaceFilter::cubeBound(const ZCubeArray& p)
{
  const auto& cubes = p.getCubeArray();
  ZBBox<glm::dvec3> res;
  for (auto& cube : cubes) {
    for (const auto& node : cube.nodes) {
      res.expand(glm::dvec3(node));
    }
  }
  return res;
}

void Z3DSurfaceFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  for (size_t i = 0; i < m_origCubeArrayList.size(); ++i) {
    m_notTransformedBoundBox.expand(cubeBound(*m_origCubeArrayList[i]));
  }
}

void Z3DSurfaceFilter::addSelectionLines()
{
  for (const auto& p : m_cubeArrayList) {
    if (p->isVisible() && p->isSelected()) {
      appendBoundboxLines(cubeBound(*p), m_selectionLines);
    }
  }
}

void Z3DSurfaceFilter::prepareColor()
{
  m_cubeArrayColors.clear();

  if (m_colorMode.isSelected("Single Color")) {
    for (size_t i = 0; i < m_cubeArrayList.size(); ++i) {
      m_cubeArrayColors.push_back(m_singleColorForAllSurface.get());
    }
    m_meshRenderer.setDataColors(&m_cubeArrayColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("CubeArray Source")) {
    for (size_t i=0; i<m_cubeArrayList.size(); i++) {
      //LOG(INFO) << m_meshList[i]->getSource().c_str() << m_sourceColorMapper[m_meshList[i]->getSource().c_str()].get();
      glm::vec4 color = m_sourceColorMapper[m_cubeArrayList[i]->getSource().c_str()]->get();
      m_cubeArrayColors.push_back(color);
    }
    m_meshRenderer.setDataColors(&m_cubeArrayColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("CubeArray Color")) {
    m_meshRenderer.setColorSource("MeshColor");
  }
}

void Z3DSurfaceFilter::adjustWidgets()
{
  m_singleColorForAllSurface.setVisible(m_colorMode.isSelected("Single Color"));

  for (auto& kv : m_sourceColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("CubeArray Source"));
  }
}

void Z3DSurfaceFilter::updateSurfaceVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DSurfaceFilter::getVisibleData()
{
  m_cubeArrayList.clear();
  for (size_t i=0; i<m_origCubeArrayList.size(); ++i) {
    if (m_origCubeArrayList[i]->isVisible())
      m_cubeArrayList.push_back(m_origCubeArrayList[i]);
  }
}
