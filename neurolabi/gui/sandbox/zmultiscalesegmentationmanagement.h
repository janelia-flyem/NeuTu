#ifndef ZMULTISCALEMANAGEMENT_H
#define ZMULTISCALEMANAGEMENT_H

#include <vector>
#include <string>
#include <QTreeWidget>

#include "zsandboxmodule.h"
#include "zstack.hxx"
#include "zstackobject.h"
#include "flyem/zstackwatershedcontainer.h"
#include "segment/zsegmentationtree.h"
#include "segment/zmstcontainer.h"


using std::string;
using std::vector;

class ZStack;
class ZStackObject;
class ZStackFrame;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QLineEdit;


class ZMultiscaleSegmentationWindow:public QWidget,public ZSegmentationTreeObserver{
  Q_OBJECT
protected:
  ZMultiscaleSegmentationWindow();

public:
  static ZMultiscaleSegmentationWindow* instance();
  ~ZMultiscaleSegmentationWindow();

public:
  void selectNode(const string& id);
  void deselectNode(const string& id);

public:
  virtual void update(const ZSegmentationTree* tree, const string& id);

private slots:
  void onOpenStack();
  void onSegment();
  void onNodeItemClicked(QTreeWidgetItem*,int);
  void onMerge();
  void onExport();
  void onShowLeaves(int state);
  void onSuperVoxel();

private:
  std::vector<ZStackObject*> getSeeds();
  void removeSeeds();
  void updateMask(const std::string& active_id);
  void updateTreeView(const std::string& active_id);
  void test();
  ZStack* makeSelectedStack();

private:
  void init();

private:
  //shared_ptr<ZSuperVoxelManager> m_super_voxels;
  ZStack* m_stack;
  ZStackFrame* m_frame;
  shared_ptr<ZSegmentationTree> m_seg_tree;
  QTreeWidget* m_view;
  string m_selected_id;
  //QLineEdit* m_merge_from;
  //QLineEdit* m_merge_to;
  QCheckBox* m_show_leaves;
  QCheckBox* m_show_contour;
  QCheckBox* m_enable_super_voxel;

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
};

#endif // ZMULTISCALEMANAGEMENT_H
