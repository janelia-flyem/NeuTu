#ifndef Z3DGRAPHFILTER_H
#define Z3DGRAPHFILTER_H

#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "z3dgraph.h"
#include "zwidgetsgroup.h"
#include "z3dlinerenderer.h"
#include "z3dconerenderer.h"
#include "z3dsphererenderer.h"
#include "zobject3d.h"

class Z3DGraphFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit Z3DGraphFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void setData(const ZPointNetwork &pointCloud, ZNormColorMap *colorMap = NULL);
  void setData(const Z3DGraph &graph);
  void addData(const Z3DGraph &graph);
  void setData(const ZObject3d &obj);

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

  inline bool showingArrow() { return m_showingArrow; }

  virtual bool isReady(Z3DEye eye) const override;

//  void setVisible(bool v);
//  bool isVisible() const;

  void configure(const ZJsonObject &obj) override;

protected:
  void prepareData();
  void prepareColor();
  void updateGraphVisibleState();

  virtual void process(Z3DEye eye) override;

  std::vector<double> boundBox();

  virtual void updateNotTransformedBoundBoxImpl() override;

private:
  Z3DGraph m_graph;

//  ZBoolParameter m_showGraph;

  Z3DLineRenderer m_lineRenderer;
  Z3DConeRenderer m_coneRenderer;
  Z3DConeRenderer m_arrowRenderer;
  Z3DSphereRenderer m_sphereRenderer;

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

  bool m_dataIsInvalid = false;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;

  bool m_showingArrow = true;
};

#endif // Z3DGRAPHFILTER_H
