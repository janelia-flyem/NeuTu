#include "zflyemtodolistfilter.h"

#include <limits>
#include <QMouseEvent>

#include "flyem/zflyemtodolist.h"
#include "flyem/zflyemtodoitem.h"
#include "z3dsphererenderer.h"
#include "z3dlinerenderer.h"
#include "z3dlinewithfixedwidthcolorrenderer.h"
#include "zeventlistenerparameter.h"

ZFlyEmTodoListFilter::ZFlyEmTodoListFilter() : m_showGraph("Visible", true),
  m_xCut("X Cut", glm::ivec2(0,0), 0, 0),
  m_yCut("Y Cut", glm::ivec2(0,0), 0, 0),
  m_zCut("Z Cut", glm::ivec2(0,0), 0, 0)
{
  init();
}

void ZFlyEmTodoListFilter::init()
{
  m_lineRenderer = NULL;
  m_sphereRenderer = NULL;
  m_dataIsInvalid = false;
  m_pressedItem = NULL;

  m_widgetsGroup = NULL;

  addParameter(m_showGraph);

  connect(&m_showGraph, SIGNAL(valueChanged(bool)),
          this, SIGNAL(visibleChanged(bool)));

  m_selectItemEvent = new ZEventListenerParameter(
        "Select Todo Item", true, false, this);
  m_selectItemEvent->listenTo(
        "select todo item", Qt::LeftButton, Qt::NoModifier,
        QEvent::MouseButtonPress);
  m_selectItemEvent->listenTo(
        "select todo item", Qt::LeftButton, Qt::NoModifier,
        QEvent::MouseButtonRelease);
  m_selectItemEvent->listenTo(
        "select todo item", Qt::LeftButton, Qt::ControlModifier,
        QEvent::MouseButtonPress);
  m_selectItemEvent->listenTo(
        "select todo item", Qt::LeftButton, Qt::ControlModifier,
        QEvent::MouseButtonRelease);
  connect(m_selectItemEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)),
          this, SLOT(selectObject(QMouseEvent*,int,int)));
  addEventListener(m_selectItemEvent);

  setStayOnTop(false);
}

ZFlyEmTodoListFilter::~ZFlyEmTodoListFilter()
{

}

void ZFlyEmTodoListFilter::initialize()
{
  Z3DGeometryFilter::initialize();

  m_rendererBase->setMaterialAmbient(glm::vec4(1, 1, 1, 1));
  m_rendererBase->setLightAmbient(glm::vec4(0.5, 0, 0.5, 1));

  m_lineRenderer = new Z3DLineRenderer;
  m_rendererBase->addRenderer(m_lineRenderer);
  m_sphereRenderer = new Z3DSphereRenderer;
  m_rendererBase->addRenderer(m_sphereRenderer);
  m_boundBoxRenderer = new Z3DLineWithFixedWidthColorRenderer();
  m_boundBoxRenderer->setUseDisplayList(false);
  m_boundBoxRenderer->setRespectRendererBaseCoordScales(false);
  m_boundBoxRenderer->setLineColorGuiName("Selection BoundBox Line Color");
  m_boundBoxRenderer->setLineWidthGuiName("Selection BoundBox Line Width");
  m_rendererBase->addRenderer(m_boundBoxRenderer);

  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i = 0; i < paras.size(); ++i) {
    addParameter(paras[i]);
  }
}

void ZFlyEmTodoListFilter::deinitialize()
{
  std::vector<ZParameter*> paras = m_rendererBase->getParameters();
  for (size_t i=0; i<paras.size(); i++) {
    removeParameter(paras[i]);
  }
  Z3DGeometryFilter::deinitialize();
}

void ZFlyEmTodoListFilter::setVisible(bool v)
{
  m_showGraph.set(v);
}

bool ZFlyEmTodoListFilter::isVisible() const
{
  return m_showGraph.get();
}

void ZFlyEmTodoListFilter::render(Z3DEye eye)
{
  if (m_graph.isEmpty()) {
    return;
  }

  if (!m_showGraph.get())
    return;

  m_rendererBase->activateRenderer(m_sphereRenderer);
  m_rendererBase->activateRenderer(m_lineRenderer, Z3DRendererBase::None);
  m_rendererBase->render(eye);
  renderSelectionBox(eye);
}

void ZFlyEmTodoListFilter::renderPicking(Z3DEye eye)
{
  if (!getPickingManager())
      return;
  if (m_graph.isEmpty())
    return;
  if (!m_showGraph.get())
    return;

  if (!m_pickingObjectsRegistered) {
    registerPickingObjects(getPickingManager());
  }
  m_rendererBase->activateRenderer(m_sphereRenderer);
  m_rendererBase->renderPicking(eye);
}

void ZFlyEmTodoListFilter::registerPickingObjects(Z3DPickingManager *pm)
{
  if (pm && !m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_itemList.size(); i++) {
      pm->registerObject(m_itemList[i]);
    }
    m_registeredItemList = m_itemList;
    m_pointPickingColors.clear();
    for (size_t i=0; i<m_itemList.size(); i++) {
      glm::col4 pickingColor = pm->getColorFromObject(m_itemList[i]);
      glm::vec4 fPickingColor(
            pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f,
          pickingColor[3]/255.f);
      m_pointPickingColors.push_back(fPickingColor);
    }
    m_sphereRenderer->setDataPickingColors(&m_pointPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void ZFlyEmTodoListFilter::renderSelectionBox(Z3DEye eye)
{
  if (m_itemList.size() > 0) {
    std::vector<glm::vec3> lines;

    for (std::vector<ZFlyEmToDoItem*>::iterator it=m_itemList.begin();
         it != m_itemList.end(); it++) {
      ZFlyEmToDoItem *selected = *it;
      if (selected->isVisible() && selected->isSelected()) {
        ZCuboid bound = selected->getBoundBox();
//        std::vector<double> bound = getPunctumBound(selectedPunctum);
        float xmin = bound.firstCorner().getX();
        float xmax = bound.lastCorner().getX();
        float ymin = bound.firstCorner().getY();
        float ymax = bound.lastCorner().getY();
        float zmin = bound.firstCorner().getZ();
        float zmax = bound.lastCorner().getZ();
        lines.push_back(glm::vec3(xmin, ymin, zmin));
        lines.push_back(glm::vec3(xmin, ymin, zmax));
        lines.push_back(glm::vec3(xmin, ymax, zmin));
        lines.push_back(glm::vec3(xmin, ymax, zmax));

        lines.push_back(glm::vec3(xmax, ymin, zmin));
        lines.push_back(glm::vec3(xmax, ymin, zmax));
        lines.push_back(glm::vec3(xmax, ymax, zmin));
        lines.push_back(glm::vec3(xmax, ymax, zmax));

        lines.push_back(glm::vec3(xmin, ymin, zmin));
        lines.push_back(glm::vec3(xmax, ymin, zmin));
        lines.push_back(glm::vec3(xmin, ymax, zmin));
        lines.push_back(glm::vec3(xmax, ymax, zmin));

        lines.push_back(glm::vec3(xmin, ymin, zmax));
        lines.push_back(glm::vec3(xmax, ymin, zmax));
        lines.push_back(glm::vec3(xmin, ymax, zmax));
        lines.push_back(glm::vec3(xmax, ymax, zmax));

        lines.push_back(glm::vec3(xmin, ymin, zmin));
        lines.push_back(glm::vec3(xmin, ymax, zmin));
        lines.push_back(glm::vec3(xmax, ymin, zmin));
        lines.push_back(glm::vec3(xmax, ymax, zmin));

        lines.push_back(glm::vec3(xmin, ymin, zmax));
        lines.push_back(glm::vec3(xmin, ymax, zmax));
        lines.push_back(glm::vec3(xmax, ymin, zmax));
        lines.push_back(glm::vec3(xmax, ymax, zmax));
      }
    }
    m_rendererBase->activateRenderer(m_boundBoxRenderer);
    m_boundBoxRenderer->setData(&lines);
    m_rendererBase->render(eye);
    m_boundBoxRenderer->setData(NULL); // lines will go out of scope
  }
}

void ZFlyEmTodoListFilter::deregisterPickingObjects(Z3DPickingManager *pm)
{
  if (pm && m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_registeredItemList.size(); i++) {
      pm->deregisterObject(m_registeredItemList[i]);
    }
    m_registeredItemList.clear();
  }

  m_pickingObjectsRegistered = false;
}

void ZFlyEmTodoListFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void ZFlyEmTodoListFilter::addItem(ZFlyEmToDoItem *item)
{
  if (item != NULL) {
    m_itemList.push_back(item);
    Z3DGraphNode node;
    node.setCenter(item->getPosition());
    node.setRadius(item->getRadius());
    node.setColor(item->getDisplayColor());
    m_graph.addNode(node);

    ZPoint center = node.center();
    double d = node.radius() + node.radius();

    int nodeCount = m_graph.getNodeNumber();

    node.setCenter(center);
    node.setX(center.getX() - d);
    node.setColor(item->getDisplayColor());
    node.setRadius(0);
    m_graph.addNode(node);

    node.setCenter(center);
    node.setX(center.getX() + d);
    m_graph.addNode(node);

    Z3DGraphEdge edge;
    edge.useNodeColor(true);
    edge.setShape(GRAPH_LINE);
    edge.setWidth(2);

    edge.setConnection(nodeCount, nodeCount + 1);
    m_graph.addEdge(edge);


    nodeCount = m_graph.getNodeNumber();

    node.setCenter(center);
    node.setY(center.getY() - d);
    m_graph.addNode(node);

    node.setCenter(center);
    node.setY(center.getY() + d);
    m_graph.addNode(node);

    edge.useNodeColor(true);
    edge.setShape(GRAPH_LINE);
    edge.setWidth(2);

    edge.setConnection(nodeCount, nodeCount + 1);
    m_graph.addEdge(edge);

    nodeCount = m_graph.getNodeNumber();

    node.setCenter(center);
    node.setZ(center.getZ() - d);
    m_graph.addNode(node);

    node.setCenter(center);
    node.setZ(center.getZ() + d);
    m_graph.addNode(node);

    edge.useNodeColor(true);
    edge.setShape(GRAPH_LINE);
    edge.setWidth(2);

    edge.setConnection(nodeCount, nodeCount + 1);
    m_graph.addEdge(edge);
  }
}

void ZFlyEmTodoListFilter::resetData()
{
  m_graph.clear();
  m_itemList.clear();
  deregisterPickingObjects(getPickingManager());
  m_selected.clear();
}

void ZFlyEmTodoListFilter::setData(const ZStackObject *obj)
{
  resetData();

  const ZFlyEmToDoList *todoList = dynamic_cast<const ZFlyEmToDoList*>(obj);
  if (todoList != NULL) {
    setData(*todoList);
  }
  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::setData(const QList<ZFlyEmToDoItem *> &todoList)
{
  resetData();

  for (QList<ZFlyEmToDoItem *>::const_iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    addItem(const_cast<ZFlyEmToDoItem*>(*iter));
  }
  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::setData(const ZFlyEmToDoList &todoList)
{
  resetData();

  ZFlyEmToDoList::ItemIterator iterator(&todoList);
//  ZCuboid nodeBox;
  while (iterator.hasNext()) {
    ZFlyEmToDoItem &item = iterator.next();
    addItem(&item);
  }

  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;


  m_pointAndRadius.clear();
  m_lines.clear();
  int xMin = std::numeric_limits<int>::max();
  int xMax = std::numeric_limits<int>::min();
  int yMin = std::numeric_limits<int>::max();
  int yMax = std::numeric_limits<int>::min();
  int zMin = std::numeric_limits<int>::max();
  int zMax = std::numeric_limits<int>::min();

  std::vector<float> edgeWidth;

  for (size_t i = 0; i <m_graph.getEdgeNumber(); i++) {
    Z3DGraphNode n1 = m_graph.getStartNode(i);
    Z3DGraphNode n2 = m_graph.getEndNode(i);

    m_lines.push_back(glm::vec3(n1.x(), n1.y(), n1.z()));
    m_lines.push_back(glm::vec3(n2.x(), n2.y(), n2.z()));
    edgeWidth.push_back(m_graph.getEdge(i).getWidth());
  }

  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    Z3DGraphNode node = m_graph.getNode(i);

    if (node.radius() > 0.0) {
      ZPoint nodePos = node.center();
      if (nodePos.x() > xMax)
        xMax = static_cast<int>(std::ceil(nodePos.x()));
      if (nodePos.x() < xMin)
        xMin = static_cast<int>(std::floor(nodePos.x()));
      if (nodePos.y() > yMax)
        yMax = static_cast<int>(std::ceil(nodePos.y()));
      if (nodePos.y() < yMin)
        yMin = static_cast<int>(std::floor(nodePos.y()));
      if (nodePos.z() > zMax)
        zMax = static_cast<int>(nodePos.z());
      if (nodePos.z() < zMin)
        zMin = static_cast<int>(nodePos.z());

      m_pointAndRadius.push_back(
            glm::vec4(nodePos.x(), nodePos.y(), nodePos.z(),
                      m_graph.getNode(i).radius()));
    }
  }

  m_xCut.setRange(xMin, xMax);
  m_xCut.set(glm::ivec2(xMin, xMax));
  m_yCut.setRange(yMin, yMax);
  m_yCut.set(glm::ivec2(yMin, yMax));
  m_zCut.setRange(zMin, zMax);
  m_zCut.set(glm::ivec2(zMin, zMax));

  m_lineRenderer->setData(&m_lines);
  m_lineRenderer->setLineWidth(3.0);
  m_lineRenderer->setLineWidth(edgeWidth);
  m_sphereRenderer->setData(&m_pointAndRadius);

  prepareColor();

  m_dataIsInvalid = false;
}

void ZFlyEmTodoListFilter::prepareColor()
{
  m_pointColors.resize(m_pointAndRadius.size());
  size_t index = 0;
  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    const Z3DGraphNode& node = m_graph.getNode(i);
    QColor color =node.color();

    if (node.radius() > 0) {
      Q_ASSERT(index < m_pointColors.size());
      m_pointColors[index++] = glm::vec4(
            color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
  }

  m_lineColors.clear();
//  m_lineStartColors.clear();
//  m_lineEndColors.clear();


  for (size_t i = 0;i < m_graph.getEdgeNumber(); i++) {
    glm::vec4 startColor;
    glm::vec4 endColor;

    if (m_graph.getEdge(i).usingNodeColor()) {
      startColor = glm::vec4(
            m_graph.getStartNode(i).color().redF(),
            m_graph.getStartNode(i).color().greenF(),
            m_graph.getStartNode(i).color().blueF(),
            m_graph.getStartNode(i).color().alphaF());

      endColor = glm::vec4(
            m_graph.getEndNode(i).color().redF(),
            m_graph.getEndNode(i).color().greenF(),
            m_graph.getEndNode(i).color().blueF(),
            m_graph.getEndNode(i).color().alphaF());
    } else {
#ifdef _DEBUG_2
      cout << m_graph.getEdge(i).startColor().alphaF() << endl;
#endif

      startColor = glm::vec4(m_graph.getEdge(i).startColor().redF(),
                                      m_graph.getEdge(i).startColor().greenF(),
                                      m_graph.getEdge(i).startColor().blueF(),
                                      m_graph.getEdge(i).startColor().alphaF());
      endColor = glm::vec4(m_graph.getEdge(i).endColor().redF(),
                                          m_graph.getEdge(i).endColor().greenF(),
                                          m_graph.getEdge(i).endColor().blueF(),
                                          m_graph.getEdge(i).endColor().alphaF());
    }

    m_lineColors.push_back(startColor);
    m_lineColors.push_back(endColor);
  }

  m_lineRenderer->setDataColors(&m_lineColors);
  m_sphereRenderer->setDataColors(&m_pointColors);
}

std::vector<double> ZFlyEmTodoListFilter::boundBox()
{
  std::vector<double> result(6, 0);

  for (size_t i = 0; i < 3; i++) {
    result[i * 2] = std::numeric_limits<double>::max();
    result[i * 2 + 1] = -std::numeric_limits<double>::max();
  }

  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    ZPoint pos = m_graph.getNode(i).center();
    result[0] = std::min(result[0], pos.x() - m_graph.getNode(i).radius() * 2.0);
    result[1] = std::max(result[1], pos.x() + m_graph.getNode(i).radius() * 2.0);
    result[2] = std::min(result[2], pos.y() - m_graph.getNode(i).radius() * 2.0);
    result[3] = std::max(result[3], pos.y() + m_graph.getNode(i).radius() * 2.0);
    result[4] = std::min(result[4], pos.z() - m_graph.getNode(i).radius() * 2.0);
    result[5] = std::max(result[5], pos.z() + m_graph.getNode(i).radius() * 2.0);
  }

  return result;
}

ZWidgetsGroup *ZFlyEmTodoListFilter::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("To Do", NULL, 1);
    new ZWidgetsGroup(&m_showGraph, m_widgetsGroup, 1);

    new ZWidgetsGroup(&m_stayOnTop, m_widgetsGroup, 1);
    std::vector<ZParameter*> paras = m_rendererBase->getParameters();
    for (size_t i=0; i<paras.size(); i++) {
      ZParameter *para = paras[i];
      if (para->getName() == "Z Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 2);
      else if (para->getName() == "Size Scale")
        new ZWidgetsGroup(para, m_widgetsGroup, 3);
      else if (para->getName() == "Rendering Method")
        new ZWidgetsGroup(para, m_widgetsGroup, 4);
      else if (para->getName() == "Opacity")
        new ZWidgetsGroup(para, m_widgetsGroup, 5);
      else
        new ZWidgetsGroup(para, m_widgetsGroup, 7);
    }
    new ZWidgetsGroup(&m_xCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_yCut, m_widgetsGroup, 5);
    new ZWidgetsGroup(&m_zCut, m_widgetsGroup, 5);
    m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

bool ZFlyEmTodoListFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && m_showGraph.get() &&
      !m_graph.isEmpty();
}

void ZFlyEmTodoListFilter::updateGraphVisibleState()
{
//  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::selectObject(QMouseEvent *e, int, int h)
{
  if (m_itemList.empty())
    return;
  if (!getPickingManager())
    return;

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();
    const void* obj = getPickingManager()->getObjectAtPos(
          glm::ivec2(e->x(), h - e->y()));
    if (obj == NULL) {
      return;
    }

    // Check if any point was selected...
    for (std::vector<ZFlyEmToDoItem*>::iterator it=m_itemList.begin();
         it!=m_itemList.end(); ++it)
      if (*it == obj) {
        m_pressedItem = *it;
        break;
      }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if ((std::abs(e->x() - m_startCoord.x) < 2) &&
        (std::abs(m_startCoord.y - e->y()) < 2)) {
      if (e->modifiers() == Qt::ControlModifier)
        emit objectSelected(dynamic_cast<ZStackObject*>(m_pressedItem), true);
      else
        emit objectSelected(dynamic_cast<ZStackObject*>(m_pressedItem), false);
      e->accept();
    }
    m_pressedItem = NULL;
  }
}

