#include "zactionfactory.h"
#include <QWidget>
#include <QObject>
#include <QUndoStack>
#include <QString>

#include "zstackdoc.h"
#include "zactionactivator.h"
#include "zstackpresenter.h"
#include "zstackframe.h"

ZActionFactory::ZActionFactory()
{
}

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

QAction* ZActionFactory::makeAction(
    EAction item, const ZStackPresenter *presenter, QWidget *parent,
    ZActionActivator *activator, bool positive)
{
  QAction *action = NULL;
  switch (item) {
  case ACTION_ADD_SWC_NODE:
    action = new QAction("Add Neuron Node", parent);
    action->setStatusTip("Add an isolated neuron node.");
    action->setIcon(QIcon(":/images/add.png"));
    action->setShortcut(Qt::Key_G);
    presenter->connect(action, SIGNAL(triggered()),
                       presenter, SLOT(trySwcAddNodeMode()));
    break;
  case ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D:
    action = new QAction("Locate node(s) in 3D", parent);
    presenter->getParentFrame()->connect(action, SIGNAL(triggered()),
                       presenter->getParentFrame(), SLOT(locateSwcNodeIn3DView()));
    action->setStatusTip("Located selected swc nodes in 3D view.");
    break;
  case ACTION_CONNECT_TO_SWC_NODE:
    action = new QAction("Connect to", parent);
    action->setShortcut(Qt::Key_C);
    action->setStatusTip(
          "Connect the currently selected node to another");
    presenter->connect(action, SIGNAL(triggered()),
            presenter, SLOT(enterSwcConnectMode()));
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
  case ACTION_CHANGE_SWC_NODE_FOCUS:
    action = new QAction("Move to Current Plane", parent);
    action->setShortcut(Qt::Key_F);
    action->setStatusTip(
          "Move the centers of the selected nodes to the current plane.");
    action->setIcon(QIcon(":/images/change_focus.png"));
    presenter->connect(action, SIGNAL(triggered()),
                       presenter, SLOT(changeSelectedSwcNodeFocus()));
    break;
  case ACTION_ESTIMATE_SWC_NODE_RADIUS:
    action = new QAction("Estimate Radius", parent);
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

QAction* ZActionFactory::makeAction(EAction actionKey, QObject *parent) const
{
  return MakeAction(actionKey, parent);
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
    action->setShortcut(Qt::Key_V);
    action->setStatusTip("Move a synapse with mouse click");
    break;
  case ACTION_SYNAPSE_DELETE:
    action = new QAction("Delete Synapse", parent);
    action->setShortcut(Qt::Key_X);
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
  case ACTION_BODY_SPLIT_START:
    action = new QAction("Launch split", parent);
    break;
  case ACTION_BODY_ANNOTATION:
    action = new QAction("Annotate", parent);
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
  case ACTION_BODY_MERGE:
    action = new QAction("Merge", parent);
    break;
  case ACTION_SHOW_ORTHO:
    action = new QAction("Show orthogonal view", parent);
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
  default:
    break;
  }

  return action;
}
