#include "z3dgraphfilter.h"

#include <algorithm>
#include "neutubeconfig.h"

using namespace std;

Z3DGraphFilter::Z3DGraphFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_lineRenderer(m_rendererBase)
  , m_coneRenderer(m_rendererBase)
  , m_arrowRenderer(m_rendererBase)
  , m_sphereRenderer(m_rendererBase)
{
  const NeutubeConfig::Z3DWindowConfig::GraphTabConfig &config =
      NeutubeConfig::getInstance().getZ3DWindowConfig().getGraphTabConfig();
  setVisible(config.isVisible());
//  m_rendererBase->setRenderMethod("Old openGL");
//  adjustWidgets();

  m_coneRenderer.setNeedLighting(false);

  setName(QString("graphfilter"));
}

Z3DGraphFilter::~Z3DGraphFilter()
{
  clear();
}

void Z3DGraphFilter::clear()
{
  foreach (Z3DGraph *graph, m_graphList) {
    delete graph;
  }

  m_graphList.clear();
}


void Z3DGraphFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DGraphFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  m_axisAndTopRadius.clear();
  m_baseAndBaseRadius.clear();
  m_arrowAxisAndTopRadius.clear();
  m_arrowBaseAndBaseRadius.clear();

  m_pointAndRadius.clear();
  m_lines.clear();

  vector<float> edgeWidth;

  foreach (const Z3DGraph* graph, m_graphList) {
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

        if (node.radius() > 0.0) {
          ZPoint nodePos = node.center();

          m_pointAndRadius.push_back(
                glm::vec4(nodePos.x(), nodePos.y(), nodePos.z(),
                          graph->getNode(i).radius()));
        }
      }
    }
  }

  initializeCutRange();
  initializeRotationCenter();

  m_coneRenderer.setData(&m_baseAndBaseRadius, &m_axisAndTopRadius);
//  m_arrowRenderer->setData(&m_arrowBaseAndBaseRadius, &m_arrowAxisAndTopRadius);
  m_lineRenderer.setData(&m_lines);
//  m_lineRenderer.setLineWidth(2.0);
  m_lineRenderer.setLineWidth(edgeWidth);
  m_sphereRenderer.setData(&m_pointAndRadius);

  prepareColor();

  m_dataIsInvalid = false;
}

void Z3DGraphFilter::prepareColor()
{
  m_pointColors.resize(m_pointAndRadius.size());
  size_t index = 0;
  foreach (const Z3DGraph* graph, m_graphList) {
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

  foreach (const Z3DGraph* graph, m_graphList) {
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
  foreach (const Z3DGraph* graph, m_graphList) {
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
  Z3DGraph *graph = new Z3DGraph;

  graph->importPointNetwork(pointNetwork, colorMap);
  m_graphList.append(graph);

  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DGraphFilter::addData(const Z3DGraph &graph)
{
  Z3DGraph *newGraph = new Z3DGraph;
  *newGraph = graph;

#ifdef _DEBUG_
  std::cout << "Adding graph: " << graph.getSource() << std::endl;
  std::cout << "Visible: " << graph.isVisible() << std::endl;
#endif

  m_graphList.append(newGraph);

  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DGraphFilter::setData(const Z3DGraph &graph)
{
  clear();
  addData(graph);
}

void Z3DGraphFilter::setData(const ZObject3d &obj)
{
  clear();

  Z3DGraph *newGraph = new Z3DGraph;
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

void Z3DGraphFilter::renderOpaque(Z3DEye eye)
{
  if (showingArrow()) {
    m_rendererBase.render(eye, m_sphereRenderer, m_arrowRenderer, m_lineRenderer, m_coneRenderer);
  } else {
    m_rendererBase.render(eye, m_sphereRenderer, m_lineRenderer, m_coneRenderer);
  }
}

void Z3DGraphFilter::renderTransparent(Z3DEye eye)
{
  if (showingArrow()) {
    m_rendererBase.render(eye, m_sphereRenderer, m_arrowRenderer, m_lineRenderer, m_coneRenderer);
  } else {
    m_rendererBase.render(eye, m_sphereRenderer, m_lineRenderer, m_coneRenderer);
  }
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

void Z3DGraphFilter::configure(const ZJsonObject &obj)
{
  Z3DGeometryFilter::configure(obj);
}
