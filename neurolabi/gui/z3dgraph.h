#ifndef Z3DGRAPH_H
#define Z3DGRAPH_H

#include <vector>
#include <string>
#include <QColor>
#include "zpoint.h"
#include "zpointnetwork.h"
#include "zjsonparser.h"
#include "zstackobject.h"

class ZNormColorMap;
class ZObject3d;
class ZStackBall;

enum EGraphShape {
  GRAPH_NO_SHAPE, GRAPH_BALL, GRAPH_CYLINDER, GRAPH_LINE
};

class Z3DGraphNode {
public:
  Z3DGraphNode();
  Z3DGraphNode(double x, double y, double z, double r = 3.0);
  Z3DGraphNode(const ZPoint &center, double radius);
  Z3DGraphNode(const Z3DGraphNode &node);

  inline const ZPoint& center() const { return m_center; }
  inline double radius() const { return m_radius; }
  inline const QColor& color() const { return m_color; }
  inline EGraphShape shape() const { return m_shape; }
  inline double x() { return m_center.x(); }
  inline double y() { return m_center.y(); }
  inline double z() { return m_center.z(); }

  inline void setColor(const QColor &color) { m_color = color; }

  void set(double x, double y, double z, double r);
  void setCenter(double x, double y, double z);
  void setRadius(double r);

  void addX(double dx);
  void addY(double dy);

  void setX(double x);
  void setY(double y);

  void loadJsonObject(json_t *obj);

  void print();

private:
  ZPoint m_center;
  double m_radius;
  QColor m_color;
  EGraphShape m_shape;
};

struct Z3DGraphEdge {
public:
  Z3DGraphEdge();
  Z3DGraphEdge(int vs, int vt);
  Z3DGraphEdge(const Z3DGraphEdge &edge);

  inline int vs() const { return m_vs; }
  inline int vt() const { return m_vt; }
  inline int getWidth() const { return m_width; }
  inline const QColor& startColor() const { return m_startColor; }
  inline const QColor& endColor() const { return m_endColor; }
  inline EGraphShape shape() const { return m_shape; }
  inline bool isValid() { return vs() >= 0 && vt() >= 0; }
  void set(int vs, int vt, double width = 1.0);
  void set(int vs, int vt, double width, bool usingNodeColor,
           const QColor &startColor, const QColor &endColor,
           EGraphShape shape);
  void setConnection(int vs, int vt);
  inline void setShape(EGraphShape shape) { m_shape = shape; }
  inline void setShape(const std::string &shape);
  inline void setWidth(double width) { m_width = width; }

  inline void setStartColor(const QColor &color) { m_startColor = color; }
  inline void setEndColor(const QColor &color) { m_endColor = color; }
  inline void useNodeColor(bool on = true) { m_usingNodeColor = on; }
  inline bool usingNodeColor() const { return m_usingNodeColor; }

  void loadJsonObject(json_t *obj);

  void print();
private:
  int m_vs;
  int m_vt;
  double m_width;
  bool m_usingNodeColor;
  QColor m_startColor;
  QColor m_endColor;
  EGraphShape m_shape;
};

class Z3DGraph : public ZStackObject
{
public:
  Z3DGraph();

public:
  bool isEmpty() const;

  inline size_t getNodeNumber() const { return m_nodeArray.size(); }
  inline size_t getEdgeNumber() const { return m_edgeArray.size(); }

  inline const Z3DGraphNode& getNode(size_t index) const {
    return m_nodeArray[index];
  }

  inline const Z3DGraphEdge& getEdge(size_t index) const {
    return m_edgeArray[index];
  }

  inline const Z3DGraphNode& getStartNode(size_t index) const {
    return m_nodeArray[m_edgeArray[index].vs()];
  }
  inline const Z3DGraphNode& getEndNode(size_t index) const {
    return m_nodeArray[m_edgeArray[index].vt()];
  }

  void append(const Z3DGraph &graph);

  void clear();

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;
  const std::string& className() const;

  void addNode(const Z3DGraphNode &node);
  void addEdge(const Z3DGraphEdge &edge);
  void addEdge(const Z3DGraphNode &node1, const Z3DGraphNode &node2,
               EGraphShape shape = GRAPH_CYLINDER);

public:
  void importPointNetwork(const ZPointNetwork &pointNetwork,
                          ZNormColorMap *colorMap = NULL);
  void importJsonFile(const std::string &filePath);
  void importObject3d(const ZObject3d &obj, double radius);
  void importObject3d(const ZObject3d &obj, double radius, int sampleStep);
  void addNode(const ZStackBall &ball);
  void addNode(double x, double y, double z, double radius);

  void print();

private:
  std::vector<Z3DGraphNode> m_nodeArray;
  std::vector<Z3DGraphEdge> m_edgeArray;
};

#endif // Z3DGRAPH_H
