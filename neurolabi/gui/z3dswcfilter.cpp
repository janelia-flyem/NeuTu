#include "z3dswcfilter.h"

#include <QMessageBox>
#include <QApplication>
#include <iostream>
#include <QSet>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QApplication>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QApplication>

#include "tz_3dgeom.h"
#include "zrandom.h"
#include "zqslog.h"
#include "swctreenode.h"
#include "tz_geometry.h"
#include "neutubeconfig.h"
#include "zintcuboid.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"
#include "zrandom.h"
#include "tz_3dgeom.h"
#include "z3dlinerenderer.h"
#include "z3dlinewithfixedwidthcolorrenderer.h"
#include "z3dsphererenderer.h"
#include "z3dconerenderer.h"
#include "zeventlistenerparameter.h"
#include "swctreenode.h"
#include "tz_geometry.h"
#include "neutubeconfig.h"
#include "zintcuboid.h"
#include "z3dswcfilter.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"
#include "z3drendertarget.h"
#include "zlabelcolortable.h"

namespace {

struct _ParameterNameComp {
  bool operator()(ZParameter* p1, ZParameter* p2) const
  {
    return p1->name() < p2->name();
  }
};

}

Z3DSwcFilter::Z3DSwcFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_lineRenderer(m_rendererBase)
  , m_coneRenderer(m_rendererBase)
  , m_sphereRenderer(m_rendererBase)
  , m_sphereRendererForCone(m_rendererBase)
  , m_renderingPrimitive("Geometry")
  , m_colorMode("Color Mode")
  , m_colorMapBranchType("Branch Type Color Map")
  , m_selectSwcEvent("Select Puncta", false)
  , m_interactionMode(EInteractionMode::Select)
{
  initTopologyColor();
  initTypeColor();
  initSubclassTypeColor();
  initLabelTypeColor();

  m_individualColorParam.m_scheme.setColorScheme(ZSwcColorScheme::UNIQUE_COLOR);
  m_randomColorParam.m_scheme.setColorScheme(ZColorScheme::RANDOM_COLOR);

  // rendering primitive
  m_renderingPrimitive.addOptions("Normal", "Line", "Sphere");
  m_renderingPrimitive.select("Sphere");
  const NeutubeConfig::Z3DWindowConfig::SwcTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getSwcTabConfig();
  if (!config.getPrimitive().empty()) {
    m_renderingPrimitive.select(config.getPrimitive().c_str());
  }
  connect(&m_renderingPrimitive, &ZStringIntOptionParameter::valueChanged,
          this, &Z3DSwcFilter::updateBoundBox);

  // Color Mode
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_colorMode.addOption("Biocytin Branch Type");
  }

  m_colorMode.addOptions(
//      #if !defined(_FLYEM_) //disabled temporarily to avoid crash
        "Individual",
        "Random Tree Color",
//      #endif
        "Branch Type",
        "Topology",
        "Colormap Branch Type",
        "Label Branch Type",
        "Subclass",
        "Direction",
        "Intrinsic");

  m_colorMode.select("Branch Type");
  if (!config.getColorMode().empty()) {
    m_colorMode.select(config.getColorMode().c_str());
  }

  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
          this, &Z3DSwcFilter::updateColorWidgets);
  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
          this, &Z3DSwcFilter::prepareColor);

  addParameter(m_renderingPrimitive);
  addParameter(m_colorMode);

  for (const auto& color : m_colorsForDifferentType) {
    addParameter(*color.get());
  }

  for (const auto& color : m_colorsForDifferentTopology) {
    addParameter(*color.get());
  }

  for (const auto& color : m_colorsForSubclassType) {
    addParameter(*color.get());
  }

  for (const auto& color : m_colorsForLabelType) {
    addParameter(*color.get());
  }

  m_selectSwcEvent.listenTo("select swc", Qt::LeftButton,
                            Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent.listenTo("select swc", Qt::LeftButton,
                            Qt::NoModifier, QEvent::MouseButtonRelease); 
  m_selectSwcEvent.listenTo("select swc connection", Qt::LeftButton,
                            Qt::ShiftModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent.listenTo("select swc connection", Qt::LeftButton,
                            Qt::ShiftModifier, QEvent::MouseButtonRelease);

  m_selectSwcEvent.listenTo("select swc flood filling", Qt::LeftButton,
                            Qt::AltModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent.listenTo("select swc flood filling", Qt::LeftButton,
                            Qt::AltModifier, QEvent::MouseButtonRelease);
  m_selectSwcEvent.listenTo("select swc flood filling", Qt::LeftButton,
                            Qt::AltModifier | Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent.listenTo("select swc flood filling", Qt::LeftButton,
                            Qt::AltModifier | Qt::ControlModifier, QEvent::MouseButtonRelease);

  m_selectSwcEvent.listenTo("append select swc", Qt::LeftButton,
                            Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectSwcEvent.listenTo("append select swc", Qt::LeftButton,
                            Qt::ControlModifier, QEvent::MouseButtonRelease);

  connect(&m_selectSwcEvent, &ZEventListenerParameter::mouseEventTriggered,
          this, &Z3DSwcFilter::selectSwc);
  addEventListener(m_selectSwcEvent);

  addParameter(m_colorMapBranchType);
  connect(&m_colorMapBranchType, &ZColorMapParameter::valueChanged,
          this, &Z3DSwcFilter::prepareColor);

  m_individualColorParam.m_prefix = "SWC";
  m_randomColorParam.m_prefix = "Random SWC";

  updateColorWidgets();

  m_guiNameList.resize(50);
  for (int i = 0; i < m_guiNameList.size(); ++i) {
    m_guiNameList[i] = QString("Type %1 Color").arg(i);
  }
}

Z3DSwcFilter::~Z3DSwcFilter()
{
  clearDecorateSwcList();
}

namespace {
glm::vec4 NormalizeColor(const glm::col4 &color)
{
  return glm::vec4(color[0] / 255.f, color[1] / 255.f,
                   color[2] / 255.f, color[3] / 255.f);
}
}

void Z3DSwcFilter::clearDecorateSwcList()
{
  for (std::vector<ZSwcTree*>::iterator iter = m_decorateSwcList.begin();
       iter != m_decorateSwcList.end(); ++iter) {
    delete *iter;
  }

  m_decorateSwcList.clear();
}


void Z3DSwcFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DSwcFilter::initTopologyColor()
{
  // topology colors (root, branch point, leaf, others)
  m_colorsForDifferentTopology.emplace_back(
        std::make_unique<ZVec4Parameter>("Root Color", glm::vec4(0 / 255.f, 0 / 255.f, 255 / 255.f, 1.f)));
  m_colorsForDifferentTopology.emplace_back(
        std::make_unique<ZVec4Parameter>("Branch Point Color", glm::vec4(0 / 255.f, 255 / 255.f, 0 / 255.f, 1.f)));
  if (GET_APPLICATION_NAME == "Biocytin") {
    m_colorsForDifferentTopology.emplace_back(
          std::make_unique<ZVec4Parameter>("Leaf Color", glm::vec4(200 / 255.f, 200 / 255.f, 0 / 255.f, 1.f)));
  } else {
    m_colorsForDifferentTopology.emplace_back(
          std::make_unique<ZVec4Parameter>("Leaf Color", glm::vec4(255 / 255.f, 200/ 255.f, 0 / 255.f, 1.f)));
  }
  m_colorsForDifferentTopology.emplace_back(
        std::make_unique<ZVec4Parameter>("Other", glm::vec4(255 / 255.f, 0 / 255.f, 0 / 255.f, 1.f)));
  for (const auto& color : m_colorsForDifferentTopology) {
    color->setStyle("COLOR");
    connect(color.get(), &ZVec4Parameter::valueChanged, this, &Z3DSwcFilter::prepareColor);
  }
}

QString Z3DSwcFilter::GetTypeName(int type)
{
  if (type < 276) {
    switch (type) {
    case 1:
      return QString("Type %1 (Soma) Color").arg(type);
    case 2:
      return QString("Type %1 (Axon) Color").arg(type);
    case 3:
      return QString("Type %1 (Basal Dendrite) Color").arg(type);
    case 4:
      return QString("Type %1 (Apical Dendrite) Color").arg(type);
    case 5:
      return QString("Type %1 (Main Trunk) Color").arg(type);
    case 6:
      return QString("Type %1 (Basal Intermediate) Color").arg(type);
    case 7:
      return QString("Type %1 (Basal Terminal) Color").arg(type);
    case 8:
      return QString("Type %1 (Apical Oblique Intermediate) Color").arg(type);
    case 9:
      return QString("Type %1 (Apical Oblique Terminal) Color").arg(type);
    case 10:
      return QString("Type %1 (Apical Tuft) Color").arg(type);
    default:
      return QString("Type %1 Color").arg(type);
    }
  }

  return "Undefined Type Color";
}

void Z3DSwcFilter::addNodeType(int type)
{
  if (m_allNodeType.count(type) == 0) {
    m_allNodeType.insert(type);
    if (type > m_maxType) {
      m_maxType = type;
    }
    updateColorWidgets();
  }
}

void Z3DSwcFilter::initTypeColor()
{
  // type colors
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    ZSwcColorScheme colorScheme;
    colorScheme.setColorScheme(ZSwcColorScheme::JIN_TYPE_COLOR);

    int index = 0;
    QStringList nameList;
    nameList << QString("Type %1 Color").arg(0)
             << QString("Type %1 (Soma) Color").arg(1)
             << QString("Type %1 (Axon) Color").arg(2)
             << QString("Type %1 (Basal Dendrite) Color").arg(3)
             << QString("Type %1 (Apical Dendrite) Color").arg(4)
             << QString("(Undefined) Color");

    foreach (QString name, nameList) {
      QColor color = colorScheme.getColor(index++);
      m_colorsForDifferentType.emplace_back(
          std::make_unique<ZVec4Parameter>(name, glm::vec4(color.redF(), color.greenF(),
                                                           color.blueF(), 1.f)));
    }
  } else {
    ZSwcColorScheme colorScheme;
    colorScheme.setColorScheme(ZSwcColorScheme::GMU_TYPE_COLOR);
    for (int type = 0; type <= 276; ++type) {
      QColor color = colorScheme.getColor(type);
      m_colorsForDifferentType.emplace_back(
          std::make_unique<ZVec4Parameter>(GetTypeName(type),
                                           glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f)));
    }
  }
  for (const auto& color : m_colorsForDifferentType) {
    color->setStyle("COLOR");
    connect(color.get(), &ZVec4Parameter::valueChanged, this, &Z3DSwcFilter::prepareColor);
  }
}

void Z3DSwcFilter::initLabelTypeColor()
{
  ZColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::LABEL_COLOR);
  for (int type = 0; type <= colorScheme.getColorNumber(); ++type) {
    QColor color = colorScheme.getColor(type);
    m_colorsForLabelType.emplace_back(
          std::make_unique<ZVec4Parameter>(
            "Label " + GetTypeName(type),
            glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f)));
  }
}

void Z3DSwcFilter::initSubclassTypeColor()
{
  // subclass type color
  QString name = QString("Soma Color");
  m_subclassTypeColorMapper[1] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 1.f)));
  name = QString("Main Trunk Color");
  m_subclassTypeColorMapper[5] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 1.f)));
  name = QString("Basal Intermediate Color");
  m_subclassTypeColorMapper[6] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0x33 / 255.f, 0xcc / 255.f, 0xff / 255.f, 1.f)));
  name = QString("Basal Terminal Color");
  m_subclassTypeColorMapper[7] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0x33 / 255.f, 0x66 / 255.f, 0xcc / 255.f, 1.f)));
  name = QString("Apical Oblique Intermediate Color");
  m_subclassTypeColorMapper[8] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0xff / 255.f, 0xff / 255.f, 0 / 255.f, 1.f)));
  name = QString("Apical Oblique Terminal Color");
  m_subclassTypeColorMapper[9] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0xcc / 255.f, 0x33 / 255.f, 0x66 / 255.f, 1.f)));
  name = QString("Apical Tuft Color");
  m_subclassTypeColorMapper[10] = m_colorsForSubclassType.size();
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0 / 255.f, 0x99 / 255.f, 0 / 255.f, 1.f)));
  name = QString("Other Undefined class Color");
  m_colorsForSubclassType.emplace_back(
        std::make_unique<ZVec4Parameter>(name, glm::vec4(0xcc / 255.f, 0xcc / 255.f, 0xcc / 255.f, 1.f)));
  for (const auto& color : m_colorsForSubclassType) {
    color->setStyle("COLOR");
    connect(color.get(), &ZVec4Parameter::valueChanged, this, &Z3DSwcFilter::prepareColor);
  }
}

void Z3DSwcFilter::configure(const ZJsonObject &obj)
{
  Z3DGeometryFilter::configure(obj);

  if (obj.hasKey(Z3DFilterSetting::COLOR_MODE_KEY)) {
    setColorMode(ZJsonParser::stringValue(obj[Z3DFilterSetting::COLOR_MODE_KEY]));
  }

  if (obj.hasKey(Z3DFilterSetting::SHAPE_MODE_KEY)) {
    setRenderingPrimitive(
          ZJsonParser::stringValue(obj[Z3DFilterSetting::SHAPE_MODE_KEY]));
  }
}

ZJsonObject Z3DSwcFilter::getConfigJson() const
{
  ZJsonObject obj = Z3DGeometryFilter::getConfigJson();

  obj.setEntry(
        Z3DFilterSetting::COLOR_MODE_KEY, m_colorMode.get().toStdString());
  obj.setEntry(
        Z3DFilterSetting::SHAPE_MODE_KEY, m_renderingPrimitive.get().toStdString());

  return obj;
}

void Z3DSwcFilter::registerPickingNodes(ZSwcTree *tree)
{
  if (m_registeredSwcTreeNodeMap.count(tree) == 0) {
    std::vector<Swc_Tree_Node*> &nodeArray = m_decomposedNodeMap[tree];
    m_registeredSwcTreeNodeMap[tree].resize(nodeArray.size());
     std::vector<Swc_Tree_Node*> &registeredNodeArray =
         m_registeredSwcTreeNodeMap[tree];
    for (size_t j = 0; j < nodeArray.size(); ++j) {
      pickingManager().registerObject(nodeArray[j]);
      registeredNodeArray[j] = nodeArray[j];
    }
  }
}

void Z3DSwcFilter::registerPickingObjectsForImmutable()
{
  if (m_enablePicking) {
    ZOUT(LTRACE(), 5) << "start";

    if (!m_pickingObjectsRegistered) {
      size_t nodePairCount = 0;
      size_t nodeCount = 0;
      for (const auto &t : m_decomposedNodeMap) {
        nodeCount += t.second.size();
      }
      for (const auto &t : m_decomposedNodePairMap) {
        nodePairCount += t.second.size();
      }

      for (size_t i=0; i<m_swcList.size(); i++) {
        ZSwcTree *tree = m_swcList.at(i);
        pickingManager().registerObject(tree);
        registerPickingNodes(tree);
      }
      m_registeredSwcList = m_swcList;

      m_swcPickingColors.resize(nodePairCount);
      m_linePickingColors.resize(nodePairCount * 2);
      m_pointPickingColors.resize(nodeCount);
      m_sphereForConePickingColors.resize(nodeCount);

      size_t nodeIndex = 0;
      size_t nodePairIndex = 0;

//      for (size_t i=0; i < m_swcList.size(); i++) {
      for (auto &t : m_decomposedNodeMap) {
        ZSwcTree *tree = t.first;
        glm::col4 pickingColor = pickingManager().colorOfObject(tree);
        glm::vec4 swcPickingColor = NormalizeColor(pickingColor);

        auto &nodeArray = t.second;

        for (Swc_Tree_Node *tn : nodeArray) {
          glm::col4 pickingColor = pickingManager().colorOfObject(tn);
          glm::vec4 fPickingColor = NormalizeColor(pickingColor);
#ifdef _DEBUG_2
          std::cout << "Node picking color: " << tn << " -> " << fPickingColor << std::endl;
#endif
          m_pointPickingColors[nodeIndex] = fPickingColor;
          m_sphereForConePickingColors[nodeIndex] = swcPickingColor;
          ++nodeIndex;
        }

        auto &nodePairArray = m_decomposedNodePairMap[tree];
        for (size_t j=0; j<nodePairArray.size(); j++) {
          m_swcPickingColors[nodePairIndex] = swcPickingColor;
          m_linePickingColors[nodePairIndex * 2] = swcPickingColor;
          m_linePickingColors[nodePairIndex * 2 + 1] = swcPickingColor;
          ++nodePairIndex;
        }
      }

      m_coneRenderer.setDataPickingColors(&m_swcPickingColors);
      m_sphereRendererForCone.setDataPickingColors(&m_sphereForConePickingColors);
      m_lineRenderer.setDataPickingColors(&m_linePickingColors);
      m_sphereRenderer.setDataPickingColors(&m_pointPickingColors);
    }

    m_pickingObjectsRegistered = true;
    ZOUT(LTRACE(), 5) << "end";
  }
}

void Z3DSwcFilter::registerPickingObjects()
{
  if (m_enablePicking) {
    if (m_swcTopologyMutable == false) {
      registerPickingObjectsForImmutable();
      return;
    }

    ZOUT(LTRACE(), 5) << "start";
    if (m_swcList.size() != m_decomposedNodes.size()) {
      ZOUT(LTRACE(), 5) << "WARNING: Unmatched SWC data.";
    }

    if (!m_pickingObjectsRegistered) {
      size_t nodePairCount = 0;
      size_t nodeCount = 0;
      for (size_t i=0; i < m_swcList.size(); i++) {
        nodePairCount += m_decompsedNodePairs[i].size();
        nodeCount += m_decomposedNodes[i].size();
      }

      m_registeredSwcTreeNodeList.resize(nodeCount);

      {
        size_t nodeIndex = 0;
        for (size_t i=0; i<m_swcList.size(); i++) {
          pickingManager().registerObject(m_swcList[i]);
          for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
            pickingManager().registerObject(m_decomposedNodes[i][j]);
//            m_registeredSwcTreeNodeList.push_back(m_decomposedNodes[i][j]);
            m_registeredSwcTreeNodeList[nodeIndex++] = m_decomposedNodes[i][j];
          }
        }
      }

      m_registeredSwcList = m_swcList;

      m_swcPickingColors.resize(nodePairCount);
//      m_swcPickingColors.clear();
//      m_linePickingColors.clear();
      m_linePickingColors.resize(nodePairCount * 2);
//      m_pointPickingColors.clear();
//      m_sphereForConePickingColors.clear();
      m_pointPickingColors.resize(nodeCount);
      m_sphereForConePickingColors.resize(nodeCount);

      size_t nodePairIndex = 0;
      size_t nodeIndex = 0;

      for (size_t i=0; i < m_swcList.size(); i++) {
        glm::col4 pickingColor = pickingManager().colorOfObject(m_swcList[i]);
        glm::vec4 swcPickingColor(
              pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f,
            pickingColor[3]/255.f);
        for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
          m_swcPickingColors[nodePairIndex] = swcPickingColor;
          m_linePickingColors[nodePairIndex * 2] = swcPickingColor;
          m_linePickingColors[nodePairIndex * 2 + 1] = swcPickingColor;
          ++nodePairIndex;
//          m_swcPickingColors.push_back(swcPickingColor);
//          m_linePickingColors.push_back(swcPickingColor);
//          m_linePickingColors.push_back(swcPickingColor);
        }
        //      m_sphereForConePickingColors = m_swcPickingColors;
        //      m_sphereForConePickingColors.push_back(fPickingColor);
//        m_pointPickingColors.reserve(
//              m_pointPickingColors.size() + m_decomposedNodes[i].size());
//        m_sphereForConePickingColors.reserve(
//              m_sphereForConePickingColors.size() + m_decomposedNodes[i].size());
        for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
          pickingColor = pickingManager().colorOfObject(m_decomposedNodes[i][j]);
          glm::vec4 fPickingColor = glm::vec4(
                pickingColor[0]/255.f, pickingColor[1]/255.f,
              pickingColor[2]/255.f, pickingColor[3]/255.f);
          m_pointPickingColors[nodeIndex] = fPickingColor;
          m_sphereForConePickingColors[nodeIndex] = swcPickingColor;
          ++nodeIndex;
//          m_pointPickingColors.push_back(fPickingColor);
//          m_sphereForConePickingColors.push_back(swcPickingColor);
        }
      }

      m_coneRenderer.setDataPickingColors(&m_swcPickingColors);
      m_sphereRendererForCone.setDataPickingColors(&m_sphereForConePickingColors);
      m_lineRenderer.setDataPickingColors(&m_linePickingColors);
      m_sphereRenderer.setDataPickingColors(&m_pointPickingColors);
    }

    m_pickingObjectsRegistered = true;
    ZOUT(LTRACE(), 5) << "end";
  }
}

void Z3DSwcFilter::deregisterPickingObjects()
{
  if (m_enablePicking) {
    ZOUT(LTRACE(), 5) << "start";
    if (m_pickingObjectsRegistered) {
      for (size_t i=0; i<m_registeredSwcList.size(); i++) {
        pickingManager().deregisterObject(m_registeredSwcList[i]);
      }
      m_registeredSwcList.clear();

      if (m_swcTopologyMutable) {
        for (size_t i=0; i<m_registeredSwcTreeNodeList.size(); i++) {
          pickingManager().deregisterObject(m_registeredSwcTreeNodeList[i]);
        }
        m_registeredSwcTreeNodeList.clear();
      } else {
        std::set<ZSwcTree*> swcSet;
        swcSet.insert(m_origSwcList.begin(), m_origSwcList.end());
        for (auto iter = m_registeredSwcTreeNodeMap.begin();
             iter != m_registeredSwcTreeNodeMap.end();) {
          if (swcSet.count(iter->first) == 0) { //invalid swc
            std::vector<Swc_Tree_Node*> &nodeArray = iter->second;
            for (Swc_Tree_Node *tn : nodeArray) {
#ifdef _DEBUG_2
              std::cout << "Deregister node: " << tn << std::endl;
#endif
              pickingManager().deregisterObject(tn);
            }
            iter = m_registeredSwcTreeNodeMap.erase(iter);
          } else {
            ++iter;
          }
        }
      }
    }

    m_pickingObjectsRegistered = false;
    ZOUT(LTRACE(), 5) << "end";
  }
}

Swc_Tree_Node* Z3DSwcFilter::pickSwcNode(double x, double y)
{
  Swc_Tree_Node *tn = NULL;
  if (isNodePicking()) {
    const void* obj = pickObject(x, y);
    if (std::find(m_swcList.begin(), m_swcList.end(), obj) == m_swcList.end()) {
      tn = (Swc_Tree_Node*) obj;
    }
  }

  return tn;
}

QList<Swc_Tree_Node *> Z3DSwcFilter::pickSwcNode(const ZObject3d &ptArray)
{
  QList<Swc_Tree_Node*> nodeArray;

  if (isNodePicking()) {
    for (size_t i = 0; i < ptArray.size(); ++i) {
      int x = ptArray.getX(i);
      int y = ptArray.getY(i);
      const void* obj = pickObject(x, y);
      if (obj != NULL) {
        nodeArray.append((Swc_Tree_Node*) obj);
      }
    }
  }

  return nodeArray;
}

void Z3DSwcFilter::selectSwcNode(const ZObject3d &ptArray)
{
  QList<Swc_Tree_Node*> nodeArray = pickSwcNode(ptArray);

  emit treeNodeSelected(nodeArray, true);
}


void Z3DSwcFilter::setData(const std::vector<ZSwcTree*>& swcList)
{
  QMutexLocker locker(&m_dataValidMutex);

  m_origSwcList = swcList;

  if (m_swcTopologyMutable == false) {
    std::set<ZSwcTree*> swcSet;
    swcSet.insert(m_origSwcList.begin(), m_origSwcList.end());
    for (auto iter = m_decomposedNodeMap.begin();
         iter != m_decomposedNodeMap.end();) {
      if (swcSet.count(iter->first) == 0) { //invalid swc
        iter = m_decomposedNodeMap.erase(iter);
      } else {
        ++iter;
      }
    }
    for (auto iter = m_decomposedNodePairMap.begin();
         iter != m_decomposedNodePairMap.end();) {
      if (swcSet.count(iter->first) == 0) { //invalid swc
        iter = m_decomposedNodePairMap.erase(iter);
      } else {
        ++iter;
      }
    }
  }

  ZOUT(LTRACE(), 5) << "Load" << m_origSwcList.size() << "SWCs.";

  loadVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DSwcFilter::setData(const QList<ZSwcTree*>& swcList)
{
  std::vector<ZSwcTree*> swcListBuffer;
  swcListBuffer.insert(swcListBuffer.end(), swcList.begin(), swcList.end());
  setData(swcListBuffer);
}

bool Z3DSwcFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origSwcList.empty();
}

std::shared_ptr<ZWidgetsGroup> Z3DSwcFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Neurons", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_renderingPrimitive, 1);
    m_widgetsGroup->addChild(m_colorMode, 1);

    for (const auto& color : m_colorsForDifferentType) {
      m_widgetsGroup->addChild(*color, 1);
    }
    for (const auto& color : m_colorsForSubclassType) {
      m_widgetsGroup->addChild(*color, 1);
    }
    for (const auto& color : m_colorsForDifferentTopology) {
      m_widgetsGroup->addChild(*color, 1);
    }
    m_widgetsGroup->addChild(m_colorMapBranchType, 1);

    //    createColorMapperWidget(m_randomTreeColorMapper, m_randomColorWidgetGroup);
    //    createColorMapperWidget(m_individualTreeColorMapper,
    //                            m_individualColorWidgetGroup);
    //    for (std::map<int, ZVec4Parameter*>::iterator it = m_biocytinColorMapper.begin();
    //         it != m_biocytinColorMapper.end(); ++it) {
    //      m_colorsForBiocytinTypeWidgetsGroup.push_back(
    //            new ZWidgetsGroup(it->second, m_widgetsGroup, 1));
    //    }


    std::set<ZParameter*, _ParameterNameComp> cps;
    for (const auto& kv : m_randomColorParam.m_mapper) {
      cps.insert(kv.second.get());
    }
    for (auto p : cps) {
      m_widgetsGroup->addChild(*p, 2);
    }
    cps.clear();
    for (const auto& kv : m_individualColorParam.m_mapper) {
      cps.insert(kv.second.get());
    }
    for (auto p : cps) {
      m_widgetsGroup->addChild(*p, 2);
    }
    for (const auto& kv : m_biocytinColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }

    const std::vector<ZParameter*>& paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Size Scale")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Rendering Method")
        m_widgetsGroup->addChild(*para, 4);
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 5);
      else
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

void Z3DSwcFilter::renderOpaque(Z3DEye eye)
{
  if (m_swcList.empty())
    return;

  if (m_renderingPrimitive.isSelected("Normal")) {
    m_rendererBase.render(eye, m_sphereRendererForCone, m_coneRenderer);
  } else if (m_renderingPrimitive.isSelected("Line")) {
    m_rendererBase.render(eye, m_lineRenderer);
  } else /* (m_renderingPrimitive.get() == "Sphere") */{
    m_rendererBase.render(eye, m_lineRenderer, m_sphereRenderer);
  }
  renderBoundBox(eye);
}

void Z3DSwcFilter::renderTransparent(Z3DEye eye)
{
  if (m_swcList.empty())
    return;

  if (m_renderingPrimitive.isSelected("Normal")) {
    m_rendererBase.render(eye, m_sphereRendererForCone, m_coneRenderer);
  } else if (m_renderingPrimitive.isSelected("Line")) {
    m_rendererBase.render(eye, m_lineRenderer);
  } else /* (m_renderingPrimitive.get() == "Sphere") */{
    m_rendererBase.render(eye, m_lineRenderer, m_sphereRenderer);
  }
  renderBoundBox(eye);
}

void Z3DSwcFilter::renderPicking(Z3DEye eye)
{
  QMutexLocker locker(&m_dataValidMutex);

  if (m_dataIsInvalid) {
    return;
  }

  if (m_enablePicking) {
    if (m_swcList.empty())
      return;

    if (!m_pickingObjectsRegistered) {
      QElapsedTimer timer;
      timer.restart();
      registerPickingObjects();
      LINFO() << "Registering time:" << timer.elapsed();
    }

    if (isNodePicking()) {
      if (m_renderingPrimitive.isSelected("Normal")) {
        m_rendererBase.renderPicking(eye, m_coneRenderer, m_sphereRenderer);
      } else {
        m_rendererBase.renderPicking(eye, m_lineRenderer, m_sphereRenderer);
      }
    } else {
      if (m_renderingPrimitive.isSelected("Normal")) {
        m_rendererBase.renderPicking(eye, m_coneRenderer, m_sphereRenderer);
      } else if (m_renderingPrimitive.isSelected("Line")) {
        m_rendererBase.renderPicking(eye, m_lineRenderer);
      }
    }
//    updatePickingTexSize();
  }
}

void Z3DSwcFilter::addSelectionBox(
    const std::pair<Swc_Tree_Node *, Swc_Tree_Node *> &nodePair,
    std::vector<glm::vec3> &lines)
{
  Swc_Tree_Node *n1 = nodePair.first;
  Swc_Tree_Node *n2 = nodePair.second;
  glm::vec3 bPos = glm::applyMatrix(coordTransform(), glm::vec3(n1->node.x, n1->node.y, n1->node.z));
  glm::vec3 tPos = glm::applyMatrix(coordTransform(), glm::vec3(n2->node.x, n2->node.y, n2->node.z));
  float bRadius = std::max(.5, n1->node.d) * m_rendererBase.sizeScale();
  float tRadius = std::max(.5, n2->node.d) * m_rendererBase.sizeScale();
  glm::vec3 axis = tPos - bPos;
  if (glm::length(axis) < std::numeric_limits<float>::epsilon() * 1e2) {
    LWARN() << "node and parent node too close";
    return;
  }
  // vector perpendicular to axis
  glm::vec3 v1, v2;
  glm::getOrthogonalVectors(axis, v1, v2);

  glm::vec3 p1 = bPos - bRadius * v1  - v2 * bRadius;
  glm::vec3 p2 = bPos - v1 * bRadius + v2 * bRadius;
  glm::vec3 p3 = bPos + v1 * bRadius + v2 * bRadius;
  glm::vec3 p4 = bPos + v1 * bRadius - v2 * bRadius;
  glm::vec3 p5 = tPos - v1 * tRadius - v2 * tRadius;
  glm::vec3 p6 = tPos - v1 * tRadius + v2 * tRadius;
  glm::vec3 p7 = tPos + v1 * tRadius + v2 * tRadius;
  glm::vec3 p8 = tPos + v1 * tRadius - v2 * tRadius;

  lines.push_back(p1); lines.push_back(p2);
  lines.push_back(p2); lines.push_back(p3);
  lines.push_back(p3); lines.push_back(p4);
  lines.push_back(p4); lines.push_back(p1);

  lines.push_back(p5); lines.push_back(p6);
  lines.push_back(p6); lines.push_back(p7);
  lines.push_back(p7); lines.push_back(p8);
  lines.push_back(p8); lines.push_back(p5);

  lines.push_back(p1); lines.push_back(p5);
  lines.push_back(p2); lines.push_back(p6);
  lines.push_back(p3); lines.push_back(p7);
  lines.push_back(p4); lines.push_back(p8);
}

void Z3DSwcFilter::addSelectionBox(
    const Swc_Tree_Node *tn, std::vector<glm::vec3> &lines)
{
  float radius = std::max(.5, tn->node.d) * m_rendererBase.sizeScale();
  glm::vec3 cent = glm::applyMatrix(coordTransform(), glm::vec3(tn->node.x, tn->node.y, tn->node.z));
  float xmin = cent.x - radius;
  float xmax = cent.x + radius;
  float ymin = cent.y - radius;
  float ymax = cent.y + radius;
  float zmin = cent.z - radius;
  float zmax = cent.z + radius;
  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmin, ymax, zmax);

  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmax, ymax, zmin);
  lines.emplace_back(xmax, ymax, zmax);

  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmax, ymax, zmin);

  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmax);
  lines.emplace_back(xmax, ymax, zmax);

  lines.emplace_back(xmin, ymin, zmin);
  lines.emplace_back(xmin, ymax, zmin);
  lines.emplace_back(xmax, ymin, zmin);
  lines.emplace_back(xmax, ymax, zmin);

  lines.emplace_back(xmin, ymin, zmax);
  lines.emplace_back(xmin, ymax, zmax);
  lines.emplace_back(xmax, ymin, zmax);
  lines.emplace_back(xmax, ymax, zmax);
}

void Z3DSwcFilter::updateBiocytinWidget()
{
  if (GET_APPLICATION_NAME == "Biocytin") {
    // do nothing if types don't change
    if (m_allNodeType.size() != m_biocytinColorMapper.size() ||
        !std::equal(m_biocytinColorMapper.begin(), m_biocytinColorMapper.end(),
                    m_allNodeType.begin(), _KeyEqual())) {
      // remove old type color parameters from widget, will add new ones later
      if (m_widgetsGroup) {
        for (auto& kv : m_biocytinColorMapper) {
          m_widgetsGroup->removeChild(*kv.second);
        }
      }

      // remove not-in-use types
      for (auto it = m_biocytinColorMapper.begin(); it != m_biocytinColorMapper.end(); ) {
        if (m_allNodeType.find(it->first) == m_allNodeType.end()) {
          removeParameter(*it->second);
          it = m_biocytinColorMapper.erase(it);
        } else {
          ++it;
        }
      }

      // create color parameters for new types
      std::set<int> newTypes;
      std::set_difference(m_allNodeType.begin(), m_allNodeType.end(),
                          m_biocytinColorMapper.begin(), m_biocytinColorMapper.end(),
                          std::inserter(newTypes, newTypes.end()),
                          _KeyLess());
      for (auto type : newTypes) {
        m_colorScheme.setColorScheme(ZSwcColorScheme::BIOCYTIN_TYPE_COLOR);
        QString guiname = type >= m_guiNameList.size() ? QString("Type %1 Color").arg(type) : m_guiNameList[type];
        QColor color = m_colorScheme.getColor(type);
        m_biocytinColorMapper.insert(std::make_pair(type,
                                                    std::make_unique<ZVec4Parameter>(guiname,
                                                                                     glm::vec4(color.redF(),
                                                                                               color.greenF(),
                                                                                               color.blueF(),
                                                                                               1.f))));
        m_biocytinColorMapper[type]->setStyle("COLOR");
        connect(m_biocytinColorMapper[type].get(), &ZVec4Parameter::valueChanged,
            this, &Z3DSwcFilter::prepareColor);
        addParameter(*m_biocytinColorMapper[type]);
      }

      // update widget group
      if (m_widgetsGroup) {
        for (const auto& kv : m_biocytinColorMapper) {
          m_widgetsGroup->addChild(*kv.second, 2);
        }
        m_widgetsGroup->emitWidgetsGroupChangedSignal();
      }
    }
  }
}

void Z3DSwcFilter::addTreeColorWidget(TreeColorParam &param)
{
  if (m_widgetsGroup) {
    std::set<ZParameter*, _ParameterNameComp> cps;
    for (const auto& kv : param.m_mapper) {
      cps.insert(kv.second.get());
    }
    for (auto p : cps) {
      m_widgetsGroup->addChild(*p, 2);
    }

    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }
}

/*
void Z3DSwcFilter::updateWidgetGroup()
{
#ifdef _DEBUG_
  std::cout << "updateWidgetGroup: " << m_individualTreeColorMapper.size() << std::endl;
#endif

  // update widget group
  if (m_widgetsGroup) {
    std::set<ZParameter*, _ParameterNameComp> cps;
    for (const auto& kv : m_randomColorParam.m_mapper) {
      cps.insert(kv.second.get());
    }
    for (auto p : cps) {
      m_widgetsGroup->addChild(*p, 2);
    }

    cps.clear();
    for (const auto& kv : m_randomColorParam.m_mapper) {
      cps.insert(kv.second.get());
    }
    for (auto p : cps) {
      m_widgetsGroup->addChild(*p, 2);
    }
    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }
}
*/

std::shared_ptr<ZVec4Parameter> Z3DSwcFilter::getTreeColorParam(
    TreeColorParam &colorParam, int index)
{
  for (int i = (int) colorParam.m_paramList.size(); i <= index; ++i) {
    QColor color = colorParam.m_scheme.getColor(i);
    std::shared_ptr<ZVec4Parameter> param = std::make_shared<ZVec4Parameter>(
          colorParam.m_prefix + QString(" %1 Color").arg(i + 1),
          glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f));
    param->setStyle("COLOR");
    connect(param.get(), &ZVec4Parameter::valueChanged,
        this, &Z3DSwcFilter::prepareColor);
    colorParam.m_paramList.push_back(param);
  }

  return colorParam.m_paramList[index];
}

std::shared_ptr<ZVec4Parameter> Z3DSwcFilter::getIndvidualColorParam(int index)
{
  return getTreeColorParam(m_individualColorParam, index);
  /*
  for (int i = (int) m_individualTreeColorList.size(); i <= index; ++i) {
    QColor color = m_individualColorScheme.getColor(i);
    std::shared_ptr<ZVec4Parameter> param = std::make_shared<ZVec4Parameter>(
          QString("Swc %1 Color").arg(i + 1),
          glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f));
    param->setStyle("COLOR");
    connect(param.get(), &ZVec4Parameter::valueChanged,
        this, &Z3DSwcFilter::prepareColor);
    m_individualTreeColorList.push_back(param);
  }

  return m_individualTreeColorList[index];
  */
}

std::shared_ptr<ZVec4Parameter> Z3DSwcFilter::getRandomColorParam(int index)
{
  return getTreeColorParam(m_randomColorParam, index);
  /*
  for (int i = (int) m_randomTreeColorList.size(); i <= index; ++i) {
    QColor color = m_randomColorScheme.getColor(i);
    std::shared_ptr<ZVec4Parameter> param = std::make_shared<ZVec4Parameter>(
          QString("Random Swc %1 Color").arg(i + 1),
          glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f));
    param->setStyle("COLOR");
    connect(param.get(), &ZVec4Parameter::valueChanged,
        this, &Z3DSwcFilter::prepareColor);
    m_randomTreeColorList.push_back(param);
  }

  return m_randomTreeColorList[index];
  */
}

bool Z3DSwcFilter::updateTreeColorParameter(TreeColorParam &colorParam,
    const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  std::map<ZSwcTree*, size_t> newSources;
  std::set_difference(
        sourceIndexMapper.begin(), sourceIndexMapper.end(),
        colorParam.m_mapper.begin(), colorParam.m_mapper.end(),
        std::inserter(newSources, newSources.end()),
        _KeyLess());
  bool updating = !newSources.empty();
  for (const auto& kv : newSources) {
    colorParam.m_mapper.insert(
          std::make_pair(kv.first, getTreeColorParam(colorParam, kv.second)));
    addParameter(*colorParam.m_mapper[kv.first]);
  }

  return updating;
}

void Z3DSwcFilter::updateTreeColorWidget(TreeColorParam &param)
{
  auto sourceIndexMapper = getSourceIndexMapper();

  removeTreeColorWidget();
  removeObsoleteColorparam(param, sourceIndexMapper);
  updateTreeColorParameter(param, sourceIndexMapper);
  addTreeColorWidget(param);
}

bool Z3DSwcFilter::updateIndividualColorParameter(
    const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  return updateTreeColorParameter(m_individualColorParam, sourceIndexMapper);
  /*
  std::map<ZSwcTree*, size_t> newSources;
  std::set_difference(
        sourceIndexMapper.begin(), sourceIndexMapper.end(),
        m_individualTreeColorMapper.begin(), m_individualTreeColorMapper.end(),
        std::inserter(newSources, newSources.end()),
                      _KeyLess());
  bool updating = !newSources.empty();
  for (const auto& kv : newSources) {
    m_individualTreeColorMapper.insert(
          std::make_pair(kv.first, getIndvidualColorParam(kv.second)));
    addParameter(*m_individualTreeColorMapper[kv.first]);
  }

  return updating;
  */
}

bool Z3DSwcFilter::updateRandomColorParameter(
    const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  return updateTreeColorParameter(m_randomColorParam, sourceIndexMapper);
  /*
  std::map<ZSwcTree*, size_t> newSources;
  std::set_difference(
        sourceIndexMapper.begin(), sourceIndexMapper.end(),
        m_randomTreeColorMapper.begin(), m_randomTreeColorMapper.end(),
        std::inserter(newSources, newSources.end()),
                      _KeyLess());
  bool updating = !newSources.empty();
  for (const auto& kv : newSources) {
    m_randomTreeColorMapper.insert(
          std::make_pair(kv.first, getIndvidualColorParam(kv.second)));
    addParameter(*m_randomTreeColorMapper[kv.first]);
  }

  return updating;
  */
}

#if 0
bool Z3DSwcFilter::updateColorParameter(
    const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  std::map<ZSwcTree*, size_t> newSources;
  std::set_difference(
        sourceIndexMapper.begin(), sourceIndexMapper.end(),
        m_randomTreeColorMapper.begin(), m_randomTreeColorMapper.end(),
        std::inserter(newSources, newSources.end()),
                      _KeyLess());
  bool updating = !newSources.empty();
  for (const auto& kv : newSources) {
    m_randomTreeColorMapper.insert(
          std::make_pair(kv.first, getRandomColorParam(kv.second)));

    /*
          std::make_pair(kv.first,
                         std::make_unique<ZVec4Parameter>(
                           QString("Random Swc %1 Color").arg(kv.second + 1),
                           glm::vec4(ZRandom::instance().randReal<float>(),
                                     ZRandom::instance().randReal<float>(),
                                     ZRandom::instance().randReal<float>(),
                                     1.f))));
*/
    m_individualTreeColorMapper.insert(
          std::make_pair(kv.first, getIndvidualColorParam(kv.second)));

//    m_randomTreeColorMapper[kv.first]->setStyle("COLOR");
//    connect(m_randomTreeColorMapper[kv.first].get(), &ZVec4Parameter::valueChanged,
//        this, &Z3DSwcFilter::prepareColor);

    addParameter(*m_randomTreeColorMapper[kv.first]);
    addParameter(*m_individualTreeColorMapper[kv.first]);
  }

  return updating;
}
#endif

void Z3DSwcFilter::removeTreeColorWidget()
{
  if (m_widgetsGroup) {
    for (auto& kv : m_individualColorParam.m_mapper) {
      m_widgetsGroup->removeChild(*kv.second);
    }

    for (auto& kv : m_randomColorParam.m_mapper) {
      m_widgetsGroup->removeChild(*kv.second);
    }
  }
}

bool Z3DSwcFilter::removeObsoleteColorparam(
    TreeColorParam &param, const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  // do nothing if sources don't change
  if (sourceIndexMapper.size() != param.m_mapper.size() ||
      !std::equal(param.m_mapper.begin(), param.m_mapper.end(),
                  sourceIndexMapper.begin(), _KeyEqual())) {
    // remove not in use sources
    for (auto it = param.m_mapper.begin(); it != param.m_mapper.end(); ) {
      if (sourceIndexMapper.find(it->first) == sourceIndexMapper.end()) {
        removeParameter(*it->second);
        it = param.m_mapper.erase(it);
      } else {
        ++it;
      }
    }

    return true;
  }

  return false;
}
#if 0
bool Z3DSwcFilter::updateTreeColorParameter(
    const std::map<ZSwcTree *, size_t> &sourceIndexMapper)
{
  // do nothing if sources don't change
  if (sourceIndexMapper.size() != m_randomTreeColorMapper.size() ||
      !std::equal(m_randomTreeColorMapper.begin(), m_randomTreeColorMapper.end(),
                  sourceIndexMapper.begin(), _KeyEqual())) {
    // remove old source color parameters from widget, will add new ones later
    if (m_widgetsGroup) {
      for (auto& kv : m_randomTreeColorMapper) {
        m_widgetsGroup->removeChild(*kv.second);
      }
      for (auto& kv : m_individualTreeColorMapper) {
        m_widgetsGroup->removeChild(*kv.second);
      }
    }

    // remove not in use sources
    for (auto it = m_randomTreeColorMapper.begin(); it != m_randomTreeColorMapper.end(); ) {
      if (sourceIndexMapper.find(it->first) == sourceIndexMapper.end()) {
        removeParameter(*it->second);
        it = m_randomTreeColorMapper.erase(it);
      } else {
        ++it;
      }
    }
    for (auto it = m_individualTreeColorMapper.begin(); it != m_individualTreeColorMapper.end(); ) {
      if (sourceIndexMapper.find(it->first) == sourceIndexMapper.end()) {
        removeParameter(*it->second);
        it = m_individualTreeColorMapper.erase(it);
      } else {
        ++it;
      }
    }

    return true;
  }

  return false;
}
#endif

void Z3DSwcFilter::updateColorMapBranchType()
{
  m_colorMapBranchType.blockSignals(true);
  if (m_allNodeType.empty())
    m_colorMapBranchType.get().reset();
  else
    m_colorMapBranchType.get().reset(m_allNodeType.begin(), m_allNodeType.end(),
                                     glm::col4(0, 0, 255, 255), glm::col4(255, 0, 0, 255));
  m_colorMapBranchType.blockSignals(false);
}

void Z3DSwcFilter::prepareNodePairData(
    const Swc_Tree_Node *n1, const Swc_Tree_Node *n2)
{
  bool checkRadius = m_renderingPrimitive.isSelected("Normal");
  if (checkRadius && n1->node.d < std::numeric_limits<double>::epsilon() &&
      n2->node.d < std::numeric_limits<double>::epsilon()) {
    checkRadius = false;
    QMessageBox::information(QApplication::activeWindow(),
                             qApp->applicationName(),
                             "Reset SWC Rendering Mode.\n"
                             "SWC contains segments with zero radius. "
                             "The geometrical primitive of SWC rendering "
                             "will be set to 'Line' to "
                             "make those segments visible.");

    m_renderingPrimitive.select("Line");
  }

  glm::vec4 baseAndbRadius, axisAndtRadius;
  // make sure base has smaller radius.
  if (Swc_Tree_Node_Const_Data(n1)->d <= Swc_Tree_Node_Const_Data(n2)->d) {
    baseAndbRadius = glm::vec4(n1->node.x, n1->node.y, n1->node.z, n1->node.d);
    axisAndtRadius = glm::vec4(n2->node.x - n1->node.x,
                               n2->node.y - n1->node.y,
                               n2->node.z - n1->node.z, n2->node.d);
  } else {
    baseAndbRadius = glm::vec4(n2->node.x, n2->node.y, n2->node.z, n2->node.d);
    axisAndtRadius = glm::vec4(n1->node.x - n2->node.x,
                               n1->node.y - n2->node.y,
                               n1->node.z - n2->node.z, n1->node.d);
  }
  m_baseAndBaseRadius.push_back(baseAndbRadius);
  m_axisAndTopRadius.push_back(axisAndtRadius);
  m_lines.push_back(baseAndbRadius.xyz());
  m_lines.push_back(glm::vec3(baseAndbRadius.xyz()) + glm::vec3(axisAndtRadius.xyz()));
}

void Z3DSwcFilter::updateColorWidget()
{
#if !defined(_FLYEM_)
  std::map<ZSwcTree*, size_t> sourceIndexMapper;
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    sourceIndexMapper[m_origSwcList[i]] = i;
  }

  // remove old param
  bool updatingTreeColorParam = updateTreeColorParameter(sourceIndexMapper);

  // create color parameters for new sources
  bool updatingTreeColorParam2 = updateColorParameter(sourceIndexMapper);

  updatingTreeColorParam = updatingTreeColorParam || updatingTreeColorParam2;

#if 1
  if (updatingTreeColorParam) {
    updateWidgetGroup();
  }
#endif
#endif
}

std::map<ZSwcTree*, size_t> Z3DSwcFilter::getSourceIndexMapper() const
{
  std::map<ZSwcTree*, size_t> sourceIndexMapper;
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    sourceIndexMapper[m_origSwcList[i]] = i;
  }

  return sourceIndexMapper;
}

/*
bool Z3DSwcFilter::updateColorParameter(EColorMode mode)
{
  bool updated = false;

  switch (mode) {
  case EColorMode::INDIVIDUAL:
    updated = updateIndividualColorParameter(getSourceIndexMapper());
    break;
  case EColorMode::RANDOM:
    updated = updateRandomColorParameter(getSourceIndexMapper());
    break;
  case EColorMode::BIOCYTIN_BRANCH_TYPE:
//    updated = updateBiocytinColorParameter();
    break;
  case EColorMode::BRANCH_TYPE:
//    updated = updateBranchTypeColorParameter();
    break;
  default:
    break;
  }

  return updated;
}
*/

void Z3DSwcFilter::prepareDataForImmutable()
{
  if (!m_dataIsInvalid)
    return;

  ZOUT(LTRACE(), 5) << "Prepare swc data";

  QElapsedTimer timer;

  timer.start();

  decomposeSwcTreeForImmutable();

  LINFO() << "Decomposing time:" << timer.elapsed();

  // get min max of type for colormap
  m_colorMapBranchType.blockSignals(true);
  if (m_allNodeType.empty())
    m_colorMapBranchType.get().reset();
  else
    m_colorMapBranchType.get().reset(m_allNodeType.begin(), m_allNodeType.end(),
                                     glm::col4(0, 0, 255, 255), glm::col4(255, 0, 0, 255));
  m_colorMapBranchType.blockSignals(false);

  timer.restart();

  deregisterPickingObjects();

  LINFO() << "Deregistering time:" << timer.elapsed();

  //convert swc to format that glsl can use
  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_pointAndRadius.clear();
  m_lines.clear();

  timer.restart();


  for (auto &t : m_decomposedNodePairMap) {
    auto &nodePairArray = t.second;
    for (auto &nodePair : nodePairArray) {
      prepareNodePairData(nodePair.first, nodePair.second);
    }
  }

  for (auto &t : m_decomposedNodeMap) {
    auto &nodeArray = t.second;
    for (auto &tn : nodeArray) {
#ifdef _DEBUG_2
      std::cout << "Decomposed node: " << tn << std::endl;
#endif
      m_pointAndRadius.emplace_back(
            tn->node.x, tn->node.y, tn->node.z, tn->node.d);
    }
  }

  ZOUT(LINFO(), 5) << "Premitive time:" << timer.elapsed();
  /*
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    m_sourceColorMapper.insert(std::pair<std::string, size_t>(m_origSwcList[i]->source(), 0));
    */

  //Causing lag
  if (m_enableCutting) {
    initializeCutRange();
  }
  initializeRotationCenter();

  ZOUT(LINFO(), 5) << "Updating swc widgets";

  // update widget if any type/swc added/removed
  updateBiocytinWidget();

  updateColorWidget();

#if 0
  std::map<ZSwcTree*, size_t> sourceIndexMapper;
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    sourceIndexMapper[m_origSwcList[i]] = i;
  }

  // remove old param
  bool updatingTreeColorParam = updateTreeColorParameter(sourceIndexMapper);

  // create color parameters for new sources
  bool updatingTreeColorParam2 = updateColorParameter(sourceIndexMapper);

  updatingTreeColorParam = updatingTreeColorParam || updatingTreeColorParam2;

#if 1
  if (updatingTreeColorParam) {
    updateWidgetGroup();
  }
#endif
#endif

  ZOUT(LINFO(), 5) << "Setting renderers ...";

  m_coneRenderer.setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
  m_lineRenderer.setData(&m_lines);
  m_sphereRenderer.setData(&m_pointAndRadius);
  m_sphereRendererForCone.setData(&m_pointAndRadius);
  prepareColorForImmutable();

  ZOUT(LINFO(), 5) << "Adjusting widgets ...";
  updateColorWidgets();
  m_dataIsInvalid = false;

  ZOUT(LINFO(), 5) << "SWC data ready";
}

void Z3DSwcFilter::prepareData()
{
  QMutexLocker locker(&m_dataValidMutex);

  if (!m_dataIsInvalid)
    return;

  if (m_swcTopologyMutable == false) {
    prepareDataForImmutable();
    return;
  }

  ZOUT(LTRACE(), 5) << "Prepare swc data";

  QElapsedTimer timer;

  timer.start();

  decomposeSwcTree();

  LINFO() << "Decomposing time:" << timer.elapsed();

  // get min max of type for colormap
  updateColorMapBranchType();

  timer.restart();

  deregisterPickingObjects();

  LINFO() << "Deregistering time:" << timer.elapsed();

  //convert swc to format that glsl can use
  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_pointAndRadius.clear();
  m_lines.clear();

  timer.restart();

  bool checkRadius = m_renderingPrimitive.isSelected("Normal");
  for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
    for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
      Swc_Tree_Node *n1 = m_decompsedNodePairs[i][j].first;
      Swc_Tree_Node *n2 = m_decompsedNodePairs[i][j].second;

      if (checkRadius && n1->node.d < std::numeric_limits<double>::epsilon() &&
          n2->node.d < std::numeric_limits<double>::epsilon()) {
        checkRadius = false;
        QMessageBox::information(QApplication::activeWindow(),
                                 qApp->applicationName(),
                                 "Reset SWC Rendering Mode.\n"
                                 "SWC contains segments with zero radius. "
                                 "The geometrical primitive of SWC rendering "
                                 "will be set to 'Line' to "
                                 "make those segments visible.");
        m_renderingPrimitive.select("Line");
      }

      glm::vec4 baseAndbRadius, axisAndtRadius;
      // make sure base has smaller radius.
      if (Swc_Tree_Node_Const_Data(n1)->d <= Swc_Tree_Node_Const_Data(n2)->d) {
        baseAndbRadius = glm::vec4(n1->node.x, n1->node.y, n1->node.z, n1->node.d);
        axisAndtRadius = glm::vec4(n2->node.x - n1->node.x,
                                   n2->node.y - n1->node.y,
                                   n2->node.z - n1->node.z, n2->node.d);
      } else {
        baseAndbRadius = glm::vec4(n2->node.x, n2->node.y, n2->node.z, n2->node.d);
        axisAndtRadius = glm::vec4(n1->node.x - n2->node.x,
                                   n1->node.y - n2->node.y,
                                   n1->node.z - n2->node.z, n1->node.d);
      }
      m_baseAndBaseRadius.push_back(baseAndbRadius);
      m_axisAndTopRadius.push_back(axisAndtRadius);
      m_lines.push_back(baseAndbRadius.xyz());
      m_lines.push_back(glm::vec3(baseAndbRadius.xyz()) + glm::vec3(axisAndtRadius.xyz()));
    }
    for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
      Swc_Tree_Node *tn = m_decomposedNodes[i][j];
      m_pointAndRadius.emplace_back(tn->node.x, tn->node.y, tn->node.z, tn->node.d);
    }
  }

  ZOUT(LINFO(), 5) << "Premitive time:" << timer.elapsed();
  /*
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    m_sourceColorMapper.insert(std::pair<std::string, size_t>(m_origSwcList[i]->source(), 0));
    */

  //Causing lag
  if (m_enableCutting) {
    initializeCutRange();
  }
  initializeRotationCenter();

  ZOUT(LINFO(), 5) << "Updating swc widgets";

  // update widget if any type/swc added/removed
  updateBiocytinWidget();

//  updateColorWidget();

#if 0
  std::map<ZSwcTree*, size_t> sourceIndexMapper;
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    sourceIndexMapper[m_origSwcList[i]] = i;
  }

  // remove old param
  bool updatingTreeColorParam = updateTreeColorParameter(sourceIndexMapper);

  // create color parameters for new sources
  bool updatingTreeColorParam2 = updateColorParameter(sourceIndexMapper);

  updatingTreeColorParam = updatingTreeColorParam || updatingTreeColorParam2;

#if 1
  if (updatingTreeColorParam) {
    updateWidgetGroup();
  }
#endif
#endif

  ZOUT(LINFO(), 5) << "Setting renderers ...";

  m_coneRenderer.setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
  m_lineRenderer.setData(&m_lines);
  m_sphereRenderer.setData(&m_pointAndRadius);
  m_sphereRendererForCone.setData(&m_pointAndRadius);

  ZOUT(LINFO(), 5) << "Adjusting widgets ...";
  updateColorWidgets();

  prepareColor();

  m_dataIsInvalid = false;

  ZOUT(LINFO(), 5) << "SWC data ready";
}

void Z3DSwcFilter::treeBound(ZSwcTree* tree, ZBBox<glm::dvec3>& res) const
{
  res.reset();

  tree->updateIterator(1);   //depth first
  Swc_Tree_Node *tn = tree->begin();
  while (Swc_Tree_Node_Is_Virtual(tn)) {
    tn = tree->next();
  }
  if (!tn) {
    return;
  }

  ZBBox<glm::dvec3> nodeBound;
  treeNodeBound(tn, nodeBound);

  for (tn = tree->next(); tn != tree->end(); tn = tree->next()) {
    treeNodeBound(tn, nodeBound);
    res.expand(nodeBound);
  }


#ifdef _DEBUG_2
  std::cout << getCoordScales().z << std::endl;
  std::cout << "Bound: " << res << std::endl;
#endif
}

void Z3DSwcFilter::treeNodeBound(Swc_Tree_Node* tn, ZBBox<glm::dvec3>& result) const
{
  glm::dvec3 cent = glm::dvec3(glm::applyMatrix(coordTransform(), glm::vec3(tn->node.x, tn->node.y, tn->node.z)));
  result.setMinCorner(cent
                      - std::max(.5, tn->node.d) * (m_renderingPrimitive.isSelected("Line") ? 1 : sizeScale()));
  result.setMaxCorner(cent
                      + std::max(.5, tn->node.d) * (m_renderingPrimitive.isSelected("Line") ? 1 : sizeScale()));
}

void Z3DSwcFilter::notTransformedTreeBound(ZSwcTree* tree, ZBBox<glm::dvec3>& res) const
{
  res.reset();

  tree->updateIterator(1);   //depth first
  Swc_Tree_Node *tn = tree->begin();
  while (Swc_Tree_Node_Is_Virtual(tn)) {
    tn = tree->next();
  }
  if (!tn) {
    return;
  }

  ZBBox<glm::dvec3> nodeBound;
  treeNodeBound(tn, nodeBound);
  res.expand(nodeBound);

  for (tn = tree->next(); tn != tree->end(); tn = tree->next()) {
    notTransformedTreeNodeBound(tn, nodeBound);
    res.expand(nodeBound);
  }
}

void Z3DSwcFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  ZBBox<glm::dvec3> treeBound;
  for (auto swcTree : m_origSwcList) {
    notTransformedTreeBound(swcTree, treeBound);
    m_notTransformedBoundBox.expand(treeBound);
  }
}

void Z3DSwcFilter::addSelectionBox(
    const std::vector<SwcTreeNode::Pair> &nodePairList)
{
  for (size_t j=0; j< nodePairList.size(); j++) {
    addSelectionBox(nodePairList[j], m_selectionLines);
  }
}

void Z3DSwcFilter::addSelectionBox(const std::vector<Swc_Tree_Node *> &nodeList)
{
  for (size_t j=0; j< nodeList.size(); j++) {
    addSelectionBox(nodeList[j], m_selectionLines);
  }
}

void Z3DSwcFilter::addSelectionLinesForImmutable()
{
  if (m_swcList.size() > 0) {
    for (std::vector<ZSwcTree*>::iterator it=m_swcList.begin();
         it != m_swcList.end(); it++) {
      ZSwcTree *tree = *it;
      if (tree->isSelected()) {
        addSelectionBox(m_decomposedNodePairMap.at(tree));
        addSelectionBox(m_decomposedNodeMap.at(tree));
      } else {
        const std::set<Swc_Tree_Node*> nodeSet = tree->getSelectedNode();
        for (std::set<Swc_Tree_Node*>::const_iterator it=nodeSet.begin();
             it != nodeSet.end(); it++) {
          addSelectionBox(*it, m_selectionLines);
        }
      }
    }
  }
}

void Z3DSwcFilter::addSelectionLines()
{
  QMutexLocker locker(&m_dataValidMutex);

  if (m_dataIsInvalid) {
    return;
  }

  if (m_swcTopologyMutable == false) {
    addSelectionLinesForImmutable();
    return;
  }

  if (m_swcList.size() > 0) {
    for (std::vector<ZSwcTree*>::iterator it=m_swcList.begin();
         it != m_swcList.end(); it++) {
      ZSwcTree *tree = *it;
      if (tree->isSelected()) {
        int index = -1;
        for (size_t i = 0; i<m_swcList.size(); i++) {
          if (m_swcList.at(i) == tree) {
            index = i;
            break;
          }
        }
        if (index == -1) {
          if (tree->isVisible()) {
            LERROR() << "selected swc not found.. Need Check..";
          }
          continue;
        }

        for (size_t j=0; j<m_decompsedNodePairs[index].size(); j++) {
          addSelectionBox(m_decompsedNodePairs[index][j], m_selectionLines);
        }

        for (size_t j=0; j<m_decomposedNodes[index].size(); j++) {
          Swc_Tree_Node *tn = m_decomposedNodes[index][j];
          if (SwcTreeNode::isRoot(tn) && !SwcTreeNode::hasChild(tn)) {
            addSelectionBox(m_decomposedNodes[index][j], m_selectionLines);
          }
        }
      } else {
        const std::set<Swc_Tree_Node*> nodeSet = tree->getSelectedNode();
        for (std::set<Swc_Tree_Node*>::const_iterator it=nodeSet.begin();
             it != nodeSet.end(); it++) {
          addSelectionBox(*it, m_selectionLines);
        }
      }
    }
  }
}

void Z3DSwcFilter::notTransformedTreeNodeBound(Swc_Tree_Node* tn, ZBBox<glm::dvec3>& result) const
{
  glm::dvec3 cent(tn->node.x, tn->node.y, tn->node.z);
  result.setMinCorner(
        cent - std::max(.5, tn->node.d) * (m_renderingPrimitive.isSelected("Line") ? 1 : sizeScale()));
  result.setMaxCorner(
        cent + std::max(.5, tn->node.d) * (m_renderingPrimitive.isSelected("Line") ? 1 : sizeScale()));
}

glm::vec4 Z3DSwcFilter::getColorByDirection(Swc_Tree_Node *tn)
{
  if (SwcTreeNode::type(tn) == 1) {
     return glm::vec4(0.8, 0.8, 0.8, 1.f);
  }

  ZPoint vec = SwcTreeNode::localDirection(tn, 15);

  vec.normalize();
  ZPoint colorVec;
/*
  ZPoint colorCode[4] = {
    ZPoint(0, 1, 0), ZPoint(1, 0, 0), ZPoint(0, 0, 1), ZPoint(1, 0, 1)
  };
*/

  ZPoint colorCode[4] = {
    ZPoint(1, 0, 0), ZPoint(0, 0, 1), ZPoint(1, 0, 1), ZPoint(0, 1, 0)
  };

  ZPoint axis[4] = {
    ZPoint(1, 0, 0), ZPoint(0, 1, 0), ZPoint(-1, 0, 0), ZPoint(0, -1, 0)
  };

  /*
  ZPoint axis[4] = {
    ZPoint(0.8801, -0.1720, 0), ZPoint(-0.2595, 0.8541, 0),
    ZPoint(-0.4797, 0.5570, 0), ZPoint(-0.0106, -0.2400, 0)
  };
  */

#if 0
  //from gmm
  ZPoint axis[4] = {
    ZPoint(-0.4586,    0.8887, 0), ZPoint(-0.7185,    0.6955, 0),
    ZPoint(0.4059,   -0.9139, 0), ZPoint(0.7220,   -0.6919, 0)
  };
#endif

#if 0
  //Direction for the TEM paper
  ZPoint axis[4] = {
    ZPoint(-0.0872,    0.9962, 0), ZPoint(-0.7185,    0.6955, 0),
    ZPoint(0.0872 ,  -0.9962, 0), ZPoint(0.9659,   -0.2588, 0)
  };
#endif

#if 0
  //from fft
  std::cout << "fft direction" << std::endl;
  ZPoint axis[4] = {
    ZPoint(-0.4276, 0.8965, 0), ZPoint(-0.8400,    0.3506, 0),
    ZPoint(0.1515,   -0.9806, 0), ZPoint(0.9368,   -0.2822, 0)
  };
#endif


  double angle[4];
  int axisIndex[2] = { 0, 0 };
  for (int k = 0; k < 4; ++k) {
    angle[k] = Vector_Angle2(
          vec.x(), vec.y(), axis[k].x(), axis[k].y(), TRUE);
  }

  double minAngle = angle[0];
  for (int k = 1; k < 4; ++k) {
    if (minAngle > angle[k]) {
      minAngle = angle[k];
      axisIndex[0] = k;
    }
  }

  axisIndex[1] = axisIndex[0] - 1;
  if (axisIndex[1] < 0) {
    axisIndex[1] = 3;
  }

#ifdef _DEBUG_2
  std::cout << axisIndex[0] << " " << axisIndex[1] << std::endl;
  std::cout << angle[axisIndex[0]] << " " << angle[axisIndex[1]] << std::endl;
#endif

  //Intepolate color
  if ((angle[axisIndex[0]] == 0) && (angle[axisIndex[1]] == 0)) {
    colorVec.set(0, 0, 0);
  } else {
    angle[axisIndex[1]] = TZ_2PI - angle[axisIndex[1]];
    double alpha = angle[axisIndex[0]] / (angle[axisIndex[1]] + angle[axisIndex[0]]);
    colorVec = colorCode[axisIndex[0]] * (1.0 - alpha) +
        colorCode[axisIndex[1]] * alpha;
  }

  double z = fabs(vec.z());
  QColor color;
  color.setRgbF(colorVec.x(), colorVec.y(), colorVec.z());
  color.setHsvF(color.hueF(), color.valueF(),
                std::min(1.0, color.saturationF() + z));

  //return glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f);

  return glm::vec4(fabs(vec.x()), fabs(vec.y()), fabs(vec.z()), 1.f);
}

void Z3DSwcFilter::setColorScheme()
{
  if (m_colorMode.isSelected("Biocytin Branch Type")) {
    m_colorScheme.setColorScheme(ZSwcColorScheme::BIOCYTIN_TYPE_COLOR);
  } else if (m_colorMode.isSelected("Label Branch Type")) {
    m_colorScheme.setColorScheme(ZColorScheme::LABEL_COLOR);
  }
}

void Z3DSwcFilter::prepareColorMapper(const TreeParamMap &colorMapper)
{
  for (const auto &t : m_decomposedNodePairMap) {
    glm::vec4 color = colorMapper.at(t.first)->get();
    m_swcColors1.resize(m_swcColors1.size() + t.second.size(), color);
    m_swcColors2.resize(m_swcColors2.size() + t.second.size(), color);
    m_lineColors.resize(m_lineColors.size() + t.second.size() * 2, color);
  }

  for (const auto &t : m_decomposedNodeMap) {
    glm::vec4 color = colorMapper.at(t.first)->get();
    m_pointColors.resize(m_pointColors.size() + t.second.size(), color);
  }
}

bool Z3DSwcFilter::isBranchTypeColor() const
{
  return m_colorMode.isSelected("Branch Type") ||
      m_colorMode.isSelected("Colormap Branch Type") ||
      m_colorMode.isSelected("Subclass") ||
      m_colorMode.isSelected("Biocytin Branch Type") ||
      m_colorMode.isSelected("Label Branch Type");
}

glm::vec4 Z3DSwcFilter::getTopologyColor(const Swc_Tree_Node *tn)
{
  glm::vec4 color;

  if (Swc_Tree_Node_Is_Regular_Root(tn))
    color = m_colorsForDifferentTopology[0]->get();
  else if (Swc_Tree_Node_Is_Branch_Point(tn))
    color = m_colorsForDifferentTopology[1]->get();
  else if (Swc_Tree_Node_Is_Leaf(tn))
    color = m_colorsForDifferentTopology[2]->get();
  else
    color = m_colorsForDifferentTopology[3]->get();

  return color;
}

namespace {
void ExtendColor(std::vector<glm::vec4> &colorList,
                 size_t es, const glm::vec4 &color)
{
  colorList.resize(colorList.size() + es, color);
}
}

void Z3DSwcFilter::prepareColorForImmutable()
{
  m_swcColors1.clear();
  m_swcColors2.clear();
  m_lineColors.clear();
  m_pointColors.clear();

  if (isBranchTypeColor()) {
    setColorScheme();

    for (const auto &t : m_decomposedNodePairMap) {
      for (const auto &nodePair : t.second) {
        glm::vec4 color1 = getColorByType(nodePair.first);
        glm::vec4 color2 = getColorByType(nodePair.second);
        if (nodePair.first->node.d > nodePair.second->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);
      }
    }

    for (const auto &t : m_decomposedNodeMap) {
      for (const auto &node : t.second) {
        m_pointColors.push_back(getColorByType(node));
      }
    }
  } else if (m_colorMode.isSelected("Random Tree Color")) {
    prepareColorMapper(m_randomColorParam.m_mapper);
  } else if (m_colorMode.isSelected("Individual")) {
    prepareColorMapper(m_individualColorParam.m_mapper);
  } else if (m_colorMode.isSelected("Topology")) {
    for (const auto &t : m_decomposedNodePairMap) {
      for (const auto &nodePair : t.second) {
        Swc_Tree_Node *tn1 = nodePair.first;
        Swc_Tree_Node *tn2 = nodePair.second;
        glm::vec4 color1 = getTopologyColor(tn1);
        glm::vec4 color2 = getTopologyColor(tn2);

        if (tn1->node.d > tn2->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);
      }
    }

    for (const auto &t : m_decomposedNodeMap) {
      for (const auto &node : t.second) {
        m_pointColors.push_back(getTopologyColor(node));
      }
    }
  } else if (m_colorMode.isSelected("Direction")) {
    for (const auto &t : m_decomposedNodePairMap) {
      for (const auto &nodePair : t.second) {
        Swc_Tree_Node *tn1 = nodePair.first;
        Swc_Tree_Node *tn2 = nodePair.second;
        glm::vec4 color1 = getColorByDirection(tn1);
        glm::vec4 color2 = getColorByDirection(tn2);

        if (tn1->node.d > tn2->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);
      }
    }

    for (const auto &t : m_decomposedNodeMap) {
      for (const auto &node : t.second) {
        m_pointColors.push_back(getColorByDirection(node));
      }
    }
  } else if (m_colorMode.isSelected("Intrinsic")) {
    for (auto &t : m_decomposedNodeMap) {
      ZSwcTree *tree = t.first;
      QColor swcColor = tree->getColor();
      glm::vec4 color(swcColor.redF(), swcColor.greenF(), swcColor.blueF(),
                      swcColor.alphaF());
      size_t nodePairCount = m_decomposedNodePairMap[tree].size();

      ExtendColor(m_swcColors1, nodePairCount, color);
      ExtendColor(m_swcColors2, nodePairCount, color);
      ExtendColor(m_lineColors, nodePairCount * 2, color);

      size_t nodeCount = t.second.size();
      ExtendColor(m_pointColors, nodeCount, color);
    }
  }

  m_coneRenderer.setDataColors(&m_swcColors1, &m_swcColors2);
  m_lineRenderer.setDataColors(&m_lineColors);
  m_sphereRenderer.setDataColors(&m_pointColors);
  m_sphereRendererForCone.setDataColors(&m_pointColors);
}

void Z3DSwcFilter::changeColorOption()
{
  prepareColor();
//  updateColorParameter();
  updateColorWidgets();
}

void Z3DSwcFilter::prepareColor()
{
  if (m_swcTopologyMutable == false) {
    prepareColorForImmutable();
    return;
  }

  m_swcColors1.clear();
  m_swcColors2.clear();
  m_lineColors.clear();
  m_pointColors.clear();

  if (m_colorMode.isSelected("Branch Type") ||
      m_colorMode.isSelected("Colormap Branch Type") ||
      m_colorMode.isSelected("Subclass") ||
      m_colorMode.isSelected("Biocytin Branch Type") ||
      m_colorMode.isSelected("Label Branch Type")) {
    if (m_colorMode.isSelected("Biocytin Branch Type")) {
      m_colorScheme.setColorScheme(ZSwcColorScheme::BIOCYTIN_TYPE_COLOR);
    } else if (m_colorMode.isSelected("Label Branch Type")) {
      m_colorScheme.setColorScheme(ZColorScheme::LABEL_COLOR);
    }

    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        glm::vec4 color1 = getColorByType(m_decompsedNodePairs[i][j].first);
        glm::vec4 color2 = getColorByType(m_decompsedNodePairs[i][j].second);
        if (m_decompsedNodePairs[i][j].first->node.d > m_decompsedNodePairs[i][j].second->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(getColorByType(m_decomposedNodes[i][j]));
      }
    }
  } else if (m_colorMode.isSelected("Random Tree Color")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      /*glm::vec4 color = m_colorsForDifferentSource[
          m_sourceColorMapper[m_swcList[i]->source()]]->get();*/
      //glm::vec4 color = m_colorsForDifferentSource[i]->get();
      glm::vec4 color = m_randomColorParam.m_mapper[m_swcList[i]]->get();
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {

        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  } else if (m_colorMode.isSelected("Individual")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      glm::vec4 color = m_individualColorParam.m_mapper[m_swcList[i]]->get();
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);
      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  } else if (m_colorMode.isSelected("Topology")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        Swc_Tree_Node *n1 = m_decompsedNodePairs[i][j].first;
        Swc_Tree_Node *n2 = m_decompsedNodePairs[i][j].second;
        glm::vec4 color1, color2;
        if (Swc_Tree_Node_Is_Regular_Root(n1))
          color1 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n1))
          color1 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n1))
          color1 = m_colorsForDifferentTopology[2]->get();
        else
          color1 = m_colorsForDifferentTopology[3]->get();
        if (Swc_Tree_Node_Is_Regular_Root(n2))
          color2 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n2))
          color2 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n2))
          color2 = m_colorsForDifferentTopology[2]->get();
        else
          color2 = m_colorsForDifferentTopology[3]->get();
        if (n1->node.d > n2->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);
      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        Swc_Tree_Node *n1 = m_decomposedNodes[i][j];
        glm::vec4 color1;
        if (Swc_Tree_Node_Is_Regular_Root(n1))
          color1 = m_colorsForDifferentTopology[0]->get();
        else if (Swc_Tree_Node_Is_Branch_Point(n1))
          color1 = m_colorsForDifferentTopology[1]->get();
        else if (Swc_Tree_Node_Is_Leaf(n1))
          color1 = m_colorsForDifferentTopology[2]->get();
        else
          color1 = m_colorsForDifferentTopology[3]->get();
        m_pointColors.push_back(color1);
      }
    }
  } else if (m_colorMode.isSelected("Direction")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {
        glm::vec4 color1 = getColorByDirection(m_decompsedNodePairs[i][j].first);
        glm::vec4 color2 = getColorByDirection(m_decompsedNodePairs[i][j].second);
        if (m_decompsedNodePairs[i][j].first->node.d > m_decompsedNodePairs[i][j].second->node.d) {
          std::swap(color1, color2);
        }

        m_swcColors1.push_back(color1);
        m_swcColors2.push_back(color2);
        m_lineColors.push_back(color1);
        m_lineColors.push_back(color2);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(getColorByDirection(m_decomposedNodes[i][j]));
      }
    }
  } else if (m_colorMode.isSelected("Intrinsic")) {
    for (size_t i=0; i<m_decompsedNodePairs.size(); i++) {
      QColor swcColor = m_swcList[i]->getColor();
      glm::vec4 color(swcColor.redF(), swcColor.greenF(), swcColor.blueF(),
                      swcColor.alphaF());
      for (size_t j=0; j<m_decompsedNodePairs[i].size(); j++) {

        m_swcColors1.push_back(color);
        m_swcColors2.push_back(color);
        m_lineColors.push_back(color);
        m_lineColors.push_back(color);

      }
      for (size_t j=0; j<m_decomposedNodes[i].size(); j++) {
        m_pointColors.push_back(color);
      }
    }
  }

  m_coneRenderer.setDataColors(&m_swcColors1, &m_swcColors2);
  m_lineRenderer.setDataColors(&m_lineColors);
  m_sphereRenderer.setDataColors(&m_pointColors);
  m_sphereRendererForCone.setDataColors(&m_pointColors);
}

void Z3DSwcFilter::updateColorWidgets()
{
  if (m_colorMode.isSelected("Random Tree Color")) {
    updateTreeColorWidget(m_randomColorParam);
  } else if (m_colorMode.isSelected("Individual")) {
    updateTreeColorWidget(m_individualColorParam);
  } else {
    removeTreeColorWidget();
    m_widgetsGroup->emitWidgetsGroupChangedSignal();
  }
  /*
  for (const auto& kv : m_randomTreeColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("Random Tree Color"));
  }
  for (const auto& kv : m_individualTreeColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("Individual"));
  }
    */

  for (const auto& kv : m_biocytinColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("Biocytin Branch Type"));
  }

  for (size_t i = 0; i < m_colorsForDifferentType.size(); ++i) {
    m_colorsForDifferentType[i]->setVisible(m_allNodeType.find(i) != m_allNodeType.end() &&
        m_colorMode.isSelected("Branch Type"));
  }
  if (m_maxType >= (int) m_colorsForDifferentType.size()) {
    m_colorsForDifferentType.back()->setVisible(true);
  }
  for (const auto& color : m_colorsForSubclassType) {
    color->setVisible(m_colorMode.isSelected("Subclass"));
  }
  for (const auto& color : m_colorsForDifferentTopology) {
    color->setVisible(m_colorMode.isSelected("Topology"));
  }
  m_colorMapBranchType.setVisible(m_colorMode.isSelected("Colormap Branch Type"));
  LINFO() << "Color widgets adjusted";
}

bool Z3DSwcFilter::isNodePicking() const
{
  return isNodeRendering() || m_forceNodePicking;
}


void Z3DSwcFilter::selectSwc(QMouseEvent *e, int w, int h)
{
  if (!m_enablePicking) {
    return;
  }

  if (m_swcList.empty())
    return;

  e->ignore();
  // Mouse button presend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();

#ifdef _DEBUG_2
    std::cout << "Picking swc at "
              << m_startCoord.x << ", " << m_startCoord.y <<  std::endl;
#endif

    const void* obj = pickObject(e->x(), e->y());

#ifdef _DEBUG_2
    std::cout << "Picked: " << obj << std::endl;
#endif

    if (obj == NULL) {
      return;
    }

    // Check if any swc or node was selected...
    for (std::vector<ZSwcTree*>::iterator it=m_swcList.begin(); it!=m_swcList.end(); ++it) {
      if (*it == obj) {
        m_pressedSwc = *it;
        return;
      }
    }
    //    for (size_t i=0; i<m_swcList.size(); i++) {
    //      for (size_t j=0; j<m_decompsedNodes[i].size(); j++) {
    //        if (m_decompsedNodes[i][j] == obj) {
    //          m_pressedSwcTreeNode = m_decompsedNodes[i][j];
    //          return;
    //        }
    //      }
    //    }
//    if (isNodePicking()) {
      QMutexLocker locker(&m_nodeSelectionMutex);
      std::vector<Swc_Tree_Node*>::iterator it = std::find(
            m_sortedNodeList.begin(), m_sortedNodeList.end(),
            (Swc_Tree_Node*) obj);
      if (it != m_sortedNodeList.end()) {
        m_pressedSwcTreeNode = *it;
      }
      /*
      std::set<Swc_Tree_Node*>::iterator it = m_allNodesSet.find((Swc_Tree_Node*)obj);
      if (it != m_allNodesSet.end())
        m_pressedSwcTreeNode = *it;
        */
//    }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) { 
    if (std::abs(e->x() - m_startCoord.x) < 2 && std::abs(m_startCoord.y - e->y()) < 2) {
      bool appending = (e->modifiers() == Qt::ControlModifier) ||
          (e->modifiers() == Qt::ShiftModifier) ||
          (e->modifiers() == Qt::AltModifier) ||
          (e->modifiers() == (Qt::AltModifier | Qt::ControlModifier));

      if (m_pressedSwc || m_pressedSwcTreeNode) {  // hit something
        // do not select tree when it is node rendering, but allow deselecting swc tree in node rendering mode
        if (!(isNodePicking() && m_pressedSwc))
          emit treeSelected(m_pressedSwc, appending);

        if (m_interactionMode == EInteractionMode::ConnectSwcNode && isNodePicking()) {
          emit(connectingSwcTreeNode(m_pressedSwcTreeNode));
        } else {
          emit treeNodeSelected(m_pressedSwcTreeNode, appending);
          if (e->modifiers() == Qt::ShiftModifier) {
            ZOUT(LTRACE(), 5) << "treeNodeSelectConnection emitted";
            emit treeNodeSelectConnection(m_pressedSwcTreeNode);
          } else if (e->modifiers() == Qt::AltModifier ||
                     e->modifiers() == (Qt::AltModifier | Qt::ControlModifier)) {
            emit treeNodeSelectFloodFilling(m_pressedSwcTreeNode);
          }
        }
        e->accept();
      } else if (m_interactionMode == EInteractionMode::Select) {  // hit nothing in Select mode, if not appending, will deselect all nodes and swcs
        emit treeSelected(m_pressedSwc, appending);
        emit treeNodeSelected(m_pressedSwcTreeNode, appending);
      } else if ((m_interactionMode == EInteractionMode::AddSwcNode ||
                 m_interactionMode == EInteractionMode::PlainExtendSwcNode) &&
                 isNodePicking()) { // hit nothing, add node
        Swc_Tree_Node *tn = NULL;

        if (m_interactionMode == EInteractionMode::AddSwcNode) {
          // search within a radius first to speed up
          const std::vector<const void*> &objs =
              pickingManager().sortObjectsByDistanceToPos(glm::ivec2(e->x(), h-e->y()), 100);
          {
            QMutexLocker locker(&m_nodeSelectionMutex);
            for (size_t i=0; i<objs.size(); ++i) {
              std::vector<Swc_Tree_Node*>::iterator it = std::find(
                    m_sortedNodeList.begin(), m_sortedNodeList.end(),
                    (Swc_Tree_Node*) objs[i]);
              if (it != m_sortedNodeList.end()) {
                tn = *it;
                break;
              }
            }
          }
          // not found, search the whole image
          if (!tn) {
            const std::vector<const void*> &objs1 =
                pickingManager().sortObjectsByDistanceToPos(glm::ivec2(e->x(), h-e->y()), -1);
            QMutexLocker locker(&m_nodeSelectionMutex);
            for (size_t i=0; i<objs1.size(); ++i) {
              std::vector<Swc_Tree_Node*>::iterator it = std::find(
                    m_sortedNodeList.begin(), m_sortedNodeList.end(),
                    (Swc_Tree_Node*) objs1[i]);
              if (it != m_sortedNodeList.end()) {
                tn = *it;
                break;
              }

              /*
            std::set<Swc_Tree_Node*>::iterator it = m_allNodesSet.find((Swc_Tree_Node*)objs1[i]);
            if (it != m_allNodesSet.end()) {
              tn = *it;
              break;
            }
            */
            }
          }
        } else {
          tn = NULL;
          for (std::vector<ZSwcTree*>::const_iterator iter = m_swcList.begin();
               iter != m_swcList.end(); ++iter) {
            const ZSwcTree *tree = *iter;
            if (tree->getSelectedNode().size() == 1) {
              if (tn == NULL) { //first time hit
                tn = *(tree->getSelectedNode().begin());
              } else {
                tn = NULL; //second selection, no extension
                break;
              }
            }
          }
        }

        if (tn) {
          glm::dvec3 v1,v2;
          rayUnderScreenPoint(v1, v2, e->x(), e->y(), w, h);
          glm::dvec3 nodePos = glm::applyMatrix(glm::dmat4(coordTransform()), glm::dvec3(tn->node.x, tn->node.y, tn->node.z));
          glm::dvec3 pos = glm::applyMatrix(glm::inverse(glm::dmat4(coordTransform())), projectPointOnRay(nodePos, v1, v2));
          /*
          Swc_Tree_Node* node = Make_Swc_Tree_Node(&(tn->node));
          node->node.x = pos.x;
          node->node.y = pos.y;
          node->node.z = pos.z;
          */
          if (m_interactionMode == EInteractionMode::AddSwcNode) {
            emit addNewSwcTreeNode(pos.x, pos.y, pos.z, SwcTreeNode::radius(tn));
          } else if (m_interactionMode == EInteractionMode::PlainExtendSwcNode) {
            emit extendSwcTreeNode(pos.x, pos.y, pos.z, SwcTreeNode::radius(tn));
          }

//          emit addNewSwcTreeNode(pos.x, pos.y, pos.z, SwcTreeNode::radius(tn));
        }
      }
    }
    m_pressedSwc = NULL;
    m_pressedSwcTreeNode = NULL;
  }
}

void Z3DSwcFilter::updateSwcVisibleState()
{
  QMutexLocker locker(&m_dataValidMutex);

  loadVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DSwcFilter::decomposeSwcTreeForImmutable()
{
  m_decompsedNodePairs.clear();
  m_decomposedNodes.clear();
  {
    QMutexLocker locker(&m_nodeSelectionMutex);
    m_sortedNodeList.clear();
  }

  for (size_t i=0; i<m_swcList.size(); i++) {
    if (m_swcList[i]->isVisible() &&
        (m_decomposedNodeMap.count(m_swcList[i]) == 0)) {
//      std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > allPairs;
//      std::vector<Swc_Tree_Node*> allNodes;
      ZSwcTree *swcTree = m_swcList.at(i);
      const std::vector<Swc_Tree_Node *> &nodeArray =
          swcTree->getSwcTreeNodeArray();
      if (!nodeArray.empty()) {
        m_decomposedNodeMap[swcTree] = nodeArray;
        std::vector<Swc_Tree_Node *> &currentNodeArray =
            m_decomposedNodeMap.at(swcTree);
        if (SwcTreeNode::isVirtual(currentNodeArray[0])) {
          currentNodeArray.erase(currentNodeArray.begin());
        }

//        std::sort(currentNodeArray.begin(), currentNodeArray.end());

        m_decomposedNodePairMap[swcTree] =std::vector<
            std::pair<Swc_Tree_Node*, Swc_Tree_Node*>>();
        std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*>> &nodePair =
            m_decomposedNodePairMap[swcTree];

        for (std::vector<Swc_Tree_Node *>::iterator iter = currentNodeArray.begin();
             iter != currentNodeArray.end(); ++iter) {
          Swc_Tree_Node *tn = *iter;
          if (SwcTreeNode::isRegular(SwcTreeNode::parent(tn))) {
            nodePair.emplace_back(tn, SwcTreeNode::parent(tn));
          }
        }
      }
    }
  }

//  m_maxType = m_allNodeType.empty() ? 0 : (*m_allNodeType.rbegin());

  if (m_enablePicking) {
    QtConcurrent::run(this, &Z3DSwcFilter::sortNodeList);
  }
}

void Z3DSwcFilter::decomposeSwcTree()
{
  m_allNodeType.clear();
  m_decompsedNodePairs.clear();
  m_decomposedNodes.clear();
  {
    QMutexLocker locker(&m_nodeSelectionMutex);
    m_sortedNodeList.clear();
  }
  //m_allNodesSet.clear();

  m_decompsedNodePairs.resize(m_swcList.size());
  m_decomposedNodes.resize(m_swcList.size());

  int prevType = -1;
  for (size_t i=0; i<m_swcList.size(); i++) {
    if (m_swcList[i]->isVisible()) {
      std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > allPairs;
      std::vector<Swc_Tree_Node*> allNodes;
      ZSwcTree *swcTree = m_swcList.at(i);
      swcTree->updateIterator(1);   //depth first
      for (Swc_Tree_Node *tn = swcTree->begin(); tn != swcTree->end(); tn = swcTree->next()) {
        if (!Swc_Tree_Node_Is_Virtual(tn)) {
          int type = SwcTreeNode::type(tn);
          if (type != prevType) {
            m_allNodeType.insert(type);
            prevType = type;
          }
          allNodes.push_back(tn);
          m_sortedNodeList.push_back(tn);
//          m_allNodesSet.insert(tn);
        }
        if (tn->parent != NULL && !Swc_Tree_Node_Is_Virtual(tn->parent)) {
          allPairs.push_back(
                std::pair<Swc_Tree_Node*, Swc_Tree_Node*>(tn, tn->parent));
        }
      }
//      m_allNodesSet.insert(allNodes.begin(), allNodes.end());
      m_decompsedNodePairs[i] = allPairs;
      m_decomposedNodes[i] = allNodes;
    }
  }
  m_maxType = m_allNodeType.empty() ? 0 : (*m_allNodeType.rbegin());

  if (m_enablePicking) {
    QtConcurrent::run(this, &Z3DSwcFilter::sortNodeList);
  }
}

void Z3DSwcFilter::sortNodeList()
{
  QMutexLocker locker(&m_nodeSelectionMutex);

  if (m_swcTopologyMutable == false) {
    m_sortedNodeList.clear();
    for (auto &t : m_decomposedNodeMap) {
      m_sortedNodeList.insert(
            m_sortedNodeList.end(), t.second.begin(), t.second.end());
    }
  }

  std::sort(m_sortedNodeList.begin(), m_sortedNodeList.end());
}

glm::vec4 Z3DSwcFilter::getColorByType(Swc_Tree_Node *n)
{
  if (m_colorMode.isSelected("Branch Type")) {
    if ((size_t)(n->node.type) < m_colorsForDifferentType.size() - 1) {
      return m_colorsForDifferentType[n->node.type]->get();
    } else {
      return m_colorsForDifferentType[m_colorsForDifferentType.size() - 1]->get();
    }
  } else if (m_colorMode.isSelected("Subclass")) {
    if (m_subclassTypeColorMapper.find(n->node.type) != m_subclassTypeColorMapper.end()) {
      return m_colorsForSubclassType[m_subclassTypeColorMapper[n->node.type]]->get();
    } else {
      return m_colorsForSubclassType[m_colorsForSubclassType.size() - 1]->get();
    }
  } else if (m_colorMode.isSelected("Biocytin Branch Type")) {
    return m_biocytinColorMapper[SwcTreeNode::type(n)]->get();
    //return m_colorsForBiocytinType[SwcTreeNode::type(n)]->get();
  } else if (m_colorMode.isSelected("Label Branch Type")) {
    if ((size_t)(n->node.type) < m_colorsForLabelType.size() - 1) {
      return m_colorsForLabelType[n->node.type]->get();
    } else {
      return m_colorsForLabelType[m_colorsForDifferentType.size() - 1]->get();
    }
  } else {
    return m_colorMapBranchType.get().mappedFColor(n->node.type);
  }
}

/*
void Z3DSwcFilter::createColorMapperWidget(
    const std::map<ZSwcTree*, ZVec4Parameter*>& colorMapper,
    std::vector<ZWidgetsGroup*> &widgetGroup)
{
  for (size_t i=0; i<widgetGroup.size(); i++) {
    delete widgetGroup[i];
  }
  widgetGroup.clear();

  std::vector<ZParameter*> allTreeColorParas;
  for (std::map<ZSwcTree*, ZVec4Parameter*>::const_iterator it = colorMapper.begin();
       it != colorMapper.end(); ++it) {
    allTreeColorParas.push_back(it->second);
  }
  std::sort(allTreeColorParas.begin(), allTreeColorParas.end(), compareParameterName);
  for (size_t i=0; i<allTreeColorParas.size(); ++i) {
    widgetGroup.push_back(
          new ZWidgetsGroup(allTreeColorParas[i], m_widgetsGroup, 1));
  }
}
*/

void Z3DSwcFilter::loadVisibleData()
{
  m_swcList.clear();
  for (size_t i=0; i<m_origSwcList.size(); ++i) {
    if (m_origSwcList[i]->isVisible())
      m_swcList.push_back(m_origSwcList[i]);
  }
}

glm::dvec3 Z3DSwcFilter::projectPointOnRay(glm::dvec3 pt, const glm::dvec3 &v1, const glm::dvec3 &v2)
{
  return v1 + glm::dot(pt-v1, v2-v1) * (v2-v1);
}

void Z3DSwcFilter::setColorMode(const std::string &mode)
{
  m_colorMode.select(mode.c_str());
}
