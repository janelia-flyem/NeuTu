#ifndef ZFLYEMTODOLISTFILTER_H
#define ZFLYEMTODOLISTFILTER_H

#include <QList>

#include "z3dgeometryfilter.h"
#include "z3dgraph.h"
#include "zwidgetsgroup.h"

class ZFlyEmToDoList;
class Z3DLineRenderer;
class Z3DSphereRenderer;
class ZFlyEmToDoItem;
class Z3DLineWithFixedWidthColorRenderer;

class ZFlyEmTodoListFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListFilter();
  virtual ~ZFlyEmTodoListFilter();



  void setData(const ZStackObject *obj);
  void setData(const ZFlyEmToDoList &todoList);
  void setData(const QList<ZFlyEmToDoItem*> &todoList);

  std::vector<double> boundBox();

  ZWidgetsGroup *getWidgetsGroup();


  bool isReady(Z3DEye eye) const;

  void setVisible(bool v);
  bool isVisible() const;

signals:
  void objectSelected(ZStackObject*, bool append);
  void visibleChanged(bool);

protected slots:
  void prepareColor();
  void selectObject(QMouseEvent *e, int w, int h);

protected:
  virtual void initialize();
  virtual void deinitialize();
  virtual void process(Z3DEye);
  void prepareData();
  void render(Z3DEye eye);
  void renderPicking(Z3DEye eye);
  void renderSelectionBox(Z3DEye eye);

  void registerPickingObjects(Z3DPickingManager *pm);
  void deregisterPickingObjects(Z3DPickingManager *pm);

private:
  void init();
  void updateGraphVisibleState();
  void addItem(ZFlyEmToDoItem *item);

  void resetData();

private:
  Z3DGraph m_graph;
  std::vector<ZFlyEmToDoItem*> m_itemList;
  std::vector<ZFlyEmToDoItem*> m_registeredItemList;    // used for picking

  ZEventListenerParameter* m_selectItemEvent;
  glm::ivec2 m_startCoord;
  ZFlyEmToDoItem *m_pressedItem;
  std::set<ZFlyEmToDoItem*> m_selected;

  ZBoolParameter m_showGraph;

  Z3DLineRenderer *m_lineRenderer;
  Z3DSphereRenderer *m_sphereRenderer;
  Z3DLineWithFixedWidthColorRenderer *m_boundBoxRenderer;

  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_pointColors;

  std::vector<glm::vec4> m_pointPickingColors;

  bool m_dataIsInvalid;
  ZIntSpanParameter m_xCut;
  ZIntSpanParameter m_yCut;
  ZIntSpanParameter m_zCut;

  ZWidgetsGroup *m_widgetsGroup;
};

#endif // ZFLYEMTODOLISTFILTER_H
