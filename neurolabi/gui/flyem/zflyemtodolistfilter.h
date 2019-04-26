#ifndef ZFLYEMTODOLISTFILTER_H
#define ZFLYEMTODOLISTFILTER_H

#include <QList>

#include "z3dgeometryfilter.h"
#include "z3dgraph.h"
#include "widgets/zwidgetsgroup.h"
#include "z3dfontrenderer.h"

class ZFlyEmToDoList;
class Z3DLineRenderer;
class Z3DSphereRenderer;
class ZFlyEmToDoItem;

class ZFlyEmTodoListFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);
  virtual ~ZFlyEmTodoListFilter();

  void setData(const ZStackObject *obj);
  void setData(const ZFlyEmToDoList &todoList);
  void setData(const QList<ZFlyEmToDoItem*> &todoList);

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual bool isReady(Z3DEye eye) const override;

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

signals:
  void objectSelected(ZStackObject*, bool append);
  void annotatingObject(ZStackObject*);

protected:
  void prepareColor();
  void selectObject(QMouseEvent *e, int w, int h);
  virtual void process(Z3DEye) override;
  void prepareData();
  void render(Z3DEye eye);
  virtual void renderPicking(Z3DEye eye) override;

  virtual void registerPickingObjects() override;

  virtual void deregisterPickingObjects() override;

  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

  void updateGraph();

private:
  void updateGraphVisibleState();
  void addItem(ZFlyEmToDoItem *item);
  void addItemNode(const ZFlyEmToDoItem *item);

  void resetData();

private:
  Z3DGraph m_graph;
  std::vector<ZFlyEmToDoItem*> m_itemList;
  std::vector<ZFlyEmToDoItem*> m_registeredItemList;    // used for picking

  ZEventListenerParameter m_selectItemEvent;
  glm::ivec2 m_startCoord;
  ZFlyEmToDoItem* m_pressedItem = nullptr;
  std::set<ZFlyEmToDoItem*> m_selected;

//  ZBoolParameter m_showGraph;

  Z3DLineRenderer m_lineRenderer;
  Z3DSphereRenderer m_sphereRenderer;
  Z3DFontRenderer m_fontRenderer;
  Z3DSphereRenderer m_sphereRendererForPicking;
  std::vector<Z3DPrimitiveRenderer*> m_sceneRender;
  std::vector<Z3DPrimitiveRenderer*> m_pickingRender;

  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec3> m_textPosition;
  std::vector<glm::vec4> m_pointAndRadiusForPicking;
  QStringList m_textList;

  std::vector<glm::vec4> m_pointPickingColors;

  bool m_dataIsInvalid = false;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
};

#endif // ZFLYEMTODOLISTFILTER_H
