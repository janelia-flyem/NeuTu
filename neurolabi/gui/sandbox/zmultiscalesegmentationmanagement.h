#ifndef ZMULTISCALEMANAGEMENT_H
#define ZMULTISCALEMANAGEMENT_H

#include <vector>
#include <qtreeview.h>
#include "zsegmentationscan.h"
#include "zsandboxmodule.h"
#include "zstack.hxx"
#include "flyem/zstackwatershedcontainer.h"

class QStandardItem;
class QStandardItemModel;
class ZStack;
class ZStackObject;
class ZStackFrame;
class QCheckBox;
class QSpinBox;


class ZSegmentationNode
{
public:
ZSegmentationNode(){m_parent = NULL; m_data = NULL; m_id = getNextId();}
~ZSegmentationNode(){destroy();}
void destroy();

public:
int id(){return m_id;}
void setData(ZSegmentationScan* data){m_data = data;}
ZSegmentationScan* data()const{return m_data;}

bool isRoot()const{return m_parent == NULL;}
bool isLeaf()const{return m_children.size()==0;}

void setParent(ZSegmentationNode* parent){m_parent = parent;}
ZSegmentationNode* parent()const{return m_parent;}


void appendChild(ZSegmentationNode* child){ child->setParent(this); m_children.push_back(child);}
void removeChild(ZSegmentationNode* child, bool b_delete=false);
std::vector<ZSegmentationNode*>& children(){return m_children;}
void clearChildren();

std::vector<ZSegmentationNode*> getLeaves();

int indexOf(ZSegmentationNode* node);
static int getNextId(){return s_id++;}

void mergeNode(ZSegmentationNode* node);

template<typename T>
void splitNode(ZStack* stack, std::vector<T*>& seeds, QString algorithm = "watershed", double alpha = 1.0, double beta = 1.0);
void regularize();

public:
ZSegmentationNode* find(int id);
void makeMask(ZSegmentationScan* mask);
void display(QStandardItem* tree);

private:
void consumeSegmentations(std::vector<ZSegmentationScan*>& segmentations);
int estimateScale(size_t volume);
void addSeed(ZStackWatershedContainer& container, std::vector<ZStackObject*>& seeds){for(auto seed: seeds)container.addSeed(seed);}
void addSeed(ZStackWatershedContainer& container, std::vector<ZStack*>& seeds){for(auto seed:seeds)container.addSeed(*seed);}
void collectLeaves(std::vector<ZSegmentationNode*>& leaves);

public:
int static s_id;
private:
int m_id;
ZSegmentationScan* m_data;
ZSegmentationNode* m_parent;
std::vector<ZSegmentationNode*> m_children;
};


class ZTreeView:public QTreeView
{
public:
  ZTreeView(QWidget* parent):QTreeView::QTreeView(parent){}
 protected:
  void  dropEvent(QDropEvent * event);
  QStandardItem* deepCopy(QStandardItem* item);
};


class ZMultiscaleSegmentationWindow:public QWidget
{
  Q_OBJECT
protected:
  ZMultiscaleSegmentationWindow();

public:
  static ZMultiscaleSegmentationWindow* instance();
  ~ZMultiscaleSegmentationWindow();

  void moveNode(QString label, QString new_parent_label);
  QStandardItem* findItemById(QStandardItem* root, int id);
  void clearTreeView();
  void selectNode(int id);
  void deselectNode(int id);

private slots:
  void onOpenStack();
  void onSegment();
  /*void onAutoSegment();*/
  void onClear();
  void onExport();
  void onSelectNode(QModelIndex index);
  void onMerge();
  //void onFlood();
  void onPromote();

private:
  std::vector<ZStackObject*> getSeeds();
  std::vector<ZStack*> seedsFromMaximum(ZStack* stack);
  void removeSeeds();
  QStandardItem* getSelectedNodeItem();
  void highLight(ZSegmentationNode* node);
  void showMask(ZObject3dScan* mask);

private:
  void init();
  void initWidgets();
public:
  ZSegmentationNode* m_root;
  ZTreeView* m_tree_view;
private:
  ZStack* m_stack;
  ZStackFrame* m_frame;
  QStandardItemModel* m_tree;
  QCheckBox* m_show_leaf;
  QCheckBox* m_leaky_boundary;
  QSpinBox *m_alpha, *m_beta;
  static ZMultiscaleSegmentationWindow* s_window;
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
