#include "zactionfactory.h"
#include <QWidget>
#include <QObject>
#include "zstackdoc.h"
#include "zactionactivator.h"

ZActionFactory::ZActionFactory()
{
}

QAction* ZActionFactory::makeAction(
    EActionItem item, const ZStackDoc *doc, QWidget *parent,
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
    action = new QAction("Set as root", parent);
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
    doc->connect(action, SIGNAL(triggered()), doc, SLOT(executeInterpolateSwcCommand()));
    break;
  default:
    break;
  }

  if (action != NULL && activator != NULL) {
    activator->registerAction(action, positive);
  }

  return action;
}
