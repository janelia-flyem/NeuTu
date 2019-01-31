#ifndef Z3DGRAPHFILTER_H
#define Z3DGRAPHFILTER_H

#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "z3dgraph.h"
#include "widgets/zwidgetsgroup.h"
#include "z3dlinerenderer.h"
#include "z3dconerenderer.h"
#include "z3dsphererenderer.h"
#include "z3dfontrenderer.h"

class ZDocPlayer;
class ZObject3d;

class Z3DGraphFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit Z3DGraphFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);
  ~Z3DGraphFilter();

  void clear();

  void setData(const ZPointNetwork &pointCloud, ZNormColorMap *colorMap = NULL);
  void setData(const Z3DGraph &graph);
  Z3DGraphPtr addData(const Z3DGraph &graph);
  void setData(const ZObject3d &obj);

  void addData(const ZDocPlayer &player);
  void addData(const QList<ZDocPlayer*> &playerList);
  void addData(Z3DGraph *graph);

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

  inline bool showingArrow() { return m_showingArrow; }

  virtual bool isReady(Z3DEye eye) const override;

//  void setVisible(bool v);
//  bool isVisible() const;

  void configure(const ZJsonObject &obj) override;

  void renderSelectionBox(Z3DEye eye);
  void deselectAllGraph();

signals:
  void objectSelected(ZStackObject *obj, bool appending);

protected:
  void prepareData();
  void prepareColor();
  void updateGraphVisibleState();
  virtual void renderPicking(Z3DEye eye) override;
  virtual void registerPickingObjects() override;
  virtual void deregisterPickingObjects() override;

  virtual void process(Z3DEye eye) override;

  std::vector<double> boundBox();

  virtual void updateNotTransformedBoundBoxImpl() override;

  void selectGraph(QMouseEvent *e, int w, int h);
  virtual void addSelectionLines() override;
  void graphBound(const Z3DGraphPtr &p, ZBBox<glm::dvec3> &result);

private:
  void updateSelection();

private:
//  Z3DGraph m_graph;
  QList<Z3DGraphPtr> m_graphList;
  QList<Z3DGraphPtr> m_registeredGraphList;
  Z3DGraphPtr m_pressedGraph;

  QMap<Z3DGraph*, ZStackObject*> m_objectMap;

//  ZBoolParameter m_showGraph;

  Z3DLineRenderer m_lineRenderer;
  Z3DConeRenderer m_coneRenderer;
  Z3DConeRenderer m_arrowRenderer;
  Z3DSphereRenderer m_sphereRenderer;
  Z3DFontRenderer m_fontRenderer;

  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;

  std::vector<glm::vec4> m_arrowBaseAndBaseRadius;
  std::vector<glm::vec4> m_arrowAxisAndTopRadius;

  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_lineStartColors;
  std::vector<glm::vec4> m_lineEndColors;
  std::vector<glm::vec4> m_arrowStartColors;
  std::vector<glm::vec4> m_arrowEndColors;
  std::vector<glm::vec3> m_textPosition;
  QStringList m_textList;

  std::vector<glm::vec4> m_graphPickingColors;
//  glm::ivec2 m_startCoord;
  ZEventListenerParameter m_selectGraphEvent;

  bool m_dataIsInvalid = false;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;

  bool m_showingArrow = true;
};

#endif // Z3DGRAPHFILTER_H
