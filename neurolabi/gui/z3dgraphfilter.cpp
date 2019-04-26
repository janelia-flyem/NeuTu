#include "z3dgraphfilter.h"

#include <algorithm>
#include "neutubeconfig.h"
#include "zobject3d.h"
#include "zdocplayer.h"

using namespace std;

Z3DGraphFilter::Z3DGraphFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_lineRenderer(m_rendererBase)
  , m_coneRenderer(m_rendererBase)
  , m_arrowRenderer(m_rendererBase)
  , m_sphereRenderer(m_rendererBase)
  , m_fontRenderer(m_rendererBase)
  , m_selectGraphEvent("Select Graph", false)
{
  const NeutubeConfig::Z3DWindowConfig::GraphTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getGraphTabConfig();
  setVisible(config.isVisible());
//  m_rendererBase->setRenderMethod("Old openGL");
//  adjustWidgets();

  m_coneRenderer.setNeedLighting(false);
  m_fontRenderer.setFollowCoordTransform(false);

  setName(QString("graphfilter"));

  m_selectGraphEvent.listenTo(
        "select graph", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectGraphEvent.listenTo(
        "select graph", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectGraphEvent.listenTo(
        "append graph", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectGraphEvent.listenTo(
        "append graph", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease);
  connect(&m_selectGraphEvent, &ZEventListenerParameter::mouseEventTriggered,
          this, &Z3DGraphFilter::selectGraph);
  addEventListener(m_selectGraphEvent);
}

Z3DGraphFilter::~Z3DGraphFilter()
{
  clear();
}

void Z3DGraphFilter::clear()
{
//  foreach (Z3DGraph *graph, m_graphList) {
//    delete graph;
//  }

  m_objectMap.clear();
  m_graphList.clear();
  m_pressedGraph.reset();
  invalidateResult();
}

void Z3DGraphFilter::graphBound(const Z3DGraphPtr &graph, ZBBox<glm::dvec3> &result)
{
  for (size_t i = 0; i < graph->getNodeNumber(); i++) {
    ZPoint pos = graph->getNode(i).center();
    glm::dvec3 p(pos.x(), pos.y(), pos.z());
    double d = graph->getNode(i).radius();
    result.expand(p - d);
    result.expand(p + d);
  }

  glm::dvec3 minCorner = result.minCorner();
  glm::dvec3 maxCorner = result.maxCorner();

  result.setMinCorner(
        glm::dvec3(glm::applyMatrix(
                     coordTransform(),
                     glm::vec3(minCorner.x, minCorner.y, minCorner.z))));
  result.setMaxCorner(
        glm::dvec3(glm::applyMatrix(
                     coordTransform(),
                     glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z))));
}

void Z3DGraphFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DGraphFilter::updateSelection()
{
  for (auto iter = m_objectMap.begin(); iter != m_objectMap.end(); ++iter) {
    iter.key()->setSelected(iter.value()->isSelected());
  }
}

void Z3DGraphFilter::addSelectionLines()
{
  updateSelection();
  foreach (const Z3DGraphPtr &graph, m_graphList) {
    if (graph->isVisible() && graph->isSelected()) {
      ZBBox<glm::dvec3> boundBox;
      graphBound(graph, boundBox);
      appendBoundboxLines(boundBox, m_selectionLines);
    }
  }
}

void Z3DGraphFilter::deselectAllGraph()
{
  for (QMap<Z3DGraph*, ZStackObject*>::iterator iter = m_objectMap.begin();
       iter != m_objectMap.end(); ++iter) {
    Z3DGraph *graph = iter.key();
    ZStackObject *obj = iter.value();
    graph->setSelected(false);
    obj->setSelected(false);
  }
}

void Z3DGraphFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects();

  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_arrowAxisAndTopRadius.clear();
  m_arrowBaseAndBaseRadius.clear();
  m_textList.clear();
  m_textPosition.clear();

  m_pointAndRadius.clear();
  m_lines.clear();

  m_textList.clear();
  m_textPosition.clear();

  vector<float> edgeWidth;

  foreach (const Z3DGraphPtr &graph, m_graphList) {
    if (graph->isVisible()) {
      for (size_t i = 0; i < graph->getEdgeNumber(); i++) {
        Z3DGraphNode n1 = graph->getStartNode(i);
        Z3DGraphNode n2 = graph->getEndNode(i);

        if (graph->getEdge(i).shape() == GRAPH_CYLINDER) {
          ZPoint startPos = ZPoint(n1.x(), n1.y(), n1.z());
          ZPoint endPos = ZPoint(n2.x(), n2.y(), n2.z());
          ZPoint vec = endPos - startPos;
          ZPoint normalizedVec = vec;
          normalizedVec.normalize();

          startPos = startPos + normalizedVec * 0.8 * graph->getStartNode(i).radius();
          endPos = endPos - normalizedVec * 0.8 * graph->getEndNode(i).radius();
          vec = endPos - startPos;

          glm::vec4 baseAndbRadius, axisAndtRadius;
          baseAndbRadius = glm::vec4(startPos.x(), startPos.y(), startPos.z(),
                                     graph->getEdge(i).getWidth());
          axisAndtRadius = glm::vec4(vec.x(), vec.y(), vec.z(),
                                     graph->getEdge(i).getWidth());

          m_baseAndBaseRadius.push_back(baseAndbRadius);
          m_axisAndTopRadius.push_back(axisAndtRadius);
        } else if (graph->getEdge(i).shape() == GRAPH_LINE) {
          m_lines.push_back(glm::vec3(n1.x(), n1.y(), n1.z()));
          m_lines.push_back(glm::vec3(n2.x(), n2.y(), n2.z()));
          float width = graph->getEdge(i).getWidth();
          if (width < 1.0) {
            width = 1.0;
          }
          edgeWidth.push_back(width);
        }
      }

      for (size_t i = 0; i < graph->getNodeNumber(); i++) {
        Z3DGraphNode node = graph->getNode(i);

        ZPoint nodePos = node.center();
        if (node.radius() > 0.0) {
          m_pointAndRadius.push_back(
                glm::vec4(nodePos.x(), nodePos.y(), nodePos.z(),
                          graph->getNode(i).radius()));
        }

        if (!node.getText().isEmpty()) {
          m_textPosition.push_back(glm::vec3(nodePos.x(), nodePos.y(), nodePos.z()));
          m_textList.append(node.getText());
        }
      }
    }
  }

  initializeCutRange();
  initializeRotationCenter();

  m_coneRenderer.setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
//  m_arrowRenderer->setData(&m_arrowBaseAndBaseRadius, &m_arrowAxisAndTopRadius);
  m_lineRenderer.setData(&m_lines);
//  m_lineRenderer.setLineWidth(5.0);
  m_lineRenderer.setUseSmoothLine(false);
  m_lineRenderer.setLineWidth(edgeWidth);
  m_sphereRenderer.setData(&m_pointAndRadius);

  m_fontRenderer.setData(&m_textPosition, m_textList);

  prepareColor();

  m_dataIsInvalid = false;
}

void Z3DGraphFilter::prepareColor()
{
  m_pointColors.resize(m_pointAndRadius.size());
  size_t index = 0;
  foreach (const Z3DGraphPtr &graph, m_graphList) {
    for (size_t i = 0; i < graph->getNodeNumber(); i++) {
      const Z3DGraphNode& node = graph->getNode(i);
      QColor color =node.color();

      if (node.radius() > 0) {
        Q_ASSERT(index < m_pointColors.size());
        m_pointColors[index++] = glm::vec4(
              color.redF(), color.greenF(), color.blueF(), color.alphaF());
      }
    }
  }

  m_lineColors.clear();
  m_lineStartColors.clear();
  m_lineEndColors.clear();
  //m_lineColors.resize(m_graph.getEdgeNumber() * 2);
  //m_lineStartColors.resize(m_graph.getEdgeNumber());
  //m_lineEndColors.resize(m_graph.getEdgeNumber());
//  m_arrowStartColors.resize(m_graph.getEdgeNumber());
//  m_arrowEndColors.resize(m_graph.getEdgeNumber());

  foreach (const Z3DGraphPtr &graph, m_graphList) {
    for (size_t i = 0;i < graph->getEdgeNumber(); i++) {
      glm::vec4 startColor;
      glm::vec4 endColor;

      if (graph->getEdge(i).usingNodeColor()) {
        startColor = glm::vec4(
              graph->getStartNode(i).color().redF(),
              graph->getStartNode(i).color().greenF(),
              graph->getStartNode(i).color().blueF(),
              graph->getStartNode(i).color().alphaF());

        endColor = glm::vec4(
              graph->getEndNode(i).color().redF(),
              graph->getEndNode(i).color().greenF(),
              graph->getEndNode(i).color().blueF(),
              graph->getEndNode(i).color().alphaF());
      } else {
        startColor = glm::vec4(graph->getEdge(i).startColor().redF(),
                               graph->getEdge(i).startColor().greenF(),
                               graph->getEdge(i).startColor().blueF(),
                               graph->getEdge(i).startColor().alphaF());
        endColor = glm::vec4(graph->getEdge(i).endColor().redF(),
                             graph->getEdge(i).endColor().greenF(),
                             graph->getEdge(i).endColor().blueF(),
                             graph->getEdge(i).endColor().alphaF());
      }

      if (graph->getEdge(i).shape() == GRAPH_CYLINDER) {
        m_lineStartColors.push_back(startColor);
        m_lineEndColors.push_back(endColor);
      } else if (graph->getEdge(i).shape() == GRAPH_LINE) {
        m_lineColors.push_back(startColor);
        m_lineColors.push_back(endColor);
      }

      // m_lineStartColors[i] = startColor;
      //m_lineEndColors[i] = endColor;

      //    m_arrowStartColors[i] = startColor * 0.4f + endColor * 0.6f;
      //    m_arrowStartColors[i][3] *= 0.5;
      //    m_arrowEndColors[i] = m_arrowStartColors[i];
    }
  }

  m_coneRenderer.setDataColors(&m_lineStartColors, &m_lineEndColors);
//  m_arrowRenderer->setDataColors(&m_arrowStartColors, &m_arrowEndColors);
  m_lineRenderer.setDataColors(&m_lineColors);
  m_sphereRenderer.setDataColors(&m_pointColors);
}

void Z3DGraphFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  foreach (const Z3DGraphPtr &graph, m_graphList) {
    for (size_t i = 0; i < graph->getNodeNumber(); i++) {
      ZPoint pos = graph->getNode(i).center();
      glm::dvec3 p(pos.x(), pos.y(), pos.z());
      double d = graph->getNode(i).radius() * 2.0;
      m_notTransformedBoundBox.expand(p - d);
      m_notTransformedBoundBox.expand(p + d);
    }
  }
}

void Z3DGraphFilter::setData(const ZPointNetwork &pointNetwork,
                               ZNormColorMap *colorMap)
{
  clear();
  Z3DGraphPtr graph = Z3DGraph::MakePointer();

  graph->importPointNetwork(pointNetwork, colorMap);
  m_graphList.append(graph);

  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DGraphFilter::addData(const ZDocPlayer &player)
{
  Z3DGraph graph = player.get3DGraph();
  Z3DGraphPtr newGraph = addData(graph);
  if (newGraph) {
    m_objectMap[newGraph.get()] = player.getData();
  }
}

void Z3DGraphFilter::addData(const QList<ZDocPlayer *> &playerList)
{
  foreach (const ZDocPlayer *player, playerList) {
    addData(*player);
  }
}

void Z3DGraphFilter::addData(Z3DGraph *graph)
{
  if (graph != NULL) {
    Z3DGraphPtr newGraph = addData(*graph);
    if (newGraph) {
      m_objectMap[newGraph.get()] = dynamic_cast<ZStackObject*>(graph);
    }
  }
}

Z3DGraphPtr Z3DGraphFilter::addData(const Z3DGraph &graph)
{
//  Z3DGraph *newGraph = new Z3DGraph;
  Z3DGraphPtr newGraph = Z3DGraph::MakePointer();

  if (!graph.isEmpty()) {
    *newGraph = graph;

#ifdef _DEBUG_2
    std::cout << "Adding graph: " << graph.getSource() << std::endl;
    std::cout << "Visible: " << graph.isVisible() << std::endl;
#endif

    m_graphList.append(newGraph);

    m_dataIsInvalid = true;
    invalidateResult();

    updateBoundBox();
  }

  return newGraph;
}

void Z3DGraphFilter::setData(const Z3DGraph &graph)
{
  clear();
  addData(graph);
}

void Z3DGraphFilter::setData(const ZObject3d &obj)
{
  clear();

//  Z3DGraph *newGraph = new Z3DGraph;
  Z3DGraphPtr newGraph = Z3DGraph::MakePointer();
  newGraph->importObject3d(obj, 1.0, 3);
  newGraph->setVisible(obj.isVisible());
  m_graphList.append(newGraph);

  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

std::shared_ptr<ZWidgetsGroup> Z3DGraphFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Graph", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);

    const std::vector<ZParameter*>& paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform")
        m_widgetsGroup->addChild(*para, 2);
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

    //m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DGraphFilter::selectGraph(QMouseEvent* e, int, int)
{
#ifdef _DEBUG_2
  std::cout << "Selecting graph" << std::endl;
#endif
  if (m_graphList.empty()) {
    return;
  }

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    recordMousePosition(e);
//    m_startCoord.x = e->x();
//    m_startCoord.y = e->y();
    const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(e->x(), e->y()));
    if (obj == NULL) {
      return;
    }

    // Check if any point was selected...
    foreach (const Z3DGraphPtr &graph, m_graphList) {
      if (graph.get() == obj) {
        m_pressedGraph = graph;
        break;
      }
    }
    return;
  }

  if ((e->type() == QEvent::MouseButtonRelease) && stayingMouse(e)) {
    ZStackObject *selectedObj  = NULL;
    bool appending = false;
    if (e->modifiers() == Qt::ControlModifier) {
      appending = true;
    }

    if (m_pressedGraph) {
      Z3DGraph *graph = m_pressedGraph.get();

      if (m_objectMap.contains(graph)) {
        selectedObj = m_objectMap[graph];


        if (selectedObj != NULL) {
#ifdef _DEBUG_
          std::cout << "Graph object selected: " << selectedObj
                    << " " << selectedObj->getSource() << std::endl;
#endif
          graph->setSelected(true);
        }
      }
    }

    emit objectSelected(selectedObj, appending);

    if (m_pressedGraph) {
      e->accept();
      m_pressedGraph.reset();
    }
  }
}

void Z3DGraphFilter::renderOpaque(Z3DEye eye)
{
  std::vector<Z3DPrimitiveRenderer*> renderList;
  renderList.push_back(&m_sphereRenderer);
  renderList.push_back(&m_coneRenderer);
  renderList.push_back(&m_lineRenderer);
  renderList.push_back(&m_fontRenderer);
  if (showingArrow()) {
    renderList.push_back(&m_arrowRenderer);
  }
  m_rendererBase.render(eye, renderList);
}

void Z3DGraphFilter::renderTransparent(Z3DEye eye)
{
  std::vector<Z3DPrimitiveRenderer*> renderList;
  renderList.push_back(&m_sphereRenderer);
  renderList.push_back(&m_coneRenderer);
  renderList.push_back(&m_lineRenderer);
  renderList.push_back(&m_fontRenderer);
  if (showingArrow()) {
    renderList.push_back(&m_arrowRenderer);
  }
  m_rendererBase.render(eye, renderList);

//  if (showingArrow()) {
//    m_rendererBase.render(eye, m_sphereRenderer, m_arrowRenderer, m_lineRenderer, m_coneRenderer);
//  } else {
//    m_rendererBase.render(eye, m_sphereRenderer, m_lineRenderer, m_coneRenderer);
//  }
}

bool Z3DGraphFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_graphList.isEmpty();
}

void Z3DGraphFilter::updateGraphVisibleState()
{
//  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DGraphFilter::registerPickingObjects()
{
  if (!m_pickingObjectsRegistered) {
    for (Z3DGraphPtr &graph : m_graphList) {
      pickingManager().registerObject(graph.get());
    }
    m_registeredGraphList = m_graphList;
    m_graphPickingColors.clear();
    for (Z3DGraphPtr &graph : m_graphList) {
      glm::col4 pickingColor = pickingManager().colorOfObject(graph.get());
      glm::vec4 fPickingColor(
          pickingColor[0] / 255.f, pickingColor[1] / 255.f,
          pickingColor[2] / 255.f, pickingColor[3] / 255.f);

      for (size_t i = 0; i < graph->getNodeNumber(); i++) {
        const Z3DGraphNode& node = graph->getNode(i);
        if (node.radius() > 0) {
          m_graphPickingColors.push_back(fPickingColor);
        }
      }
    }
    m_sphereRenderer.setDataPickingColors(&m_graphPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void Z3DGraphFilter::deregisterPickingObjects()
{
  if (m_pickingObjectsRegistered) {
    for (Z3DGraphPtr &graph : m_graphList) {
      pickingManager().deregisterObject(graph.get());
    }
    m_registeredGraphList.clear();
  }

  m_pickingObjectsRegistered = false;
}

void Z3DGraphFilter::renderPicking(Z3DEye eye)
{
  if (!m_pickingObjectsRegistered) {
    registerPickingObjects();
  }
  m_rendererBase.renderPicking(eye, m_lineRenderer, m_sphereRenderer);
}

void Z3DGraphFilter::configure(const ZJsonObject &obj)
{
  Z3DGeometryFilter::configure(obj);
}
