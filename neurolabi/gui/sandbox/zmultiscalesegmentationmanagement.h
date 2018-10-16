#ifndef ZMULTISCALEMANAGEMENT_H
#define ZMULTISCALEMANAGEMENT_H

#include <vector>
#include <map>
#include <memory>
#include <QWidget>
#include <QTreeView>
#include "zsandboxmodule.h"

class ZObject3dScan;
class ZObject3dScanArray;
class QStandardItem;
class ZStack;
class ZStackObject;

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
int getColorIndex();

private:
void consumeSegmentations(ZObject3dScanArray& segmentations);

private:
QString m_label;
ZObject3dScan* m_data;
ZSegmentationNode* m_parent;
std::vector<ZSegmentationNode*> m_children;

public:
std::map<QString,int> static m_color_table;
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

  QStandardItem* getSelectedNodeItem();

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
