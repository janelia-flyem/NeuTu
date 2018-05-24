#ifndef ZOBJSMANAGERWIDGET_H
#define ZOBJSMANAGERWIDGET_H

#include <QWidget>
#include <QTreeView>

class ZStackDoc;
class ZPunctum;
class ZSwcTree;
struct _Swc_Tree_Node;
typedef _Swc_Tree_Node Swc_Tree_Node;
class ZMesh;
class QSortFilterProxyModel;
class ZSwcObjsModel;
class ZPunctaObjsModel;
class ZMeshObjsModel;
class ZDocPlayerObjsModel;
class ZGraphObjsModel;

class ZObjsManagerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZObjsManagerWidget(ZStackDoc *doc, QWidget *parent = 0);
  virtual ~ZObjsManagerWidget();
  
signals:
  void swcDoubleClicked(ZSwcTree *tree);
  void swcNodeDoubleClicked(Swc_Tree_Node* node);
  void punctaDoubleClicked(ZPunctum* p);
  void meshDoubleClicked(ZMesh* p);
  
public slots:
  void swcItemDoubleClicked(QModelIndex index);
//  void processDoubleClickOnCategorizedSwcNode(QModelIndex index);
  void swcSelectionChangedFromTreeView(
      QItemSelection selected, QItemSelection deselected);
//  void updateSelectionFromCategorizedSwcNode(
//      QItemSelection selected, QItemSelection deselected);

  void punctaItemDoubleClicked(QModelIndex index);
  void punctaSelectionChangedFromTreeView(
      QItemSelection selected, QItemSelection deselected);

  void meshItemDoubleClicked(QModelIndex index);
  void meshSelectionChangedFromTreeView(
      QItemSelection selected, QItemSelection deselected);

  void punctaSelectionChanged(
      QList<ZPunctum*> selected, QList<ZPunctum*> deselected);
  void meshSelectionChanged(
      QList<ZMesh*> selected, QList<ZMesh*> deselected);
  void swcSelectionChanged(
      QList<ZSwcTree*> selected, QList<ZSwcTree*> deselected);
  /*
  void swcTreeNodeSelectionChanged(
      QList<Swc_Tree_Node*> selected, QList<Swc_Tree_Node*> deselected);
      */

  //void updateSelection();

protected:
  void createWidget();

  virtual void keyPressEvent(QKeyEvent *event);

  void buildItemSelectionFromList(const QList<ZPunctum*> &list, QItemSelection &is);
  void buildItemSelectionFromList(const QList<ZSwcTree*> &list, QItemSelection &is);
//  void buildItemSelectionFromList(const QList<Swc_Tree_Node*> &list, QItemSelection &is);
  void buildItemSelectionFromList(const QList<ZMesh*> &list, QItemSelection &is);

  QList<Swc_Tree_Node*> getSwcNodeList(QItemSelection &is);

  ZSwcObjsModel* getSwcObjsModel();
  ZPunctaObjsModel* getPunctaObjsModel();
  ZMeshObjsModel* getMeshObjsModel();
  ZDocPlayerObjsModel* getSeedObjsModel();
  ZGraphObjsModel* getGraphObjsModel();

private:

  ZStackDoc *m_doc;
  //old selection
  QTreeView *m_swcObjsTreeView;
  QTreeView *m_swcNodeObjsTreeView;
  QTreeView *m_punctaObjsTreeView;
  QTreeView *m_seedObjsTreeView;
  QTreeView *m_graphObjsTreeView;
  QTreeView *m_surfaceObjsTreeView;
  QTreeView *m_meshObjsTreeView;
  QTreeView *m_roiObjsTreeView;
  QSortFilterProxyModel *m_punctaProxyModel;
};

#endif // ZOBJSMANAGERWIDGET_H
