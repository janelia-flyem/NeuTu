#ifndef ZMULTISCALEMANAGEMENT_H
#define ZMULTISCALEMANAGEMENT_H

#include <vector>
#include <qtreeview.h>
#include "zsandboxmodule.h"
#include "zstack.hxx"
#include "zsegmentationrepresentation.h"
#include "flyem/zstackwatershedcontainer.h"

class QStandardItem;
class QStandardItemModel;
class ZStack;
class ZStackObject;
class ZStackFrame;
class QCheckBox;
class QSpinBox;


/*
class ZTreeView:public QTreeView
{
public:
  ZTreeView(QWidget* parent):QTreeView::QTreeView(parent){}
 protected:
  void  dropEvent(QDropEvent * event);
  QStandardItem* deepCopy(QStandardItem* item);
};*/


class ZMultiscaleSegmentationWindow:public QWidget
{
  Q_OBJECT
protected:
  ZMultiscaleSegmentationWindow();

public:
  static ZMultiscaleSegmentationWindow* instance();
  ~ZMultiscaleSegmentationWindow();

  //void moveNode(QString label, QString new_parent_label);
  //QStandardItem* findItemById(QStandardItem* root, int id);
  //void clearTreeView();
  //void selectNode(int id);
  //void deselectNode(int id);

private slots:
  void onOpenStack();
  void onSegment();
  /*void onAutoSegment();*/
  //void onClear();
  //void onExport();
  //void onSelectNode(QModelIndex index);
  //void onMerge();
  //void onFlood();
  //void onPromote();

private:
  std::vector<ZStackObject*> getSeeds();
  //std::vector<ZStack*> seedsFromMaximum(ZStack* stack);
  void removeSeeds();
  //QStandardItem* getSelectedNodeItem();
  //void highLight(ZSegmentationNode* node);

private:
  void init();
  void initWidgets();
public:
  //ZSegmentationViewNode* m_root;
  //ZTreeView* m_tree_view;
private:
  ZStack* m_stack;
  ZStackFrame* m_frame;
  ZSegmentationRepresentationDefault m_segmentation;
  //QStandardItemModel* m_tree;
  //QCheckBox* m_show_leaf;
  //QCheckBox* m_leaky_boundary;
  //QSpinBox *m_alpha, *m_beta;
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

  //ZMultiscaleSegmentationWindow* m_window;
};

#endif // ZMULTISCALEMANAGEMENT_H
