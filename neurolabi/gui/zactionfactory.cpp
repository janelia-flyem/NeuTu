#include "zactionfactory.h"
#include <QWidget>
#include <QObject>
#include <QUndoStack>
#include <QString>
#include <QUndoStack>

#include "zstackdoc.h"
#include "zactionactivator.h"
#include "zstackpresenter.h"
#include "zstackframe.h"
#include "flyem/zflyembodycoloroption.h"

ZActionFactory::ZActionFactory()
{
}

void ZActionFactory::setUndoStack(QUndoStack *undoStack)
{
  m_undoStack = undoStack;
}

bool ZActionFactory::IsRegularAction(EAction actionKey)
{
  return actionKey != ZActionFactory::ACTION_NULL &&
      actionKey != ZActionFactory::ACTION_SEPARATOR;
}

namespace  {

static QAction* CreateColorAction(ZFlyEmBodyColorOption::EColorOption option,
                                  QObject *parent)
{
  QAction *action =
      new QAction(ZFlyEmBodyColorOption::GetColorMapName(option), parent);
  action->setCheckable(true);

  return action;
}

}

QAction* ZActionFactory::makeAction(EAction actionKey, QObject *parent) const
{
  QAction *action = NULL;

  if ((actionKey == ACTION_UNDO || actionKey == ACTION_REDO) &&
      m_undoStack != NULL) {
    switch (actionKey) {
    case ACTION_UNDO:
      action = m_undoStack->createUndoAction(parent, "&Undo");
      action->setIcon(QIcon(":/images/undo.png"));
      action->setShortcuts(QKeySequence::Undo);
      break;
    case ACTION_REDO:
      action = m_undoStack->createRedoAction(parent, "&Redo");
      action->setIcon(QIcon(":/images/redo.png"));
      action->setShortcuts(QKeySequence::Redo);
      break;
    default:
      break;
    }
  }

  if (action == NULL) {
    action = MakeAction(actionKey, parent);
  }

  return action;
}

QAction* ZActionFactory::MakeAction(EAction actionKey, QObject *parent)
{
  QAction *action = NULL;
  switch (actionKey) {
  case ACTION_UNDO:
  {
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(parent);
    if (undoStack != NULL) {
      action = undoStack->createUndoAction(parent, "&Undo");
      action->setIcon(QIcon(":/images/undo.png"));
      action->setShortcuts(QKeySequence::Undo);
    }
  }
    break;
  case ACTION_REDO:
  {
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(parent);
    if (undoStack != NULL) {
      action = undoStack->createRedoAction(parent, "&Redo");
      action->setIcon(QIcon(":/images/redo.png"));
      action->setShortcuts(QKeySequence::Redo);
    }
  }
    break;
  case ACTION_TEST:
    action = new QAction("Test", parent);
    action->setIcon(QIcon(":/images/test.png"));
    break;
  case ACTION_ABOUT:
    action = new QAction("About", parent);
    break;
  case ACTION_ADD_SWC_NODE:
    action = new QAction("Add Neuron Node", parent);
    action->setStatusTip("Add an isolated neuron node.");
    action->setIcon(QIcon(":/images/add.png"));
    action->setShortcut(Qt::Key_G);
    break;
  case ACTION_SELECT_DOWNSTREAM:
    action = new QAction("Downstream", parent);
    action->setStatusTip("Select downstream nodes");
    break;
  case ACTION_SELECT_UPSTREAM:
    action = new QAction("Upstream", parent);
    action->setStatusTip("Select upstream nodes");
    break;
  case ACTION_SELECT_NEIGHBOR_SWC_NODE:
    action = new QAction("Neighbors", parent);
    action->setStatusTip(
          "Select neighbors (nodes coonected directly) of the currently selected nodes");
    break;
  case ACTION_SELECT_SWC_BRANCH:
    action = new QAction("Host branch", parent);
    action->setStatusTip("Select branches containing the currently selected nodes");
    break;
  case ACTION_SELECT_CONNECTED_SWC_NODE:
    action = new QAction("All connected nodes", parent);
    action->setStatusTip("Select all nodes connected (directly or indirectly) "
                         "of the currently selected nodes");
    break;
  case ACTION_SELECT_ALL_SWC_NODE:
    action = new QAction("All nodes", parent);
    action->setShortcut(QKeySequence::SelectAll);
    action->setStatusTip("Selet all nodes");
    break;
  case ACTION_RESOLVE_CROSSOVER:
    action = new QAction("Resolve crossover", parent);
    action->setStatusTip("Create a crossover near the selected node if it is detected");
    break;
  case ACTION_REMOVE_TURN:
    action = new QAction("Remove turn", parent);
    action->setStatusTip("Remove a nearby sharp turn");
    break;
  case ACTION_MEASURE_SWC_NODE_LENGTH:
    action = new QAction("Path length", parent);
    break;
  case ACTION_MEASURE_SCALED_SWC_NODE_LENGTH:
    action = new QAction("Scaled Path length", parent);
    break;
  case ACTION_DELETE_SWC_NODE:
    action = new QAction("Delete", parent);
    action->setShortcut(Qt::Key_X);
    action->setStatusTip("Delete selected nodes");
    action->setIcon(QIcon(":/images/delete.png"));
    break;
  case ACTION_DELETE_UNSELECTED_SWC_NODE:
    action = new QAction("Delete Unselected", parent);
    action->setStatusTip("Delete unselected nodes");
    break;
  case ACTION_INSERT_SWC_NODE:
    action = new QAction("Insert", parent);
    action->setStatusTip("Insert a node between two adjacent nodes");
    action->setShortcut(Qt::Key_I);
    action->setIcon(QIcon(":/images/insert.png"));
    break;
  case ACTION_BREAK_SWC_NODE:
    action = new QAction("Break", parent);
    action->setStatusTip("Remove connections among the selected nodes");
    action->setShortcut(Qt::Key_B);
    action->setIcon(QIcon(":/images/cut.png"));
    break;
  case ACTION_CONNECT_SWC_NODE:
    action = new QAction("Connect", parent);
    action->setStatusTip("Connect selected nodes");
    action->setShortcut(Qt::Key_C);
    action->setIcon(QIcon(":/images/connect.png"));
    break;
  case ACTION_MERGE_SWC_NODE:
    action = new QAction("Merge", parent);
    action->setStatusTip("Merge selected nodes, which should form a single subtree");
    action->setIcon(QIcon(":/images/merge.png"));
    break;
  case ACTION_TRANSLATE_SWC_NODE:
    action = new QAction("Translate", parent);
    break;
  case ACTION_CHANGE_SWC_SIZE:
    action = new QAction("Change size", parent);
    break;
  case ACTION_SET_SWC_ROOT:
    action = new QAction("Set as a root", parent);
    action->setStatusTip("Set the selected node as a root");
    break;
  case ACTION_SET_BRANCH_POINT:
    action = new QAction("Join isolated branch", parent);
    action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes");
    break;
  case ACTION_CONNECTED_ISOLATED_SWC:
    action = new QAction("Join isolated brach (across trees)", parent);
    action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes. "
                         "The branch can be in another neuron.");
    break;
  case ACTION_RESET_BRANCH_POINT:
    action = new QAction("Reset branch point", parent);
    action->setStatusTip("Move a neighboring branch point to the selected node");
    break;
  case ACTION_SWC_Z_INTERPOLATION:
    action = new QAction("Z", parent);
    break;
  case ACTION_SWC_RADIUS_INTERPOLATION:
    action = new QAction("Radius", parent);
    break;
    //m_singleSwcNodeActionActivator.registerAction(action, false);
  case ACTION_SWC_POSITION_INTERPOLATION:
    action = new QAction("Position", parent);
    break;
    //m_singleSwcNodeActionActivator.registerAction(action, false);
  case ACTION_SWC_INTERPOLATION:
    action = new QAction("Position and Radius", parent);
    break;
  case ACTION_SWC_SUMMARIZE:
    action = new QAction("Summary", parent);
    break;
  case ACTION_SYNAPSE_VERIFY:
    action = new QAction("Verify", parent);
    action->setIcon(QIcon(":/images/verify.png"));
    break;
  case ACTION_SYNAPSE_UNVERIFY:
    action = new QAction("Unverfiy", parent);
    break;
  case ACTION_SYNAPSE_ADD_PRE:
    action = new QAction("Add TBar", parent);
    action->setIcon(QIcon(":/images/add.png"));
    action->setStatusTip("Add a TBar with mouse click");
    break;
  case ACTION_SYNAPSE_ADD_POST:
    action = new QAction("Add PSD", parent);
    action->setIcon(QIcon(":/images/add_post.png"));
    action->setStatusTip("Add a PSD with mouse click");
    break;
  case ACTION_SYNAPSE_MOVE:
    action = new QAction("Move Synapse", parent);
    action->setIcon(QIcon(":/images/move.png"));
//    action->setShortcut(Qt::Key_V);
    action->setStatusTip("Move a synapse with mouse click");
    break;
  case ACTION_SYNAPSE_DELETE:
    action = new QAction("Delete Synapse", parent);
//    action->setShortcut(Qt::Key_X);
    action->setIcon(QIcon(":/images/delete.png"));
    action->setStatusTip("Delete selected synapses");
    break;
  case ACTION_SYNAPSE_LINK:
    action = new QAction("Link Synapses", parent);
    action->setShortcut(QObject::tr("Ctrl+C"));
    action->setIcon(QIcon(":/images/connect.png"));
    action->setStatusTip("Link selected synapses");
    break;
  case ACTION_SYNAPSE_UNLINK:
    action = new QAction("Unlink Synapses", parent);
    action->setShortcut(QObject::tr("Ctrl+B"));
    action->setIcon(QIcon(":/images/cut.png"));
    action->setStatusTip("Unlink selected synapses");
    break;
  case ACTION_SYNAPSE_FILTER:
    action = new QAction("Filter Synapses", parent);
    action->setStatusTip("Filter synapses in the window");
    break;
  case ACTION_SYNAPSE_HLPSD:
    action = new QAction("Highlight Partner PSDs", parent);
    action->setIcon(QIcon(":/images/hl_post.png"));
    action->setCheckable(true);
    action->setChecked(true);
    break;
  case ACTION_SYNAPSE_REPAIR:
    action = new QAction("Repair Synapses", parent);
    action->setIcon(QIcon(":/images/repair.png"));
    action->setStatusTip("Repair selected synapses");
    break;
  case ACTION_TOGGLE_SWC_SKELETON:
    action = new QAction("Show Full Skeleton", parent);
    action->setCheckable(true);
    action->setChecked(true);
    break;
  case ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D:
    action = new QAction("Locate node(s) in 3D", parent);
    action->setStatusTip("Located selected swc nodes in 3D view.");
    break;
  case ACTION_CONNECT_TO_SWC_NODE:
    action = new QAction("Connect to", parent);
    action->setShortcut(Qt::Key_C);
    action->setStatusTip("Connect the currently selected node to another");
    action->setIcon(QIcon(":/images/connect_to.png"));
    break;
  case ACTION_EXTEND_SWC_NODE:
    action = new QAction("Extend", parent);
    action->setShortcut(Qt::Key_Space);
    action->setStatusTip(
          "Extend the currently selected node with mouse click.");
    action->setIcon(QIcon(":/images/extend.png"));
    break;
  case ACTION_MOVE_SWC_NODE:
    action = new QAction("Move Selected (Shift+Mouse)", parent);
    action->setShortcut(Qt::Key_V);
    action->setStatusTip("Move selected nodes with mouse.");
    action->setIcon(QIcon(":/images/move.png"));
    break;
  case ACTION_LOCK_SWC_NODE_FOCUS:
    action = new QAction("Lock Focus", parent);
    action->setIcon(QIcon(":/images/change_focus.png"));
    break;
  case ACTION_CHANGE_SWC_NODE_FOCUS:
    action = new QAction("Move to Current Plane", parent);
    action->setShortcut(Qt::Key_F);
    action->setStatusTip(
          "Move the centers of the selected nodes to the current plane.");
    action->setIcon(QIcon(":/images/change_focus.png"));
    break;
  case ACTION_ESTIMATE_SWC_NODE_RADIUS:
    action = new QAction("Estimate Radius", parent);
    break;
  case ACTION_PAINT_STROKE:
    action = new QAction("Paint Mask", parent);
//    action->setShortcut("Ctrl+R");
    break;
  case ACTION_ADD_SPLIT_SEED:
    action = new QAction("Paint Seed", parent);
//    action->setShortcut("Ctrl+R");
    break;
  case ACTION_ERASE_STROKE:
    action = new QAction("Erase Mask", parent);
//    action->setShortcut("Ctrl+E");
    break;
  case ACTION_SPLIT_DATA:
    action = new QAction("Split", parent);
    break;
  case ACTION_BODY_SPLIT_START:
    action = new QAction("Launch Split", parent);
    break;
  case ACTION_BODY_ANNOTATION:
    action = new QAction("Annotate", parent);
    break;
  case ACTION_BODY_EXPERT_STATUS:
    action = new QAction("Roughly Traced", parent);
    break;
  case ACTION_BODY_PROFILE:
    action = new QAction("Body Profile", parent);
    break;
  case ACTION_BODY_CONNECTION:
    action = new QAction("Show Connection", parent);
    break;
  case ACTION_BODY_CHECKIN:
    action = new QAction("Unlock", parent);
    break;
  case ACTION_BODY_FORCE_CHECKIN:
    action = new QAction("Unlock (Administrator)", parent);
    break;
  case ACTION_BODY_CHECKOUT:
    action = new QAction("Lock", parent);
    break;
  case ACTION_BODY_DECOMPOSE:
    action = new QAction("Decompose", parent);
    break;
  case ACTION_BODY_CROP:
    action = new QAction("Crop", parent);
    break;
  case ACTION_BODY_CHOP:
    action = new QAction("Slice", parent);
    action->setToolTip("Cut the body with plane(s) at the current position");
    break;
  case ACTION_BODY_MERGE:
    action = new QAction("Merge", parent);
    break;
  case ACTION_BODY_UNMERGE:
    action = new QAction("Unmerge", parent);
    break;
  case ACTION_DESELECT_BODY:
    action = new QAction("Deselect Body", parent);
    break;
  case ACTION_SAVE_OBJECT_AS:
    action = new QAction("Save as", parent);
    break;
  case ACTION_SHOW_ORTHO:
    action = new QAction("Show Orthogonal View", parent);
    break;
  case ACTION_SHOW_ORTHO_BIG:
    action = new QAction("Show Orthogonal View (1024)", parent);
    break;
  case ACTION_COPY_POSITION:
    action = new QAction("Copy Position", parent);
    break;
  case ACTION_COPY_BODY_ID:
    action = new QAction("Copy Body ID", parent);
    break;
  case ACTION_SHOW_SUPERVOXEL_LIST:
    action = new QAction("Show Supervoxel List", parent);
    break;
  case ACTION_COPY_SUPERVOXEL_ID:
    action = new QAction("Copy Supervoxel ID", parent);
    break;
  case ACTION_BOOKMARK_CHECK:
    action = new QAction("Set Checked", parent);
    break;
  case ACTION_BOOKMARK_UNCHECK:
    action = new QAction("Set Unchecked", parent);
    break;
  case ACTION_TRACE:
    action = new QAction("trace", parent);
    action->setStatusTip("Trace an individual branch");
    action->setToolTip("Trace an individual branch");
    break;
  case ACTION_ADD_TODO_ITEM:
    action = new QAction("Todo here", parent);
    break;
  case ACTION_ACTIVATE_TODO_ITEM:
    action = new QAction("Add Todo", parent);
    action->setIcon(QIcon(":/images/add_todo2.png"));
    action->setCheckable(true);
    break;
  case ACTION_ACTIVATE_TOSPLIT_ITEM:
    action = new QAction("Add To Split", parent);
    action->setIcon(QIcon(":/images/add_todo2.png"));
    action->setCheckable(true);
    break;
  case ACTION_ACTIVATE_LOCATE:
    action = new QAction("Locate", parent);
    action->setIcon(QIcon(":/images/locate.png"));
    action->setCheckable(true);
    break;
  case ACTION_ADD_TODO_ITEM_CHECKED:
    action = new QAction("Done here", parent);
    break;
  case ACTION_ADD_TODO_MERGE:
    action = new QAction("To merge here", parent);
    break;
  case ACTION_ADD_TODO_SPLIT:
    action = new QAction("To split here", parent);
    break;
  case ACTION_ADD_TODO_SVSPLIT:
    action = new QAction("To split supervoxel here", parent);
    break;
  case ACTION_REMOVE_TODO_ITEM:
    action = new QAction("Remove todo", parent);
    break;
  case ACTION_CHECK_TODO_ITEM:
    action = new QAction("Set checked", parent);
    break;
  case ACTION_UNCHECK_TODO_ITEM:
    action = new QAction("Set unchecked", parent);
    break;
  case ACTION_SHOW_NORMAL_TODO:
    action = new QAction("Show Normal Todo", parent);
    action->setCheckable(true);
    action->setChecked(true);
    break;
  case ACTION_TODO_ITEM_ANNOT_NORMAL:
    action = new QAction("Normal todo", parent);
    break;
  case ACTION_TODO_ITEM_ANNOT_IRRELEVANT:
    action = new QAction("Irrelevant todo", parent);
    break;
  case ACTION_TODO_ITEM_ANNOT_MERGE:
    action = new QAction("To merge", parent);
    break;
  case ACTION_TODO_ITEM_ANNOT_SPLIT:
    action = new QAction("To split", parent);
    break;
  case ACTION_SELECT_BODY_IN_RECT:
    action = new QAction("Select bodies", parent);
    break;
  case ACTION_ZOOM_TO_RECT:
    action = new QAction("Zoom in", parent);
    action->setIcon(QIcon(":/images/zoom2.png"));
    break;
  case ACTION_ENTER_RECT_ROI_MODE:
    action = new QAction("Draw rectangle ROI", parent);
    action->setToolTip("Use mouse to draw a rectangle ROI");
    action->setIcon(QIcon(":/images/roi_box.png"));
    break;
  case ACTION_CANCEL_RECT_ROI:
    action = new QAction("Cancel", parent);
    action->setToolTip("Cancel the current rectangle ROI");
    action->setIcon(QIcon(":/images/cancel.png"));
    break;
  case ACTION_PUNCTA_CHANGE_COLOR:
    action = new QAction("Change Color of Selected Puncta", parent);
    break;
  case ACTION_PUNCTA_HIDE_SELECTED:
    action = new QAction("Hide Selected Puncta", parent);
    break;
  case ACTION_PUNCTA_SHOW_SELECTED:
    action = new QAction("Show Selected Puncta", parent);
    break;
  case ACTION_REWRITE_SEGMENTATION:
    action = new QAction("Rewrite segmentation", parent);
    action->setToolTip("Rewrite segmentation in the current ROI. "
                       "Mainly used for fixing sync errors.");
    break;
  case ACTION_REFRESH_SEGMENTATION:
    action = new QAction("Refresh segmentation", parent);
    action->setToolTip("Refresh segmentation to get the latest data from DVID");
    break;
  case ACTION_FLYEM_UPDATE_BODY:
    action = new QAction("Update Bodies", parent);
    action->setToolTip("Update bodies from DVID");
    break;
  case ACTION_FLYEM_COMPARE_BODY:
    action = new QAction("Compare Body", parent);
    action->setToolTip("Compare the body with one from another version");
    break;
  case ACTION_SAVE_STACK:
    action = new QAction("Save Stack", parent);
    action->setIcon(QIcon(":/images/save.png"));
    action->setToolTip("Save the stack data into a file.");
    break;
  case ACTION_SHOW_SYNAPSE:
    action = new QAction("Synapses", parent);
    action->setIcon(QIcon(":/images/synapse2.png"));
    action->setCheckable(true);
    action->setChecked(true);
    break;
  case ACTION_SHOW_TODO:
    action = new QAction("To Do", parent);
    action->setIcon((QIcon(":/images/view_todo2.png")));
    action->setCheckable(true);
    action->setChecked(true);
    break;
  case ACTION_SAVE_SPLIT_TASK:
    action = new QAction("Save Split Task", parent);
    action->setIcon(QIcon(":/images/save_seed2.png"));
    action->setToolTip("Save the split task defined by current seeds.");
    break;
  case ACTION_DELETE_SPLIT_SEED:
    action = new QAction("Delete Seeds", parent);
    action->setIcon(QIcon(":/images/delete_seed.png"));
    action->setToolTip("Delete all seeds for splitting");
    break;
  case ACTION_DELETE_SELECTED_SPLIT_SEED:
    action = new QAction("Delete Seleted Seeds", parent);
    action->setIcon(QIcon(":/images/delete_selected_seed.png"));
    action->setToolTip("Delete selected seeds for splitting");
    break;
  case ACTION_VIEW_DATA_EXTERNALLY:
    action = new QAction("View data externally", parent);
    action->setIcon(QIcon(":/images/binoculars.png"));
    action->setToolTip("View grayscale/segmentation");
    action->setCheckable(true);
    break;
    break;
  case ACTION_MEASURE_SWC_NODE_DIST:
    action = new QAction("Measure Distance", parent);
    break;
  case ACTION_SHOW_SPLIT_MESH_ONLY:
    action = new QAction("Show Mesh to Split Only", parent);
    action->setCheckable(true);
    break;
  case ACTION_EXIT_SPLIT:
    action = new QAction("Exit Split", parent);
    break;
  case ACTION_START_SPLIT:
    action = new QAction("Start Split", parent);
    break;
  case ACTION_COMMIT_SPLIT:
    action = new QAction("Commit Split", parent);
    break;
  case ACTION_GO_TO_BODY:
    action = new QAction("Go to Body", parent);
    action->setShortcut(Qt::Key_F3);
    break;
  case ACTION_GO_TO_POSITION:
    action = new QAction("Go to Position", parent);
    action->setShortcuts(
          QList<QKeySequence>() << Qt::Key_F1 << Qt::SHIFT + Qt::Key_G);
    break;
  case ACTION_SELECT_BODY:
    action = new QAction("Select Body", parent);
    action->setShortcut(Qt::Key_F2);
    break;
  case ACTION_BODY_COLOR_NORMAL:
    action = CreateColorAction(
          ZFlyEmBodyColorOption::BODY_COLOR_NORMAL, parent);
    break;
  case ACTION_BODY_COLOR_NAME:
    action = CreateColorAction(
          ZFlyEmBodyColorOption::BODY_COLOR_NAME, parent);
    break;
  case ACTION_BODY_COLOR_SEQUENCER:
    action = CreateColorAction(
          ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER, parent);
    break;
  case ACTION_BODY_COLOR_PROTOCOL:
    action = CreateColorAction(
          ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL, parent);
    break;
  case ACTION_INFORMATION:
    action = new QAction("Information", parent);
    break;
  case ACTION_BODY_QUERY:
    action = new QAction("Query", parent);
    break;
  case ACTION_BODY_FIND_SIMILIAR:
    action = new QAction("Find Similar Neuorns", parent);
    break;
  case ACTION_BODY_EXPORT_SELECTED:
    action = new QAction("Export Selected Bodies", parent);
    break;
  case ACTION_BODY_EXPORT_SELECTED_LEVEL:
    action = new QAction("Export Selected Bodies (leveled)", parent);
    break;
  case ACTION_BODY_EXPORT_STACK:
    action = new QAction("Export Body Stack", parent);
    break;
  case ACTION_BODY_SKELETONIZE_TOP:
    action = new QAction("Skeletonize Top Bodies", parent);
    break;
  case ACTION_BODY_SKELETONIZE_LIST:
    action = new QAction("Skeletonize Body List", parent);
    break;
  case ACTION_BODY_UPDATE_MESH:
    action = new QAction("Update Meshes for Selected", parent);
    break;
  case ACTION_CLEAR_ALL_MERGE:
    action = new QAction("Clear All Merges", parent);
    break;
  default:
    break;
  }

  return action;
}

#if 0
QAction* ZActionFactory::makeAction(
    EAction item, const ZStackDoc *doc, QWidget *parent,
    ZActionActivator *activator, bool positive)
{
  QAction *action = NULL;
  switch (item) {
  case ACTION_SELECT_DOWNSTREAM:
    action = new QAction("Downstream", parent);
    action->setStatusTip("Select downstream nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectDownstreamNode()));
    break;
  case ACTION_SELECT_UPSTREAM:
    action = new QAction("Upstream", parent);
    action->setStatusTip("Select upstream nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectUpstreamNode()));
    break;
  case ACTION_SELECT_NEIGHBOR_SWC_NODE:
    action = new QAction("Neighbors", parent);
    action->setStatusTip(
          "Select neighbors (nodes coonected directly) of the currently selected nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectNeighborSwcNode()));
    break;
  case ACTION_SELECT_SWC_BRANCH:
    action = new QAction("Host branch", parent);
    action->setStatusTip("Select branches containing the currently selected nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectBranchNode()));
    break;
  case ACTION_SELECT_CONNECTED_SWC_NODE:
    action = new QAction("All connected nodes", parent);
    action->setStatusTip("Select all nodes connected (directly or indirectly) "
                         "of the currently selected nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectConnectedNode()));
    break;
  case ACTION_SELECT_ALL_SWC_NODE:
    action = new QAction("All nodes", parent);
    action->setShortcut(QKeySequence::SelectAll);
    action->setStatusTip("Selet all nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(selectAllSwcTreeNode()));
    break;
  case ACTION_RESOLVE_CROSSOVER:
    action = new QAction("Resolve crossover", parent);
    action->setStatusTip("Create a crossover near the selected node if it is detected");
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(executeResolveCrossoverCommand()));
    break;
  case ACTION_REMOVE_TURN:
    action = new QAction("Remove turn", parent);
    action->setStatusTip("Remove a nearby sharp turn");
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(executeRemoveTurnCommand()));
    break;
  case ACTION_MEASURE_SWC_NODE_LENGTH:
    action = new QAction("Path length", parent);
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(showSeletedSwcNodeLength()));
    break;
  case ACTION_MEASURE_SCALED_SWC_NODE_LENGTH:
    action = new QAction("Scaled Path length", parent);
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(showSeletedSwcNodeScaledLength()));
    break;
  case ACTION_MEASURE_SWC_NODE_DIST:
    action = new QAction("Measure Distance", parent);
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(showSeletedSwcNodeDist()));
    break;
  case ACTION_DELETE_SWC_NODE:
    action = new QAction("Delete", parent);
    action->setShortcut(Qt::Key_X);
    action->setStatusTip("Delete selected nodes");
    action->setIcon(QIcon(":/images/delete.png"));
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeDeleteSwcNodeCommand()));
    break;
  case ACTION_DELETE_UNSELECTED_SWC_NODE:
    action = new QAction("Delete Unselected", parent);
    action->setStatusTip("Delete unselected nodes");
//    action->setIcon(QIcon(":/images/delete.png"));
    doc->connect(action, SIGNAL(triggered()),
                 doc, SLOT(executeDeleteUnselectedSwcNodeCommand()));
    break;
  case ACTION_INSERT_SWC_NODE:
    action = new QAction("Insert", parent);
    action->setStatusTip("Insert a node between two adjacent nodes");
    action->setShortcut(Qt::Key_I);
    action->setIcon(QIcon(":/images/insert.png"));
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeInsertSwcNode()));
    break;
  case ACTION_BREAK_SWC_NODE:
    action = new QAction("Break", parent);
    action->setStatusTip("Remove connections among the selected nodes");
    action->setShortcut(Qt::Key_B);
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(executeBreakSwcConnectionCommand()));
    action->setIcon(QIcon(":/images/cut.png"));
    break;
  case ACTION_CONNECT_SWC_NODE:
    action = new QAction("Connect", parent);
    action->setStatusTip("Connect selected nodes");
    action->setShortcut(Qt::Key_C);
    action->setIcon(QIcon(":/images/connect.png"));
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeConnectSwcNodeCommand()));
    break;
  case ACTION_MERGE_SWC_NODE:
    action = new QAction("Merge", parent);
    action->setStatusTip("Merge selected nodes, which should form a single subtree");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeMergeSwcNodeCommand()));
    action->setIcon(QIcon(":/images/merge.png"));
    break;
  case ACTION_TRANSLATE_SWC_NODE:
    action = new QAction("Translate", parent);
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(executeTranslateSelectedSwcNode()));
    break;
  case ACTION_CHANGE_SWC_SIZE:
    action = new QAction("Change size", parent);
    doc->connect(action, SIGNAL(triggered()),
            doc, SLOT(executeChangeSelectedSwcNodeSize()));
    break;
  case ACTION_SET_SWC_ROOT:
    action = new QAction("Set as a root", parent);
    action->setStatusTip("Set the selected node as a root");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeSetRootCommand()));
    break;
  case ACTION_SET_BRANCH_POINT:
    action = new QAction("Join isolated branch", parent);
    action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeSetBranchPoint()));
    break;
  case ACTION_CONNECTED_ISOLATED_SWC:
    action = new QAction("Join isolated brach (across trees)", parent);
    action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes. "
                         "The branch can be in another neuron.");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeConnectIsolatedSwc()));
    break;
  case ACTION_RESET_BRANCH_POINT:
    action = new QAction("Reset branch point", parent);
    action->setStatusTip("Move a neighboring branch point to the selected node");
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeResetBranchPoint()));
    break;
  case ACTION_SWC_Z_INTERPOLATION:
    action = new QAction("Z", parent);
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeInterpolateSwcZCommand()));
    break;
  case ACTION_SWC_RADIUS_INTERPOLATION:
    action = new QAction("Radius", parent);
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeInterpolateSwcRadiusCommand()));
    break;
    //m_singleSwcNodeActionActivator.registerAction(action, false);
  case ACTION_SWC_POSITION_INTERPOLATION:
    action = new QAction("Position", parent);
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeInterpolateSwcPositionCommand()));
    break;
    //m_singleSwcNodeActionActivator.registerAction(action, false);
  case ACTION_SWC_INTERPOLATION:
    action = new QAction("Position and Radius", parent);
    doc->connect(action, SIGNAL(triggered()),
                 doc, SLOT(executeInterpolateSwcCommand()));
    break;
  case ACTION_SWC_SUMMARIZE:
    action = new QAction("Summary", parent);
    doc->connect(action, SIGNAL(triggered()),
                 doc, SLOT(showSwcSummary()));
    break;
  case ACTION_TRACE:
    action = new QAction("trace", parent);
    action->setStatusTip("Trace an individual branch");
    action->setToolTip("Trace an individual branch");
    break;
  case ACTION_FITSEG:
    action = new QAction("fit", parent);
    break;
  case ACTION_DROPSEG:
    action = new QAction("drop", parent);
    break;
  case ACTION_PUNCTA_MARK:
    action = new QAction("Mark Puncta", parent);
    break;
  case ACTION_PUNCTA_ENLARGE:
    action = new QAction("Enlarge Puncta", parent);
    break;
  case ACTION_PUNCTA_NARROW:
    action = new QAction("Narrow Puncta", parent);
    break;
  case ACTION_PUNCTA_MEANSHIFT:
    action = new QAction("Mean Shift Puncta", parent);
    break;
  case ACTION_PUNCTA_MEANSHIFT_ALL:
    action = new QAction("Mean Shift All Puncta", parent);
    break;
  case ACTION_PUNCTA_CHANGE_COLOR:
    action = new QAction("Change Color of Selected Puncta", parent);
    break;
  case ACTION_DELETE_SELECTED:
    action = new QAction("Delete Selected Object", parent);
    action->setIcon(QIcon(":/images/delete.png"));
    break;
  case ACTION_FIT_ELLIPSE:
    action = new QAction("fit ellipse", parent);
    break;
  default:
    break;
  }

  if (action != NULL && activator != NULL) {
    activator->registerAction(action, positive);
  }

  return action;
}

#endif

#if 0
QAction* ZActionFactory::makeAction(
    EAction item, const ZStackPresenter *presenter, QWidget *parent,
    ZActionActivator *activator, bool positive)
{
  QAction *action = MakeAction(item, parent);
  switch (item) {
  case ACTION_ADD_SWC_NODE:
    presenter->connect(action, SIGNAL(triggered()),
                       presenter, SLOT(trySwcAddNodeMode()));
    break;
  case ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D:
    QObject::connect(action, SIGNAL(triggered()),
                     presenter->getParentFrame(), SLOT(locateSwcNodeIn3DView()));
    action->setStatusTip("Located selected swc nodes in 3D view.");
    break;
  case ACTION_CONNECT_TO_SWC_NODE:
    presenter->connect(action, SIGNAL(triggered()),
            presenter, SLOT(enterSwcConnectMode()));
    break;
  case ACTION_CHANGE_SWC_NODE_FOCUS:
    presenter->connect(action, SIGNAL(triggered()),
                       presenter, SLOT(changeSelectedSwcNodeFocus()));
    break;
  case ACTION_ESTIMATE_SWC_NODE_RADIUS:
    presenter->connect(action, SIGNAL(triggered()),
                       presenter, SLOT(estimateSelectedSwcRadius()));
    break;
  default:
    break;
  }

  if (action != NULL && activator != NULL) {
    activator->registerAction(action, positive);
  }

  return action;
}
#endif


