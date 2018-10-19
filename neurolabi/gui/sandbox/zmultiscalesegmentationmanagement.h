#ifndef ZMULTISCALEMANAGEMENT_H
#define ZMULTISCALEMANAGEMENT_H

#include <vector>
#include <map>
#include <memory>
#include <QWidget>
#include <QTreeView>
#include "zintpoint.h"
#include "zsandboxmodule.h"

class ZObject3dScan;
class ZObject3dScanArray;
class QStandardItem;
class ZStack;
class ZStackObject;
class ZIntCuboid;

class ZSegmentationScan
{
public:
ZSegmentationScan();
~ZSegmentationScan();
//void init();

public:
void loadStack(ZStack* stack);
void maskStack(ZStack* stack);
void labelStack(ZStack* stack, int v=1);

//void setOffset(ZIntPoint offset){m_offset = offset;}
//void setOffset(int x, int y, int z){m_offset = ZIntPoint(x,y,z);}
//ZIntPoint getOffset()const{return m_offset;}
void unify(ZSegmentationScan* obj);
inline std::vector<int>& getStrip(int z, int y){return (z < m_sz || z > m_ez || y < m_sy || y > m_ey) ? m_empty_vec : getSlice(z)[y-m_sy];}
inline std::vector<std::vector<int>>& getSlice(int z){return (z < m_sz || z > m_ez) ? m_empty_vec_vec : m_data[z-m_sz];}
std::vector<std::vector<std::vector<int>>>& getData(){return m_data;}
public:
inline int minZ()const {return m_sz;}
inline int maxZ()const {return m_ez;}
inline int minY()const {return m_sy;}
inline int maxY()const {return m_ey;}

private:
inline void addSegment(int z, int y, int sx, int ex);
ZIntCuboid getStackForegroundBoundBox(ZStack* stack);
//void canonizeStrip(int z, int y);
std::vector<int> canonize(std::vector<int>&a, std::vector<int>& b);

void clearData();
void preprareData(int depth, int height);

private:
std::vector<std::vector<std::vector<int>>> m_data;
std::vector<int> m_empty_vec;
std::vector<std::vector<int>> m_empty_vec_vec;
//ZIntPoint m_offset;
int m_sz, m_ez;
int m_sy, m_ey;
};


class ZSegmentationNode
{
public:
ZSegmentationNode(){m_parent = NULL; m_data = NULL; m_label="";}
~ZSegmentationNode(){destroy();}
void destroy();

public:
QString label(){return m_label;}
void setLabel(QString label){m_label = label;}
void updateChildrenLabel();
void setData(ZObject3dScan* data){m_data = data;}
ZObject3dScan* data()const{return m_data;}

bool isRoot()const{return m_parent == NULL;}
bool isLeaf()const{return m_children.size()==0;}

void setParent(ZSegmentationNode* parent){m_parent = parent;}
ZSegmentationNode* parent()const{return m_parent;}

void appendChild(ZSegmentationNode* child){ child->setParent(this); m_children.push_back(child);}
void removeChild(ZSegmentationNode* child, bool b_delete=false);
std::vector<ZSegmentationNode*>& children(){return m_children;}
void clearChildren();

int indexOf(ZSegmentationNode* node);

void mergeNode(ZSegmentationNode* node);
void splitNode(ZStack* stack, std::vector<ZStackObject*>& seeds);
void regularize();

public:
ZSegmentationNode* find(QString label);
void makeMask(ZObject3dScan* mask);
void display(QStandardItem* tree);

private:
void consumeSegmentations(ZObject3dScanArray& segmentations);
int estimateScale(size_t volume);

private:
QString m_label;
ZObject3dScan* m_data;
ZSegmentationNode* m_parent;
std::vector<ZSegmentationNode*> m_children;
};


class ZTreeView:public QTreeView
{
public:
  ZTreeView(QWidget* parent):QTreeView::QTreeView(parent){}
 protected:
  void  dropEvent(QDropEvent * event);
};

class ZStack;
class ZStackObject;
class ZStackFrame;
class QStandardItemModel;

class ZMultiscaleSegmentationWindow:public QWidget
{
  Q_OBJECT
public:
  ZMultiscaleSegmentationWindow(QWidget *parent = 0);
  ~ZMultiscaleSegmentationWindow();
  void moveNode(QString label, QString new_parent_label);
  QStandardItem* findItemByText(QStandardItem* root, QString text);

private slots:
  void onOpenStack();
  void onSegment();
  void onClear();
  void onExport();
  void onSelectNode(QModelIndex index);

private:
  std::vector<ZStackObject*> getSeeds();
  void removeSeeds();
  void clearTreeView();
  QStandardItem* getSelectedNodeItem();
  void highLight(ZSegmentationNode* node, QColor color= QColor(255,0,0));

private:
  void init();
  void initWidgets();

private:
  ZStack* m_stack;
  ZStackFrame* m_frame;
  ZTreeView* m_tree_view;
  QStandardItemModel* m_tree;
  ZSegmentationNode* m_root;
};


class ZMultiscaleSegManagementModule:public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZMultiscaleSegManagementModule(QObject *parent = 0);
  ~ZMultiscaleSegManagementModule();

private slots:
    void execute();

private:
  void init();

private:

  ZMultiscaleSegmentationWindow* m_window;
};

#endif // ZMULTISCALEMANAGEMENT_H
