#include <iostream>

#include "z3dgraph.h"
#include "znormcolormap.h"
#include "zobject3d.h"
#include "zstackball.h"

using namespace std;

Z3DGraphNode::Z3DGraphNode()
{
  Z3DGraphNode(0.0, 0.0, 0.0);
}

Z3DGraphNode::Z3DGraphNode(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_radius = r;
  m_color.setRgb(0, 0, 0);
  m_shape = GRAPH_BALL;
}

Z3DGraphNode::Z3DGraphNode(const ZPoint &center, double radius)
{
  set(center.x(), center.y(), center.z(), radius);
}

Z3DGraphNode::Z3DGraphNode(const Z3DGraphNode &node)
{
  m_center = node.m_center;
  m_radius = node.m_radius;
  m_color = node.m_color;
  m_shape = node.m_shape;
}

void Z3DGraphNode::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_radius = r;
}

void Z3DGraphNode::setCenter(double x, double y, double z)
{
  m_center.set(x, y, z);
}

void Z3DGraphNode::addX(double dx)
{
  m_center.setX(m_center.x() + dx);
}

void Z3DGraphNode::addY(double dy)
{
  m_center.setY(m_center.y() + dy);
}

void Z3DGraphNode::setX(double x)
{
  m_center.setX(x);
}

void Z3DGraphNode::setY(double y)
{
  m_center.setY(y);
}

void Z3DGraphNode::setRadius(double r)
{
  m_radius = r;
}

void Z3DGraphNode::loadJsonObject(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (eqstr(key, "center")) {
      ZJsonArray center;
      center.set(value, false);
      m_center.set(ZJsonParser::numberValue(center.at(0)),
                   ZJsonParser::numberValue(center.at(1)),
                   ZJsonParser::numberValue(center.at(2)));
    } else if (eqstr(key, "radius")) {
      m_radius = ZJsonParser::numberValue(value);
    } else if (eqstr(key, "color")) {
      ZJsonArray color;
      color.set(value, false);
      //cout << "color: " << ZJsonParser::integerValue(color.at(0)) << endl;
      m_color.setRgb(ZJsonParser::integerValue(color.at(0)),
               ZJsonParser::integerValue(color.at(1)),
               ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_color.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
    }
  }
}

void Z3DGraphNode::print()
{
  cout << "(" << m_center.x() << ", " << m_center.y() << ", "
       << m_center.z() << "), " << m_radius << ": " << "rgb("
       << m_color.red() << ", " << m_color.green() << ", " << m_color.blue()
       << ", " << m_color.alpha()
       << ")" << endl;
}

Z3DGraphEdge::Z3DGraphEdge() : m_shape(GRAPH_CYLINDER)
{
  set(-1, -1);
}

Z3DGraphEdge::Z3DGraphEdge(int vs, int vt) : m_shape(GRAPH_CYLINDER)
{
  QColor color;
  color.setRgb(0, 0, 255);
  color.setAlphaF(0.5);
  set(vs, vt, 1.0, false, color, color, GRAPH_LINE);
}

Z3DGraphEdge::Z3DGraphEdge(const Z3DGraphEdge &edge)
{
  set(edge.m_vs, edge.m_vt, edge.m_width, edge.m_usingNodeColor,
      edge.m_startColor, edge.m_endColor, edge.m_shape);
}

void Z3DGraphEdge::set(int vs, int vt, double width)
{
  m_vs = vs;
  m_vt = vt;
  m_width = width;
}

void Z3DGraphEdge::setConnection(int vs, int vt)
{
  m_vs = vs;
  m_vt = vt;
}

void Z3DGraphEdge::set(int vs, int vt, double width, bool usingNodeColor,
                       const QColor &startColor, const QColor &endColor,
                       EGraphShape shape)
{
  set(vs, vt, width);
  m_usingNodeColor = usingNodeColor;
  m_width = width;
  m_startColor = startColor;
  m_endColor = endColor;
  m_shape = shape;
}

void Z3DGraphEdge::loadJsonObject(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (eqstr(key, "node")) {
      ZJsonArray node;
      node.set(value, false);
      set(ZJsonParser::integerValue(node.at(0)),
          ZJsonParser::integerValue(node.at(1)));
    } else if (eqstr(key, "radius")) {
      m_width = ZJsonParser::numberValue(value);
    } else if (eqstr(key, "color1")) {
      ZJsonArray color;
      color.set(value, false);
      m_startColor.setRgb(ZJsonParser::integerValue(color.at(0)),
                          ZJsonParser::integerValue(color.at(1)),
                          ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_startColor.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
      m_usingNodeColor = false;
    } else if (eqstr(key, "color2")) {
      ZJsonArray color;
      color.set(value, false);
      m_endColor.setRgb(ZJsonParser::integerValue(color.at(0)),
                          ZJsonParser::integerValue(color.at(1)),
                          ZJsonParser::integerValue(color.at(2)));
      if (color.size() == 4) {
        m_endColor.setAlpha(ZJsonParser::integerValue(color.at(3)));
      }
      m_usingNodeColor = false;
    } else if (eqstr(key, "shape")) {
      const char *shape = ZJsonParser::stringValue(value);
      setShape(shape);
    }
  }
}

void Z3DGraphEdge::setShape(const string &shape)
{
  if (shape == "cylinder") {
    setShape(GRAPH_CYLINDER);
  } else if (shape == "line") {
    setShape(GRAPH_LINE);
  }
}

void Z3DGraphEdge::print()
{
  cout << m_vs << "--" << m_vt << ": " << m_width << ", " << "rgb("
       << m_startColor.red() << ", " << m_startColor.green() << ", "
       << m_startColor.blue() << ", " << m_startColor.alpha()
       << ")" << " " << "rgb("
       << m_endColor.red() << ", " << m_endColor.green() << ", "
       << m_endColor.blue() << ", " << m_endColor.alpha()
       << ")" << endl;
}

Z3DGraph::Z3DGraph()
{
  m_type = ZStackObject::TYPE_3D_GRAPH;
  m_target = ZStackObject::TARGET_3D_ONLY;
}

bool Z3DGraph::isEmpty() const
{
  return getNodeNumber() == 0;
}

void Z3DGraph::append(const Z3DGraph &graph)
{
  size_t newStartIndex = m_nodeArray.size();
  m_nodeArray.insert(m_nodeArray.end(), graph.m_nodeArray.begin(),
                     graph.m_nodeArray.end());

  for (size_t i = 0; i < graph.m_edgeArray.size(); i++) {
    Z3DGraphEdge edge = graph.m_edgeArray[i];
    edge.setConnection(edge.vs() + newStartIndex, edge.vt() + newStartIndex);
    m_edgeArray.push_back(edge);
  }
}

void Z3DGraph::importPointNetwork(const ZPointNetwork &pointNetwork,
                                  ZNormColorMap *colorMap)
{
  m_nodeArray.clear();
  m_edgeArray.clear();

  for (size_t i = 0; i < pointNetwork.pointNumber(); i++) {
    m_nodeArray.push_back(Z3DGraphNode(pointNetwork.getPoint(i).x(),
                                       pointNetwork.getPoint(i).y(),
                                       pointNetwork.getPoint(i).z(),
                                       pointNetwork.getPoint(i).weight()));
  }

  for (size_t i = 0; i < pointNetwork.edgeNumber(); i++) {
    m_edgeArray.push_back(Z3DGraphEdge(pointNetwork.getEdgeNodeStart(i),
                                       pointNetwork.getEdgeNodeEnd(i)));

    m_nodeArray[m_edgeArray.back().vs()].setColor(QColor(255, 255, 0, 128));
    m_nodeArray[m_edgeArray.back().vt()].setColor(QColor(0, 0, 0, 128));

    if (colorMap != NULL) {
      QColor color = colorMap->mapColor(pointNetwork.getEdgeWeight(i));
      color.setAlphaF(0.5);

#ifdef _DEBUG_
      cout << "weight: " << pointNetwork.getEdgeWeight(i) << endl;
      cout << "color: " << color.red() << " " << color.green() << " "
           << color.blue() << " " << color.alpha() << endl;
#endif

      m_edgeArray.back().setStartColor(color);
      m_edgeArray.back().setEndColor(color);
    } else {
      m_edgeArray.back().useNodeColor();
    }
  }
}

void Z3DGraph::importJsonFile(const string &filePath)
{
  m_nodeArray.clear();
  m_edgeArray.clear();

  ZJsonObject jsonObject;
  jsonObject.load(filePath);

  const char *key;
  json_t *value;

  json_object_foreach(jsonObject.getData(), key, value) {
    if (eqstr(key, "3DGraph")) {
      const char *graphKey;
      json_t *graphObject;
      json_object_foreach(value, graphKey, graphObject) {
        if (eqstr(graphKey, "Node")) {
          ZJsonArray nodeArray;
          nodeArray.set(graphObject, false);
          m_nodeArray.resize(nodeArray.size());
          for (size_t i = 0; i < nodeArray.size(); ++i) {
            m_nodeArray[i].loadJsonObject(nodeArray.at(i));
          }
        } else if (eqstr(graphKey, "Edge")) {
          ZJsonArray edgeArray;
          edgeArray.set(graphObject, false         );
          m_edgeArray.resize(edgeArray.size());
          for (size_t i = 0; i < edgeArray.size(); ++i) {
            m_edgeArray[i].loadJsonObject(edgeArray.at(i));
          }
        }
      }
      break;
    }
  }
}

void Z3DGraph::print()
{
  cout << m_nodeArray.size() << " nodes" << endl;
  for (size_t i = 0; i < m_nodeArray.size(); ++i) {
    m_nodeArray[i].print();
  }

  cout << m_edgeArray.size() << " edges" << endl;
  for (size_t i = 0; i < m_edgeArray.size(); ++i) {
    m_edgeArray[i].print();
  }
}

void Z3DGraph::addNode(const ZStackBall &ball)
{
  m_nodeArray.push_back(
        Z3DGraphNode(ball.getX(), ball.getY(), ball.getZ(), ball.getRadius()));
  m_nodeArray.back().setColor(ball.getColor());
}

void Z3DGraph::addNode(double x, double y, double z, double radius)
{
  m_nodeArray.push_back(Z3DGraphNode(x, y, z, radius));
}

void Z3DGraph::importObject3d(
    const ZObject3d &obj, double radius, int sampleStep)
{
  m_nodeArray.clear();
  m_edgeArray.clear();

  for (size_t i = 0; i < obj.size(); i += sampleStep) {
    m_nodeArray.push_back(Z3DGraphNode(
                            obj.getX(i), obj.getY(i), obj.getZ(i), radius));
    m_nodeArray.back().setColor(obj.getColor());
  }
}

void Z3DGraph::importObject3d(
    const ZObject3d &obj, double radius)
{
  const static int maxSampleStep = 3;
  const static size_t maxNodeNumber = 10000;
  int sampleStep = imin2(maxSampleStep, obj.size() / maxNodeNumber + 1);
  importObject3d(obj, radius, sampleStep);
}

void Z3DGraph::clear()
{
  m_nodeArray.clear();
  m_edgeArray.clear();
}

void Z3DGraph::display(
    ZPainter &/*painter*/, int /*slice*/, EDisplayStyle /*option*/,
    NeuTube::EAxis /*sliceAxis*/) const
{
}

void Z3DGraph::addEdge(const Z3DGraphEdge &edge)
{
  m_edgeArray.push_back(edge);
}

void Z3DGraph::addNode(const Z3DGraphNode &node)
{
  m_nodeArray.push_back(node);
}

void Z3DGraph::addEdge(
    const Z3DGraphNode &node1, const Z3DGraphNode &node2, EGraphShape shape)
{
  addNode(node1);
  addNode(node2);

  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.set(m_nodeArray.size() - 2, m_nodeArray.size() - 1, node1.radius() * 2.0);
  edge.setShape(shape);
  addEdge(edge);
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(Z3DGraph)
