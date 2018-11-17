#include "zflyemtodolistfilter.h"

#include <limits>
#include <QMouseEvent>
#include <QApplication>

#include "flyem/zflyemtodolist.h"
#include "flyem/zflyemtodoitem.h"
#include "z3dsphererenderer.h"
#include "z3dlinerenderer.h"
#include "z3dlinewithfixedwidthcolorrenderer.h"
#include "zeventlistenerparameter.h"
#include "neutubeconfig.h"

ZFlyEmTodoListFilter::ZFlyEmTodoListFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_selectItemEvent("Select Todo Item", false)
  , m_lineRenderer(m_rendererBase)
  , m_sphereRenderer(m_rendererBase)
{
  m_selectItemEvent.listenTo("select todo item", Qt::LeftButton,
                             Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectItemEvent.listenTo("select todo item", Qt::LeftButton,
                             Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectItemEvent.listenTo("select todo item", Qt::LeftButton,
                                Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectItemEvent.listenTo("select todo item", Qt::LeftButton,
                                Qt::ControlModifier, QEvent::MouseButtonRelease);
  connect(&m_selectItemEvent, &ZEventListenerParameter::mouseEventTriggered,
          this, &ZFlyEmTodoListFilter::selectObject);
  addEventListener(m_selectItemEvent);


//  m_rendererBase.setMaterialAmbient(glm::vec4(0.3, 0.3, 0.3, 1));
//  m_rendererBase.setMaterialAmbient(glm::vec4(1, 1, 1, 1));
//  m_rendererBase.setMaterialSpecular(glm::vec4(0, 0, 0, 1));
//  m_rendererBase.setOpacity(0.8);

  m_rendererBase.setMaterialAmbient(glm::vec4(1.f, 1.f, 1.f, 1.f));
}

ZFlyEmTodoListFilter::~ZFlyEmTodoListFilter()
{

}

void ZFlyEmTodoListFilter::renderPicking(Z3DEye eye)
{
  if (!m_pickingObjectsRegistered) {
    registerPickingObjects();
  }

  m_rendererBase.renderPicking(eye, m_sphereRenderer);
}

void ZFlyEmTodoListFilter::registerPickingObjects()
{
  ZOUT(LTRACE(), 5) << "start";
  if (!m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_itemList.size(); i++) {
      pickingManager().registerObject(m_itemList[i]);
    }
    m_registeredItemList = m_itemList;
    m_pointPickingColors.clear();
    for (size_t i=0; i<m_itemList.size(); i++) {
      glm::col4 pickingColor = pickingManager().colorOfObject(m_itemList[i]);
      glm::vec4 fPickingColor(
            pickingColor[0]/255.f, pickingColor[1]/255.f, pickingColor[2]/255.f,
          pickingColor[3]/255.f);
      m_pointPickingColors.push_back(fPickingColor);
    }
    m_sphereRenderer.setDataPickingColors(&m_pointPickingColors);
  }

  m_pickingObjectsRegistered = true;
  ZOUT(LTRACE(), 5) << "end";
}

void ZFlyEmTodoListFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  for (size_t i = 0; i < m_graph.getNodeNumber(); i++) {
    ZPoint pos = m_graph.getNode(i).center();
    glm::dvec3 p(pos.x(), pos.y(), pos.z());
    double d = m_graph.getNode(i).radius() * 2.0;
    m_notTransformedBoundBox.expand(p - d);
    m_notTransformedBoundBox.expand(p + d);
  }
}

void ZFlyEmTodoListFilter::addSelectionLines()
{
  ZBBox<glm::dvec3> bb;
  for (auto item : m_itemList) {
    if (item->isVisible() && item->isSelected()) {
      ZCuboid bound = item->getBoundBox();
      bb.setMinCorner(glm::dvec3(bound.firstCorner().x(), bound.firstCorner().y(), bound.firstCorner().z()));
      bb.setMaxCorner(glm::dvec3(bound.lastCorner().x(), bound.lastCorner().y(), bound.lastCorner().z()));
      appendBoundboxLines(bb, m_selectionLines);
    }
  }
}

void ZFlyEmTodoListFilter::deregisterPickingObjects()
{
  ZOUT(LTRACE(), 5) << "start";
  if (m_pickingObjectsRegistered) {
    for (size_t i=0; i<m_registeredItemList.size(); i++) {
      pickingManager().deregisterObject(m_registeredItemList[i]);
    }
    m_registeredItemList.clear();
  }

  m_pickingObjectsRegistered = false;
  ZOUT(LTRACE(), 5) << "end";
}

void ZFlyEmTodoListFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void ZFlyEmTodoListFilter::updateGraph()
{
  deregisterPickingObjects();
  m_graph.clear();
  m_selected.clear();

  for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = m_itemList.begin();
       iter != m_itemList.end(); ++iter) {
    const ZFlyEmToDoItem *item = *iter;
    if (item->isVisible()) {
      addItemNode(item);
    }
  }

  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::addItemNode(const ZFlyEmToDoItem *item)
{
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

void ZFlyEmTodoListFilter::addItem(ZFlyEmToDoItem *item)
{
  if (item != NULL) {
    m_itemList.push_back(item);

    if (item->isVisible()) {
      addItemNode(item);
    }
  }
}

void ZFlyEmTodoListFilter::resetData()
{
  m_graph.clear();
  m_itemList.clear();
  deregisterPickingObjects();
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

std::shared_ptr<ZWidgetsGroup> ZFlyEmTodoListFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("To Do", 1);
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

      m_pointAndRadius.push_back(
            glm::vec4(nodePos.x(), nodePos.y(), nodePos.z(),
                      m_graph.getNode(i).radius()));
    }
  }

  initializeCutRange();
  initializeRotationCenter();

  m_lineRenderer.setData(&m_lines);
  m_lineRenderer.setLineWidth(3.0);
  m_lineRenderer.setLineWidth(edgeWidth);
  m_sphereRenderer.setData(&m_pointAndRadius);

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

  m_lineRenderer.setDataColors(&m_lineColors);
  m_sphereRenderer.setDataColors(&m_pointColors);
}


bool ZFlyEmTodoListFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_graph.isEmpty();
}

void ZFlyEmTodoListFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_lineRenderer, m_sphereRenderer);
}

void ZFlyEmTodoListFilter::renderTransparent(Z3DEye eye)
{
  m_rendererBase.render(eye, m_lineRenderer, m_sphereRenderer);
}

void ZFlyEmTodoListFilter::updateGraphVisibleState()
{
//  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void ZFlyEmTodoListFilter::selectObject(QMouseEvent *e, int, int /*h*/)
{
  if (m_itemList.empty())
    return;

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();

    const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(e->x(), e->y()));

#ifdef _DEBUG_2
    std::cout << "Picking env: " << "dpr: " << dpr << " tex: " << m_pickingTexSize << std::endl;
#endif

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
//    if (m_pressedItem != NULL) {
      if ((std::abs(e->x() - m_startCoord.x) < 2) &&
          (std::abs(m_startCoord.y - e->y()) < 2)) {
        if (e->modifiers() == Qt::ControlModifier)
          emit objectSelected(dynamic_cast<ZStackObject*>(m_pressedItem), true);
        else
          emit objectSelected(dynamic_cast<ZStackObject*>(m_pressedItem), false);
        if (m_pressedItem != NULL) {
          e->accept();
        }
      }
      m_pressedItem = NULL;
//    }
  }
}

