#ifndef Z3DSWCFILTER_H
#define Z3DSWCFILTER_H

#include "z3dgeometryfilter.h"
#include "zoptionparameter.h"

#include <map>
#include <QString>
#include <vector>
#include <utility>
#include <QList>

#include "zswctree.h"
#include "zcolormap.h"
#include "zswccolorscheme.h"
#include "zwidgetsgroup.h"
#include "z3dlinerenderer.h"
#include "z3dconerenderer.h"
#include "z3dsphererenderer.h"
#include "zeventlistenerparameter.h"
#include "zobject3d.h"

class Z3DSwcFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  enum InteractionMode {
    Select, AddSwcNode, ConnectSwcNode, SmartExtendSwcNode, PlainExtendSwcNode
  };

  explicit Z3DSwcFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);
  ~Z3DSwcFilter();

  void setData(const std::vector<ZSwcTree*>& swcList);
  void setData(const QList<ZSwcTree*>& swcList);

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  inline void setRenderingPrimitive(const std::string& mode)
  {
    m_renderingPrimitive.select(mode.c_str());
  }

  bool isNodeRendering() const
  { return m_renderingPrimitive.isSelected("Sphere"); }

  void setInteractionMode(InteractionMode mode)
  { m_interactionMode = mode; }

  inline InteractionMode interactionMode()
  { return m_interactionMode; }

  virtual bool hasOpaque(Z3DEye /*unused*/) const override
  { return m_rendererBase.opacity() == 1.f && !m_renderingPrimitive.isSelected("Line"); }

  virtual void renderOpaque(Z3DEye eye) override;

  virtual bool hasTransparent(Z3DEye /*unused*/) const override
  { return m_rendererBase.opacity() < 1.f || m_renderingPrimitive.isSelected("Line"); }

  virtual void renderTransparent(Z3DEye eye) override;

  bool isNodePicking() const;


  void enablePicking(bool picking) {
    m_enablePicking = picking;
  }

  virtual void configure(const ZJsonObject &obj) override;
  ZJsonObject getConfigJson() const override;

  void setColorMode(const std::string& mode);

  //get bounding box of swc tree in world coordinate
  void treeBound(ZSwcTree* tree, ZBBox<glm::dvec3>& res) const;
  void forceNodePicking(bool picking) {
    m_forceNodePicking = picking;
  }

//  void setVisible(bool v);
//  bool isVisible() const;

  //get bounding box of swc tree node in world coordinate
  void treeNodeBound(Swc_Tree_Node* tn, ZBBox<glm::dvec3>& result) const;

  void addNodeType(int type);

  void updateSwcVisibleState();

  Swc_Tree_Node* pickSwcNode(double x, double y);
  QList<Swc_Tree_Node*> pickSwcNode(const ZObject3d &ptArray);
  void selectSwcNode(const ZObject3d &ptArray);

  void changeGeometryMode()
  { m_renderingPrimitive.selectNext(); }

signals:
  void treeSelected(ZSwcTree*, bool append);
  void treeNodeSelected(Swc_Tree_Node*, bool append);
  void treeNodeSelected(QList<Swc_Tree_Node*>, bool append);
  void connectingSwcTreeNode(Swc_Tree_Node*);
  void treeNodeSelectConnection(Swc_Tree_Node*);
  void treeNodeSelectFloodFilling(Swc_Tree_Node*);
  void addNewSwcTreeNode(double x, double y, double z, double r);
  void extendSwcTreeNode(double x, double y, double z, double r);

protected:
  void prepareColor();

  void adjustWidgets();

  void selectSwc(QMouseEvent* e, int w, int h);

  virtual void process(Z3DEye /*unused*/) override;

  virtual void registerPickingObjects() override;

  virtual void deregisterPickingObjects() override;

  void renderPicking(Z3DEye eye) override;

  void prepareData();

  void notTransformedTreeBound(ZSwcTree* tree, ZBBox<glm::dvec3>& result) const;

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

  void notTransformedTreeNodeBound(Swc_Tree_Node* tn, ZBBox<glm::dvec3>& result) const;

private:
  void initTopologyColor();

  void initTypeColor();
  void initLabelTypeColor();
  void initSubclassTypeColor();

  void decompseSwcTree();

  glm::vec4 getColorByType(Swc_Tree_Node *n);

  glm::vec4 getColorByDirection(Swc_Tree_Node *tn);

  glm::dvec3 projectPointOnRay(
      glm::dvec3 pt, const glm::dvec3 &v1, const glm::dvec3 &v2);

  void addSelectionBox(const std::pair<Swc_Tree_Node *, Swc_Tree_Node *> &nodePair,
                       std::vector<glm::vec3> &lines);

  void addSelectionBox(const Swc_Tree_Node *tn, std::vector<glm::vec3> &lines);

  static QString GetTypeName(int type);

  // get visible data from origSwcList put into swcList
  void loadVisibleData();

  void sortNodeList();
  void clearDecorateSwcList();

private:
  Z3DLineRenderer m_lineRenderer;
  Z3DConeRenderer m_coneRenderer;
  Z3DSphereRenderer m_sphereRenderer;
  Z3DSphereRenderer m_sphereRendererForCone;

  ZStringIntOptionParameter m_renderingPrimitive;
  ZStringIntOptionParameter m_colorMode;

  std::map<ZSwcTree*, std::unique_ptr<ZVec4Parameter>> m_individualTreeColorMapper;
  std::map<ZSwcTree*, std::unique_ptr<ZVec4Parameter>> m_randomTreeColorMapper;
  std::map<int, std::unique_ptr<ZVec4Parameter>> m_biocytinColorMapper;
  std::map<int, size_t> m_subclassTypeColorMapper;

  std::vector<std::unique_ptr<ZVec4Parameter>> m_colorsForDifferentType;
  std::vector<std::unique_ptr<ZVec4Parameter>> m_colorsForSubclassType;
  std::vector<std::unique_ptr<ZVec4Parameter>> m_colorsForLabelType;
  std::vector<std::unique_ptr<ZVec4Parameter>> m_colorsForDifferentTopology;
  ZColorMapParameter m_colorMapBranchType;


  // swc list used for rendering, it is a subset of m_origSwcList. Some swcs are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single swc tree.
  std::vector<ZSwcTree*> m_swcList;
  std::vector<ZSwcTree*> m_registeredSwcList;    // used for picking
  std::vector<ZSwcTree*> m_decorateSwcList;  //For decoration. Self-owned.
  std::vector<Swc_Tree_Node*> m_registeredSwcTreeNodeList;    // used for picking

  ZEventListenerParameter m_selectSwcEvent;
  glm::ivec2 m_startCoord;

  ZSwcTree* m_pressedSwc = nullptr;
  Swc_Tree_Node* m_pressedSwcTreeNode = nullptr;
  std::set<ZSwcTree*> m_selectedSwcs;   //point to all selected swcs, managed by other class

  std::vector<glm::vec4> m_baseAndBaseRadius;
  std::vector<glm::vec4> m_axisAndTopRadius;
  std::vector<glm::vec4> m_swcColors1;
  std::vector<glm::vec4> m_swcColors2;
  std::vector<glm::vec4> m_swcPickingColors;
  std::vector<glm::vec4> m_sphereForConePickingColors;
  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_linePickingColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;

  std::vector<std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*>>> m_decompsedNodePairs;
  std::vector<std::vector<Swc_Tree_Node*>> m_decomposedNodes;
  std::vector<Swc_Tree_Node*> m_sortedNodeList;
//  std::set<Swc_Tree_Node*> m_allNodesSet;  // for fast search
  std::set<int> m_allNodeType;   // all node type of current opened swc, used for adjust widget (hide irrelavant stuff)
  int m_maxType;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  bool m_dataIsInvalid = false;

  std::vector<ZSwcTree*> m_origSwcList;

  InteractionMode m_interactionMode = Select;
  ZSwcColorScheme m_colorScheme;

  bool m_enableCutting = true;
  bool m_enablePicking = true;
  bool m_forceNodePicking = false;

  QVector<QString> m_guiNameList;

  mutable QMutex m_nodeSelectionMutex;
  mutable QMutex m_dataValidMutex;
};

#endif // Z3DSWCFILTER_H
