#include "z3dgl.h"
#include <iostream>
#include <QtGui>
#ifdef _QT5_
#include <QtWidgets>
#endif
#include <QTextStream>
#include <QtDebug>
#include <iterator>
#include <QSet>
#include <vector>
#include <QTimer>
#include <QInputDialog>
#include <QApplication>
#include <QtConcurrentRun>

#include "QsLog.h"

#include "informationdialog.h"
#include "tz_image_io.h"
#include "tz_math.h"
#include "zstackdoc.h"
#include "tz_stack_lib.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "zlocalneuroseg.h"
#include "zlocsegchain.h"
#include "tz_vrml_io.h"
#include "tz_vrml_material.h"
#include "tz_color.h"
#include "zstackframe.h"
#include "zellipse.h"
#include "tz_workspace.h"
#include "tz_string.h"
#include "zlocsegchainconn.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_objdetect.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_sampling.h"
#include "tz_stack_utils.h"
#include "tz_darray.h"
#include "tz_stack_lib.h"
#include "tz_stack_math.h"
#include "tz_local_rpi_neuroseg.h"
#include "zlocalrect.h"
#include "tz_geo3d_ball.h"
#include "tz_workspace.h"
#include "zdirectionaltemplatechain.h"
#include "tz_r2_ellipse.h"
#include "zstack.hxx"
#include "tz_stack_stat.h"
#include "mainwindow.h"
#include "tz_error.h"
#include "zswcnetwork.h"
#include "zstring.h"
#include "zcolormap.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zneuronnetwork.h"
#include "zfiletype.h"
#include "zstackfile.h"
#include "zstackprocessor.h"
#include "zswcobjsmodel.h"
#include "zdocplayerobjsmodel.h"
#include "zswcnodeobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "swctreenode.h"
#include "zstackgraph.h"
#include "zgraphcompressor.h"
#include "zgraph.h"
#include "neutubeconfig.h"
#include "tz_stack_bwmorph.h"
#include "zstackdoccommand.h"
#include "zstroke2d.h"
#include "zqtmessagereporter.h"
#include "zswcconnector.h"
#include "biocytin/biocytin.h"
#include "zpunctumio.h"
#include "biocytin/zbiocytinfilenameparser.h"
#include "swcskeletontransformdialog.h"
#include "swcsizedialog.h"
#include "tz_stack_watershed.h"
#include "zstackwatershed.h"
#include "zstackarray.h"
#include "zstackfactory.h"
#include "zsparseobject.h"
#include "zsparsestack.h"

using namespace std;

ZStackDoc::ZStackDoc(ZStack *stack, QObject *parent) : QObject(parent)
{
  m_stack = stack;
  m_sparseStack = NULL;
  m_labelField = NULL;
  m_parentFrame = NULL;
  //m_masterChain = NULL;
  m_isTraceMaskObsolete = true;
  m_swcNetwork = NULL;
  m_stackFactory = NULL;

  initNeuronTracer();
  m_swcObjsModel = new ZSwcObjsModel(this, this);
  m_swcNodeObjsModel = new ZSwcNodeObjsModel(this, this);
  m_punctaObjsModel = new ZPunctaObjsModel(this, this);
  m_seedObjsModel = new ZDocPlayerObjsModel(this, ZDocPlayer::ROLE_SEED, this);
  m_undoStack = new QUndoStack(this);

  connectSignalSlot();

  setReporter(new ZQtMessageReporter());

  if (NeutubeConfig::getInstance().isAutoSaveEnabled()) {
    QTimer *timer = new QTimer(this);
    timer->start(NeutubeConfig::getInstance().getAutoSaveInterval());
    connect(timer, SIGNAL(timeout()), this, SLOT(autoSave()));
  }

  createActions();

  setTag(NeuTube::Document::NORMAL);
  setStackBackground(NeuTube::IMAGE_BACKGROUND_DARK);
}

ZStackDoc::~ZStackDoc()
{
  deprecate(STACK);
  deprecate(SPARSE_STACK);

  qDebug() << "ZStackDoc destroyed";

  foreach (ZStackObject *obj, m_objectList) {
    delete obj;
  }
  m_objectList.clear();

  if (m_swcNetwork != NULL) {
    delete m_swcNetwork;
  }

  delete m_undoStack;
  delete m_labelField;
  delete m_stackFactory;
  //delete m_swcNodeContextMenu;

  destroyReporter();
}

void ZStackDoc::initNeuronTracer()
{
  m_neuronTracer.initTraceWorkspace(getStack());
  m_neuronTracer.initConnectionTestWorkspace();
  if (getStack() != NULL) {
    m_neuronTracer.setIntensityField(getStack()->c_stack());
  }
  m_neuronTracer.setBackgroundType(getStackBackground());
  if (getTag() == NeuTube::Document::FLYEM_BODY) {
    m_neuronTracer.setVertexOption(ZStackGraph::VO_SURFACE);
  }

  if (GET_APPLICATION_NAME == "Biocytin") {
    m_neuronTracer.setResolution(1, 1, 10);
  }

  ZIntPoint offset = getStackOffset();
  m_neuronTracer.setStackOffset(offset.getX(), offset.getY(), offset.getZ());
}

void ZStackDoc::setParentFrame(ZStackFrame *parent)
{
  m_parentFrame = parent;
}

ZStack* ZStackDoc::getStack() const
{
  return m_stack;
}

ZStack* ZStackDoc::stackMask() const
{
  return NULL;
}

void ZStackDoc::setStackBackground(NeuTube::EImageBackground bg)
{
    m_stackBackground = bg;
    m_neuronTracer.setBackgroundType(bg);
}

void ZStackDoc::emptySlot()
{
  QMessageBox::information(NULL, "empty slot", "To be implemented");
}

void ZStackDoc::connectSignalSlot()
{
  connect(this, SIGNAL(swcModified()), m_swcObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(swcModified()), m_swcNodeObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(punctaModified()), m_punctaObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(seedModified()), m_seedObjsModel, SLOT(updateModelData()));

  connect(this, SIGNAL(chainModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(punctaModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(obj3dModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(sparseObjectModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(strokeModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(swcModified()), this, SIGNAL(objectModified()));

  connect(m_undoStack, SIGNAL(cleanChanged(bool)),
          this, SIGNAL(cleanChanged(bool)));
  connect(&m_reader, SIGNAL(finished()), this, SIGNAL(stackReadDone()));
  connect(this, SIGNAL(stackReadDone()), this, SLOT(loadReaderResult()));
  connect(this, SIGNAL(stackModified()), this, SIGNAL(volumeModified()));
}

void ZStackDoc::createActions()
{
  m_undoAction = m_undoStack->createUndoAction(this, tr("&Undo"));
  m_undoAction->setIcon(QIcon(":/images/undo.png"));
  m_undoAction->setShortcuts(QKeySequence::Undo);

  m_redoAction = m_undoStack->createRedoAction(this, tr("&Redo"));
  m_redoAction->setIcon(QIcon(":/images/redo.png"));
  m_redoAction->setShortcuts(QKeySequence::Redo);

  QAction *action = new QAction("Downstream", this);
  action->setStatusTip("Select downstream nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectDownstreamNode()));
  m_actionMap[ACTION_SELECT_DOWNSTREAM] = action;

  action = new QAction("Upstream", this);
  action->setStatusTip("Select upstream nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectUpstreamNode()));
  m_actionMap[ACTION_SELECT_UPSTREAM] = action;

  action = new QAction("Neighbors", this);
  action->setStatusTip(
        "Select neighbors (nodes coonected directly) of the currently selected nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectNeighborSwcNode()));
  m_actionMap[ACTION_SELECT_NEIGHBOR_SWC_NODE] = action;

  action = new QAction("Host branch", this);
  action->setStatusTip("Select branches containing the currently selected nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectBranchNode()));
  m_actionMap[ACTION_SELECT_SWC_BRANCH] = action;

  action = new QAction("All connected nodes", this);
  action->setStatusTip("Select all nodes connected (directly or indirectly) "
                       "of the currently selected nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectConnectedNode()));
  m_actionMap[ACTION_SELECT_CONNECTED_SWC_NODE] = action;

  action = new QAction("All nodes", this);
  action->setShortcut(QKeySequence::SelectAll);
  action->setStatusTip("Selet all nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(selectAllSwcTreeNode()));
  m_actionMap[ACTION_SELECT_ALL_SWC_NODE] = action;

  action = new QAction("Resolve crossover", this);
  action->setStatusTip("Create a crossover near the selected node if it is detected");
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeResolveCrossoverCommand()));
  m_actionMap[ACTION_RESOLVE_CROSSOVER] = action;

  action = new QAction("Remove turn", this);
  action->setStatusTip("Remove a nearby sharp turn");
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeRemoveTurnCommand()));
  m_actionMap[ACTION_REMOVE_TURN] = action;

  action = new QAction("Path length", this);
  connect(action, SIGNAL(triggered()), this, SLOT(showSeletedSwcNodeLength()));
  m_actionMap[ACTION_MEASURE_SWC_NODE_LENGTH] = action;

  action = new QAction("Scaled Path length", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(showSeletedSwcNodeScaledLength()));
  m_actionMap[ACTION_MEASURE_SCALED_SWC_NODE_LENGTH] = action;

  action = new QAction("Delete", this);
  action->setShortcut(Qt::Key_X);
  action->setStatusTip("Delete selected nodes");
  action->setIcon(QIcon(":/images/delete.png"));
  connect(action, SIGNAL(triggered()), this, SLOT(executeDeleteSwcNodeCommand()));
  m_actionMap[ACTION_DELETE_SWC_NODE] = action;

  action = new QAction("Insert", this);
  action->setStatusTip("Insert a node between two adjacent nodes");
  action->setShortcut(Qt::Key_I);
  action->setIcon(QIcon(":/images/insert.png"));
  connect(action, SIGNAL(triggered()), this, SLOT(executeInsertSwcNode()));
  m_actionMap[ACTION_INSERT_SWC_NODE] = action;

  action = new QAction("Break", this);
  action->setStatusTip("Delect connections among the selected nodes");
  action->setShortcut(Qt::Key_B);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeBreakSwcConnectionCommand()));
  action->setIcon(QIcon(":/images/cut.png"));
  m_actionMap[ACTION_BREAK_SWC_NODE] = action;

  action = new QAction("Connect", this);
  action->setStatusTip("Connect selected nodes");
  action->setShortcut(Qt::Key_C);
  action->setIcon(QIcon(":/images/connect.png"));
  connect(action, SIGNAL(triggered()), this, SLOT(executeConnectSwcNodeCommand()));
  m_actionMap[ACTION_CONNECT_SWC_NODE] = action;

  action = new QAction("Merge", this);
  action->setStatusTip("Merge selected nodes, which for a single tree");
  connect(action, SIGNAL(triggered()), this, SLOT(executeMergeSwcNodeCommand()));
  action->setIcon(QIcon(":/images/merge.png"));
  m_actionMap[ACTION_MERGE_SWC_NODE] = action;

  action = new QAction("Translate", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeTranslateSelectedSwcNode()));
  m_actionMap[ACTION_TRANSLATE_SWC_NODE] = action;

  action = new QAction("Change size", this);
  connect(action, SIGNAL(triggered()),
          this, SLOT(executeChangeSelectedSwcNodeSize()));
  m_actionMap[ACTION_CHANGE_SWC_SIZE] = action;

  action = new QAction("Set as root", this);
  action->setStatusTip("Set the selected node as a root");
  connect(action, SIGNAL(triggered()), this, SLOT(executeSetRootCommand()));
  m_actionMap[ACTION_SET_SWC_ROOT] = action;

  action = new QAction("Join isolated branch", this);
  action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes");
  connect(action, SIGNAL(triggered()), this, SLOT(executeSetBranchPoint()));
  m_actionMap[ACTION_SET_BRANCH_POINT] = action;

  action = new QAction("Join isolated brach (across trees)", this);
  action->setStatusTip("Connect to the nearest branch that does not have a path to the selected nodes. "
                       "The branch can be in another neuron.");
  connect(action, SIGNAL(triggered()), this, SLOT(executeConnectIsolatedSwc()));
  m_actionMap[ACTION_CONNECTED_ISOLATED_SWC] = action;

  action = new QAction("Reset branch point", this);
  action->setStatusTip("Move a neighboring branch point to the selected node");
  connect(action, SIGNAL(triggered()), this, SLOT(executeResetBranchPoint()));
  m_actionMap[ACTION_RESET_BRANCH_POINT] = action;

  action = new QAction("Z", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInterpolateSwcZCommand()));
  m_actionMap[ACTION_SWC_Z_INTERPOLATION] = action;

  action = new QAction("Radius", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInterpolateSwcRadiusCommand()));
  m_actionMap[ACTION_SWC_RADIUS_INTERPOLATION] = action;
  //m_singleSwcNodeActionActivator.registerAction(action, false);

  action = new QAction("Position", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInterpolateSwcPositionCommand()));
  m_actionMap[ACTION_SWC_POSITION_INTERPOLATION] = action;
  //m_singleSwcNodeActionActivator.registerAction(action, false);

  action = new QAction("Position and Radius", this);
  connect(action, SIGNAL(triggered()), this, SLOT(executeInterpolateSwcCommand()));
  m_actionMap[ACTION_SWC_INTERPOLATION] = action;
  //m_singleSwcNodeActionActivator.registerAction(action, false);

  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_MEASURE_SWC_NODE_LENGTH], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_MEASURE_SCALED_SWC_NODE_LENGTH], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_BREAK_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_CONNECT_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_MERGE_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_SET_SWC_ROOT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_INSERT_SWC_NODE], false);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_SET_BRANCH_POINT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_RESET_BRANCH_POINT], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ACTION_CONNECTED_ISOLATED_SWC], true);
}

void ZStackDoc::updateSwcNodeAction()
{
  m_singleSwcNodeActionActivator.update(this);
}

void ZStackDoc::autoSave()
{
  if (isSwcSavingRequired()) {
    qDebug() << "Auto save triggered in " << this;
    if (!swcList()->empty()) {
      std::string autoSaveDir = NeutubeConfig::getInstance().getPath(
            NeutubeConfig::AUTO_SAVE);
      QDir dir(autoSaveDir.c_str());
      if (dir.exists()) {
        ostringstream stream;
        stream << this;
        std::string autoSavePath =
            autoSaveDir + ZString::FileSeparator;
        if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
          autoSavePath +=
              ZBiocytinFileNameParser::getCoreName(getStack()->sourcePath()) +
              ".autosave.swc";
        } else {
          autoSavePath += "~" + stream.str() + ".swc";
        }

        FILE *fp = fopen(autoSavePath.c_str(), "w");
        if (fp != NULL) {
          fclose(fp);
          ZSwcTree *tree = new ZSwcTree;
          foreach (ZSwcTree *treeItem, m_swcList) {
            tree->merge(Copy_Swc_Tree(treeItem->data()), true);
          }
          tree->resortId();
          tree->save(autoSavePath.c_str());
          qDebug() << autoSavePath.c_str();

          delete tree;
        }
      }
    }
  }
}

string ZStackDoc::getSwcSource() const
{
  string swcSource;
  if (m_swcList.size() == 1) {
    swcSource = m_swcList.first()->source();
  }

  return swcSource;
}

ZSwcTree* ZStackDoc::getMergedSwc()
{
  ZSwcTree *tree = NULL;

  if (!swcList()->empty()) {
    tree = new ZSwcTree;
    foreach (ZSwcTree *treeItem, m_swcList) {
      tree->merge(Copy_Swc_Tree(treeItem->data()), true);
    }
    tree->resortId();
  }

  return tree;
}

bool ZStackDoc::saveSwc(const string &filePath)
{
  if (!swcList()->empty()) {
    ZSwcTree *tree = NULL;
    if (swcList()->size() > 1) {
      tree = new ZSwcTree;
      foreach (ZSwcTree *treeItem, m_swcList) {
        tree->merge(Copy_Swc_Tree(treeItem->data()), true);
      }
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      foreach (ZSwcTree* oldTree, m_swcList) {
        new ZStackDocCommand::SwcEdit::RemoveSwc(this, oldTree, command);
      }
      new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

      pushUndoCommand(command);
    } else {
      tree = swcList()->front();
    }
    tree->resortId();
    tree->save(filePath.c_str());
    tree->setSource(filePath);
    qDebug() << filePath.c_str();

    return true;
  }

  return false;
}

void ZStackDoc::updateTraceWorkspace(int traceEffort, bool traceMasked,
                                     double xRes, double yRes, double zRes)
{
  m_neuronTracer.updateTraceWorkspace(traceEffort, traceMasked,
                                      xRes, yRes, zRes);
}

void ZStackDoc::updateConnectionTestWorkspace(
    double xRes, double yRes, double zRes,
    char unit, double distThre, bool spTest, bool crossoverTest)
{
  m_neuronTracer.updateConnectionTestWorkspace(
        xRes, yRes, zRes, unit, distThre, spTest, crossoverTest);
}

bool ZStackDoc::isEmpty()
{
  return (!hasStackData()) && (!hasObject());
}

bool ZStackDoc::hasObject()
{
  return !m_objectList.isEmpty();
}

bool ZStackDoc::hasSparseObject()
{
  return !getSparseObjectList().isEmpty();
}

bool ZStackDoc::hasSparseStack() const
{
  return m_sparseStack != NULL;
}

bool ZStackDoc::hasSwc() const
{
  return !m_swcList.isEmpty();
}

bool ZStackDoc::hasSwcList()
{
  return !m_swcList.isEmpty();
}

ZResolution ZStackDoc::stackResolution() const
{
  if (hasStackData())
    return m_stack->resolution();
  else
    return ZResolution();
}

std::string ZStackDoc::stackSourcePath() const
{
  if (hasStackData()) {
    return m_stack->sourcePath();
  }

  return "";
}

bool ZStackDoc::hasChainList()
{
  return !m_chainList.isEmpty();
}

bool ZStackDoc::isUndoClean()
{
  return m_undoStack->isClean();
}

bool ZStackDoc::isSwcSavingRequired()
{
  qDebug() << m_swcList.empty();
  qDebug() << isUndoClean();

  return !m_swcList.empty() && !isUndoClean();
}

void ZStackDoc::swcTreeTranslateRootTo(double x, double y, double z)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->translateRootTo(x, y, z);
  }
  if (!m_swcList.empty()) {
    emit swcModified();
  }
}

void ZStackDoc::swcTreeRescale(double scaleX, double scaleY, double scaleZ)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescale(scaleX, scaleY, scaleZ);
  }
}

void ZStackDoc::swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
                               double dstPixelPerUmXY, double dstPixelPerUmZ)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescale(srcPixelPerUmXY, srcPixelPerUmZ,
                          dstPixelPerUmXY, dstPixelPerUmZ);
  }
}

void ZStackDoc::swcTreeRescaleRadius(double scale, int startdepth, int enddepth)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->rescaleRadius(scale, startdepth, enddepth);
  }
}

void ZStackDoc::swcTreeReduceNodeNumber(double lengthThre)
{
  for (int i = 0; i < m_swcList.size(); i++) {
    m_swcList[i]->reduceNodeNumber(lengthThre);
  }
}

void ZStackDoc::deleteSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      SwcTreeNode::detachParent(*iter);
      Swc_Tree_Node *child = (*iter)->first_child;
      while (child != NULL) {
        Swc_Tree_Node *nextChild = child->next_sibling;
        Swc_Tree_Node_Detach_Parent(child);
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(child);
        addSwcTree(tree, false);
        child = nextChild;
      }
#ifdef _DEBUG_
      std::cout << "Node deleted: " << SwcTreeNode::toString(*iter) << std::endl;
#endif
      Kill_Swc_Tree_Node(*iter);
    }
    nodeSet->clear();

    removeEmptySwcTree();

    notifySwcModified();
  }
}

void ZStackDoc::addSizeForSelectedSwcNode(double dr)
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    static const double minRadius = 0.5;
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      double newRadius = SwcTreeNode::radius(*iter) + dr;
      if (newRadius < minRadius) {
        newRadius = minRadius;
      }
      SwcTreeNode::setRadius(*iter, newRadius);
    }

    emit swcModified();
  }
}

void ZStackDoc::selectSwcNodeFloodFilling(Swc_Tree_Node *lastSelectedNode)
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> newSelectedSet;

  QQueue<Swc_Tree_Node*> tnQueue;
  tnQueue.enqueue(lastSelectedNode);

  while (!tnQueue.isEmpty()) {
    Swc_Tree_Node *tn = tnQueue.dequeue();
    std::vector<Swc_Tree_Node*> neighborArray =
        SwcTreeNode::neighborArray(tn);
    for (std::vector<Swc_Tree_Node*>::iterator
         iter = neighborArray.begin(); iter != neighborArray.end();
         ++iter) {
      if (nodeSet->count(*iter) == 0 &&
          newSelectedSet.count(*iter) == 0) {
        newSelectedSet.insert(*iter);
        tnQueue.enqueue(*iter);
      }
    }
  }

  setSwcTreeNodeSelected(newSelectedSet.begin(), newSelectedSet.end(), true);
}

void ZStackDoc::selectSwcNodeConnection(Swc_Tree_Node *lastSelectedNode)
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> newSelectedSet;
  std::vector<bool> labeled(nodeSet->size(), false);

  if (lastSelectedNode != NULL) {
    for (std::set<Swc_Tree_Node*>::iterator targetIter = nodeSet->begin();
         targetIter != nodeSet->end(); ++targetIter) {
      Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(lastSelectedNode,
                                                            *targetIter);
      if (SwcTreeNode::isRegular(ancestor)) {
        if (lastSelectedNode == ancestor) {
          std::vector<Swc_Tree_Node*> tnArray;
          Swc_Tree_Node *tn = *targetIter;
          while (tn != lastSelectedNode) {
            tnArray.push_back(tn);
            tn = SwcTreeNode::parent(tn);
          }
          for (std::vector<Swc_Tree_Node*>::reverse_iterator iter = tnArray.rbegin();
               iter != tnArray.rend(); ++iter) {
            if (nodeSet->count(*iter) == 0) {
              newSelectedSet.insert(*iter);
            } else {
              break;
            }
          }
        } else {
          ZSwcPath path(lastSelectedNode, *targetIter);
          ZSwcPath::iterator iter = path.begin();
          ++iter;
          for (; iter != path.end(); ++iter) {
            if (nodeSet->count(*iter) == 0) {
              newSelectedSet.insert(*iter);
            } else {
              break;
            }
          }
        }
      }
    }
  } else {
    int sourceIndex = 0;
    for (std::set<Swc_Tree_Node*>::iterator sourceIter = nodeSet->begin();
         sourceIter != nodeSet->end(); ++sourceIter, ++sourceIndex) {
      if (!labeled[sourceIndex]) {
        //Swc_Tree_Node *ancestor = *sourceIter;

        //Swc_Tree_Node *ancestor = *(nodeSet->begin());

        int index = sourceIndex + 1;
        std::set<Swc_Tree_Node*>::iterator targetIter = sourceIter;
        ++targetIter;
        for (; targetIter != nodeSet->end(); ++targetIter, ++index) {
          Swc_Tree_Node *ancestor =
              SwcTreeNode::commonAncestor(*sourceIter, *targetIter);
          if (SwcTreeNode::isRegular(ancestor)) {
            labeled[index] = true;

            Swc_Tree_Node *tn = *sourceIter;
            while (SwcTreeNode::isRegular(tn)) {
              newSelectedSet.insert(tn);
              if (tn == ancestor) {
                break;
              }
              tn = SwcTreeNode::parent(tn);
            }

            tn = *targetIter;
            while (SwcTreeNode::isRegular(tn)) {
              newSelectedSet.insert(tn);
              if (tn == ancestor) {
                break;
              }
              tn = SwcTreeNode::parent(tn);
            }
          }
        }
      }
    }
  }

  setSwcTreeNodeSelected(newSelectedSet.begin(), newSelectedSet.end(), true);
}

void ZStackDoc::selectUpstreamNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> upstreamNodes;

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (tn != NULL && !Swc_Tree_Node_Is_Root(tn) && Swc_Tree_Node_Type(tn) != 1) {
      upstreamNodes.insert(tn);
      tn = tn->parent;
    }
  }
  setSwcTreeNodeSelected(upstreamNodes.begin(), upstreamNodes.end(), true);
}

void ZStackDoc::selectBranchNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> branchNodes;

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (SwcTreeNode::isRegular(tn) && !Swc_Tree_Node_Is_Branch_Point_S(tn)) {
      branchNodes.insert(tn);
      tn = tn->parent;
    }
    tn = *iter;
    while (SwcTreeNode::isRegular(tn) && !Swc_Tree_Node_Is_Branch_Point_S(tn)) {
      branchNodes.insert(tn);
      tn = tn->first_child;
    }
  }
  setSwcTreeNodeSelected(branchNodes.begin(), branchNodes.end(), true);
}

void ZStackDoc::selectTreeNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<ZSwcTree*> trees;
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    trees.insert(nodeToSwcTree(*iter));
  }
  std::vector<Swc_Tree_Node*> treeNodes;
  for (std::set<ZSwcTree*>::iterator iter = trees.begin(); iter != trees.end(); ++iter) {
    ZSwcTree* tree = *iter;
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end(); tn = tn->next)
      treeNodes.push_back(tn);
  }
  setSwcTreeNodeSelected(treeNodes.begin(), treeNodes.end(), true);
}

void ZStackDoc::selectConnectedNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  std::set<Swc_Tree_Node*> regularRoots;
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node* tn = *iter;
    while (tn != NULL && !Swc_Tree_Node_Is_Regular_Root(tn))
      tn = tn->parent;
    if (tn)
      regularRoots.insert(tn);
  }
  std::vector<Swc_Tree_Node*> treeNodes;
  for (std::set<Swc_Tree_Node*>::iterator iter = regularRoots.begin(); iter != regularRoots.end(); ++iter) {
    ZSwcTree* tree = nodeToSwcTree(*iter);
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, *iter, FALSE);
    for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end(); tn = tn->next)
      treeNodes.push_back(tn);
  }
  setSwcTreeNodeSelected(treeNodes.begin(), treeNodes.end(), true);
}

void ZStackDoc::selectNeighborSwcNode()
{
  QList<Swc_Tree_Node*> selected;
  //QList<Swc_Tree_Node*> deselected;
  for (std::set<Swc_Tree_Node*>::const_iterator
       iter = selectedSwcTreeNodes()->begin();
       iter != selectedSwcTreeNodes()->end(); ++iter) {
    const Swc_Tree_Node *tn = *iter;
    std::vector<Swc_Tree_Node*> neighborArray = SwcTreeNode::neighborArray(tn);
    for (std::vector<Swc_Tree_Node*>::iterator nbrIter = neighborArray.begin();
         nbrIter != neighborArray.end(); ++nbrIter) {
      selected.append(*nbrIter);
    }
  }

#ifdef _DEBUG_
  qDebug() << selected.size() << "Neighbor selected";
#endif

  setSwcTreeNodeSelected(selected.begin(), selected.end(), true);
  //emit swcTreeNodeSelectionChanged(selected, deselected);
}

void ZStackDoc::hideSelectedPuncta()
{
  for (std::set<ZPunctum*>::iterator it = selectedPuncta()->begin();
       it != selectedPuncta()->end(); ++it) {
    setPunctumVisible(*it, false);
  }
}

void ZStackDoc::showSelectedPuncta()
{
  for (std::set<ZPunctum*>::iterator it = selectedPuncta()->begin();
       it != selectedPuncta()->end(); ++it) {
    setPunctumVisible(*it, true);
  }
}

void ZStackDoc::selectSwcNodeNeighbor()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    setSwcTreeNodeSelected(SwcTreeNode::parent(tn), true);
    tn = tn->first_child;
    while (tn != NULL) {
      setSwcTreeNodeSelected(tn, true);
      tn = SwcTreeNode::nextSibling(tn);
    }
  }
}

void ZStackDoc::updateVirtualStackSize()
{
  if (!hasStackData()) {
    Stack *stack = new Stack;
    double corner[6] = {.0, .0, .0, .0, .0, .0};
    for (int i = 0; i < m_swcList.size(); i++) {
      double tmpcorner[6];
      m_swcList[i]->boundBox(tmpcorner);
      corner[3] = std::max(corner[3], tmpcorner[3]);
      corner[4] = std::max(corner[4], tmpcorner[4]);
      corner[5] = std::max(corner[5], tmpcorner[5]);
    }
    static const double Lateral_Margin = 10.0;
    static const double Axial_Margin = 1.0;
    Stack_Set_Attribute(stack, round(corner[3] + Lateral_Margin),
        round(corner[4] + Lateral_Margin),
        round(corner[5] + Axial_Margin),
        GREY);
    stack->array = NULL;
    loadStack(stack, true);
  }
}

bool ZStackDoc::hasDrawable()
{
  return !m_objectList.isEmpty();
}

int ZStackDoc::stackWidth() const
{
  if (getStack() == NULL) {
    return 0;
  }

  return getStack()->width();
}

int ZStackDoc::stackHeight() const
{
  if (getStack() == NULL) {
    return 0;
  }

  return getStack()->height();
}

int ZStackDoc::stackChannelNumber() const
{
  if (hasStackData())
    return m_stack->channelNumber();
  else
    return 0;
}

ZStack*& ZStackDoc::stackRef()
{
  return m_stack;
}

const ZStack* ZStackDoc::stackRef() const
{
  return m_stack;
}

void ZStackDoc::loadStack(Stack *stack, bool isOwner)
{
  if (stack == NULL)
    return;

  deprecate(STACK);
  ZStack* &mainStack = stackRef();
  mainStack = new ZStack;

  if (mainStack != NULL) {
    mainStack->load(stack, isOwner);
    initNeuronTracer();
    emit stackModified();
  }
}

void ZStackDoc::loadStack(ZStack *zstack)
{
  if (zstack == NULL)
    return;

  // load it only when the pointer is different
  ZStack* &mainStack = stackRef();

  if (zstack != mainStack) {
    deprecate(STACK);
    mainStack = zstack;
    initNeuronTracer();
    emit stackModified();
  }
}

void ZStackDoc::loadReaderResult()
{
  deprecate(STACK);

  ZStack*& mainStack = stackRef();
  mainStack = m_reader.getStack();

  if (mainStack != NULL) {
    if (mainStack->data() != NULL) {
      initNeuronTracer();
      setStackSource(m_reader.getStackFile()->firstUrl().c_str());
    }
  }

#ifdef _DEBUG_2
  std::cout << "emit stackLoaded()" << std::endl;
#endif

  emit stackLoaded();
}

void ZStackDoc::selectDownstreamNode()
{
#ifdef _DEBUG_
  std::cout << "Select downstream" << std::endl;
#endif

  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
       iter != nodeSet->end(); ++iter) {
    Swc_Tree_Node_Build_Downstream_List(*iter);
    Swc_Tree_Node *tn = *iter;
    while (tn != NULL) {
      setSwcTreeNodeSelected(tn, true);
      tn = tn->next;
    }
  }
}

void ZStackDoc::readStack(const char *filePath, bool newThread)
{
  m_stackSource.import(filePath);
  if (newThread) {
    m_reader.setStackFile(&m_stackSource);
    m_reader.start();
  } else {
    deprecate(STACK);

    //ZStack*& mainStack = stackRef();
    //mainStack = m_stackSource.readStack();
    loadStack(m_stackSource.readStack());

    emit stackModified();
  }
}

bool ZStackDoc::importImageSequence(const char *filePath)
{
  ZStackFile file;
  file.importImageSeries(filePath);

  deprecate(STACK);

  ZStack*& mainStack = stackRef();
  mainStack = file.readStack();

  if (mainStack == NULL) {
    return false;
  }

  if (mainStack->data() == NULL) {
    delete mainStack;
    mainStack = NULL;

    return false;
  } else {
    initNeuronTracer();
    setStackSource(filePath);
  }

  return true;
}

void ZStackDoc::readSwc(const char *filePath)
{
  removeAllObject(true);
  ZSwcTree *tree = new ZSwcTree;
  tree->load(filePath);
  if (!tree->hasData())
    return;
  Stack stack;
  double corner[6];
  tree->boundBox(corner);
  static const double Lateral_Margin = 10.0;
  static const double Axial_Margin = 1.0;
  Stack_Set_Attribute(&stack, round(corner[3] + Lateral_Margin - corner[0] + 1),
                      round(corner[4] + Lateral_Margin - corner[1] + 1),
                      round(corner[5] + Axial_Margin - corner[2] + 1),
                      GREY);
  /*
  stack->width = round(corner[3] + Lateral_Margin);
  stack->height = round(corner[4] + Lateral_Margin);
  stack->depth = round(corner[5] + Axial_Margin);
  stack->kind = GREY;
  */
  stack.array = NULL;
  loadStack(&stack, false);
  setStackSource(filePath);
  addSwcTree(tree);
  emit swcModified();
}

void ZStackDoc::loadSwcNetwork(const QString &filePath)
{
  loadSwcNetwork(filePath.toStdString().c_str());
}

void ZStackDoc::loadSwcNetwork(const char *filePath)
{
  if (m_swcNetwork == NULL) {
    m_swcNetwork = new ZSwcNetwork;
  }

  m_swcNetwork->importTxtFile(filePath);

  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addSwcTree(m_swcNetwork->getTree(i));
  }
}

void ZStackDoc::importFlyEmNetwork(const char *filePath)
{
  if (m_swcNetwork != NULL) {
    delete m_swcNetwork;
  }

  FlyEm::ZNeuronNetwork flyemNetwork;
  flyemNetwork.import(filePath);
  flyemNetwork.layoutSwc();
  m_swcNetwork = flyemNetwork.toSwcNetwork();

  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addSwcTree(m_swcNetwork->getTree(i));
  }
}

void ZStackDoc::setStackSource(const ZStackFile &stackFile)
{
  m_stackSource = stackFile;
  setStackSource(m_stackSource.firstUrl().c_str());
}

void ZStackDoc::setStackSource(const char *filePath)
{
  if (m_stack != NULL) {
    m_stack->setSource(filePath);
  }
}

bool ZStackDoc::hasStack() const
{
  if (getStack() != NULL) {
    if (getStack()->data() != NULL) {
      return true;
    }
  }

  return false;
}


bool ZStackDoc::hasStackData() const
{
  if (getStack() != NULL) {
    if (getStack()->data() != NULL) {
      if (!getStack()->isVirtual()) {
        return true;
      }
    }
  }

  return false;
}

bool ZStackDoc::hasStackMask()
{
  bool maskAvailable = false;

  if(stackMask() != NULL) {
    if (stackMask()->channelNumber() > 0) {
      maskAvailable = true;
    }
  }

  return maskAvailable;
}

bool ZStackDoc::hasTracable()
{
  if (hasStackData()) {
    return getStack()->isTracable();
  }

  return false;
}

ZPunctum* ZStackDoc::markPunctum(int x, int y, int z, double r)
{
  if (m_stack != NULL) {
    ZPunctum *zpunctum = new ZPunctum(x, y, z, r);
    zpunctum->setMaxIntensity(m_stack->value(x, y, z));
    zpunctum->setMeanIntensity(m_stack->value(x, y, z));
    zpunctum->updateVolSize();
    zpunctum->updateMass();
    zpunctum->setSource("manually marked");
    addPunctum(zpunctum);
    emit punctaModified();
    return zpunctum;
  }
  return NULL;
}

ZLocsegChain* ZStackDoc::fitseg(int x, int y, int z, double r)
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    Locseg_Fit_Workspace *ws =
        (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace;

    if (ws->sws->field_func == Neurofield_Rpi) {
      return fitRpiseg(x, y, z, r);
    }

    Geo3d_Ball ball;
    ball.r = 3.0;
    ball.center[0] = x;
    ball.center[1] = y;
    ball.center[2] = z;
    //Geo3d_Ball_Mean_Shift(&ball, m_stack->data(), 1.0, 0.5);

    double pos[3];
    pos[0] = ball.center[0];
    pos[1] = ball.center[1];
    pos[2] = ball.center[2];

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(),
                              mainStack->preferredZScale(), 1, ws);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitRpiseg(int x, int y, int z, double r)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    Geo3d_Ball ball;
    ball.r = 3.0;
    ball.center[0] = x;
    ball.center[1] = y;
    ball.center[2] = z;
    //Geo3d_Ball_Mean_Shift(&ball, m_stack->data(), 1.0, 0.5);

    double pos[3];
    pos[0] = ball.center[0];
    pos[1] = ball.center[1];
    pos[2] = ball.center[2];

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_Locseg_Fit_Workspace(ws);

    Fit_Local_Neuroseg_W(locseg, mainStack->c_stack(),
                         mainStack->preferredZScale(), ws);

    Kill_Receptor_Fit_Workspace(ws);

    Local_Rpi_Neuroseg rpiseg;
    Local_Rpi_Neuroseg_From_Local_Neuroseg(&rpiseg, locseg);

    ws = New_Receptor_Fit_Workspace();
    Default_Rpi_Locseg_Fit_Workspace(ws);

    Fit_Local_Rpi_Neuroseg_W(&rpiseg, mainStack->c_stack(),
                             mainStack->preferredZScale(), ws);
//    Local_Rpi_Neuroseg_Optimize_W(&rpiseg, mainStack->data(),
//                              mainStack->preferredZScale(), 1, ws);

    Kill_Receptor_Fit_Workspace(ws);
    Local_Rpi_Neuroseg_To_Local_Neuroseg(&rpiseg, locseg);

    /*
    Local_Rpi_Neuroseg_Optimize_W(&rpiseg, mainStack->data(),
                                  mainStack->preferredZScale(), 1, ws);
                                  */

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitRect(int x, int y, int z, double r)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    ZLocalRect rect(x, y, z, 0.0, r);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_R2_Rect_Fit_Workspace(ws);
    rect.fitStack(mainStack->c_stack(), ws);

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    rect.toLocalNeuroseg(locseg);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

#ifdef _DEBUG_2
    ZLocalRect *new_rect = (ZLocalRect*) rect.extend();
    new_rect->fitStack(mainStack->data(), ws);
    locseg = new_rect->toLocalNeuroseg();
    Locseg_Chain_Add(locseg_chain, locseg, New_Trace_Record(), DL_TAIL);

    ZLocalRect *cur_rect = new_rect;
    for (int i = 0; i < 7; i++) {
      new_rect = (ZLocalRect*) cur_rect->extend();
      new_rect->fitStack(mainStack->data(), ws);
      locseg = new_rect->toLocalNeuroseg();
      Locseg_Chain_Add(locseg_chain, locseg, New_Trace_Record(), DL_TAIL);
      delete cur_rect;
      cur_rect = new_rect;
    }
    delete new_rect;
#endif

    Kill_Receptor_Fit_Workspace(ws);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::fitEllipse(int x, int y, int z, double r)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    Local_R2_Ellipse ellipse;
    Default_Local_R2_Ellipse(&ellipse);

    double center[3];
    center[0] = x;
    center[1] = y;
    center[2] = z;
    Local_R2_Ellipse_Set_Center(&ellipse, center);
    r = 3.0;
    Local_R2_Ellipse_Set_Size(&ellipse, r, r);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_R2_Ellipse_Fit_Workspace(ws);
    ws->pos_adjust = 0;

    Local_R2_Ellipse_Optimize_W(&ellipse, mainStack->c_stack(), 1.0, 1, ws);
    Kill_Receptor_Fit_Workspace(ws);

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Local_R2_Ellipse_To_Local_Neuroseg(&ellipse, locseg);

    Locseg_Chain *locsegChain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locsegChain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locsegChain);
    obj->setZScale(mainStack->preferredZScale());

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::dropseg(int x, int y, int z, double r)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(mainStack->preferredZScale());
    obj->setIgnorable(true);

    addLocsegChain(obj);
    emit chainModified();

    return obj;
  }

  return NULL;
}

#if 0
void ZStackDoc::loadTraceMask(bool traceMasked)
{
  if (traceMasked) {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, m_traceWorkspace->trace_mask);
  } else {
    Trace_Workspace_Set_Fit_Mask(m_traceWorkspace, NULL);
  }
}
#endif

void ZStackDoc::refreshTraceMask()
{
  if (m_isTraceMaskObsolete) {
    if (getTraceWorkspace()->trace_mask == NULL) {
      getTraceWorkspace()->trace_mask =
          C_Stack::make(GREY, getStack()->width(), getStack()->height(),
                        getStack()->depth());
    }
    Zero_Stack(getTraceWorkspace()->trace_mask);

    foreach (ZSwcTree *tree, m_swcList) {
      tree->labelStack(getTraceWorkspace()->trace_mask);
    }
    m_isTraceMaskObsolete = false;
  }
}

ZLocsegChain* ZStackDoc::traceTube(int x, int y, int z, double r, int c)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    //updateTraceWorkspace();

    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Set_Neuroseg(&(locseg->seg), r, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Fit_Workspace *ws =
        (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace;
    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(c),
                              mainStack->preferredZScale(), 1, ws);

    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Node *p = Make_Locseg_Node(locseg, tr);
    Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

    Trace_Workspace_Set_Trace_Status(getTraceWorkspace(), TRACE_NORMAL,
    		TRACE_NORMAL);
    Trace_Locseg(mainStack->c_stack(c), mainStack->preferredZScale(), locseg_chain,
            getTraceWorkspace());
    Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
    Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    if (!obj->isEmpty()) {
      obj->setZScale(mainStack->preferredZScale());
      addLocsegChain(obj);
      emit chainModified();
      /*
      m_parent->setLocsegChainInfo(obj, "Traced: ",
                                   QString(" Confidence: %1")
                                   .arg(obj->confidence(mainStack->data(), 1.0)));
                                   */
      return obj;
    } else {
      /*
      m_parent->setLocsegChainInfo(NULL,
                                   "Tracing failed: no tube-like structure found nearby.");
                                   */
      delete obj;
    }
  }

  return NULL;
}

ZLocsegChain* ZStackDoc::traceRect(int x, int y, int z, double r, int c)
{
  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    //updateTraceWorkspace();

    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    if (mainStack->preferredZScale() != 1.0) {
      pos[2] /= mainStack->preferredZScale();
    }

    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);

    ZLocalRect *rect = new ZLocalRect(x, y, z, 0.0, r);

    Receptor_Fit_Workspace *rfw =
        (Receptor_Fit_Workspace*) getTraceWorkspace()->fit_workspace;
    rect->fitStack(mainStack->c_stack(c), rfw);

    ZDirectionalTemplateChain chain;
    chain.append(rect);

    Trace_Workspace_Set_Trace_Status(getTraceWorkspace(), TRACE_NORMAL,
                TRACE_NORMAL);
    chain.trace(mainStack, getTraceWorkspace());

    ZLocsegChain *obj = chain.toLocsegChain();
    if (!obj->isEmpty()) {
      obj->setZScale(mainStack->preferredZScale());
      addLocsegChain(obj);
      emit chainModified();
      /*
      m_parent->setLocsegChainInfo(obj, "Traced: ",
                                   QString(" Confidence: %1")
                                   .arg(obj->confidence(mainStack->data(), 1.0)));
                                   */
      return obj;
    } else {
      /*
      m_parent->setLocsegChainInfo(NULL,
                                   "Tracing failed: no tube-like structure found nearby.");
                                   */
      delete obj;
    }
  }

  return NULL;
}

void ZStackDoc::cutLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain *> *pResult)
{
  //ZLocsegChain *chain = obj;
  if (pResult) {
    pResult->clear();
  }

  if (obj->heldNode() >= 0) {
    removeObject(obj, false);
    ZLocsegChain *chain = new ZLocsegChain(*obj);
    ZLocsegChain *new_chain = chain->cutHeldNode();
    if (new_chain != NULL) {
      addLocsegChain(new_chain);
      emit chainModified();
      if (pResult) {
        pResult->append(new_chain);
      }
    }
    if (chain->isEmpty() == false) {
      addLocsegChain(chain);
      emit chainModified();
      if (pResult) {
        pResult->append(chain);
      }
    } else {
      delete chain;
    }
  }
}

void ZStackDoc::breakLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain *> *pResult)
{
  //ZLocsegChain *chain = obj;
  if (pResult) {
    pResult->clear();
  }

  removeObject(obj, false);

  ZLocsegChain *chain = new ZLocsegChain(*obj);
  ZLocsegChain *newChain = chain->breakBetween(0, chain->length() - 1);

  if (newChain->isEmpty() == false) {
    addLocsegChain(newChain);
    emit chainModified();
    if (pResult) {
      pResult->append(newChain);
    }
  } else {
    delete newChain;
  }

  if (chain->isEmpty() == false) {
    addLocsegChain(chain);
    emit chainModified();
    if (pResult) {
      pResult->append(chain);
    }
  } else {
    delete chain;
  }
}

void ZStackDoc::cutSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected() == true) {
      cutLocsegChain(m_chainList.at(i));
      break;
    }
  }
}

void ZStackDoc::breakSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected() == true) {
      breakLocsegChain(m_chainList.at(i));
      break;
    }
  }
}

int ZStackDoc::autoThreshold(Stack *stack)
{
  int thre = 0;
  if (stack->array != NULL) {
    int conn = 18;
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }

    int *hist = Stack_Hist_M(stack, locmax);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);

    thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);

    free(hist);
  }
  return thre;
}

int ZStackDoc::autoThreshold()
{
  int thre = 0;
  ZStack *mainStack = getStack();

  if (!mainStack->isVirtual()) {
    m_progressReporter->start();
    Stack *stack = mainStack->c_stack();
    double scale = 1.0*stack->width * stack->height * stack->depth * stack->kind /
        (2.0*1024*1024*1024);
    if (scale >= 1.0) {
      scale = std::ceil(std::sqrt(scale + 0.1));
      stack = C_Stack::resize(stack, stack->width/scale, stack->height/scale, stack->depth);
    }

    int conn = 18;
    m_progressReporter->advance(0.1);
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    m_progressReporter->advance(0.1);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    m_progressReporter->advance(0.2);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }
    m_progressReporter->advance(0.1);

    int *hist = Stack_Hist_M(stack, locmax);
    m_progressReporter->advance(0.1);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);
    m_progressReporter->advance(0.1);

    if (high > low) {
      thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
    } else {
      free(hist);
      hist = Stack_Hist(stack);
      Int_Histogram_Range(hist, &low, &high);
      thre = Int_Histogram_Rc_Threshold(hist, low, high);
    }
    m_progressReporter->advance(0.1);
    free(hist);

    if (stack != mainStack->c_stack())
      C_Stack::kill(stack);
    m_progressReporter->end();
  }
  return thre;
}

void ZStackDoc::addSwcTree(ZSwcTree *obj, bool uniqueSource)
{
  if (obj == NULL) {
    return;
  }

  if (uniqueSource) {
    if (!obj->source().empty()) {
      QList<ZSwcTree*> treesToRemove;
      for (int i=0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i)->source() == obj->source()) {
          treesToRemove.push_back(m_swcList.at(i));
        }
      }
      for (int i=0; i<treesToRemove.size(); i++) {
        removeObject(treesToRemove.at(i), true);
      }
    }
  }

  obj->forceVirtualRoot();
  m_swcList.append(obj);
  m_objectList.append(obj);

  if (obj->isSelected()) {
    setSwcSelected(obj, true);
  }

  notifySwcModified();
}

void ZStackDoc::addSwcTree(ZSwcTree *obj, bool uniqueSource, bool translatingWithStack)
{
  if (obj == NULL) {
    return;
  }

  if (translatingWithStack) {
      obj->translate(getStackOffset());
  }

  addSwcTree(obj, uniqueSource);
}

void ZStackDoc::addSwcTree(const QList<ZSwcTree *> &swcList, bool uniqueSource)
{
  blockSignals(true);
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    addSwcTree(*iter, uniqueSource);
  }
  blockSignals(false);

  if (!swcList.isEmpty()) {
    notifySwcModified();
  }
}

void ZStackDoc::addSparseObject(const QList<ZSparseObject*> &objList)
{
  for (QList<ZSparseObject*>::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    addSparseObject(*iter);
  }
}

void ZStackDoc::addPunctum(const QList<ZPunctum *> &punctaList)
{
  blockSignals(true);
  foreach (ZPunctum *punctum, punctaList) {
    addPunctum(punctum);
  }
  blockSignals(false);

  notifyPunctumModified();
}

void ZStackDoc::addPunctum(ZPunctum *obj)
{
  if (obj == NULL) {
    return;
  }

  m_punctaList.append(obj);
  m_objectList.append(obj);

  if (obj->isSelected()) {
    setPunctumSelected(obj, true);
  }
  notifyPunctumModified();
}

void ZStackDoc::addObj3d(ZObject3d *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackObject::OBJECT_CANVAS);
  m_obj3dList.prepend(obj);
  m_objectList.prepend(obj);

  notifyObj3dModified();
}

void ZStackDoc::addSparseObject(ZSparseObject *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackObject::OBJECT_CANVAS);
  m_sparseObjectList.prepend(obj);
  m_objectList.prepend(obj);
  m_playerList.append(new ZSparseObjectPlayer(obj, ZDocPlayer::ROLE_SEED));
  emit seedModified();
}

void ZStackDoc::addStroke(ZStroke2d *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackObject::OBJECT_CANVAS);
  m_strokeList.prepend(obj);
  m_objectList.prepend(obj);

  m_playerList.append(new ZStroke2dPlayer(obj, ZDocPlayer::ROLE_SEED));
  emit seedModified();

#ifdef _DEBUG_2
  std::cout << "New stroke added" << std::endl;
  obj->print();
#endif
}

void ZStackDoc::addLocsegChain(const QList<ZLocsegChain *> &chainList)
{
  foreach (ZLocsegChain *chain, chainList) {
    addLocsegChain(chain);
  }
}

void ZStackDoc::addLocsegChain(ZLocsegChain *obj)
{
  if (obj == NULL) {
    return;
  }

  ZStack *mainStack = getStack();

  if (mainStack != NULL) {
    if (getTraceWorkspace()->trace_mask == NULL) {
      getTraceWorkspace()->trace_mask =
          Make_Stack(GREY16, mainStack->width(), mainStack->height(),
                     mainStack->depth());
      Zero_Stack(getTraceWorkspace()->trace_mask);
    }
  }

  obj->setId(getTraceWorkspace()->chain_id);
  obj->labelTraceMask(getTraceWorkspace()->trace_mask);

  //m_swcObjects.append(obj);
  //m_vrmlObjects.append(obj);
  m_chainList.append(obj);
  m_objectList.append(obj);

  getTraceWorkspace()->chain_id++;

  if (obj->isSelected()) {
    setChainSelected(obj, true);
  }
}

void ZStackDoc::updateLocsegChain(ZLocsegChain *obj)
{
  if (obj != NULL) {
    obj->labelTraceMask(getTraceWorkspace()->trace_mask);
  }
}

void ZStackDoc::exportPuncta(const char *filePath)
{
  ZPunctumIO::save(filePath, m_punctaList);
}

ZSwcTree *ZStackDoc::nodeToSwcTree(Swc_Tree_Node *node) const
{
  for (int i=0; i<m_swcList.size(); ++i) {
    if (m_swcList[i]->contains(node))
      return m_swcList[i];
  }
  assert(false);
  return NULL;
}

void ZStackDoc::exportSvg(const char *filePath)
{
  if (!m_swcList.isEmpty()) {
    m_swcList.at(0)->toSvgFile(filePath);
  }
}

void ZStackDoc::exportBinary(const char *prefix)
{
  if (m_objectList.size() > 0) {
    char *filePath = new char[strlen(prefix) + 10];
    for (int i = 0; i < m_chainList.size(); i++) {
      sprintf(filePath, "%s%d.tb", prefix, i);
      m_chainList.at(i)->save(filePath);
    }
    ////   ?
    int startNum = m_chainList.size();
    for (int i = 0; i < m_swcList.size(); i++) {
      startNum = m_swcList.at(i)->saveAsLocsegChains(prefix, startNum);
    }
    delete []filePath;
    emit chainModified();  // chain source is modified after saving
  }
}

void ZStackDoc::exportChainFileList(const char *filepath)
{
  QFile file(filepath);
  file.open(QIODevice::WriteOnly);
  QTextStream stream(&file);
  for (int i = 0; i < m_chainList.size(); i++) {
    stream << m_chainList.at(i)->source() << '\n';
  }
  file.close();
}

int ZStackDoc::xmlConnNode(QXmlStreamReader *xml,
                           QString *filePath, int *spot)
{
  int succ = 0;
  *spot = -1;
  while (!xml->atEnd()) {
    QXmlStreamReader::TokenType token = xml->readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml->name() == "filePath") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          *filePath = xml->text().toString();
          succ = 1;
        }
      } else if (xml->name() == "spot") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          *spot = xml->text().toString().toInt();
          succ = 2;
        }
      }
    } else if (token == QXmlStreamReader::EndElement) {
      if ((xml->name() == "loop") || (xml->name() == "hook")){
        break;
      }
    }
  }

  return succ;
}

int ZStackDoc::xmlConnMode(QXmlStreamReader *xml)
{
  int mode = NEUROCOMP_CONN_HL;

  while (!xml->atEnd()) {
    QXmlStreamReader::TokenType token = xml->readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml->name() == "mode") {
        if (xml->readNext() == QXmlStreamReader::Characters) {
          mode = xml->text().toString().toInt();
        }
      }
    } else if (token == QXmlStreamReader::EndElement) {
      break;
    }
  }

  return mode;
}

void ZStackDoc::importLocsegChain(const QStringList &fileList,
                                  TubeImportOption option,
                                  LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject(true);
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZLocsegChain*> chainsToRemove;
      for (int i=0; i<m_chainList.size(); i++) {
        if (m_chainList.at(i)->source() == file) {
          chainsToRemove.push_back(m_chainList.at(i));
        }
      }
      for (int i=0; i<chainsToRemove.size(); i++) {
        removeObject(chainsToRemove.at(i), true);
      }
    }
    ZLocsegChain *chain = new ZLocsegChain();

    qDebug() << file.toLocal8Bit().constData() << "\n";

    chain->load(file.toLocal8Bit().constData());

    if (!chain->isEmpty()) {
      bool loadIt = true;

      if (option != ALL_TUBE) {
        double conf = chain->confidence(getStack()->c_stack());
        qDebug() << conf << "\n";
        if (option == GOOD_TUBE) {
          if (conf < 0.5) {
            loadIt = false;
          }
        } else if (option == BAD_TUBE) {
          if (conf >= 0.5) {
            loadIt = false;
          }
        }
      }

      if (loadIt == true) {
        addLocsegChain(chain);
      } else {
        delete chain;
      }
    }
  }
  emit chainModified();
}

void ZStackDoc::importGoodTube(const char *dirpath, const char *prefix,
                               QProgressBar *pb)
{
  UNUSED_PARAMETER(pb);

  char file_path[MAX_PATH_LENGTH];

  QStringList fileList;

  sprintf(file_path, "^%s.*\\.tb", getTraceWorkspace()->save_prefix);

  if (dirpath == NULL) {
    dirpath = getTraceWorkspace()->save_path;
  }

  if (prefix == NULL) {
    prefix = getTraceWorkspace()->save_prefix;
  }

  int n = dir_fnum_s(dirpath, file_path);

  int *mask = (int *) Guarded_Calloc(n + 1, sizeof(int), "main");
  int i;
  for (i = 0; i <= n; i++) {
    sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
    if (fexist(file_path)) {
      mask[i] = 1;
    }
  }


  TubeImportOption importOption = ALL_TUBE;

  if (!m_badChainScreen.isEmpty()) {
    if (m_badChainScreen != "auto") {
      sprintf(file_path, "%s/%s", dirpath, m_badChainScreen.toLocal8Bit().constData());

      if (QFile(QString(file_path)).exists()) {
        FILE *fp = fopen(file_path, "r");
        String_Workspace *sw = New_String_Workspace();
        char *line;
        while ((line = Read_Line(fp, sw)) != NULL) {
          int id = String_Last_Integer(line);
          if (id >= 0) {
            mask[id] = 2; //bad tubes
          }
        }
        Kill_String_Workspace(sw);
        fclose(fp);
      }
    } else {
      importOption = GOOD_TUBE;
    }
  }

  for (i = 0; i <= n; i++) {
    if (mask[i] == 1) {
      /* Read the tube file */
      sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
      fileList.append(QString(file_path));
    }
  }

  free(mask);

  importLocsegChain(fileList, importOption);
}

void ZStackDoc::importBadTube(const char *dirpath, const char *prefix)
{
  char file_path[100];
  sprintf(file_path, "%s/badtube.txt", dirpath);

  if (QFile(QString(file_path)).exists() == false) {
    QMessageBox::warning(NULL, tr("Operation Failed"),
                         tr("There is no good/bad tube information."));
    return;
  }

  if (prefix == NULL) {
    prefix = getTraceWorkspace()->save_prefix;
  }

  QStringList fileList;

  int n = dir_fnum_s(dirpath, "^chain.*\\.tb");
  int *mask = (int *) Guarded_Calloc(n + 1, sizeof(int), "main");
  int i;
  for (i = 0; i <= n; i++) {
    sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
    if (fexist(file_path)) {
      mask[i] = 1;
    }
  }

  sprintf(file_path, "%s/badtube.txt", dirpath);

  FILE *fp = fopen(file_path, "r");
  String_Workspace *sw = New_String_Workspace();
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int id = String_Last_Integer(line);
    if (id >= 0) {
      mask[id] = 2; //bad tubes
    }
  }
  Kill_String_Workspace(sw);
  fclose(fp);

  for (i = 0; i <= n; i++) {
    if (mask[i] == 2) {
      /* Read the tube file */
      sprintf(file_path, "%s/%s%d.tb", dirpath, prefix, i);
      fileList.append(QString(file_path));
    }
  }

  free(mask);

  importLocsegChain(fileList);
}

void ZStackDoc::loadSwc(const QString &filePath)
{
  ZSwcTree *tree = new ZSwcTree();
  tree->load(filePath.toLocal8Bit().constData());
  addSwcTree(tree);
}

void ZStackDoc::loadLocsegChain(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QList<ZLocsegChain*> chainsToRemove;
    for (int i=0; i<m_chainList.size(); i++) {
      if (m_chainList.at(i)->source() == filePath) {
        chainsToRemove.push_back(m_chainList.at(i));
      }
    }
    for (QList<ZLocsegChain*>::iterator iter = chainsToRemove.begin();
         iter != chainsToRemove.end(); ++iter) {
      removeObject(*iter, true);
    }
  }

  ZLocsegChain *chain = new ZLocsegChain();

  chain->load(filePath.toLocal8Bit().constData());

  addLocsegChain(chain);
}

void ZStackDoc::importSwc(QStringList fileList, LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject(true);
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZSwcTree*> treesToRemove;
      for (int i=0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i)->source() == file.toStdString()) {
          treesToRemove.push_back(m_swcList.at(i));
        }
      }
      for (int i=0; i<treesToRemove.size(); i++) {
        removeObject(treesToRemove.at(i), true);
      }
    }

    if (file.endsWith(".swc", Qt::CaseInsensitive)) {
      ZSwcTree *tree = new ZSwcTree();
      tree->load(file.toLocal8Bit().constData());
      addSwcTree(tree);
    } else if (file.endsWith(".json", Qt::CaseInsensitive))  {
      importSynapseAnnotation(file.toStdString());
    }
  }
  emit swcModified();
}

bool ZStackDoc::importPuncta(const char *filePath)
{
  QStringList fileList;
  fileList.append(filePath);

  importPuncta(fileList);

  return true;
}

void ZStackDoc::importPuncta(const QStringList &fileList, LoadObjectOption objopt)
{
  if (fileList.empty())
    return;
  if (objopt == REPLACE_OBJECT) {
    removeAllObject();
  }

  QString file;
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZPunctum*> punctaToRemove;
      for (int i=0; i<m_punctaList.size(); i++) {
        if (m_punctaList.at(i)->source() == file) {
          punctaToRemove.push_back(m_punctaList.at(i));
        }
      }
      for (int i=0; i<punctaToRemove.size(); i++) {
        removeObject(punctaToRemove.at(i), true);
      }
    }
    QList<ZPunctum*> plist = ZPunctumIO::load(file);
    for (int i=0; i<plist.size(); i++) {
      addPunctum(plist[i]);
    }
  }
  emit punctaModified();
}

int ZStackDoc::pickLocsegChainId(int x, int y, int z) const
{
  if (getTraceWorkspace() == NULL) {
    return -1;
  }

  if (getTraceWorkspace()->trace_mask == NULL) {
    return -1;
  }

  int id = -1;

  if (IS_IN_CLOSE_RANGE(x, 0, getTraceWorkspace()->trace_mask->width - 1) &&
      IS_IN_CLOSE_RANGE(y, 0, getTraceWorkspace()->trace_mask->height - 1)) {
    if (z >= 0) {
      id = ((int) Get_Stack_Pixel(getTraceWorkspace()->trace_mask, x, y, z, 0)) - 1;
    } else {
      id = ((int) Stack_Hittest_Z(getTraceWorkspace()->trace_mask, x, y)) - 1;
    }
  }

  return id;
}

int ZStackDoc::pickPunctaIndex(int x, int y, int z) const
{
  int index = -1;
  for (int i=0; i<m_punctaList.size(); i++) {
    if (z >= 0) {
      if (IS_IN_CLOSE_RANGE3(x, y, z,
                             m_punctaList[i]->x() - m_punctaList[i]->radius(),
                             m_punctaList[i]->x() + m_punctaList[i]->radius(),
                             m_punctaList[i]->y() - m_punctaList[i]->radius(),
                             m_punctaList[i]->y() + m_punctaList[i]->radius(),
                             iround(m_punctaList[i]->z()),
                             iround(m_punctaList[i]->z()))) {
        index = i;
        break;
      }
    } else {
      if (IS_IN_CLOSE_RANGE3(x, y, z,
                             m_punctaList[i]->x() - m_punctaList[i]->radius(),
                             m_punctaList[i]->x() + m_punctaList[i]->radius(),
                             m_punctaList[i]->y() - m_punctaList[i]->radius(),
                             m_punctaList[i]->y() + m_punctaList[i]->radius(),
                             -1, -1)) {
        index = i;
        break;
      }
    }
  }
  return index;
}

bool ZStackDoc::selectPuncta(int index)
{
  if (index < m_punctaList.size() && index >= 0) {
    setPunctumSelected(m_punctaList[index], true);
    return true;
  }
  return false;
}

bool ZStackDoc::deleteAllPuncta()
{
  int objStartPos = m_objectList.size()-1;
  bool result = true;
  while (!m_punctaList.isEmpty()) {
    bool match = false;
    ZPunctum *obj = m_punctaList.takeLast();
    for (int i=objStartPos; i>=0; i--) {
      if (m_objectList.at(i) == dynamic_cast<ZStackObject*>(obj)) {
        m_objectList.removeAt(i);
        match = true;
        objStartPos = i-1;
        break;
      }
    }
    if (!match) {
      std::cout << "error remove puncta objs!" << std::endl;
      result = false;
    }
  }
  return result;
}

bool ZStackDoc::expandSelectedPuncta()
{
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      iter.value()->setRadius(iter.value()->radius() + 1);
    }
  }
  return true;
}

bool ZStackDoc::shrinkSelectedPuncta()
{
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      if (iter.value()->radius() > 1) {
        iter.value()->setRadius(iter.value()->radius() - 1);
      }
    }
  }
  return true;
}

bool ZStackDoc::meanshiftSelectedPuncta()
{
  if (getStack()->isVirtual()) {
    return false;
  }
  QMutableListIterator<ZPunctum*> iter(m_punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      Geo3d_Ball *gb = New_Geo3d_Ball();
      gb->center[0] = iter.value()->x();
      gb->center[1] = iter.value()->y();
      gb->center[2] = iter.value()->z();
      gb->r = iter.value()->radius();
      Geo3d_Ball_Mean_Shift(gb, getStack()->c_stack(), 1, 0.5);
      iter.value()->setX(gb->center[0]);
      iter.value()->setY(gb->center[1]);
      iter.value()->setZ(gb->center[2]);
      Delete_Geo3d_Ball(gb);
    }
  }
  return true;
}

bool ZStackDoc::meanshiftAllPuncta()
{
  if (getStack()->isVirtual()) {
    return false;
  }
  for (int i=0; i<m_punctaList.size(); i++) {
    Geo3d_Ball *gb = New_Geo3d_Ball();
    gb->center[0] = m_punctaList[i]->x();
    gb->center[1] = m_punctaList[i]->y();
    gb->center[2] = m_punctaList[i]->z();
    gb->r = m_punctaList[i]->radius();
    Geo3d_Ball_Mean_Shift(gb, getStack()->c_stack(), 1, 0.5);
    m_punctaList[i]->setX(gb->center[0]);
    m_punctaList[i]->setY(gb->center[1]);
    m_punctaList[i]->setZ(gb->center[2]);
    Delete_Geo3d_Ball(gb);
  }
  return true;
}

void ZStackDoc::holdClosestSeg(int id, int x, int y, int z)
{
  ZLocsegChain *chain;
  foreach (chain, m_chainList) {
    if (chain->id() == id) {
      chain->holdClosestSeg(x, y, z);
      break;
    }
  }
}

int ZStackDoc::selectLocsegChain(int id, int x, int y, int z, bool showProfile)
{
  int found = 0;

  ZLocsegChain *chain;
  foreach (chain, m_chainList) {
    if (chain->id() == id) {
      //chain->setSelected(true);

      //m_masterChain = chain;
      found = -1;

      if (x > 0) {
        found = iround(chain->holdClosestSeg(x, y, z)) + 1;

#if defined _ADVANCED_2
        Local_Neuroseg *locseg = chain->heldNeuroseg();

        if (showProfile == true) {
          if (locseg != NULL) {
            Stack *profile_stack = Local_Neuroseg_Stack(locseg, stack()->c_stack());
            ZStackFrame *frame = new ZStackFrame(NULL);
            ZEllipse *ellipse =
                new ZEllipse(QPointF(profile_stack->width / 2,
                                     profile_stack->height / 2),
                             Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM),
                             Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM));

            frame->addDecoration(ellipse);
            frame->loadStack(profile_stack, true);

            emit frameDelivered(frame);
          }
        }
#else
        UNUSED_PARAMETER(showProfile);
#endif
      }

      emit locsegChainSelected(chain);
      setChainSelected(chain, true);
    } else {
      //chain->setSelected(false);
      setChainSelected(chain, false);
    }
  }

  return found;
}

bool ZStackDoc::selectSwcTreeBranch(int x, int y, int z)
{
  if (!m_swcList.isEmpty()) {
    return m_swcList.at(0)->labelBranch(x, y, z, 5.0);
  }

  return false;
}

void ZStackDoc::removeLastObject(bool deleteObject)
{
  if (!m_objectList.isEmpty()) {
    ZStackObject *obj = m_objectList.takeLast();

    if (!m_chainList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_chainList.last()) == obj) {
        m_chainList.removeLast();
      }
    }

    if (!m_swcList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_swcList.last()) == obj) {
        m_swcList.removeLast();
      }
    }

    if (!m_punctaList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_punctaList.last()) == obj) {
        m_punctaList.removeLast();
      }
    }

    if (!m_obj3dList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_obj3dList.last()) == obj) {
        m_obj3dList.removeLast();
      }
    }

    if (!m_strokeList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_strokeList.last()) == obj) {
        m_strokeList.removeLast();
      }
    }

    if (!m_objectList.isEmpty()) {
      if (static_cast<ZStackObject*>(m_objectList.last()) == obj) {
        m_objectList.removeLast();
      }
    }

    m_playerList.removePlayer(obj);

    if (deleteObject == true) {
      delete obj;
    }
  }
}

void ZStackDoc::removeAllObject(bool deleteObject)
{
  while (!m_objectList.isEmpty()) {
    removeLastObject(deleteObject);
  }
}


void ZStackDoc::removeSmallLocsegChain(double thre)
{
  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    if (chain->geoLength() < thre) {
      removeObject(chain, true);
    }
  }

  notifyChainModified();
}

void ZStackDoc::removeAllLocsegChain()
{
  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    removeObject(chain, true);
  }

  notifyChainModified();
}

void ZStackDoc::removeAllObj3d()
{
  QMutableListIterator<ZObject3d*> objIter(m_obj3dList);
  while (objIter.hasNext()) {
    ZObject3d *obj = objIter.next();
    removeObject(obj, true);
  }

  notifyObj3dModified();
}

void ZStackDoc::removeAllSparseObject()
{
  QMutableListIterator<ZSparseObject*> objIter(m_sparseObjectList);
  while (objIter.hasNext()) {
    ZSparseObject *obj = objIter.next();
    removeObject(obj, true);
  }

  notifySparseObjectModified();
}

#define REMOVE_OBJECT(list, obj)				\
  for (int i = 0; i < list.size(); i++) {			\
    if (static_cast<ZStackObject*>(list.at(i)) == obj) {		\
      list.removeAt(i);					\
      break;					\
    }						\
  }

ZDocPlayer::TRole ZStackDoc::removeObject(ZStackObject *obj, bool deleteObject)
{
  REMOVE_OBJECT(m_swcList, obj);
  REMOVE_OBJECT(m_obj3dList, obj);
  REMOVE_OBJECT(m_punctaList, obj);
  REMOVE_OBJECT(m_strokeList, obj);
  REMOVE_OBJECT(m_sparseObjectList, obj);
  REMOVE_OBJECT(m_chainList, obj);
  REMOVE_OBJECT(m_objectList, obj);

  ZDocPlayer::TRole role = m_playerList.removePlayer(obj);

  if (deleteObject == true) {
    delete obj;
  }

  notifyObjectModified();
  notifyPlayerChanged(role);

  return role;
}

void ZStackDoc::removeObject(ZDocPlayer::TRole role, bool deleteObject)
{
  std::set<ZStackObject*> removeSet;
  for (ZDocPlayerList::iterator iter = m_playerList.begin();
       iter != m_playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      removeSet.insert(player->getData());
    }
  }

  blockSignals(true);
  for (std::set<ZStackObject*>::iterator iter = removeSet.begin();
       iter != removeSet.end(); ++iter) {
    removeObject(*iter, deleteObject);
  }
  blockSignals(false);

  if (!removeSet.empty()) {
    notifyObjectModified();
    notifyPlayerChanged(role);
  }
}

std::set<ZSwcTree *> ZStackDoc::removeEmptySwcTree(bool deleteObject)
{
  std::set<ZSwcTree *> emptyTreeSet;
  QMutableListIterator<ZSwcTree*> swcIter(m_swcList);

  blockSignals(true);
  while (swcIter.hasNext()) {
    ZSwcTree *tree = swcIter.next();
    if (!tree->hasRegularNode()) {
      swcIter.remove();
      removeObject(tree, deleteObject);
      if (!deleteObject) {
        emptyTreeSet.insert(tree);
      }
    }
  }
  blockSignals(false);

  notifySwcModified();

  return emptyTreeSet;
}

void ZStackDoc::removeAllSwcTree(bool deleteObject)
{
  blockSignals(true);
  QMutableListIterator<ZSwcTree*> swcIter(m_swcList);
  while (swcIter.hasNext()) {
    ZSwcTree *tree = swcIter.next();
    removeObject(tree, deleteObject);
  }
  blockSignals(false);
  notifySwcModified();
}

#define REMOVE_SELECTED_OBJECT(objtype, list, iter)	\
  QMutableListIterator<objtype*> iter(list);	\
  while (iter.hasNext()) {	\
    if (iter.next()->isSelected()) {	\
      iter.remove();	\
    }	\
  }

void ZStackDoc::removeSelectedObject(bool deleteObject)
{
  REMOVE_SELECTED_OBJECT(ZSwcTree, m_swcList, swcIter);
  REMOVE_SELECTED_OBJECT(ZObject3d, m_obj3dList, obj3dIter);
  REMOVE_SELECTED_OBJECT(ZSparseObject, m_sparseObjectList, sparseObjIter);
  REMOVE_SELECTED_OBJECT(ZStackObject, m_objectList, drawableIter);
  REMOVE_SELECTED_OBJECT(ZPunctum, m_punctaList, punctaIter);
  REMOVE_SELECTED_OBJECT(ZStroke2d, m_strokeList, strokeIter);

  QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *obj = chainIter.next();
    if (obj->isSelected()) {
      /*
      if (obj == m_masterChain) {
        m_masterChain = NULL;
      }
      */
      obj->eraseTraceMask(getTraceWorkspace()->trace_mask);
      chainIter.remove();
    }
  }


  QMutableListIterator<ZStackObject*> docIter(m_objectList);
  while (docIter.hasNext()) {
    ZStackObject *obj = docIter.next();
    if (obj->isSelected()) {
      docIter.remove();
      if (deleteObject == true) {
        delete obj;
      }
    }
  }
}

void ZStackDoc::removeSelectedPuncta(bool deleteObject)
{
  REMOVE_SELECTED_OBJECT(ZPunctum, m_punctaList, punctaIter);

  QMutableListIterator<ZStackObject*> objIter(m_objectList);
  while (objIter.hasNext()) {
    ZStackObject *obj = objIter.next();

    if (obj->isSelected()) {
      if (obj->className() == "ZPunctum") {
        objIter.remove();
      }
    }
    if (deleteObject) {
      delete obj;
    }
  }

  notifyPunctumModified();
}

bool ZStackDoc::pushLocsegChain(ZStackObject *obj)
{
  bool found =false;

  ZLocsegChain *chain = NULL;
  foreach (chain, m_chainList) {
    if ((ZStackObject*) chain == obj) {
      found = true;
      break;
    }
  }

  if (found) {
    ZLocsegChain *newChain = NULL;
    if (getTraceWorkspace() == NULL) {
      newChain = chain->pushHeldNode(getStack()->c_stack());
    } else {
      chain->eraseTraceMask(getTraceWorkspace()->trace_mask);
      newChain = chain->pushHeldNode(getStack()->c_stack(),
                    getTraceWorkspace()->trace_mask);
    }
    if (newChain != NULL) {
      chain->merge(newChain);
      if (getTraceWorkspace() != NULL) {
        chain->labelTraceMask(getTraceWorkspace()->trace_mask);
      }
      delete newChain;
    }
  }

  return found;
}

void ZStackDoc::pushSelectedLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    if (m_chainList.at(i)->isSelected()) {
      pushLocsegChain(m_chainList.at(i));
    }
  }
}

void ZStackDoc::setPunctumSelected(ZPunctum *punctum, bool select)
{
  if (punctum->isSelected() != select) {
    punctum->setSelected(select);
    QList<ZPunctum*> selected;
    QList<ZPunctum*> deselected;
    if (select) {
      m_selectedPuncta.insert(punctum);
      selected.push_back(punctum);
    } else {
      m_selectedPuncta.erase(punctum);
      deselected.push_back(punctum);
    }
    emit punctaSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectAllPuncta()
{
  QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;
  m_selectedPuncta.clear();
  for (int i=0; i<m_punctaList.size(); i++) {
    if (m_punctaList[i]->isSelected()) {
      m_punctaList[i]->setSelected(false);
      deselected.push_back(m_punctaList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit punctaSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setChainSelected(ZLocsegChain *chain, bool select)
{
  if (chain->isSelected() != select) {
    QList<ZLocsegChain*> selected;
    QList<ZLocsegChain*> deselected;
    chain->setSelected(select);
    if (select) {
      m_selectedChains.insert(chain);
      selected.push_back(chain);
    } else {
      m_selectedChains.erase(chain);
      deselected.push_back(chain);
    }
    emit chainSelectionChanged(selected, deselected);
  } else {
    emit holdSegChanged();
  }

}

void ZStackDoc::setChainSelected(const std::vector<ZLocsegChain *> &chains, bool select)
{
  QList<ZLocsegChain*> selected;
  QList<ZLocsegChain*> deselected;
  for (size_t i=0; i<chains.size(); ++i) {
    ZLocsegChain *chain = chains[i];
    if (chain->isSelected() != select) {
      chain->setSelected(select);
      if (select) {
        m_selectedChains.insert(chain);
        selected.push_back(chain);
      } else {
        m_selectedChains.erase(chain);
        deselected.push_back(chain);
      }
    }
  }
  if (!selected.empty() || !deselected.empty())
    emit chainSelectionChanged(selected, deselected);
}

void ZStackDoc::deselectAllChains()
{
  QList<ZLocsegChain*> selected;
  QList<ZLocsegChain*> deselected;
  m_selectedChains.clear();
  for (int i=0; i<m_chainList.size(); i++) {
    if (m_chainList[i]->isSelected()) {
      m_chainList[i]->setSelected(false);
      deselected.push_back(m_chainList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit chainSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setSwcSelected(ZSwcTree *tree, bool select)
{
  if (tree->isSelected() != select) {
    tree->setSelected(select);
    QList<ZSwcTree*> selected;
    QList<ZSwcTree*> deselected;
    if (select) {
      m_selectedSwcs.insert(tree);
      selected.push_back(tree);
      // deselect its nodes
      std::vector<Swc_Tree_Node *> tns;
      for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin();
           it != m_selectedSwcTreeNodes.end(); ++it) {
        if (tree == nodeToSwcTree(*it))
          tns.push_back(*it);
      }
      setSwcTreeNodeSelected(tns.begin(), tns.end(), false);
    } else {
      m_selectedSwcs.erase(tree);
      deselected.push_back(tree);
    }
    emit swcSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectAllSwcs()
{
  QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;
  m_selectedSwcs.clear();
  for (int i=0; i<m_swcList.size(); i++) {
    if (m_swcList[i]->isSelected()) {
      m_swcList[i]->setSelected(false);
      deselected.push_back(m_swcList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit swcSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::setSwcTreeNodeSelected(Swc_Tree_Node *tn, bool select)
{
  if (SwcTreeNode::isRegular(tn)) {
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;
    if (select) {
      if ((m_selectedSwcTreeNodes.insert(tn)).second) {
        selected.push_back(tn);
        // deselect its tree
        setSwcSelected(nodeToSwcTree(tn), false);
      }
    } else {
      if (m_selectedSwcTreeNodes.erase(tn) > 0) {
        deselected.push_back(tn);
      }
    }

    if (selected.size() > 0 || deselected.size() > 0) {
      emit swcTreeNodeSelectionChanged(selected, deselected);
    }
  }
}

void ZStackDoc::deselectAllSwcTreeNodes()
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin(); it != m_selectedSwcTreeNodes.end(); it++) {
    deselected.push_back(*it);
  }
  m_selectedSwcTreeNodes.clear();
  if (deselected.size() > 0)
    emit swcTreeNodeSelectionChanged(selected, deselected);
}

void ZStackDoc::deselectAllObject()
{
  deselectAllPuncta();
  deselectAllChains();
  deselectAllSwcs();
  deselectAllSwcTreeNodes();
  QMutableListIterator<ZStackObject*> iter0(m_objectList);
  while (iter0.hasNext()) {
    ZStackObject *obj = iter0.next();
    if (obj->isSelected()) {
      obj->setSelected(false);
    }
  }
}

void ZStackDoc::setPunctumVisible(ZPunctum *punctum, bool visible)
{
  if (punctum->isVisible() != visible) {
    punctum->setVisible(visible);
    emit punctumVisibleStateChanged(punctum, visible);
  }
}

void ZStackDoc::setChainVisible(ZLocsegChain *chain, bool visible)
{
  if (chain->isVisible() != visible) {
    chain->setVisible(visible);
    emit chainVisibleStateChanged(chain, visible);
  }
}

void ZStackDoc::setSwcVisible(ZSwcTree *tree, bool visible)
{
  if (tree->isVisible() != visible) {
    tree->setVisible(visible);
    emit swcVisibleStateChanged(tree, visible);
  }
}

QString ZStackDoc::toString()
{
  return QString("Number of chains: %1").arg(m_chainList.size());
}

QStringList ZStackDoc::toStringList() const
{
  ZStack *mainStack = getStack();

  QStringList list;
  list.append(QString("Number of objects: %1").arg(m_objectList.size()));
  if (mainStack != NULL) {
    list.append(QString("Stack size: %1 x %2 x %3").arg(mainStack->width())
                .arg(mainStack->height()).arg(mainStack->depth()));
    list.append(QString("Stack offset: ") +
                mainStack->getOffset().toString().c_str());
  }

  return list;
}

ZCurve ZStackDoc::locsegProfileCurve(int option) const
{
  ZCurve curve;

  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (!mainStack->isVirtual() &&
        Stack_Channel_Number(mainStack->c_stack()) == 1) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->isSelected()) {
          if (chain->heldNode() >= 0) {
            int nsample = 11;
            double *profile = Local_Neuroseg_Height_Profile(
                  chain->heldNeuroseg(), mainStack->c_stack(), chain->zScale(),
                  nsample, option, NULL, NULL);
            curve.loadArray(profile, nsample);
            free(profile);
          }
          break;
        }
      }
    }
  }

  return curve;
}

void ZStackDoc::addObject(ZStackObject *obj)
{
  m_objectList.append(obj);
  notifyObjectModified();
}

void ZStackDoc::appendSwcNetwork(ZSwcNetwork &network)
{
  if (m_swcNetwork == NULL) {
    m_swcNetwork = new ZSwcNetwork;
  }
  for (size_t i = 0; i < network.treeNumber(); i++) {
    addSwcTree(network.getTree(i));
  }

  m_swcNetwork->merge(network);

  ZStack *mainStack = getStack();
  if (mainStack == NULL) {
    Stack *stack = new Stack;
    double corner[6];
    m_swcNetwork->boundBox(corner);
    static const double Lateral_Margin = 10.0;
    static const double Axial_Margin = 1.0;
    Stack_Set_Attribute(stack, round(corner[3] + Lateral_Margin - corner[0] + 1),
        round(corner[4] + Lateral_Margin - corner[1] + 1),
        round(corner[5] + Axial_Margin - corner[2] + 1),
        GREY);

    stack->array = NULL;
    loadStack(stack, true);
    setStackSource("swc network");
  }

  emit swcNetworkModified();
}

void ZStackDoc::setTraceMinScore(double score)
{
  getTraceWorkspace()->min_score = score;
}

void ZStackDoc::setReceptor(int option, bool cone)
{
  ((Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace)->sws->field_func =
      Neuroseg_Slice_Field_Func(option);

  if (cone == TRUE) {
    Locseg_Fit_Workspace_Enable_Cone(
        (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace);
  } else {
    Locseg_Fit_Workspace_Disable_Cone(
        (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace);
  }
}

/*
void ZStackDoc::updateMasterLocsegChain()
{
  for (int i = 0; i < m_chainList.size(); i++) {
    ZLocsegChain *chain = m_chainList.at(i);
    if (chain->isSelected()) {
      m_masterChain = chain;
      break;
    }
  }
}
*/

ZStackObject* ZStackDoc::bringChainToFront()
{
  ZLocsegChain *chain = NULL;

  for (int i = 0; i < m_chainList.size(); i++) {
    chain = m_chainList.at(i);
    if (chain->isSelected()) {
      if (i > 0) {
        m_chainList.move(i, 0);
        chain->labelTraceMask(getTraceWorkspace()->trace_mask, -1);
        int j;
        for (j = 0; j < m_objectList.size(); j++) {
          if (m_objectList.at(j) == (ZStackObject*) chain) {
            m_objectList.move(j, 0);
            break;
          }
        }

        for (j = 0; j < m_objectList.size(); j++) {
          if (m_objectList.at(j) == (ZStackObject*) chain) {
            m_objectList.move(j, 0);
            break;
          }
        }
      }
      break;
    }
  }

  return chain;
}

ZStackObject *ZStackDoc::sendChainToBack()
{
  ZLocsegChain *chain = NULL;

  for (int i = 0; i < m_chainList.size(); i++) {
    chain = m_chainList.at(i);
    if (chain->isSelected()) {
      if (i < m_chainList.size() - 1) {
        int j;
        for (j = i + 1; j < m_chainList.size(); j++) {
          m_chainList.at(j)->labelTraceMask(getTraceWorkspace()->trace_mask,
                                            chain->id() + 1);
        }

        m_chainList.move(i, m_chainList.size() - 1);

        for (j = 0; j < m_objectList.size(); j++) {
          if (m_objectList.at(j) == (ZStackObject*) chain) {
            m_objectList.move(j, m_objectList.size() - 1);
            break;
          }
        }

        /*
        for (j = 0; j < m_swcObjects.size(); j++) {
          if ((ZInterface*) m_swcObjects.at(j) == (ZInterface*) chain) {
            m_swcObjects.move(j, m_swcObjects.size() - 1);
            break;
          }
        }

        for (j = 0; j < m_vrmlObjects.size(); j++) {
          if ((ZInterface*) m_vrmlObjects.at(j) == (ZInterface*) chain) {
            m_vrmlObjects.move(j, m_vrmlObjects.size() - 1);
            break;
          }
        }
*/

        for (j = 0; j < m_objectList.size(); j++) {
          if ((ZInterface*) m_objectList.at(j) == (ZInterface*) chain) {
            m_objectList.move(j, m_objectList.size() - 1);
            break;
          }
        }
      }
      break;
    }
  }

  return chain;
}

/*
bool ZStackDoc::linkChain(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //if (chain->isSelected()) {
          addLocsegChain(m_masterChain->bridge(chain));
          if (m_masterChain->isIgnorable()) {
            removeObject(m_masterChain, true);
          }
          if (chain->isIgnorable()) {
            removeObject(chain, true);
          }
          emit chainModified();
          return true;
          //}
        }
      }
    }
  }

  return false;
}
*/

/*
bool ZStackDoc::hookChain(int id, int option)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //if (chain->isSelected()) {
          ZLocsegChain *newchain = NULL;
          switch (option) {
          case 0:
            newchain = m_masterChain->bridge(chain, false);
            break;
          default:
            {
              Locseg_Fit_Workspace *ws =
                  (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace;
              newchain = m_masterChain->spBridge(chain, getStack()->c_stack(), ws);
            }
          }
          if (m_masterChain->isIgnorable()) {
            removeObject(m_masterChain, true);
          }

          if (chain->isIgnorable()) {
            removeObject(chain, true);
          }

          if (newchain != NULL) {
            addLocsegChain(newchain);
          } else {
            return false;
          }
          emit chainModified();
          return true;
        }
      }
    }
  }

  return false;
}
*/

/*
bool ZStackDoc::mergeChain(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);

        if (chain->id() == id) {
          removeObject(chain, false);
          m_masterChain->merge(chain);
          chain->detachData();
          delete chain;
          updateLocsegChain(m_masterChain);

          emit chainModified();
          return true;
        }
      }
    }
  }

  return false;
}
*/

#if 0
bool ZStackDoc::chainShortestPath(int id)
{
  bool succ = false;

  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          //addLocsegChainConn(m_masterChain, chain);
          Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
          sgw->conn = 26;
          sgw->wf = Stack_Voxel_Weight_S;
          sgw->resolution[0] = getTraceWorkspace()->resolution[0];// m_parent->xResolution();
          sgw->resolution[1] = getTraceWorkspace()->resolution[1];//m_parent->yResolution();
          sgw->resolution[2] = getTraceWorkspace()->resolution[2];//m_parent->zResolution();
          /*
          double inner = Locseg_Chain_Average_Score(m_masterChain->data(),
                                                    stack()->stack(), 1.0,
                                                    STACK_FIT_MEAN_SIGNAL);
          double outer = Locseg_Chain_Average_Score(m_masterChain->data(),
                                                    stack()->stack(), 1.0,
                                                    STACK_FIT_OUTER_SIGNAL);
*/
          //sgw->argv[3] = inner * 0.1 + outer * 0.9;
          //sgw->argv[4] = (inner - outer) / 4.6 * 1.8;
          //sgw->argv[4] = 2.0;
          Int_Arraylist *path =
              Locseg_Chain_Shortest_Path(m_masterChain->data(), chain->data(),
                                         getStack()->c_stack(), 1.0, sgw);

          if (path != NULL) {
            Object_3d *obj = Make_Object_3d(path->length, 0);
            for (int i = 0; i < path->length; i++) {
              Stack_Util_Coord(path->array[i], getStack()->width(),
                               getStack()->height(),
                               obj->voxels[i], obj->voxels[i] + 1,
                               obj->voxels[i] + 2);
            }
            addObj3d(new ZObject3d(obj));
            Kill_Int_Arraylist(path);
            succ = true;
          }

          Kill_Stack_Graph_Workspace(sgw);
        }
      }
    }
  }

  return succ;
}

void ZStackDoc::chainConnInfo(int id)
{
  if (m_masterChain != NULL) {
    if (m_masterChain->id() != id) {
      double feats[100];
      for (int i = 0; i < m_chainList.size(); i++) {
        ZLocsegChain *chain = m_chainList.at(i);
        if (chain->id() == id) {
          double res[3];
          res[0] = getTraceWorkspace()->resolution[0];//m_parent->xResolution();
          res[1] = getTraceWorkspace()->resolution[1];//m_parent->yResolution();
          res[2] = getTraceWorkspace()->resolution[2];//m_parent->zResolution();
          int n;
          Locseg_Chain_Conn_Feature(m_masterChain->data(),
                                                    chain->data(),
                                                    getStack()->c_stack(),
                                                    res, feats, &n);
          QString info;
          info.append(QString("Planar distance: %1\n").arg(feats[1]));
          info.append(QString("Euclidean distance: %1\n").arg(feats[8]));
          info.append(QString("Geodesic distance: %1\n").arg(feats[9]));
          info.append(QString("Maxmin distance: %1\n").arg(feats[10]));

          QMessageBox::information(NULL, QString("test"), info);
        }
      }
    }
  }
}
#endif

/*
void ZStackDoc::extendChain(double x, double y, double z)
{
  if (m_masterChain != NULL) {
    m_masterChain->extendHeldEnd(x, y, z);
    updateLocsegChain(m_masterChain);
  }
}
*/
/*
bool ZStackDoc::isMasterChainId(int id)
{
  if (m_masterChain->id() == id) {
    return true;
  } else {
    return false;
  }
}
*/

void ZStackDoc::mergeAllChain()
{
  foreach(ZLocsegChain *chain, m_chainList) {
    chain->eraseTraceMask(getTraceWorkspace()->trace_mask);
  }

  if (m_chainList.size() > 0) {
     int chain_number = m_chainList.size();

     /* alloc <chain_array> */
     Neuron_Component *chain_array =
       Make_Neuron_Component_Array(chain_number);

     int chain_number2 = 0;

     for (int i = 0; i < chain_number; i++) {
       if ((m_chainList.at(i)->length() > 0) &&
           !(m_chainList.at(i)->isIgnorable())) {
         Set_Neuron_Component(chain_array + chain_number2,
                              NEUROCOMP_TYPE_LOCSEG_CHAIN,
                              m_chainList.at(i)->data());
         chain_number2++;
       }
     }

     /* reconstruct neuron */
     Connection_Test_Workspace ctw = *getConnectionTestWorkspace();
     ctw.sp_test = FALSE;

     double zscale = m_chainList.at(0)->zScale();

     /* alloc <ns> */
     ZStack *mainStack = getStack();
     Neuron_Structure *ns = NULL;
     if (mainStack != NULL) {
       ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                          mainStack->c_stack(), zscale, &ctw);
     } else {
       ns = Locseg_Chain_Comp_Neurostruct(chain_array, chain_number2,
                                          NULL, zscale, &ctw);
     }

     Process_Neuron_Structure(ns);

     if (getConnectionTestWorkspace()->crossover_test) {
       Neuron_Structure_Crossover_Test(ns, zscale);
     }

     Neuron_Structure_To_Tree(ns);
     Neuron_Structure_Merge_Locseg_Chain(ns);
     free(ns->comp);
     ns->comp = NULL;
     /* free <ns> */
     Kill_Neuron_Structure(ns);

     QMutableListIterator<ZLocsegChain*> chainIter(m_chainList);
     while (chainIter.hasNext()) {
       ZLocsegChain *obj = chainIter.next();
       if (obj->isEmpty()) {
         //obj->setSelected(true);
         ////removeLocsegChain(obj);
         setChainSelected(obj, true);
       } else {
         obj->updateBufferChain();
         obj->labelTraceMask(getTraceWorkspace()->trace_mask);
         //obj->setSelected(false);
         setChainSelected(obj, false);
       }
     }
     removeSelectedObject(true);
   }
}

QString ZStackDoc::dataInfo(double cx, double cy, int z) const
{
  int x = iround(cx);
  int y = iround(cy);

  QString info = QString("%1, %2").arg(x).arg(y);
  if (x >= 0 && y >= 0) {
    if (z < 0) {
      info += " (MIP): ";
    } else {
      info += QString(", %3: ").arg(z);
    }

    if (getStack() != NULL) {
      if (!getStack()->isVirtual()) {
        if (getStack()->channelNumber() == 1) {
          info += QString("%4").arg(getStack()->value(x, y, z));
        } else {
          info += QString("(");
          for (int i=0; i<getStack()->channelNumber(); i++) {
            if (i==0) {
              info += QString("%1").arg(getStack()->value(x, y, z, i));
            } else {
              info += QString(", %1").arg(getStack()->value(x, y, z, i));
            }
          }
          info += QString(")");
        }
      }

      if (stackMask() != NULL) {
        info += " | Mask: ";
        if (stackMask()->channelNumber() == 1) {
          info += QString("%4").arg(stackMask()->value(x, y, z));
        } else {
          info += QString("(");
          for (int i=0; i<stackMask()->channelNumber(); i++) {
            if (i==0) {
              info += QString("%1").arg(stackMask()->value(x, y, z, i));
            } else {
              info += QString(", %1").arg(stackMask()->value(x, y, z, i));
            }
          }
          info += QString(")");
        }
      }

      if (getStack()->hasOffset()) {
        info += QString("; Data coordinates: (%1, %2, %3)").
            arg(getStackOffset().getX() + x).arg(getStackOffset().getY() + y).
            arg(getStackOffset().getZ() + z);
      }
    }
  }

  return info;
}

void ZStackDoc::setWorkdir(const QString &filePath)
{
  setWorkdir(filePath.toLocal8Bit().constData());
}

void ZStackDoc::setWorkdir(const char *filePath)
{
  strcpy(getTraceWorkspace()->save_path, filePath);
}

void ZStackDoc::setTubePrefix(const char *prefix)
{
  strcpy(getTraceWorkspace()->save_prefix, prefix);
}

void ZStackDoc::setBadChainScreen(const char *screen)
{
  if (screen == NULL) {
    m_badChainScreen.clear();
  } else {
    m_badChainScreen = screen;
  }
}

void ZStackDoc::eraseTraceMask(const ZLocsegChain *chain)
{
  chain->eraseTraceMask(getTraceWorkspace()->trace_mask);
}

bool ZStackDoc::binarize(int threshold)
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (threshold < 0) {
      threshold = 0;
    }

    if (mainStack->binarize(threshold)) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::bwsolid()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (mainStack->bwsolid()) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::bwperim()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (mainStack->bwperim()) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::invert()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    ZStackProcessor::invert(mainStack);
    emit stackModified();
    return true;
  }

  return false;
}

bool ZStackDoc::enhanceLine()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (mainStack->enhanceLine()) {
      emit stackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::importSynapseAnnotation(const std::string &filePath)
{
  FlyEm::ZSynapseAnnotationArray synapseArray;
  if (synapseArray.loadJson(filePath)) {
    ZSwcTree *tree = synapseArray.toSwcTree();
    ZStack *mainStack = getStack();
    if (mainStack != NULL) {
      tree->flipY(mainStack->height() - 1);
    }

    addSwcTree(tree);
    return true;
  }

  return false;
}

void ZStackDoc::loadFileList(const QList<QUrl> &urlList)
{
  QStringList fileList;
  for (QList<QUrl>::const_iterator iter = urlList.begin();
       iter != urlList.end(); ++iter) {
    // load files inside if is folder
    QFileInfo dirCheck(iter->toLocalFile());
    if (dirCheck.isDir()) {
      QDir dir = dirCheck.absoluteDir();
      QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
      for (int i=0; i<list.size(); i++) {
        fileList.append(list.at(i).absoluteFilePath());
      }
    } else {
      fileList.append(dirCheck.absoluteFilePath());
    }
  }

  loadFileList(fileList);
}

void ZStackDoc::loadFileList(const QStringList &fileList)
{
  bool swcLoaded = false;
  bool chainLoaded = false;
  bool networkLoaded = false;
  bool punctaLoaded = false;
  //bool apoLoaded = false;

  for (QStringList::const_iterator iter = fileList.begin(); iter != fileList.end();
       ++iter) {
    switch (ZFileType::fileType(iter->toStdString())) {
    case ZFileType::SWC_FILE:
    case ZFileType::SYNAPSE_ANNOTATON_FILE:
      swcLoaded = true;
      break;
    case ZFileType::SWC_NETWORK_FILE:
    case ZFileType::FLYEM_NETWORK_FILE:
      swcLoaded = true;
      networkLoaded = true;
      break;
    case ZFileType::LOCSEG_CHAIN_FILE:
      chainLoaded = true;
      break;
    case ZFileType::V3D_APO_FILE:
    case ZFileType::V3D_MARKER_FILE:
    case ZFileType::RAVELER_BOOKMARK:
      punctaLoaded = true;
      break;
    default:
      break;
    }

    blockSignals(true);
    loadFile(*iter, false);
    blockSignals(false);
  }

  if (swcLoaded) {
    emit swcModified();
  }

  if (chainLoaded) {
    emit chainModified();
  }

  if (networkLoaded) {
    emit swcNetworkModified();
  }

  if (punctaLoaded) {
    emit punctaModified();
  }

#ifdef _FLYEM_2
  emit punctaModified();
#endif
}

bool ZStackDoc::loadFile(const QString &filePath, bool emitMessage)
{
  switch (ZFileType::fileType(filePath.toStdString())) {
  case ZFileType::SWC_FILE:
#ifdef _FLYEM_2
    removeAllObject();
#endif
    loadSwc(filePath);
    if (emitMessage) {
      emit swcModified();
    }
    break;
  case ZFileType::LOCSEG_CHAIN_FILE:
    loadLocsegChain(filePath);
    if (emitMessage) {
      emit chainModified();
    }
    break;
  case ZFileType::SWC_NETWORK_FILE:
    loadSwcNetwork(filePath);
    if (emitMessage) {
      emit swcModified();
      emit swcNetworkModified();
    }
    break;
  case ZFileType::OBJECT_SCAN_FILE:
    setTag(NeuTube::Document::FLYEM_BODY);
  {
    ZSparseObject *sobj = new ZSparseObject;
    sobj->load(filePath.toStdString().c_str());
    addSparseObject(sobj);
    sobj->setColor(255, 255, 255, 255);

    ZIntCuboid cuboid = sobj->getBoundBox();
    ZStack *stack = ZStackFactory::makeVirtualStack(
          cuboid.getWidth(), cuboid.getHeight(), cuboid.getDepth());
    stack->setOffset(cuboid.getFirstCorner());
    loadStack(stack);

    emit stackModified();
    emit sparseObjectModified();
  }
    break; //experimenting _DEBUG_
  case ZFileType::TIFF_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
    readStack(filePath.toStdString().c_str(), false);
    if (emitMessage) {
      stackModified();
    }
    break;
  case ZFileType::FLYEM_NETWORK_FILE:
    importFlyEmNetwork(filePath.toStdString().c_str());
    if (emitMessage) {
      emit swcModified();
      emit swcNetworkModified();
    }
    break;
  case ZFileType::SYNAPSE_ANNOTATON_FILE:
    if (importSynapseAnnotation(filePath.toStdString())) {
      if (emitMessage) {
        emit swcModified();
      }
    } else {
      return false;
    }
    break;
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
  case ZFileType::RAVELER_BOOKMARK:
    if (importPuncta(filePath.toStdString().c_str())) {
      if (emitMessage) {
        emit punctaModified();
      }
    } else {
      return false;
    }
    break;
  default:
    return false;
    break;
  }

  return true;
}

void ZStackDoc::deprecateDependent(EComponent component)
{
  switch (component) {
  case STACK:
    break;
  default:
    break;
  }
}

void ZStackDoc::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case STACK:
    delete stackRef();
    stackRef() = NULL;
    m_neuronTracer.clear();
    break;
  case SPARSE_STACK:
    delete m_sparseStack;
    m_sparseStack = NULL;
    break;
  default:
    break;
  }
}

bool ZStackDoc::isDeprecated(EComponent component)
{
  switch (component) {
  case STACK:
    return stackRef() == NULL;
    break;
  default:
    return false;
  }

  return false;
}

Swc_Tree_Node* ZStackDoc::swcHitTest(int x, int y, int z)
{
  Swc_Tree_Node *selected = NULL;
  const double Margin = 0.5;

  for (QList<ZSwcTree*>::iterator iter = m_swcList.begin();
       iter != m_swcList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    if (z < 0) {
      selected = tree->hitTest(x, y);
    } else {
      selected = tree->hitTest(x, y, z, Margin);
    }

    if (selected != NULL) {
      break;
    }
  }

  return selected;
}

void ZStackDoc::selectSwcTreeNode(Swc_Tree_Node *selected, bool append)
{
  if (!append) {
    selectedSwcTreeNodes()->clear();
    if (selected != NULL) {
      selectedSwcTreeNodes()->insert(selected);
    }
  } else {
    if (selected != NULL) {
      if (selectedSwcTreeNodes()->count(selected) > 0) {
        selectedSwcTreeNodes()->erase(selected);
      } else {
        selectedSwcTreeNodes()->insert(selected);
      }
    }
  }
}

Swc_Tree_Node *ZStackDoc::selectSwcTreeNode(int x, int y, int z, bool append)
{
  Swc_Tree_Node *selected = swcHitTest(x, y, z);

  selectSwcTreeNode(selected, append);

  return selected;
}

void ZStackDoc::reloadStack()
{
  if (m_stackFactory != NULL) {
    if (m_stackFactory->makeStack(getStack())) {
      notifyStackModified();
    }
  } else {
    updateStackFromSource();
  }
}

void ZStackDoc::updateStackFromSource()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (mainStack->isSwc()) {
      readSwc(mainStack->sourcePath().c_str());
      emit stackModified();
    } else {
      if (mainStack->updateFromSource()) {
        emit stackModified();
      }
    }
  }
}

int ZStackDoc::maxIntesityDepth(int x, int y)
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    return mainStack->maxIntensityDepth(x, y);
  }

  return 0;
}

void ZStackDoc::test(QProgressBar *pb)
{
#if 0
  importLocsegChainConn("/Users/zhaot/work/neurolabi/data/diadem_e1/conn.xml");
  for (int i = 0; i < m_connList.size(); i++) {
    m_connList.at(i)->print();
  }
#endif
  UNUSED_PARAMETER(pb);

  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    mainStack->enhanceLine();
  }
}

const char* ZStackDoc::tubePrefix() const
{
  if (getTraceWorkspace() != NULL) {
    return getTraceWorkspace()->save_prefix;
  }

  return NULL;
}

void ZStackDoc::notifySwcModified()
{
  foreach (ZSwcTree *tree, m_swcList) {
    tree->deprecate(ZSwcTree::ALL_COMPONENT);
  }

  emit swcModified();
}

void ZStackDoc::notifyStatusMessageUpdated(const QString &message)
{
  if (!message.isEmpty()) {
    emit statusMessageUpdated(message);
  }
}

void ZStackDoc::notifyPunctumModified()
{
  emit punctaModified();
}

void ZStackDoc::notifyChainModified()
{
  emit chainModified();
}

void ZStackDoc::notifyObj3dModified()
{
  emit obj3dModified();
}

void ZStackDoc::notifySparseObjectModified()
{
  emit sparseObjectModified();
}


void ZStackDoc::notifyStackModified()
{
  emit stackModified();
}

void ZStackDoc::notifySparseStackModified()
{
  emit sparseStackModified();
}

void ZStackDoc::notifyVolumeModified()
{
  emit volumeModified();
}

void ZStackDoc::notifyStrokeModified()
{
  emit strokeModified();
}

void ZStackDoc::notify3DGraphModified()
{
  emit graph3dModified();
}

void ZStackDoc::notifyObjectModified()
{
  emit objectModified();
}

void ZStackDoc::notifyAllObjectModified()
{
  notifySwcModified();
  notifyPunctumModified();
  notifyChainModified();
  notifyObj3dModified();
  notifyStrokeModified();
}

bool ZStackDoc::watershed()
{
  ZStack *mainStack = getStack();
  m_progressReporter->start();
  m_progressReporter->advance(0.5);
  if (mainStack != NULL) {
    if (mainStack->watershed()) {
      emit stackModified();
      return true;
    }
  }
  m_progressReporter->end();

  return false;
}

int ZStackDoc::findLoop(int minLoopSize)
{
  int loopNumber = 0;

  if (hasStackData()) {
    m_progressReporter->start();

    Stack *data = C_Stack::clone(getStack()->c_stack(0));

    m_progressReporter->advance(0.1);

    Stack_Binarize(data);
    Stack *filled = Stack_Fill_Hole_N(data, NULL, 1, 6, NULL);

    m_progressReporter->advance(0.1);
    Stack *shrinked = Stack_Bwpeel(filled, REMOVE_ARC, NULL);
    C_Stack::kill(filled);

    m_progressReporter->advance(0.2);
#ifdef _DEBUG_2
    const NeutubeConfig &config = NeutubeConfig::getInstance();
    C_Stack::write(config.getPath(NeutubeConfig::DATA) + "/test.tif", shrinked);
#endif
    //Stack_Threshold(shrinked, 100);
    //m_progressReporter->update(40);

    ZStackGraph stackGraph;
    ZGraph *graph = stackGraph.buildForegroundGraph(shrinked);

    graph->setProgressReporter(m_progressReporter);
    m_progressReporter->advance(0.1);

    ZGraphCompressor compressor;
    compressor.setGraph(graph);
    compressor.compress();
    m_progressReporter->advance(0.1);

    m_progressReporter->startSubprogress(0.3);
    std::vector<std::vector<int> > cycleArray = graph->getCycle();
    graph->setProgressReporter(m_progressReporter);
    for (size_t i = 0; i < cycleArray.size(); ++i) {
      vector<int> path = cycleArray[i];
#ifdef _DEBUG_
      cout << "Cycle size: " << path.size() << endl;
#endif
      if ((int) path.size() >= minLoopSize) {
        ZObject3d *obj = new ZObject3d;
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          int x, y, z;
          C_Stack::indexToCoord(compressor.uncompress(*iter), C_Stack::width(data),
                                C_Stack::height(data), &x, &y, &z);
          obj->append(x, y, z);
        }
        addObj3d(obj);
        ++loopNumber;
      }
    }
    m_progressReporter->endSubprogress(0.3);
    /*
    vector<bool> labeled(graph->getVertexNumber(), false);
    //For each loop in the graph, label it
    for (int i = 0; i < graph->getVertexNumber(); ++i) {
      if (!labeled[i]) {
        vector<int> path = graph->getPath(i, i);
        ZObject3d *obj = new ZObject3d;
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;

          if (path.size() > 100) {
            int x, y, z;
            C_Stack::indexToCoord(compressor.uncompress(*iter), C_Stack::width(data),
                                  C_Stack::height(data), &x, &y, &z);
            obj->append(x, y, z);
          }
        }
        if (obj->size() > 0) {
          addObj3d(obj);
        } else {
          delete obj;
        }

        ++loopNumber;
      }
    }
    */
    m_progressReporter->advance(0.1);

    if (loopNumber > 0) {
      emit obj3dModified();
    }

    delete graph;
    Kill_Stack(shrinked);
    Kill_Stack(data);

    m_progressReporter->end();
  }

  return loopNumber;
}

void ZStackDoc::bwthin()
{
  if (hasStackData()) {
    m_progressReporter->start();

    if (C_Stack::kind(getStack()->c_stack(0)) == GREY) {
      m_progressReporter->advance(0.1);
      Stack *out = Stack_Bwthin(getStack()->c_stack(0), NULL);
      m_progressReporter->advance(0.5);
      C_Stack::copyValue(out, getStack()->c_stack(0));
      C_Stack::kill(out);
      m_progressReporter->advance(0.3);
      getStack()->deprecateSingleChannelView(0);
      emit stackModified();
    }

    m_progressReporter->end();
  }
}

void ZStackDoc::executeSwcRescaleCommand(const ZRescaleSwcSetting &setting)
{
  QUndoCommand *allcommand = new QUndoCommand();
  if (setting.bTranslateSoma) {
    new ZStackDocCommand::SwcEdit::TranslateRoot(
          this, setting.translateDstX, setting.translateDstY,
          setting.translateDstZ, allcommand);
  }
  if (setting.scaleX != 1 || setting.scaleY != 1 || setting.scaleZ != 1) {
    new ZStackDocCommand::SwcEdit::Rescale(
          this, setting.scaleX, setting.scaleY, setting.scaleZ, allcommand);
  }
  if (setting.bRescaleBranchRadius) {
    new ZStackDocCommand::SwcEdit::RescaleRadius(
          this, setting.rescaleBranchRadiusScale, setting.somaCutLevel+1, -1,
          allcommand);
  }
  if (setting.bRescaleSomaRadius) {
    new ZStackDocCommand::SwcEdit::RescaleRadius(
          this, setting.rescaleSomaRadiusScale, 0, setting.somaCutLevel+1,
          allcommand);
  }
  if (setting.bReduceSwcNodes) {
    new ZStackDocCommand::SwcEdit::ReduceNodeNumber(
          this, setting.reduceSwcNodesLengthThre, allcommand);
  }

  if (allcommand->childCount() > 0) {
    allcommand->setText(QObject::tr("rescale swc"));
    pushUndoCommand(allcommand);
    deprecateTraceMask();
  } else {
    delete allcommand;
  }
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center)
{
  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], SwcTreeNode::radius(prevNode));
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(this, tn, prevNode);
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center, double radius)
{
  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], radius);
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(this, tn, prevNode);
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeSwcNodeSmartExtendCommand(const ZPoint &center)
{
//  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      return executeSwcNodeSmartExtendCommand(
            center, SwcTreeNode::radius(prevNode));
    }
  }
#if 0
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        ZNeuronTracer tracer;
        tracer.setIntensityField(stack()->c_stack());
        tracer.setTraceWorkspace(m_traceWorkspace);
        if (m_traceWorkspace->trace_mask == NULL) {
          m_traceWorkspace->trace_mask =
              C_Stack::make(GREY, stack()->width(), stack()->height(),
                            stack()->depth());
        }

        Swc_Tree *branch = tracer.trace(
              SwcTreeNode::x(prevNode), SwcTreeNode::y(prevNode),
              SwcTreeNode::z(prevNode), SwcTreeNode::radius(prevNode),
              center.x(), center.y(), center.z(),
              SwcTreeNode::radius(prevNode));
        if (branch != NULL) {
          if (Swc_Tree_Has_Branch(branch)) {
            //tracer.updateMask(branch);
            Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
            Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);
            SwcTreeNode::detachParent(begin);
            Kill_Swc_Tree(branch);

            Swc_Tree_Node *leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
            }
            ZSwcPath path(begin, leaf);

            command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
            new ZStackDocCommand::SwcEdit::AddSwcNode(this, begin, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, begin, prevNode, command);
            new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
            /*
            new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(
                  this, path, command);
                  */
          }
        }
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }
#endif

  return false;
}


bool ZStackDoc::executeSwcNodeSmartExtendCommand(
    const ZPoint &center, double radius)
{
  bool succ = false;
  QString message;

  QUndoCommand *command = NULL;
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet->empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet->begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        //ZNeuronTracer tracer;
        //tracer.setBackgroundType(getStackBackground());
        //tracer.setIntensityField(stack()->c_stack());
        //tracer.setTraceWorkspace(getTraceWorkspace());
        if (getTraceWorkspace()->trace_mask == NULL) {
          getTraceWorkspace()->trace_mask =
              C_Stack::make(GREY, getStack()->width(), getStack()->height(),
                            getStack()->depth());
        }
        if (GET_APPLICATION_NAME == "Biocytin") {
          m_neuronTracer.setResolution(1, 1, 10);
        }

        m_neuronTracer.setStackOffset(getStackOffset());

        Swc_Tree *branch = m_neuronTracer.trace(
              SwcTreeNode::x(prevNode), SwcTreeNode::y(prevNode),
              SwcTreeNode::z(prevNode), SwcTreeNode::radius(prevNode),
              center.x(), center.y(), center.z(), radius);
        if (branch != NULL) {
          if (Swc_Tree_Has_Branch(branch)) {
            //tracer.updateMask(branch);
            Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
            Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);
            SwcTreeNode::detachParent(begin);
            Kill_Swc_Tree(branch);

            Swc_Tree_Node *leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
            }
            ZSwcPath path(begin, leaf);

            message = QString("%1 nodes are added").arg(path.size());

            command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
            new ZStackDocCommand::SwcEdit::AddSwcNode(this, begin, command);
            std::set<Swc_Tree_Node*> nodeSet;
            nodeSet.insert(leaf);
            new ZStackDocCommand::SwcEdit::SetSwcNodeSeletion(
                  this, nodeSet, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, begin, prevNode, command);
            new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
            /*
            new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(
                  this, path, command);
                  */
          }
        }
      }
    }
  }

  if (command != NULL) {
    pushUndoCommand(command);
    deprecateTraceMask();
    notifyStatusMessageUpdated(message);
    succ = true;
  }

  return succ;
}

bool ZStackDoc::executeInterpolateSwcZCommand()
{
  bool succ = false;
  QString message;

  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               m_selectedSwcTreeNodes.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               m_selectedSwcTreeNodes.count(downEnd) == 1) { /* continuation and selected*/
          downEnd = SwcTreeNode::firstChild(downEnd);
        }

        double dist1 = SwcTreeNode::planePathLength(*iter, upEnd);
        double dist2 = SwcTreeNode::planePathLength(*iter, downEnd);

        double z = SwcTreeNode::z(*iter);
        if (dist1 == 0.0 && dist2 == 0.0) {
          z = SwcTreeNode::z(upEnd);
        } else {
          double lambda = dist1 / (dist1 + dist2);
          z = SwcTreeNode::z(upEnd) * (1.0 - lambda) +
              SwcTreeNode::z(downEnd) * lambda;
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNodeZ(
              this, *iter, z, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Z Interpolation"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("The Z coordinates of %1 node(s) are intepolated").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeInterpolateSwcPositionCommand()
{
  bool succ = false;
  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               m_selectedSwcTreeNodes.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               m_selectedSwcTreeNodes.count(downEnd) == 1) { /* continuation and selected*/
          downEnd = SwcTreeNode::firstChild(downEnd);
        }

        double dist1 = SwcTreeNode::planePathLength(*iter, upEnd);
        double dist2 = SwcTreeNode::planePathLength(*iter, downEnd);

        double x = SwcTreeNode::x(*iter);
        double y = SwcTreeNode::y(*iter);
        double z = SwcTreeNode::z(*iter);

        if (dist1 == 0.0 && dist2 == 0.0) {
          x = SwcTreeNode::x(upEnd);
          y = SwcTreeNode::y(upEnd);
          z = SwcTreeNode::z(upEnd);
        } else {
          double lambda = dist1 / (dist1 + dist2);
          x = SwcTreeNode::x(upEnd) * (1.0 - lambda) +
              SwcTreeNode::x(downEnd) * lambda;
          y = SwcTreeNode::y(upEnd) * (1.0 - lambda) +
              SwcTreeNode::y(downEnd) * lambda;
          z = SwcTreeNode::z(upEnd) * (1.0 - lambda) +
              SwcTreeNode::z(downEnd) * lambda;
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, *iter, x, y, z, SwcTreeNode::radius(*iter), allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Position Interpolation"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("The coordinates of %1 node(s) are intepolated").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeInterpolateSwcCommand()
{
  bool succ = false;
  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               m_selectedSwcTreeNodes.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               m_selectedSwcTreeNodes.count(downEnd) == 1) { /* continuation and selected*/
          downEnd = SwcTreeNode::firstChild(downEnd);
        }

        double dist1 = SwcTreeNode::planePathLength(*iter, upEnd);
        double dist2 = SwcTreeNode::planePathLength(*iter, downEnd);

        double x = SwcTreeNode::x(*iter);
        double y = SwcTreeNode::y(*iter);
        double z = SwcTreeNode::z(*iter);
        double radius = SwcTreeNode::radius(*iter);

        if (dist1 == 0.0 && dist2 == 0.0) {
          x = SwcTreeNode::x(upEnd);
          y = SwcTreeNode::y(upEnd);
          z = SwcTreeNode::z(upEnd);
          radius = SwcTreeNode::radius(upEnd);
        } else {
          double lambda = dist1 / (dist1 + dist2);
          x = SwcTreeNode::x(upEnd) * (1.0 - lambda) +
              SwcTreeNode::x(downEnd) * lambda;
          y = SwcTreeNode::y(upEnd) * (1.0 - lambda) +
              SwcTreeNode::y(downEnd) * lambda;
          z = SwcTreeNode::z(upEnd) * (1.0 - lambda) +
              SwcTreeNode::z(downEnd) * lambda;
          radius = SwcTreeNode::radius(upEnd) * (1.0 - lambda) +
              SwcTreeNode::radius(downEnd) * lambda;
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, *iter, x, y, z, radius, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Interpolation"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("%1 node(s) are interpolated").arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeInterpolateSwcRadiusCommand()
{
  bool succ = false;
  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               m_selectedSwcTreeNodes.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               m_selectedSwcTreeNodes.count(downEnd) == 1) { /* continuation and selected*/
          downEnd = SwcTreeNode::firstChild(downEnd);
        }

        double dist1 = SwcTreeNode::planePathLength(*iter, upEnd);
        double dist2 = SwcTreeNode::planePathLength(*iter, downEnd);

        double radius = SwcTreeNode::radius(*iter);
        if (dist1 == 0.0 && dist2 == 0.0) {
          radius = SwcTreeNode::radius(upEnd);
        } else {
          double lambda = dist1 / (dist1 + dist2);
          radius = SwcTreeNode::radius(upEnd) * (1.0 - lambda) +
              SwcTreeNode::radius(downEnd) * lambda;
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNodeRadius(
              this, *iter, radius, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Radius Interpolation"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("Radii of %1 node(s) are interpolated.").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeSwcNodeChangeZCommand(double z)
{
  bool succ = false;
  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL && SwcTreeNode::z(*iter) != z) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeZ(
              this, *iter, z, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Change Z of Selected Node"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("Z of %1 node(s) is set to %2").
          arg(allCommand->childCount()).arg(z);
    } else {
      delete allCommand;
    }
    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeMoveSwcNodeCommand(double dx, double dy, double dz)
{
  bool succ = false;
  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL && (dx != 0 || dy != 0 || dz != 0)) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::translate(&newNode, dx, dy, dz);
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Move Selected Node"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = "Nodes moved.";
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeTranslateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::pos(node + 1) - SwcTreeNode::pos(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    }
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      double sx = dlg.getScaleValue(SwcSkeletonTransformDialog::X);
      double sy = dlg.getScaleValue(SwcSkeletonTransformDialog::Y);
      double sz = dlg.getScaleValue(SwcSkeletonTransformDialog::Z);

      ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        Swc_Tree_Node newNode = *(*iter);
        if (dlg.isTranslateFirst()) {
          SwcTreeNode::translate(&newNode, dx, dy, dz);
        }

        SwcTreeNode::setPos(&newNode, SwcTreeNode::x(&newNode) * sx,
                            SwcTreeNode::y(&newNode) * sy,
                            SwcTreeNode::z(&newNode) * sz);
        if (!dlg.isTranslateFirst()) {
          SwcTreeNode::translate(&newNode, dx, dy, dz);
        }

        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeChangeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();

  if (!nodeSet->empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
           iter != nodeSet->end(); ++iter) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::changeRadius(&newNode, dlg.getAddValue(), dlg.getMulValue());
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }

      pushUndoCommand(allCommand);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeSwcNodeChangeSizeCommand(double dr)
{
  bool succ = false;
  QString message;
  int nodeCount = 0;

  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (dr != 0) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::changeRadius(&newNode, dr, 1.0);
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
        ++nodeCount;
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Node - Change Size"));

      pushUndoCommand(allCommand);
      deprecateTraceMask();

      message = QString("Size(s) of %1 node(s) are changed.").arg(nodeCount);
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

void ZStackDoc::estimateSwcRadius(ZSwcTree *tree, int maxIter)
{
  if (tree != NULL) {
    startProgress();
    int count = tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    double step = 1.0 / count / maxIter;

    for (int iter = 0; iter < maxIter; ++iter) {
      for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
        if (SwcTreeNode::isRegular(tn)) {
          SwcTreeNode::fitSignal(tn, getStack()->c_stack(), getStackBackground());
        }
        advanceProgress(step);
      }
    }
    endProgress();
  }
}

void ZStackDoc::estimateSwcRadius()
{
  foreach (ZSwcTree *tree, m_swcList){
    estimateSwcRadius(tree);
  }
}

bool ZStackDoc::executeSwcNodeEstimateRadiusCommand()
{
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    startProgress();
    double step = 1.0 / m_selectedSwcTreeNodes.size();
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      Swc_Tree_Node newNode = *(*iter);
      if (SwcTreeNode::fitSignal(&newNode,  getStack()->c_stack(),
                                 getStackBackground())) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
        advanceProgress(step);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Node - Estimate Radius"));
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    endProgress();

    return true;
  }

  return false;
}

static bool isMergable(const std::set<Swc_Tree_Node*> &nodeSet)
{
  std::set<Swc_Tree_Node*> newSelectedSet;

  QQueue<Swc_Tree_Node*> tnQueue;
  tnQueue.enqueue(*(nodeSet.begin()));

  while (!tnQueue.isEmpty()) {
    Swc_Tree_Node *tn = tnQueue.dequeue();
    std::vector<Swc_Tree_Node*> neighborArray =
        SwcTreeNode::neighborArray(tn);
    for (std::vector<Swc_Tree_Node*>::iterator
         iter = neighborArray.begin(); iter != neighborArray.end();
         ++iter) {
      if (nodeSet.count(*iter) > 0 &&
          newSelectedSet.count(*iter) == 0) {
        newSelectedSet.insert(*iter);
        tnQueue.enqueue(*iter);
      }
    }
  }

  return nodeSet.size() == newSelectedSet.size();
}

bool ZStackDoc::executeMergeSwcNodeCommand()
{
  bool succ = false;
  QString message;
  if (selectedSwcTreeNodes()->size() > 1 &&
      /*SwcTreeNode::isAllConnected(*selectedSwcTreeNodes()) &&*/
      isMergable(*selectedSwcTreeNodes())) {
    message = QString("%1 nodes are merged.").arg(selectedSwcTreeNodes()->size());
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::MergeSwcNode(this);
    pushUndoCommand(command);
    deprecateTraceMask();
    succ = true;
  } else {
    message = QString("Cannot merge the nodes, "
                      "which should be directly connected.");
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeSetRootCommand()
{
  bool succ = false;

  QString message;

  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    Swc_Tree_Node *tn = *nodeSet->begin();
    if (!SwcTreeNode::isRoot(tn)) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::SetRoot(this, tn);
      pushUndoCommand(command);
      succ = true;
      message = "A node is set to root.";
    } else {
      message = "The selected node is already a root. Nothing is done.";
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeRemoveTurnCommand()
{
  bool succ = false;

  QString message;

  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    Swc_Tree_Node *tn = *(nodeSet->begin());
    Swc_Tree_Node *tn1 = NULL;
    Swc_Tree_Node *tn2 = NULL;
    if (SwcTreeNode::isContinuation(tn)) {
      tn1 = SwcTreeNode::firstChild(tn);
      tn2 = SwcTreeNode::parent(tn);
    } else {
      std::vector<Swc_Tree_Node*> neighborArray =
          SwcTreeNode::neighborArray(tn);
      double minDot = 0.0;
      for (size_t i = 0; i < neighborArray.size(); ++i) {
        for (size_t j = 0; j < neighborArray.size(); ++j) {
          if (i != j) {
            double dot = Swc_Tree_Node_Dot(
                  neighborArray[i], tn, neighborArray[j]);
            if (dot < minDot) {
              minDot = dot;
              tn1 = neighborArray[i];
              tn2 = neighborArray[j];
            }
          }
        }
      }
    }

    if (SwcTreeNode::isTurn(tn1, tn, tn2)) {
      double lambda = SwcTreeNode::pathLengthRatio(tn2, tn1, tn);
      double x, y, z, r;
      SwcTreeNode::interpolate(tn1, tn2, lambda, &x, &y, &z, &r);
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
            this, tn, x, y, z, r);
      pushUndoCommand(command);
      deprecateTraceMask();

      message = "A turn is detected and removed.";

      succ = true;
    } else {
      message = "No turn is detected. Nothing is done.";
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeResolveCrossoverCommand()
{
  bool succ = false;
  QString message;

  std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    Swc_Tree_Node *center = *(nodeSet->begin());
    std::map<Swc_Tree_Node*, Swc_Tree_Node*> matched =
        SwcTreeNode::crossoverMatch(center, TZ_PI_2);
    if (!matched.empty()) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      Swc_Tree_Node *root = SwcTreeNode::root(center);
      for (std::map<Swc_Tree_Node*, Swc_Tree_Node*>::const_iterator
           iter = matched.begin(); iter != matched.end(); ++iter) {
        if (SwcTreeNode::parent(iter->first) == center &&
            SwcTreeNode::parent(iter->second) == center) {
          new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->first, iter->second, command);
          new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->second, root, command);
        } else {
          new ZStackDocCommand::SwcEdit::SetParent(this, center, root, command);
          if (SwcTreeNode::parent(iter->first) == center) {
            new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->first, iter->second, command);
          } else {
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, iter->second, iter->first, command);
          }
        }

        if (matched.size() * 2 == SwcTreeNode::neighborArray(center).size()) {
          new ZStackDocCommand::SwcEdit::DeleteSwcNode(
                this, center, root, command);
        }
      }
      pushUndoCommand(command);
      deprecateTraceMask();

      message = "A crossover is created.";
      succ = true;
    } else {
      message = "No crossover is detected. Nothing is done";
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeWatershedCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::Watershed(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeBinarizeCommand(int thre)
{
  if (hasStackData()) {
    QUndoCommand *command =
        new ZStackDocCommand::StackProcess::Binarize(this, thre);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeBwsolidCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::BwSolid(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeEnhanceLineCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::StackProcess::EnhanceLine(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeDeleteSwcNodeCommand()
{
  bool succ = false;
  QString message;

  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (*iter != NULL) {
        new ZStackDocCommand::SwcEdit::DeleteSwcNode(
              this, *iter, SwcTreeNode::root(*iter), allCommand);
      }
    }
    new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, allCommand);

    if (allCommand->childCount() > 0) {
      message = QString("%1 node(s) are deleted").arg(m_selectedSwcTreeNodes.size());
      allCommand->setText(QObject::tr("Delete Selected Node"));
      blockSignals(true);
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
      m_selectedSwcTreeNodes.clear();
      blockSignals(false);
      notifySwcModified();
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeConnectSwcNodeCommand()
{
  bool succ = false;
  QString message;

  if (m_selectedSwcTreeNodes.size() > 1) {
    QUndoCommand *command =  new ZStackDocCommand::SwcEdit::ConnectSwcNode(this);
    pushUndoCommand(command);
    deprecateTraceMask();

    message = "Nodes are connected.";
    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeConnectSwcNodeCommand(Swc_Tree_Node *tn)
{
  if (!selectedSwcTreeNodes()->empty()) {
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          *selectedSwcTreeNodes(), tn);
    return executeConnectSwcNodeCommand(target, tn);
  }

  return false;
}

bool ZStackDoc::executeConnectSwcNodeCommand(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  QString message = "The nodes are already connected. Nothing is done";
  if (!SwcTreeNode::isRegular(tn1) || !SwcTreeNode::isRegular(tn2)) {
    return false;
  }

  if (SwcTreeNode::isRegular(SwcTreeNode::commonAncestor(tn1, tn2))) {
    notifyStatusMessageUpdated(message);
    return false;
  }

  QUndoCommand *command =
      new ZStackDocCommand::SwcEdit::CompositeCommand(this);
  new ZStackDocCommand::SwcEdit::SetRoot(this, tn2, command);
  new ZStackDocCommand::SwcEdit::SetParent(this, tn2, tn1, command);
  new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);

  pushUndoCommand(command);
  deprecateTraceMask();

  notifySwcModified();
  message = "Two nodes are connected.";
  notifyStatusMessageUpdated(message);

  return true;
}

bool ZStackDoc::executeSmartConnectSwcNodeCommand()
{
  if (selectedSwcTreeNodes()->size() == 2) {
    std::set<Swc_Tree_Node*>::iterator first = selectedSwcTreeNodes()->begin();
    std::set<Swc_Tree_Node*>::iterator second = first;
    ++second;
    return executeSmartConnectSwcNodeCommand(*first, *second);
  }

  return false;
}

bool ZStackDoc::executeSmartConnectSwcNodeCommand(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  bool succ  = false;

  QString message;

  if (!SwcTreeNode::isRegular(tn1) || !SwcTreeNode::isRegular(tn2)) {
    return false;
  }

  if (SwcTreeNode::isRegular(SwcTreeNode::commonAncestor(tn1, tn2))) {
    notifyStatusMessageUpdated("Nothing is done because the nodes are already connected.");
    return false;
  }

  //ZNeuronTracer tracer;
  //tracer.setBackgroundType(getStackBackground());
  //tracer.setIntensityField(stack()->c_stack());
  //tracer.setTraceWorkspace(getTraceWorkspace());
  if (getTraceWorkspace()->trace_mask == NULL) {
    getTraceWorkspace()->trace_mask =
        C_Stack::make(GREY, getStack()->width(), getStack()->height(),
                      getStack()->depth());
  }

  Swc_Tree *branch = m_neuronTracer.trace(
        SwcTreeNode::x(tn1), SwcTreeNode::y(tn1),
        SwcTreeNode::z(tn1), SwcTreeNode::radius(tn1),
        SwcTreeNode::x(tn2), SwcTreeNode::y(tn2),
        SwcTreeNode::z(tn2), SwcTreeNode::radius(tn2));

  /*
  Swc_Tree *branch = tracer.trace(
        SwcTreeNode::x(tn1) - offset.x(), SwcTreeNode::y(tn1) - offset.y(),
        SwcTreeNode::z(tn1) - offset.z(), SwcTreeNode::radius(tn1),
        SwcTreeNode::x(tn2) - offset.x(), SwcTreeNode::y(tn2) - offset.y(),
        SwcTreeNode::z(tn2) - offset.z(), SwcTreeNode::radius(tn2));

  Swc_Tree_Translate(branch, offset.x(), offset.y(), offset.z());
  */

  if (branch != NULL) {
    if (Swc_Tree_Has_Branch(branch)) {
      //tracer.updateMask(branch);
      Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
      Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);

      Swc_Tree_Node *leaf = begin;
      while (SwcTreeNode::firstChild(leaf) != NULL) {
        leaf = SwcTreeNode::firstChild(leaf);
      }

      if (leaf == begin || begin == NULL) { //Less than three nodes
        Kill_Swc_Tree(branch);
        branch = NULL;
      }

      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      new ZStackDocCommand::SwcEdit::SetRoot(this, tn2, command);

      if (branch != NULL) {
        SwcTreeNode::detachParent(begin);
        Kill_Swc_Tree(branch);

        Swc_Tree_Node *terminal = SwcTreeNode::parent(leaf);
        SwcTreeNode::detachParent(leaf);
        SwcTreeNode::kill(leaf);
        //ZSwcPath path(begin, terminal);
        new ZStackDocCommand::SwcEdit::SetParent(this, tn2, terminal, command);
        new ZStackDocCommand::SwcEdit::SetParent(this, begin, tn1, command);
      } else {
        new ZStackDocCommand::SwcEdit::SetParent(this, tn2, tn1, command);
      }
      new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);

      pushUndoCommand(command);
      deprecateTraceMask();

      notifySwcModified();
      message = "Nodes are connected";
      succ = true;
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeBreakSwcConnectionCommand()
{
  bool succ = false;

  QString message;
  if (!m_selectedSwcTreeNodes.empty()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = m_selectedSwcTreeNodes.begin();
         iter != m_selectedSwcTreeNodes.end(); ++iter) {
      if (m_selectedSwcTreeNodes.count(SwcTreeNode::parent(*iter)) > 0) {
        new ZStackDocCommand::SwcEdit::SetParent(
              this, *iter, SwcTreeNode::root(*iter), allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Break Connection"));
      blockSignals(true);
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
      blockSignals(false);
      notifySwcModified();
      deprecateTraceMask();

      message = "Connections removed.";
      succ = true;
    } else {
      delete allCommand;
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeBreakForestCommand()
{
  if (!m_selectedSwcs.empty()) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::BreakForest(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeGroupSwcCommand()
{
  if (m_selectedSwcs.size() > 1) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::GroupSwc(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAddSwcCommand(ZSwcTree *tree)
{
  if (tree != NULL) {
    QUndoCommand *command = new ZStackDocCommand::SwcEdit::AddSwc(this, tree);
    pushUndoCommand(command);
    deprecateTraceMask();
  }

  return false;
}

bool ZStackDoc::executeAddSwcNodeCommand(const ZPoint &center, double radius)
{
  if (radius > 0) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(center, radius);
    ZStackDocCommand::SwcEdit::AddSwcNode *command = new
        ZStackDocCommand::SwcEdit::AddSwcNode(this, tn);
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeAddStrokeCommand(ZStroke2d *stroke)
{
  if (stroke != NULL) {
    QUndoCommand *command =
        new ZStackDocCommand::StrokeEdit::AddStroke(this, stroke);
    if (!stroke->isEmpty()) {
      pushUndoCommand(command);
      return true;
    } else {
      delete command;
    }
  }

  return false;
}

bool ZStackDoc::executeAddStrokeCommand(const QList<ZStroke2d*> &strokeList)
{
  bool succ = false;
  QString message;
  if (!strokeList.isEmpty()) {
    QUndoCommand *allCommand =
        new ZStackDocCommand::StrokeEdit::CompositeCommand(this);

    foreach (ZStroke2d* stroke, strokeList) {
      if (stroke != NULL) {
        new ZStackDocCommand::StrokeEdit::AddStroke(this, stroke, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      pushUndoCommand(allCommand);
      message = QString("%1 stroke(s) added").arg(allCommand->childCount());
      succ = true;
    } else {
      delete allCommand;
    }
  }

  emit statusMessageUpdated(message);

  return succ;
}

void ZStackDoc::addObject(
    ZStackObject *obj, NeuTube::EDocumentableType type, ZDocPlayer::TRole role)
{
  switch (type) {
  case NeuTube::Documentable_SWC:
    addSwcTree(dynamic_cast<ZSwcTree*>(obj), false);
    break;
  case NeuTube::Documentable_PUNCTUM:
    addPunctum(dynamic_cast<ZPunctum*>(obj));
    break;
  case NeuTube::Documentable_OBJ3D:
    addObj3d(dynamic_cast<ZObject3d*>(obj));
    break;
  case NeuTube::Documentable_LOCSEG_CHAIN:
    addLocsegChain(dynamic_cast<ZLocsegChain*>(obj));
    break;
  case NeuTube::Documentable_STROKE:
    addStroke(dynamic_cast<ZStroke2d*>(obj));
    break;
  case NeuTube::Documentable_SPARSE_OBJECT:
    addSparseObject(dynamic_cast<ZSparseObject*>(obj));
    break;
  default:
    addObject(obj);
    break;
  }

  addPlayer(obj, type, role);
}

void ZStackDoc::notifyPlayerChanged(ZDocPlayer::TRole role)
{
  if (role & ZDocPlayer::ROLE_SEED) {
    emit seedModified();
  }

  if (role & ZDocPlayer::ROLE_3DPAINT) {
    notifyVolumeModified();
  }

  if (role & ZDocPlayer::ROLE_3DGRAPH_DECORATOR) {
    notify3DGraphModified();
  }
}

void ZStackDoc::addPlayer(
    ZStackObject *obj, NeuTube::EDocumentableType type, ZDocPlayer::TRole role)
{
  if (role != ZDocPlayer::ROLE_NONE) {
    ZDocPlayer *player = NULL;
    switch (type) {
    case NeuTube::Documentable_OBJ3D:
      player = new ZObject3dPlayer(obj, role);
      break;
    default:
      player = new ZDocPlayer(obj, role);
      break;
    }

    m_playerList.append(player);
    notifyPlayerChanged(player->getRole());
  }
}

bool ZStackDoc::hasObjectSelected()
{
  return !(m_selectedPuncta.empty() && m_selectedChains.empty() &&
           m_selectedSwcs.empty());
}

bool ZStackDoc::executeAddObjectCommand(
    ZStackObject *obj, NeuTube::EDocumentableType type, ZDocPlayer::TRole role)
{
  if (obj != NULL) {
    ZStackDocCommand::ObjectEdit::AddObject *command =
        new ZStackDocCommand::ObjectEdit::AddObject(this, obj, type, role);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveObjectCommand()
{
  if (hasObjectSelected()) {
    ZStackDocCommand::ObjectEdit::RemoveSelected *command = new
        ZStackDocCommand::ObjectEdit::RemoveSelected(this);
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}
/*
bool ZStackDoc::executeRemoveUnselectedObjectCommand()
{

  return false;
}
*/
bool ZStackDoc::executeMoveObjectCommand(double x, double y, double z,
    double punctaScaleX, double punctaScaleY, double punctaScaleZ,
    double swcScaleX, double swcScaleY, double swcScaleZ)
{
  if (selectedSwcs()->empty() && selectedPuncta()->empty() &&
      selectedSwcTreeNodes()->empty())
    return false;

  ZStackDocCommand::ObjectEdit::MoveSelected *moveSelectedObjectCommand =
      new ZStackDocCommand::ObjectEdit::MoveSelected(this, x, y, z);
  moveSelectedObjectCommand->setPunctaCoordScale(punctaScaleX,
                                                 punctaScaleY,
                                                 punctaScaleZ);
  moveSelectedObjectCommand->setSwcCoordScale(swcScaleX,
                                              swcScaleY,
                                              swcScaleZ);
  pushUndoCommand(moveSelectedObjectCommand);

  return true;
}

bool ZStackDoc::executeTraceTubeCommand(double x, double y, double z, int c)
{
  QUndoCommand *traceTubeCommand =
      new ZStackDocCommand::TubeEdit::Trace(this, x, y, z, c);
  pushUndoCommand(traceTubeCommand);

  return true;
}

bool ZStackDoc::executeTraceSwcBranchCommand(
    double x, double y, double z, int c)
{
  /*
  QUndoCommand *command =
      new ZStackDocCommand::SwcEdit::TraceSwcBranch(this, x, y, z, c);
  pushUndoCommand(command);
  */

  //ZNeuronTracer tracer;
  m_neuronTracer.setIntensityField(getStack()->c_stack(c));
  //tracer.setTraceWorkspace(getTraceWorkspace());
  //tracer.setStackOffset(getStackOffset().x(), getStackOffset().y(),
  //                      getStackOffset().z());

  refreshTraceMask();
  ZSwcPath branch = m_neuronTracer.trace(x, y, z);

  if (branch.size() > 1) {
    ZSwcConnector swcConnector;

    std::pair<Swc_Tree_Node*, Swc_Tree_Node*> conn =
        swcConnector.identifyConnection(branch, getSwcArray());

    Swc_Tree_Node *branchRoot = branch.front();

    if (conn.first != NULL) {
      bool needAdjust = false;
      if (!SwcTreeNode::isRoot(conn.first)) {
        SwcTreeNode::setAsRoot(conn.first);
        branchRoot = conn.first;
      }

      if (SwcTreeNode::hasOverlap(conn.first, conn.second)) {
        needAdjust = true;
      } else {
        if (SwcTreeNode::isTurn(conn.second, conn.first,
                                SwcTreeNode::firstChild(conn.first))) {
          needAdjust = true;
        }
      }
      if (needAdjust) {
        SwcTreeNode::average(branchRoot, SwcTreeNode::firstChild(branchRoot),
                             branchRoot);
      }
    } else {
      if (SwcTreeNode::isRegular(SwcTreeNode::firstChild(branchRoot))) {
        Swc_Tree_Node *rootNeighbor = SwcTreeNode::firstChild(branchRoot);
        ZPoint rootCenter = SwcTreeNode::pos(branchRoot);
        ZPoint nbrCenter = SwcTreeNode::pos(rootNeighbor);

        double lambda = ZNeuronTracer::findBestTerminalBreak(
              rootCenter, SwcTreeNode::radius(branchRoot),
              nbrCenter, SwcTreeNode::radius(rootNeighbor),
              getStack()->c_stack());

        if (lambda < 1.0) {
          SwcTreeNode::interpolate(
                branchRoot, rootNeighbor, lambda, branchRoot);
        }
      }
#if 0
      if (SwcTreeNode::isRegular(SwcTreeNode::firstChild(branchRoot))) {
        Swc_Tree_Node *rootNeighbor = SwcTreeNode::firstChild(branchRoot);
        ZPoint rootCenter = SwcTreeNode::pos(branchRoot);
        ZPoint nbrCenter = SwcTreeNode::pos(rootNeighbor);
        double rootIntensity = Stack_Point_Sampling(
              stack()->c_stack(), rootCenter.x(), rootCenter.y(), rootCenter.z());
        if (rootIntensity == 0.0) {
          needAdjust = true;
        } else {
          double nbrIntensity = Stack_Point_Sampling(
                stack()->c_stack(), nbrCenter.x(), nbrCenter.y(), nbrCenter.z());
          if (nbrIntensity / rootIntensity >= 3.0) {
            needAdjust = true;
          }
        }
      }
#endif
    }

    Swc_Tree_Node *loop = conn.second;
    Swc_Tree_Node *hook = conn.first;

    if (hook != NULL) {
      //Adjust the branch point
      std::vector<Swc_Tree_Node*> neighborArray =
          SwcTreeNode::neighborArray(loop);
      for (std::vector<Swc_Tree_Node*>::iterator iter = neighborArray.begin();
           iter != neighborArray.end(); ++iter) {
        Swc_Tree_Node *tn = *iter;
        if (SwcTreeNode::hasSignificantOverlap(tn, hook)) {
          loop = tn;
          Swc_Tree_Node *newHook = hook;
          newHook = SwcTreeNode::firstChild(hook);
          SwcTreeNode::detachParent(newHook);
          SwcTreeNode::kill(hook);
          hook = newHook;
          branchRoot = hook;
        }
      }
    }


    ZSwcTree *tree = new ZSwcTree();
    tree->setDataFromNode(branchRoot);

    if (SwcTreeNode::isRegular(SwcTreeNode::firstChild(branchRoot))) {
      Swc_Tree_Node *terminal = tree->firstLeaf();
      Swc_Tree_Node *terminalNeighbor = SwcTreeNode::parent(tree->firstLeaf());
      ZPoint terminalCenter = SwcTreeNode::pos(terminal);
      ZPoint nbrCenter = SwcTreeNode::pos(terminalNeighbor);

      double lambda = ZNeuronTracer::findBestTerminalBreak(
            terminalCenter, SwcTreeNode::radius(terminal),
            nbrCenter, SwcTreeNode::radius(terminalNeighbor),
            getStack()->c_stack());

      if (lambda < 1.0) {
        SwcTreeNode::interpolate(terminal, terminalNeighbor, lambda, terminal);
      }
#if 0
      double terminalIntensity = Stack_Point_Sampling(
            stack()->c_stack(), terminalCenter.x(), terminalCenter.y(), terminalCenter.z());
      if (terminalIntensity == 0.0) {
        SwcTreeNode::average(terminal, terminalNeighbor, terminal);
      } else {
        double nbrIntensity = Stack_Point_Sampling(
              stack()->c_stack(), nbrCenter.x(), nbrCenter.y(), nbrCenter.z());
        if (nbrIntensity / terminalIntensity >= 3.0) {
          SwcTreeNode::average(terminal, terminalNeighbor, terminal);
        }
      }
#endif
    }

    ZSwcPath path(branchRoot, tree->firstLeaf());


    QUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);



    new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

    if (conn.first != NULL) {
      new ZStackDocCommand::SwcEdit::SetParent(
            this, hook, loop, command);
      new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
    }

    new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(this, path, command);

    pushUndoCommand(command);

    QString statusMessage;
    statusMessage = QString("%1 nodes added.").arg(path.size());
    emit statusMessageUpdated(statusMessage);
    //tracer.updateMask(branch);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveTubeCommand()
{
  if (!selectedChains()->empty()) {
    QUndoCommand *command =
        new ZStackDocCommand::TubeEdit::RemoveSelected(this);
    pushUndoCommand(command);
    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceCommand()
{
#if 0
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::TubeEdit::AutoTrace(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
#endif

#if 0
  autoTrace();
  Swc_Tree *rawTree = this->swcReconstruction(0, false, true);
  removeAllLocsegChain();
  Zero_Stack(getTraceWorkspace()->trace_mask);
#endif

  m_neuronTracer.setProgressReporter(getProgressReporter());

  startProgress(0.9);
  ZSwcTree *tree = m_neuronTracer.trace(getStack()->c_stack());
  endProgress(0.9);

  Zero_Stack(getTraceWorkspace()->trace_mask);

  if (tree != NULL) {
    //ZSwcTree *tree = new ZSwcTree;
    //tree->setData(rawTree);
    //QUndoCommand *command = new ZStackDocCommand::SwcEdit::AddSwc(this, tree);
    ZStackDocCommand::SwcEdit::CompositeCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);
    new ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask(this, tree->data(), command);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceAxonCommand()
{
  if (hasStackData()) {
    QUndoCommand *command = new ZStackDocCommand::TubeEdit::AutoTraceAxon(this);
    pushUndoCommand(command);

    return true;
  }

  return false;
}

void ZStackDoc::saveSwc(QWidget *parentWidget)
{
  //Assume there is no empty tree
  if (!m_swcList.empty()) {
    if (m_swcList.size() > 1) {
      report("Save failed", "More than one SWC tree exist.",
             ZMessageReporter::Error);
    } else {
      ZSwcTree *tree = m_swcList[0];
      if (tree->hasGoodSourceName()) {
        tree->resortId();
        tree->save(tree->source().c_str());
        emit statusMessageUpdated(QString(tree->source().c_str()) + " saved.");
      } else {
        ZString stackSource = stackSourcePath();
        QString fileName;
        if (!stackSource.empty()) {
          fileName = stackSource.changeExt("Edit.swc").c_str();
        }

        if (fileName.isEmpty()) {
          fileName = "untitled.swc";
        }

        fileName = QFileDialog::getSaveFileName(
              parentWidget, tr("Save SWC"), fileName, tr("SWC File"), 0);
        if (!fileName.isEmpty()) {
          tree->resortId();
          tree->save(fileName.toStdString().c_str());
          tree->setSource(fileName.toStdString());
          notifySwcModified();
          emit statusMessageUpdated(QString(tree->source().c_str()) + " saved.");
        }
      }
    }
  }
}

std::vector<ZSwcTree*> ZStackDoc::getSwcArray() const
{
  std::vector<ZSwcTree*> swcArray;
  for (QList<ZSwcTree*>::const_iterator iter = m_swcList.begin();
       iter != m_swcList.end(); ++iter) {
    swcArray.push_back(const_cast<ZSwcTree*>(*iter));
  }

  return swcArray;
}

ZStack* ZStackDoc::projectBiocytinStack(
    Biocytin::ZStackProjector &projector)
{
  projector.setProgressReporter(m_progressReporter);

  ZStack *proj = projector.project(getStack(), true);

  if (proj != NULL) {
    if (proj->channelNumber() == 2) {
      proj->initChannelColors();
      proj->setChannelColor(0, 1, 1, 1);
      proj->setChannelColor(1, 0, 0, 0);
    }

   // ZString filePath(stack()->sourcePath());
    proj->setSource(projector.getDefaultResultFilePath(getStack()->sourcePath()));
#ifdef _DEBUG2
    const vector<int>& depthArray = projector.getDepthArray();
    ZStack depthImage(GREY16, proj->width(), proj->height(), 1, 1);
    uint16_t *array = depthImage.array16();
    size_t index = 0;
    for (int y = 0; y < proj->height(); ++y) {
      for (int x = 0; x < proj->width(); ++x) {
        array[index] = depthArray[index];
        ++index;
      }
    }
    depthImage.save(NeutubeConfig::getInstance().getPath(NeutubeConfig::DATA) +
                    "/test.tif");
#endif
  }

  return proj;
}

void ZStackDoc::selectAllSwcTreeNode()
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  foreach (ZSwcTree *tree, m_swcList) {
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      if ((m_selectedSwcTreeNodes.insert(tn)).second) {
        selected.push_back(tn);
        // deselect its tree
        setSwcSelected(nodeToSwcTree(tn), false);
      }
    }
  }
  if (!selected.empty() || !deselected.empty()) {
    emit swcTreeNodeSelectionChanged(selected, deselected);
  }
}

bool ZStackDoc::getLastStrokePoint(int *x, int *y) const
{
  if (!m_strokeList.empty()) {
    if (!m_strokeList[0]->isEmpty()) {
      return m_strokeList[0]->getLastPoint(x, y);
    }
  }

  return false;
}

bool ZStackDoc::getLastStrokePoint(double *x, double *y) const
{
  if (!m_strokeList.empty()) {
    if (!m_strokeList[0]->isEmpty()) {
      return m_strokeList[0]->getLastPoint(x, y);
    }
  }

  return false;
}

bool ZStackDoc::hasSelectedSwc() const
{
  return !selectedSwcs()->empty();
}

bool ZStackDoc::hasSelectedSwcNode() const
{
  return !selectedSwcTreeNodes()->empty();
}

bool ZStackDoc::hasMultipleSelectedSwcNode() const
{
  return selectedSwcTreeNodes()->size() > 1;
}

void ZStackDoc::updateModelData(EDocumentDataType type)
{
  switch (type) {
  case SWC_DATA:
    swcObjsModel()->updateModelData();
    break;
  case PUNCTA_DATA:
    punctaObjsModel()->updateModelData();
    break;
  default:
    break;
  }
}

void ZStackDoc::showSeletedSwcNodeScaledLength()
{
  double resolution[3] = {1, 1, 1};
  if (m_resDlg.exec()) {
    resolution[0] = m_resDlg.getXYScale();
    resolution[1] = m_resDlg.getXYScale();
    resolution[2] = m_resDlg.getZScale();
    showSeletedSwcNodeLength(resolution);
  }
}

void ZStackDoc::showSeletedSwcNodeLength(double *resolution)
{
  double length = 0.0;

  if (resolution == NULL) {
    length = SwcTreeNode::segmentLength(*selectedSwcTreeNodes());
  } else {
    length = SwcTreeNode::scaledSegmentLength(
          *selectedSwcTreeNodes(), resolution[0], resolution[1], resolution[2]);
  }

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (selectedSwcTreeNodes()->size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter =
        selectedSwcTreeNodes()->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;

    if (!SwcTreeNode::isConnected(tn1, tn2)) {
      double dist = 0.0;
      if (resolution == NULL) {
        dist = SwcTreeNode::distance(tn1, tn2);
      } else {
        dist = SwcTreeNode::scaledDistance(tn1, tn2, resolution[0],
            resolution[1], resolution[2]);
      }
      textStream << "<p>Straight line distance between the two selected nodes: "
                 << dist << "</p>";
    }
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void ZStackDoc::showSwcSummary()
{
  InformationDialog dlg;

  std::ostringstream textStream;
  if (swcList()->isEmpty()) {
    textStream << "<p>No neuron data.</p>";
  } else {
    textStream << "<p>Overall length of " << swcList()->size() << " Neuron(s): ";
    double length = 0.0;
    foreach (ZSwcTree* tree, *swcList()) {
      length += tree->length();
    }
    textStream << length << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

bool ZStackDoc::executeInsertSwcNode()
{
  bool succ = false;

  QString message;
  int insertionCount = 0;
  if (selectedSwcTreeNodes()->size() >= 2) {
    QUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (set<Swc_Tree_Node*>::iterator iter = selectedSwcTreeNodes()->begin();
         iter != selectedSwcTreeNodes()->end(); ++iter) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(*iter);
      if (selectedSwcTreeNodes()->count(parent) > 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer();
        SwcTreeNode::interpolate(*iter, parent, 0.5, tn);
        new ZStackDocCommand::SwcEdit::SetParent(this, tn, parent, command);
        new ZStackDocCommand::SwcEdit::SetParent(this, *iter, tn, command);
        ++insertionCount;
      }
    }
    if (command->childCount() > 0) {
      pushUndoCommand(command);
      deprecateTraceMask();
      message = QString("%1 nodes inserted.").arg(insertionCount);
      succ = true;
    } else {
      message = QString("Cannont insert a node. "
                        "At least two adjacent nodes should be selected.");
      delete command;
    }
  }

  if (!message.isEmpty()) {
    emit statusMessageUpdated(message);
  }

  return succ;
}

bool ZStackDoc::executeSetBranchPoint()
{
  bool succ = false;
  QString message;

  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *branchPoint = *(selectedSwcTreeNodes()->begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);
    if (SwcTreeNode::childNumber(masterRoot) > 1) {
      QUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      ZSwcTree tree;
      tree.setDataFromNode(masterRoot);

      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      bool isConnected = false;
      double minDist = Infinity;
      Swc_Tree_Node *closestNode = NULL;
      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
        if (SwcTreeNode::isRegular(tn)) {
          if (SwcTreeNode::isRoot(tn)) {
            if (tn == hostRoot) {
              isConnected = false;
            } else {
              isConnected = true;
            }
          }

          if (isConnected) {
            double dist = SwcTreeNode::distance(
                  tn, branchPoint, SwcTreeNode::EUCLIDEAN_SURFACE);
            if (dist < minDist) {
              minDist = dist;
              closestNode = tn;
            }
          }
        }
      }
      tree.setDataFromNode(NULL, ZSwcTree::LEAVE_ALONE);

      if (!SwcTreeNode::isRoot(closestNode)) {
        new ZStackDocCommand::SwcEdit::SetRoot(this, closestNode, command);
      }
      new ZStackDocCommand::SwcEdit::SetParent(
            this, closestNode, branchPoint, command);

      pushUndoCommand(command);
      deprecateTraceMask();

      message = "A branch point is created.";
      succ = true;
    }
  }

  if (!message.isEmpty()) {
    emit statusMessageUpdated(message);
  }

  return succ;
}

bool ZStackDoc::executeConnectIsolatedSwc()
{
  bool succ = false;
  QString message;

  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *branchPoint = *(selectedSwcTreeNodes()->begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);

    if (SwcTreeNode::childNumber(masterRoot) > 1 || swcList()->size() > 1) {
      ZSwcTree tree;
      tree.setDataFromNode(masterRoot);

      tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      bool isConnected = false;
      double minDist = Infinity;
      Swc_Tree_Node *closestNode = NULL;
      for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
        if (SwcTreeNode::isRegular(tn)) {
          if (SwcTreeNode::isRoot(tn)) {
            if (tn == hostRoot) {
              isConnected = false;
            } else {
              isConnected = true;
            }
          }

          if (isConnected) {
            double dist = SwcTreeNode::distance(
                  tn, branchPoint, SwcTreeNode::EUCLIDEAN_SURFACE);
            if (dist < minDist) {
              minDist = dist;
              closestNode = tn;
            }
          }
        }
      }
      tree.setDataFromNode(NULL, ZSwcTree::LEAVE_ALONE);

      foreach (ZSwcTree *buddyTree, *swcList()) {
        if (buddyTree->root() != masterRoot) {
          Swc_Tree_Node *tn = NULL;
          double dist = buddyTree->distanceTo(branchPoint, &tn);
          if (dist < minDist) {
            minDist = dist;
            closestNode = tn;
          }
        }
      }

      if (closestNode != NULL) {
        QUndoCommand *command =
            new ZStackDocCommand::SwcEdit::CompositeCommand(this);
        if (!SwcTreeNode::isRoot(closestNode)) {
          new ZStackDocCommand::SwcEdit::SetRoot(this, closestNode, command);
        }
        new ZStackDocCommand::SwcEdit::SetParent(
              this, closestNode, branchPoint, command);
        new ZStackDocCommand::SwcEdit::RemoveEmptyTree(this, command);
        pushUndoCommand(command);
        deprecateTraceMask();

        message = "Two nodes are connected.";
        succ = true;
      }
    }
  }

  if (!message.isEmpty()) {
    emit statusMessageUpdated(message);
  }
  return succ;
}

bool ZStackDoc::executeResetBranchPoint()
{
  if (selectedSwcTreeNodes()->size() == 1) {
    Swc_Tree_Node *loop = *(selectedSwcTreeNodes()->begin());
    std::vector<Swc_Tree_Node*> neighborArray = SwcTreeNode::neighborArray(loop);
    for (std::vector<Swc_Tree_Node*>::iterator iter = neighborArray.begin();
         iter != neighborArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;
      if (SwcTreeNode::isBranchPoint(tn)) {
        std::vector<Swc_Tree_Node*> candidateHookArray =
            SwcTreeNode::neighborArray(tn);
        Swc_Tree_Node *hook = NULL;
        double minDot = Infinity;
        for (std::vector<Swc_Tree_Node*>::iterator iter = candidateHookArray.begin();
             iter != candidateHookArray.end(); ++iter) {
          Swc_Tree_Node *hookCandidate = *iter;
          if (hookCandidate != loop && SwcTreeNode::isRegular(hookCandidate)) {
            double dot = SwcTreeNode::normalizedDot(hookCandidate, tn, loop);
            if (dot < minDot) {
              minDot = dot;
              hook = hookCandidate;
            }
          }
        }

        if (hook != NULL) {
          QUndoCommand *command =
              new ZStackDocCommand::SwcEdit::CompositeCommand(this);
          new ZStackDocCommand::SwcEdit::SetParent(this, hook, NULL, command);
          if (SwcTreeNode::parent(hook) != tn) {
            new ZStackDocCommand::SwcEdit::SetRoot(this, hook, command);
          }

          new ZStackDocCommand::SwcEdit::SetParent(this, hook, loop, command);
          pushUndoCommand(command);
          deprecateTraceMask();
        }
        break;
      }
    }
  }

  return false;
}

ZIntPoint ZStackDoc::getStackOffset() const
{
  if (hasStack()) {
    return stackRef()->getOffset();
  }

  return ZIntPoint(0, 0, 0);
}

void ZStackDoc::setStackOffset(int x, int y, int z)
{
  if (stackRef() != NULL) {
    stackRef()->setOffset(x, y, z);
  }
}

void ZStackDoc::setStackOffset(const ZIntPoint &offset)
{
  if (stackRef() != NULL) {
    stackRef()->setOffset(offset);
  }
}

void ZStackDoc::setStackOffset(const ZPoint &offset)
{
  if (stackRef() != NULL) {
    stackRef()->setOffset(iround(offset.x()),
                          iround(offset.y()),
                          iround(offset.z()));
  }
}

ZIntPoint ZStackDoc::getDataCoord(const ZIntPoint &pt)
{
  return pt + getStackOffset();
}

ZIntPoint ZStackDoc::getDataCoord(int x, int y, int z)
{
  return ZIntPoint(x + getStackOffset().getX(), y + getStackOffset().getY(),
                z + getStackOffset().getZ());
}

void ZStackDoc::mapToDataCoord(ZPoint *pt)
{
  if (pt != NULL) {
    pt->translate(getStackOffset().getX(), getStackOffset().getY(),
                  getStackOffset().getZ());
  }
}

void ZStackDoc::mapToDataCoord(double *x, double *y, double *z)
{
  if (x != NULL) {
    *x += getStackOffset().getX();
  }

  if (y != NULL) {
    *y += getStackOffset().getY();
  }

  if (z != NULL) {
    *z += getStackOffset().getZ();
  }
}

void ZStackDoc::mapToStackCoord(ZPoint *pt)
{
  if (pt != NULL) {
    //*pt -= getStackOffset();
    pt->translate(-getStackOffset().getX(), -getStackOffset().getY(),
                  -getStackOffset().getZ());
  }
}

void ZStackDoc::mapToStackCoord(double *x, double *y, double *z)
{
  if (x != NULL && y != NULL && z != NULL) {
    *x -= getStackOffset().getX();
    *y -= getStackOffset().getY();
    *z -= getStackOffset().getZ();
  }
}

void ZStackDoc::setSparseStack(ZSparseStack *spStack)
{
  if (m_sparseStack != NULL) {
    delete m_sparseStack;
  }
  m_sparseStack = spStack;

  if (spStack != NULL) {
    if (m_stack != NULL) {
      deprecate(STACK);
    }

    m_stack = ZStackFactory::makeVirtualStack(spStack->getBoundBox());
    notifyStackModified();
  }

  notifySparseStackModified();
}

void ZStackDoc::addData(const ZStackDocReader &reader)
{
  if (!reader.getSwcList().isEmpty()) {
    addSwcTree(reader.getSwcList());
    notifySwcModified();
  }

  if (reader.getStack() != NULL) {
    loadStack(reader.getStack());
    setStackSource(reader.getStackSource());
    initNeuronTracer();
    notifyStackModified();
  }

  if (reader.getSparseStack() != NULL) {
    setSparseStack(reader.getSparseStack());
  }

  if (!reader.getChainList().isEmpty()) {
    addLocsegChain(reader.getChainList());
    notifyChainModified();
  }

  if (!reader.getPunctaList().isEmpty()) {
    addPunctum(reader.getPunctaList());
    notifyPunctumModified();
  }

  if (!reader.getSparseObjectList().isEmpty()) {
    addSparseObject(reader.getSparseObjectList());
    notifySparseObjectModified();
  }
}


std::vector<ZStack*> ZStackDoc::createWatershedMask()
{
  std::vector<ZStack*> maskArray;
#if 0
  foreach (ZStroke2d* stroke, m_strokeList) {
    if (!stroke->isEmpty()) {
      maskArray.push_back(stroke->toStack());
    }
  }
#endif
  for (ZDocPlayerList::const_iterator iter = m_playerList.begin();
       iter != m_playerList.end(); ++iter) {
    const ZDocPlayer *player = *iter;
    if (player->hasRole(ZDocPlayer::ROLE_SEED)) {
      ZStack *stack = player->toStack();
      if (stack != NULL) {
        maskArray.push_back(stack);
      }
    }
  }

  return maskArray;
}

void ZStackDoc::localSeededWatershed()
{
  removeObject(ZDocPlayer::ROLE_TMP_RESULT, true);

  if (!m_strokeList.isEmpty()) {
    ZStackWatershed engine;
    ZStackArray seedMask = createWatershedMask();

    ZStack *signalStack = m_stack;
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      if (m_sparseStack != NULL) {
        signalStack = m_sparseStack->getStack();
        dsIntv = m_sparseStack->getDownsampleInterval();
      }
    }

    if (signalStack != NULL) {
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

      advanceProgress(0.1);
      QApplication::processEvents();

      Cuboid_I box;
      seedMask.getBoundBox(&box);
      const int xMargin = 10;
      const int yMargin = 10;
      const int zMargin = 20;
      Cuboid_I_Expand_X(&box, xMargin);
      Cuboid_I_Expand_Y(&box, yMargin);
      Cuboid_I_Expand_Z(&box, zMargin);

      engine.setRange(box);
      ZStack *out = engine.run(signalStack, seedMask);

      advanceProgress(0.1);
      QApplication::processEvents();

      Object_3d *objData = Stack_Region_Border(out->c_stack(), 6, TRUE);

      advanceProgress(0.1);
      QApplication::processEvents();

      if (objData != NULL) {
        ZObject3d *obj = new ZObject3d(objData);
        /*
      obj->translate(iround(getStackOffset().x()),
                     iround(getStackOffset().y()),
                     iround(getStackOffset().z()));
                     */

        obj->translate(out->getOffset());
        if (dsIntv.getX() > 0 || dsIntv.getY() > 0 || dsIntv.getZ() > 0) {
          obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
        }

        /*
      obj->translate(iround(out->getOffset().getX()),
                     iround(out->getOffset().getY()),
                     iround(out->getOffset().getZ()));
                     */
        obj->setColor(255, 255, 0, 180);

        addObj3d(obj);
        addPlayer(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_TMP_RESULT);
        notifyObj3dModified();
      }

      // C_Stack::kill(out);
      delete out;
    }
  }
}

void ZStackDoc::seededWatershed()
{
  removeObject(ZDocPlayer::ROLE_TMP_RESULT, true);
  //removeAllObj3d();
  ZStackWatershed engine;
  ZStackArray seedMask = createWatershedMask();

  if (!seedMask.empty()) {
    ZStack *signalStack = m_stack;
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      if (m_sparseStack != NULL) {
        signalStack = m_sparseStack->getStack();
        dsIntv = m_sparseStack->getDownsampleInterval();
      }
    }

    if (signalStack != NULL) {
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
      ZStack *out = engine.run(signalStack, seedMask);

#ifdef _DEBUG_2
      out->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

      Object_3d *objData = Stack_Region_Border(out->c_stack(), 6, TRUE);

      if (objData != NULL) {
        ZObject3d *obj = new ZObject3d(objData);

        obj->translate(out->getOffset());


        if (dsIntv.getX() > 0 || dsIntv.getY() > 0 || dsIntv.getZ() > 0) {
          obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
        }

        /*
      obj->translate(iround(out->getOffset().getX()),
                     iround(out->getOffset().getY()),
                     iround(out->getOffset().getZ()));
                     */
        obj->setColor(255, 255, 0, 255);

        addObj3d(obj);
        addPlayer(obj, NeuTube::Documentable_OBJ3D, ZDocPlayer::ROLE_TMP_RESULT);

        notifyObj3dModified();
      }
      //delete out;

      setLabelField(out);
    }
  }
}

void ZStackDoc::runLocalSeededWatershed()
{
  startProgress();
  QApplication::processEvents();

  localSeededWatershed();

  //QtConcurrent::run(this, &ZStackDoc::localSeededWatershed); //crashed for unknown reason

  endProgress();
}

void ZStackDoc::runSeededWatershed()
{
#if 1
  seededWatershed();
  emit labelFieldModified();
#else //old code
  Stack *stack = m_stack->c_stack();
  Stack_Watershed_Workspace *ws =
      Make_Stack_Watershed_Workspace(stack);
  ws->conn = 6;
  Stack *mask = C_Stack::make(GREY, C_Stack::width(stack),
                              C_Stack::height(stack), C_Stack::depth(stack));
  C_Stack::setZero(mask);
  ws->mask = mask;

  /*
  ZStackArray stackArray = createWatershedMask();
  Mc_Stack maskView;
  C_Stack::view(ws->mask, &maskView);
  ZStack maskWrapper(&maskView, NULL);
  maskWrapper.setOffset(m_stack->getOffset());
  stackArray.paste(&maskWrapper, 0);
*/

  foreach (ZStroke2d* stroke, m_strokeList) {
    ZPoint stackOffset = getStackOffset();
    ZStroke2d tmpStroke = *stroke;
    tmpStroke.translate(-stackOffset);
    tmpStroke.labelGrey(mask);
  }

  size_t voxelNumber = m_stack->getVoxelNumber();
  for (size_t i = 0; i < voxelNumber; ++i) {
    if (stack->array[i] == 0) {
      mask->array[i] = STACK_WATERSHED_BARRIER;
    }
  }
#if 0
  if (m_stack->isBinary()) {
    size_t voxelNumber = m_stack->getVoxelNumber();
    for (size_t i = 0; i < voxelNumber; ++i) {
      if (stack->array[i] == 0) {
        mask->array[i] = STACK_WATERSHED_BARRIER;
      }
    }
    stack = Stack_Bwdist_L_U16P(stack, NULL, 0);
  }
#endif

  Stack *out= Stack_Watershed(stack, ws);

  if (stack != m_stack->c_stack()) {
    C_Stack::kill(stack);
  }

  Object_3d *objData = Stack_Region_Border(out, 6, TRUE);
  removeAllObj3d();
  if (objData != NULL) {
    ZObject3d *obj = new ZObject3d(objData);
    obj->translate(iround(getStackOffset().x()),
                   iround(getStackOffset().y()),
                   iround(getStackOffset().z()));
    addObj3d(obj);
    notifyObj3dModified();
  }

  C_Stack::kill(out);
#ifdef _DEBUG_2
  C_Stack::write(GET_DATA_DIR + "/test.tif", ws->mask);
#endif
  //return out;

#endif

}

ZStack* ZStackDoc::makeLabelStack(ZStack *stack) const
{
  ZStack *out = NULL;

  const ZStack *signalStack = getStack();
  if (signalStack->isVirtual()) {
    if (hasSparseStack()) {
      signalStack = getSparseStack()->getStack();
    }
  }

  TZ_ASSERT(signalStack->kind() == GREY, "Only GREY kind is supported.");

  const ZStack* labelField = getLabelField();

  out = new ZStack(signalStack->kind(), signalStack->getBoundBox(), 3);
  out->setZero();

  if (!signalStack->isVirtual()) {
    C_Stack::copyChannelValue(out->data(), 0, signalStack->c_stack());
    C_Stack::copyChannelValue(out->data(), 1, signalStack->c_stack());
    C_Stack::copyChannelValue(out->data(), 2, signalStack->c_stack());
  }

  if (labelField != NULL) {
    size_t voxelNumber = out->getVoxelNumber();
    /*
    Stack ch1;
    Stack ch2;
    C_Stack::view(out->data(), &ch1, 0);
    C_Stack::view(out->data(), &ch2, 1);
    */
    const uint8_t *labelArray = labelField->array8();
    uint8_t *ch1Array = out->array8(0);
    uint8_t *ch2Array = out->array8(1);
    uint8_t *ch3Array = out->array8(2);

    for (size_t i = 0; i < voxelNumber; ++i) {
      if (labelArray[i] == 1) {
        ch2Array[i] = 0;
        ch3Array[i] = 0;
      } else if (labelArray[i] == 2) {
        ch1Array[i] = 0;
        ch3Array[i] = 0;
      } else if (labelArray[i] == 3) {
        ch1Array[i] = 0;
        ch2Array[i] = 0;
      }
    }
  } else {
    emit statusMessageUpdated("No label field.");
  }

  if (stack == NULL) {
    stack = out;
  } else {
    stack->consume(out);
  }

  return stack;
}

void ZStackDoc::setLabelField(ZStack *stack)
{
  if (m_labelField != NULL) {
    delete m_labelField;
  }

  m_labelField = stack;
}

void ZStackDoc::setStackFactory(ZStackFactory *factory)
{
  delete m_stackFactory;

  m_stackFactory = factory;
}

QList<const ZDocPlayer*> ZStackDoc::getPlayerList(ZDocPlayer::TRole role) const
{

  return m_playerList.getPlayerList(role);
  /*
  QList<const ZDocPlayer*> playerList;
  for (ZDocPlayerList::const_iterator iter = m_playerList.begin();
       iter != m_playerList.end(); ++iter) {
    const ZDocPlayer *player = *iter;
    if (player->hasRole(role)) {
      playerList.append(player);
    }
  }

  return playerList;
  */
}

bool ZStackDoc::hasPlayer(ZDocPlayer::TRole role) const
{
  return m_playerList.hasPlayer(role);
}

Z3DGraph ZStackDoc::get3DGraphDecoration() const
{
  Z3DGraph graph;
  QList<const ZDocPlayer *> playerList =
      getPlayerList(ZDocPlayer::ROLE_3DGRAPH_DECORATOR);
  foreach(const ZDocPlayer *player, playerList) {
    graph.append(player->get3DGraph());
  }

  return graph;
}

///////////Stack Reader///////////

ZStackDocReader::ZStackDocReader() : m_stack(NULL), m_sparseStack(NULL),
  m_swcNetwork(NULL)
{

}

bool ZStackDocReader::readFile(const QString &filePath)
{
  m_filePath = filePath;

  switch (ZFileType::fileType(filePath.toStdString())) {
  case ZFileType::SWC_FILE:
    loadSwc(filePath);
    break;
  case ZFileType::LOCSEG_CHAIN_FILE:
    loadLocsegChain(filePath);
    break;
  case ZFileType::SWC_NETWORK_FILE:
    loadSwcNetwork(filePath);
    break;
  case ZFileType::OBJECT_SCAN_FILE:
  case ZFileType::TIFF_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
    loadStack(filePath);
    break;
    /*
  case ZFileType::FLYEM_NETWORK_FILE:
    importFlyEmNetwork(filePath.toStdString().c_str());
    break;
  case ZFileType::SYNAPSE_ANNOTATON_FILE:
    importSynapseAnnotation(filePath.toStdString());
    break;
    */
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
  case ZFileType::RAVELER_BOOKMARK:
    loadPuncta(filePath);
    break;
  default:
    return false;
    break;
  }

  return true;
}

void ZStackDocReader::loadSwc(const QString &filePath)
{
  ZSwcTree *tree = new ZSwcTree();
  tree->load(filePath.toLocal8Bit().constData());
  if (!tree->isEmpty()) {
    addSwcTree(tree);
  } else {
    delete tree;
  }
}

void ZStackDocReader::addSwcTree(ZSwcTree *tree)
{
  if (tree != NULL) {
    m_swcList.append(tree);
  }
}

void ZStackDocReader::loadLocsegChain(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    ZLocsegChain *chain = new ZLocsegChain();
    chain->load(filePath.toLocal8Bit().constData());
    if (!chain->isEmpty()) {
      addLocsegChain(chain);
    } else {
      delete chain;
    }
  }
}

void ZStackDocReader::addLocsegChain(ZLocsegChain *chain)
{
  if (chain != NULL) {
    m_chainList.append(chain);
  }
}

void ZStackDocReader::loadStack(const QString &filePath)
{
  if (ZFileType::fileType(filePath.toStdString()) == ZFileType::OBJECT_SCAN_FILE) {
    ZSparseObject *sobj = new ZSparseObject;
    sobj->load(filePath.toStdString().c_str());
    if (!sobj->isEmpty()) {
      addSparseObject(sobj);
      sobj->setColor(128, 0, 0, 255);

      ZIntCuboid cuboid = sobj->getBoundBox();
      m_stack = ZStackFactory::makeVirtualStack(
            cuboid.getWidth(), cuboid.getHeight(), cuboid.getDepth());
      m_stack->setOffset(cuboid.getFirstCorner());
    }
  } else {
    m_stackSource.import(filePath.toStdString());
    m_stack = m_stackSource.readStack();
  }
}

void ZStackDocReader::clear()
{
  m_stack = NULL;
  m_sparseStack = NULL;
  m_swcList.clear();
  m_stackSource.clear();
  m_swcList.clear();
  m_punctaList.clear();
  m_strokeList.clear();
  m_obj3dList.clear();
  m_sparseObjectList.clear();
}

void ZStackDocReader::loadSwcNetwork(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    if (m_swcNetwork == NULL) {
      m_swcNetwork = new ZSwcNetwork;
    }

    m_swcNetwork->importTxtFile(filePath.toStdString());

    for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
      addSwcTree(m_swcNetwork->getTree(i));
    }
  }
}

void ZStackDocReader::loadPuncta(const QString &filePath)
{
  if (!filePath.isEmpty()) {
    QList<ZPunctum*> plist = ZPunctumIO::load(filePath);
    foreach (ZPunctum* punctum, plist) {
      addPunctum(punctum);
    }
  }
}

void ZStackDocReader::addPunctum(ZPunctum *p)
{
  if (p != NULL) {
    m_punctaList.append(p);
  }
}

void ZStackDocReader::addStroke(ZStroke2d *stroke)
{
  if (stroke != NULL) {
    m_strokeList.append(stroke);
  }
}

void ZStackDocReader::addSparseObject(ZSparseObject *obj)
{
  if (obj != NULL) {
    m_sparseObjectList.append(obj);
  }
}

void ZStackDocReader::setSparseStack(ZSparseStack *spStack)
{
  m_sparseStack = spStack;
}

void ZStackDocReader::setStack(ZStack *stack)
{
  m_stack = stack;
}

void ZStackDocReader::setStackSource(const ZStackFile &stackFile)
{
  m_stackSource = stackFile;
}

bool ZStackDocReader::hasData() const
{
  if (getStack() != NULL) {
    return true;
  }

  if (m_sparseStack != NULL) {
    return true;
  }

  if (!getSwcList().isEmpty()) {
    return true;
  }

  if (!getPunctaList().isEmpty()) {
    return true;
  }

  if (!getStrokeList().isEmpty()) {
    return true;
  }

  if (!getObjectList().isEmpty()) {
    return true;
  }

  if (!getChainList().isEmpty()) {
    return true;
  }

  return false;
}
