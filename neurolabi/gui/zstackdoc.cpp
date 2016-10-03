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
#include <QMutexLocker>

#include "QsLog.h"

#include "dialogs/informationdialog.h"
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
#include "zerror.h"
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
#include "dialogs/swcskeletontransformdialog.h"
#include "dialogs/swcsizedialog.h"
#include "tz_stack_watershed.h"
#include "zstackwatershed.h"
#include "zstackarray.h"
#include "zstackfactory.h"
#include "zsparseobject.h"
#include "zsparsestack.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackobjectsourcefactory.h"
#include "zstackpatch.h"
#include "zobjectcolorscheme.h"
#include "zstackdocreader.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackmvc.h"
#include "dvid/zdvidsparsestack.h"
#include "zprogresssignal.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidsparsevolslice.h"
#include "zwidgetmessage.h"
#include "swc/zswcresampler.h"
#include "zswcforest.h"
#include "swc/zswcsignalfitter.h"
#include "zgraphobjsmodel.h"
#include "zsurfaceobjsmodel.h"

using namespace std;

ZStackDoc::ZStackDoc(QObject *parent) : QObject(parent)
{
  init();
}

ZStackDoc::~ZStackDoc()
{
  if (m_futureMap.hasThreadAlive()) {
    m_futureMap.waitForFinished();
  }

  deprecate(STACK);
  deprecate(SPARSE_STACK);

  ZOUT(LTRACE(), 5) << "ZStackDoc destroyed: " << this;

  m_objectGroup.removeAllObject(true);

  if (m_swcNetwork != NULL) {
    delete m_swcNetwork;
  }

  delete m_undoStack;
  delete m_labelField;
  delete m_stackFactory;
  delete m_actionFactory;

  if (m_resDlg != NULL) {
    delete m_resDlg;
  }

  destroyReporter();
}

void ZStackDoc::init()
{
  m_resDlg = NULL;
  m_selectionSilent = false;
  m_isReadyForPaint = true;
  m_isSegmentationReady = false;
  m_changingSaveState = true;
  m_autoSaving = true;

  m_stack = NULL;
  m_sparseStack = NULL;
  m_labelField = NULL;
  m_parentFrame = NULL;
  //m_masterChain = NULL;
  m_isTraceMaskObsolete = true;
  m_swcNetwork = NULL;
  m_stackFactory = NULL;

  m_actionFactory = new ZActionFactory;

  initNeuronTracer();
  m_swcObjsModel = new ZSwcObjsModel(this, this);
  m_swcNodeObjsModel = new ZSwcNodeObjsModel(this, this);
  m_punctaObjsModel = new ZPunctaObjsModel(this, this);
  m_seedObjsModel = new ZDocPlayerObjsModel(
        this, ZStackObjectRole::ROLE_SEED, this);
  m_graphObjsModel = new ZGraphObjsModel(this, this);
  m_surfaceObjsModel = new ZSurfaceObjsModel(this, this);
  m_undoStack = new QUndoStack(this);

//  m_undoAction = NULL;
//  m_redoAction = NULL;

  qRegisterMetaType<QSet<ZStackObject::ETarget> >("QSet<ZStackObject::ETarget>");
  connectSignalSlot();

  //setReporter(new ZQtMessageReporter());

  if (NeutubeConfig::getInstance().isAutoSaveEnabled()) {
    QTimer *timer = new QTimer(this);
    timer->start(NeutubeConfig::getInstance().getAutoSaveInterval());
    connect(timer, SIGNAL(timeout()), this, SLOT(autoSaveSlot()));
  }

//  createActions();

  setTag(NeuTube::Document::NORMAL);
  setStackBackground(NeuTube::IMAGE_BACKGROUND_DARK);

  m_objColorSheme.setColorScheme(ZColorScheme::RANDOM_COLOR);

  m_progressSignal = new ZProgressSignal(this);
}

void ZStackDoc::clearData()
{
  deprecate(STACK);
  deprecate(SPARSE_STACK);
  m_objectGroup.removeAllObject(true);
  m_playerList.clear();

  delete m_swcNetwork;
  m_swcNetwork = NULL;

  delete m_labelField;
  m_labelField = NULL;
  delete m_stackFactory;
  m_stackFactory = NULL;

  /* workspaces */
  m_isTraceMaskObsolete = true;
  m_neuronTracer.clear();

  //Meta information
  m_stackSource.clear();
  m_additionalSource.clear();

  //Thread
  m_reader.clear();

  //Actions
  //  Undo/Redo
  if (m_undoStack != NULL) {
    m_undoStack->clear();
  }

  setSegmentationReady(false);
//  m_isSegmentationReady = false;
}

void ZStackDoc::initNeuronTracer()
{
  m_neuronTracer.initTraceWorkspace(getStack());
  m_neuronTracer.initConnectionTestWorkspace();
//  m_neuronTracer.getConnectionTestWorkspace()->sp_test = 1;
  if (getStack() != NULL) {
    if (getTag() == NeuTube::Document::BIOCYTIN_STACK &&
        getStack()->channelNumber() > 1) {
      m_neuronTracer.setSignalChannel(1);
//      m_neuronTracer.setIntensityField(getStack()->c_stack(1));
    } else {
//      m_neuronTracer.setIntensityField(getStack()->c_stack());
    }
    m_neuronTracer.setIntensityField(getStack());
  }
  m_neuronTracer.setBackgroundType(getStackBackground());
  if (getTag() == NeuTube::Document::FLYEM_BODY) {
    m_neuronTracer.setVertexOption(ZStackGraph::VO_SURFACE);
  }

  m_neuronTracer.setResolution(getResolution().voxelSizeX(),
                               getResolution().voxelSizeY(),
                               getResolution().voxelSizeZ());

  /*
  if (GET_APPLICATION_NAME == "Biocytin") {
    m_neuronTracer.setResolution(1, 1, 10);
  }
  */

//  ZIntPoint offset = getStackOffset();
//  m_neuronTracer.setStackOffset(offset.getX(), offset.getY(), offset.getZ());
}

/*
void ZStackDoc::setParentFrame(ZStackFrame *parent)
{
  m_parentFrame = parent;
}
*/
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

void ZStackDoc::disconnectSwcNodeModelUpdate()
{
  disconnect(this, SIGNAL(swcModified()),
             m_swcNodeObjsModel, SLOT(updateModelData()));
}


void ZStackDoc::disconnectPunctaModelUpdate()
{
  disconnect(this, SIGNAL(punctaModified()),
             m_punctaObjsModel, SLOT(updateModelData()));
}

void ZStackDoc::connectSignalSlot()
{
  connect(this, SIGNAL(swcModified()), m_swcObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(swcModified()), m_swcNodeObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(punctaModified()), m_punctaObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(seedModified()), m_seedObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(graph3dModified()), m_graphObjsModel, SLOT(updateModelData()));
  connect(this, SIGNAL(cube3dModified()), m_surfaceObjsModel, SLOT(updateModelData()));

  connect(this, SIGNAL(addingObject(ZStackObject*,bool)),
          this, SLOT(addObject(ZStackObject*,bool)));
  connect(this, SIGNAL(addingObject(ZStackObject*)),
          this, SLOT(addObject(ZStackObject*)));
  /*
  connect(this, SIGNAL(chainModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(punctaModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(obj3dModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(object3dScanModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(stackPatchModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(sparseObjectModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(strokeModified()), this, SIGNAL(objectModified()));
  connect(this, SIGNAL(swcModified()), this, SIGNAL(objectModified()));
  */

  /*
  connect(m_undoStack, SIGNAL(cleanChanged(bool)),
          this, SIGNAL(cleanChanged(bool)));
          */

  connect(&m_reader, SIGNAL(finished()), this, SIGNAL(stackReadDone()));
  connect(this, SIGNAL(stackReadDone()), this, SLOT(loadReaderResult()));
  connect(this, SIGNAL(stackModified()), this, SIGNAL(volumeModified()));

  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgressSlot(double)));
  connect(this, SIGNAL(progressStarted()), this, SLOT(startProgressSlot()));
  connect(this, SIGNAL(progressEnded()), this, SLOT(endProgressSlot()));

//  connect(this, SIGNAL(newDocReady(ZStackDocReader)),
//          this, SLOT(reloadData(ZStackDocReader)));
}

void ZStackDoc::advanceProgressSlot(double dp)
{
  advanceProgress(dp);
}

void ZStackDoc::startProgressSlot()
{
  startProgress();
}

void ZStackDoc::endProgressSlot()
{
  endProgress();
}

void ZStackDoc::notifyProgressStart()
{
  emit startProgress();
}

void ZStackDoc::notifyProgressEnd()
{
  emit endProgress();
}

void ZStackDoc::notifyProgressAdvanced(double dp)
{
  emit progressAdvanced(dp);
}


void ZStackDoc::updateSwcNodeAction()
{
  m_singleSwcNodeActionActivator.update(this);
}

void ZStackDoc::autoSave()
{
  if (isSwcSavingRequired()) {
    if (getTag() == NeuTube::Document::FLYEM_BODY_DISPLAY) {
      return;
    }
    /*
    if (NeutubeConfig::getInstance().getApplication() == "FlyEM") { //needs modification
      return;
    }
    */

    ZOUT(LTRACE(), 5) << "Auto save triggered in" << this;
    if (hasSwc()) {
      std::string autoSaveDir = NeutubeConfig::getInstance().getPath(
            NeutubeConfig::AUTO_SAVE);
      QDir dir(autoSaveDir.c_str());
      if (dir.exists()) {
        ostringstream stream;
        stream << this;
        std::string autoSavePath =
            autoSaveDir + ZString::FileSeparator;
        if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
          if (getStack() != NULL) {
            autoSavePath +=
                ZBiocytinFileNameParser::getCoreName(getStack()->sourcePath()) +
                ".autosave.swc";
          }
        } else {
          autoSavePath += "~" + stream.str() + ".swc";
        }

        FILE *fp = fopen(autoSavePath.c_str(), "w");
        if (fp != NULL) {
          fclose(fp);
          ZSwcTree *tree = new ZSwcTree;
          QList<ZSwcTree*> swcList = getSwcList();
          foreach (ZSwcTree *treeItem, swcList) {
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

void ZStackDoc::autoSaveSlot()
{
  LINFO() << "Autosave triggered.";
  if (m_autoSaving) {
    autoSave();
  }
}

void ZStackDoc::customNotifyObjectModified(ZStackObject::EType /*type*/)
{

}

string ZStackDoc::getSwcSource() const
{
  string swcSource;
  ZOUT(LTRACE(), 5) << "Get SWC source";
  const TStackObjectList &swcSet =
      getObjectList(ZStackObject::TYPE_SWC);
  if (swcSet.size() == 1) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*swcSet.begin());
    swcSource = tree->getSource();
  }

  ZOUT(LTRACE(), 5) << "Source obtained: " << swcSource;

  return swcSource;
}

ZSwcTree* ZStackDoc::getMergedSwc()
{
  ZSwcTree *tree = NULL;

  QList<ZSwcTree*> swcList = getSwcList();
  if (!swcList.empty()) {
    tree = new ZSwcTree;
    foreach (ZSwcTree *treeItem, swcList) {
      tree->merge(Copy_Swc_Tree(treeItem->data()), true);
    }
    tree->resortId();
  }

  return tree;
}

bool ZStackDoc::saveSwc(const string &filePath)
{
  QList<ZSwcTree*> swcList = getSwcList();
  if (!swcList.empty()) {
    ZSwcTree *tree = NULL;
    if (swcList.size() > 1) {
      tree = new ZSwcTree;
      foreach (ZSwcTree *treeItem, swcList) {
        tree->merge(Copy_Swc_Tree(treeItem->data()), true);
      }
      ZUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      foreach (ZSwcTree* oldTree, swcList) {
        new ZStackDocCommand::SwcEdit::RemoveSwc(this, oldTree, command);
      }
      new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

      command->setLogMessage("Save SWC to " + filePath);
      pushUndoCommand(command);
    } else {
      tree = swcList.front();
    }
//    tree->resortId();
    tree->save(filePath.c_str());
    tree->setSource(filePath);
    qDebug() << filePath.c_str();

    setSaved(ZStackObject::TYPE_SWC, true);

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

bool ZStackDoc::hasObject() const
{
  return !m_objectGroup.isEmpty();
}

bool ZStackDoc::hasObject(ZStackObjectRole::TRole role) const
{
  return m_playerList.hasPlayer(role);
}

bool ZStackDoc::hasObject(ZStackObject::EType type) const
{
  ZOUT(LTRACE(), 5) << "Has object?";
  return !m_objectGroup.getObjectList(type).isEmpty();
}

bool ZStackDoc::hasObject(ZStackObject::EType type, const string &source) const
{
    return m_objectGroup.findFirstSameSource(type, source) != NULL;
}

ZStackObject* ZStackDoc::getObject(ZStackObject::EType type, const std::string &source) const
{
    return m_objectGroup.findFirstSameSource(type, source);
}

bool ZStackDoc::hasSparseObject() const
{
  ZOUT(LTRACE(), 5) << "Has sparse object?";
  return !m_objectGroup.getObjectList(ZStackObject::TYPE_SPARSE_OBJECT).isEmpty();
}

bool ZStackDoc::hasSparseStack() const
{
  return getSparseStack() != NULL;
}

bool ZStackDoc::hasVisibleSparseStack() const
{
  return hasSparseStack();
}

const ZSparseStack* ZStackDoc::getSparseStack() const
{
  return m_sparseStack;
}

const ZSparseStack* ZStackDoc::getConstSparseStack() const
{
  return getSparseStack();
}

ZSparseStack* ZStackDoc::getSparseStack()
{
  return m_sparseStack;
}

ZObject3dScan* ZStackDoc::getSparseStackMask() const
{
  if (getConstSparseStack() != NULL) {
    return const_cast<ZObject3dScan*>(getConstSparseStack()->getObjectMask());
  }

  return NULL;
}

bool ZStackDoc::hasSwc() const
{
  ZOUT(LTRACE(), 5) << "Has swc?";
  return !m_objectGroup.getObjectList(ZStackObject::TYPE_SWC).isEmpty();
}

bool ZStackDoc::hasPuncta() const
{
  ZOUT(LTRACE(), 5) << "Has puncta?";
  return !getObjectList(ZStackObject::TYPE_PUNCTUM).isEmpty();
}


std::string ZStackDoc::stackSourcePath() const
{
  if (hasStack()) {
    return getStack()->sourcePath();
  }

  return "";
}

bool ZStackDoc::hasChainList()
{
  return !m_objectGroup.getObjectList(ZStackObject::TYPE_LOCSEG_CHAIN).isEmpty();
}

bool ZStackDoc::isUndoClean() const
{
  return m_undoStack->isClean();
}

const ZUndoCommand* ZStackDoc::getLastUndoCommand() const
{
  if (m_undoStack->isClean()) {
    return NULL;
  }

  const QUndoCommand *command = m_undoStack->command(m_undoStack->index() - 1);

  qDebug() << command->text();

  return dynamic_cast<const ZUndoCommand*>(command);
}

bool ZStackDoc::isSaved(ZStackObject::EType type) const
{
  return !m_unsavedSet.contains(type);
}

void ZStackDoc::setSaved(ZStackObject::EType type, bool state)
{
  if (m_changingSaveState) {
    if (state == true) {
      m_unsavedSet.remove(type);
    } else {
      m_unsavedSet.insert(type);
    }

    emit cleanChanged(isSwcSavingRequired());
  }

#if 0
  ZUndoCommand* command = const_cast<ZUndoCommand*>(getLastUndoCommand());
  if (command != NULL) {
    switch (type) {
    case
      command->setSaved(type, state);
      break;
    default:
      break;
    }
  }
#endif

}

bool ZStackDoc::isSavingRequired() const
{
  return !m_unsavedSet.empty();
}

bool ZStackDoc::isSwcSavingRequired() const
{
//  qDebug() << m_swcList.empty();
//  qDebug() << isUndoClean();

#if 0
  bool isSaved = true;
  if (m_undoStack->canUndo()) {
    const ZUndoCommand* command = getLastUndoCommand();
    if (command != NULL) {
      isSaved = command->isSaved(NeuTube::Documentable_SWC);
    }
  }
#endif

  return hasSwc() && !isSaved(ZStackObject::TYPE_SWC);
}

void ZStackDoc::swcTreeTranslateRootTo(double x, double y, double z)
{
  QList<ZSwcTree*> swcList = getSwcList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < swcList.size(); i++) {
    swcList[i]->translateRootTo(x, y, z);
    processObjectModified(swcList[i]);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::swcTreeRescale(double scaleX, double scaleY, double scaleZ)
{
  QList<ZSwcTree*> swcList = getSwcList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < swcList.size(); i++) {
    swcList[i]->rescale(scaleX, scaleY, scaleZ);
    processObjectModified(swcList[i]);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
                               double dstPixelPerUmXY, double dstPixelPerUmZ)
{
  QList<ZSwcTree*> swcList = getSwcList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < swcList.size(); i++) {
    swcList[i]->rescale(srcPixelPerUmXY, srcPixelPerUmZ,
                        dstPixelPerUmXY, dstPixelPerUmZ);
    processObjectModified(swcList[i]);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::swcTreeRescaleRadius(double scale, int startdepth, int enddepth)
{
  QList<ZSwcTree*> swcList = getSwcList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < swcList.size(); i++) {
    swcList[i]->rescaleRadius(scale, startdepth, enddepth);
    processObjectModified(swcList[i]);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::swcTreeReduceNodeNumber(double lengthThre)
{
  QList<ZSwcTree*> swcList = getSwcList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < swcList.size(); i++) {
    swcList[i]->reduceNodeNumber(lengthThre);
    processObjectModified(swcList[i]);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::addSizeForSelectedSwcNode(double dr)
{
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    static const double minRadius = 0.5;
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      double newRadius = SwcTreeNode::radius(*iter) + dr;
      if (newRadius < minRadius) {
        newRadius = minRadius;
      }
      SwcTreeNode::setRadius(*iter, newRadius);
    }

    processSwcModified();

//    emit swcModified();
  }
}

void ZStackDoc::selectSwcNodeFloodFilling(Swc_Tree_Node *lastSelectedNode)
{
  if (lastSelectedNode != NULL) {
    ZSwcTree *tree = nodeToSwcTree(lastSelectedNode);
    std::set<Swc_Tree_Node*> oldSelected = tree->getSelectedNode();
    tree->selectNodeFloodFilling(lastSelectedNode);
    std::set<Swc_Tree_Node*> newSelected = tree->getSelectedNode();
    notifySelectionAdded(oldSelected, newSelected);
  }
}

void ZStackDoc::selectHitSwcTreeNodeFloodFilling(ZSwcTree *tree)
{
  if (tree == NULL) {
    return;
  }

  if (tree->getHitNode() == NULL) {
    return;
  }

  std::set<Swc_Tree_Node*> oldSelected = tree->getSelectedNode();

  tree->selectHitNodeFloodFilling();

  const std::set<Swc_Tree_Node*> &newSelected = tree->getSelectedNode();

  notifySelectionAdded(oldSelected, newSelected);
}

void ZStackDoc::notifySelectionChanged(
    const std::set<ZStackObject *> &selected,
    const std::set<ZStackObject *> &deselected)
{
  QList<ZStackObject*> selectedList;
  QList<ZStackObject*> deselectedList;

  for (std::set<ZStackObject *>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    selectedList.append(const_cast<ZStackObject*>(*iter));
  }

  for (std::set<ZStackObject *>::const_iterator iter = deselected.begin();
       iter != deselected.end(); ++iter) {
    deselectedList.append(const_cast<ZStackObject*>(*iter));
  }

  notifySelectionChanged(selectedList, deselectedList);
}

void ZStackDoc::notifySelectionChanged(
    const std::set<const ZStackObject *> &selected,
    const std::set<const ZStackObject *> &deselected)
{
  QList<ZStackObject*> selectedList;
  QList<ZStackObject*> deselectedList;

  for (std::set<const ZStackObject *>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    selectedList.append(const_cast<ZStackObject*>(*iter));
  }

  for (std::set<const ZStackObject *>::const_iterator iter = deselected.begin();
       iter != deselected.end(); ++iter) {
    deselectedList.append(const_cast<ZStackObject*>(*iter));
  }

//  emit objectSelectionChanged(selectedList, deselectedList);
  notifySelectionChanged(selectedList, deselectedList);
}

#define DEFINE_NOTIFY_SELECTION_CHANGED(Type, Signal) \
  void ZStackDoc::notifySelectionChanged(\
     const QList<Type *> &selected, \
     const QList<Type *> &deselected) \
  {\
    if (!isSelectionSlient()) {\
      if (!selected.empty() || !deselected.empty()) {\
        std::cout << "Object selelection changed." << std::endl; \
        emit Signal(selected, deselected);\
      }\
    }\
  }

DEFINE_NOTIFY_SELECTION_CHANGED(Swc_Tree_Node, swcTreeNodeSelectionChanged)
DEFINE_NOTIFY_SELECTION_CHANGED(ZSwcTree, swcSelectionChanged)
DEFINE_NOTIFY_SELECTION_CHANGED(ZPunctum, punctaSelectionChanged)
DEFINE_NOTIFY_SELECTION_CHANGED(ZLocsegChain, chainSelectionChanged)
DEFINE_NOTIFY_SELECTION_CHANGED(ZStackObject, objectSelectionChanged)

void ZStackDoc::selectHitSwcTreeNodeConnection(ZSwcTree *tree)
{
  if (tree == NULL) {
    return;
  }

  std::set<Swc_Tree_Node*> oldSelected = tree->getSelectedNode();

  tree->selectHitNodeConnection();

  const std::set<Swc_Tree_Node*> &newSelected = tree->getSelectedNode();

  notifySelectionAdded(oldSelected, newSelected);

  //setSwcTreeNodeSelected(newSelectedSet.begin(), newSelectedSet.end(), true);
}

void ZStackDoc::selectSwcNodeConnection(Swc_Tree_Node *lastSelectedNode)
{
  if (lastSelectedNode != NULL) {
    ZSwcTree *tree = nodeToSwcTree(lastSelectedNode);
    std::set<Swc_Tree_Node*> oldSelected = tree->getSelectedNode();
    tree->selectNodeConnection(lastSelectedNode);
    std::set<Swc_Tree_Node*> newSelected = tree->getSelectedNode();
    notifySelectionAdded(oldSelected, newSelected);
  }
}

void ZStackDoc::inverseSwcNodeSelection()
{
  ZOUT(LTRACE(), 5) << "Invert swc selection";
  QList<Swc_Tree_Node*> oldSelected = getSelectedSwcNodeList();

  QList<ZSwcTree*> treeList = getObjectList<ZSwcTree>();

  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    tree->inverseSelection();
  }

  QList<Swc_Tree_Node*> newSelected = getSelectedSwcNodeList();
  emit swcTreeNodeSelectionChanged(newSelected, oldSelected);
//  notifySwcTreeNodeSelectionChanged();
}

static double ComputeSwcNodeDistance(
    ZSwcTree::DownstreamIterator &iter1, ZSwcTree::DownstreamIterator &iter2)
{
  iter1.begin();
  iter2.begin();

  double minDist = Infinity;
  while (iter1.hasNext()) {
    Swc_Tree_Node *tn1 = iter1.next();
    while (iter2.hasNext()) {
      Swc_Tree_Node *tn2 = iter2.next();
      double d =SwcTreeNode::distance(tn1, tn2, SwcTreeNode::EUCLIDEAN_SURFACE);
      if (d < minDist) {
        minDist = d;
      }
    }
  }

  return minDist;
}

void ZStackDoc::selectNoisyTrees()
{
  QList<Swc_Tree_Node*> oldSelected = getSelectedSwcNodeList();

  std::vector<double> sizeVector;
//  std::vector<double> distanceVector;
  std::vector<ZSwcTree::DownstreamIterator*> iterVector;
  std::vector<ZSwcTree*> treeVector;

  double minLength = 0.0;
  double maxLength = 0.0;

  ZOUT(LTRACE(), 5) << "Select noisy trees";
  QList<ZSwcTree*> treeList = getObjectList<ZSwcTree>();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    tree->deselectAllNode();

    ZSwcTree::RegularRootIterator riter(tree);
    while (riter.hasNext()) {
      Swc_Tree_Node *tn = riter.next();
      double length = SwcTreeNode::downstreamLength(tn);
      sizeVector.push_back(length);

      iterVector.push_back(new ZSwcTree::DownstreamIterator(tn));
      treeVector.push_back(tree);

      if (minLength == 0.0 || length < minLength) {
        minLength = length;
      }

      if (length > maxLength) {
        maxLength = length;
      }
    }
  }

  if (maxLength > 0.0) {
    if (maxLength > minLength * 3.0) {
      double thre = maxLength / 2.0;
      for (size_t i = 0; i < sizeVector.size(); ++i) {
        if (sizeVector[i] < thre) { //small trees
          ZSwcTree::DownstreamIterator *siter = iterVector[i];
          double anchorDist = Infinity;
          for (size_t j = 0; j < sizeVector.size(); ++j) {
            if (sizeVector[j] > thre) {
              ZSwcTree::DownstreamIterator *anchorIter = iterVector[j];
              double d = ComputeSwcNodeDistance(*siter, *anchorIter);
              if (anchorDist > d) {
                anchorDist = d;
              }
            }
          }

          if (anchorDist > 30.0) {
            ZSwcTree *tree = treeVector[i];
            ZSwcTree::DownstreamIterator *siter = iterVector[i];
            siter->restart();
            while (siter->hasNext()) {
              tree->selectNode(siter->next(), true);
            }
          }
        }
      }
    }
  }

  for (std::vector<ZSwcTree::DownstreamIterator*>::iterator
       iter = iterVector.begin(); iter != iterVector.end(); ++iter) {
    delete *iter;
  }

  QList<Swc_Tree_Node*> newSelected = getSelectedSwcNodeList();
  emit swcTreeNodeSelectionChanged(newSelected, oldSelected);
}

void ZStackDoc::selectUpstreamNode()
{
  std::set<Swc_Tree_Node*> oldSelected = getSelectedSwcNodeSet();

  ZOUT(LTRACE(), 5) << "Select upstream";
  TStackObjectList &objList =
      m_objectGroup.getObjectList(ZStackObject::TYPE_SWC);
  for (TStackObjectList::iterator iter= objList.begin(); iter != objList.end();
       ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    tree->selectUpstreamNode();
  }

  std::set<Swc_Tree_Node*> newSelected = getSelectedSwcNodeSet();
  notifySelectionAdded(oldSelected, newSelected);

#if 0
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
#endif
}

void ZStackDoc::selectBranchNode()
{
  std::set<Swc_Tree_Node*> oldSelected = getSelectedSwcNodeSet();

  ZOUT(LTRACE(), 5) << "Select branch";
  TStackObjectList &objList =
      m_objectGroup.getObjectList(ZStackObject::TYPE_SWC);
  for (TStackObjectList::iterator iter= objList.begin(); iter != objList.end();
       ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    tree->selectBranchNode();
  }

  std::set<Swc_Tree_Node*> newSelected = getSelectedSwcNodeSet();
  notifySelectionAdded(oldSelected, newSelected);

#if 0
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
#endif
}

void ZStackDoc::selectTreeNode()
{
#if 0
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
#endif
}

void ZStackDoc::selectConnectedNode()
{
  std::set<Swc_Tree_Node*> oldSet = getSelectedSwcNodeSet();

  ZOUT(LTRACE(), 5) << "Select connection";
  TStackObjectList &swcList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    tree->selectConnectedNode();
  }

  notifySelectionAdded(oldSet, getSelectedSwcNodeSet());

#if 0
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
#endif
}

void ZStackDoc::selectNeighborSwcNode()
{
  std::set<Swc_Tree_Node*> oldSet = getSelectedSwcNodeSet();

  ZOUT(LTRACE(), 5) << "Select neighbor nodes";
  TStackObjectList &swcList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    tree->selectNeighborNode();
  }

  notifySelectionAdded(oldSet, getSelectedSwcNodeSet());
}

void ZStackDoc::setPunctaVisible(bool visible)
{
  TStackObjectSet objSet =
      m_objectGroup.getSelectedSet(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator it = objSet.begin();
       it != objSet.end(); ++it) {
    ZStackObject *obj = *it;
    obj->setVisible(visible);
    //setPunctumVisible(*it, false);
  }
  if (!objSet.empty()) {
    emit punctumVisibleStateChanged();
  }
}

void ZStackDoc::hideSelectedPuncta()
{
  setPunctaVisible(false);
}

ResolutionDialog* ZStackDoc::getResolutionDialog()
{
  if (m_resDlg == NULL) {
    m_resDlg = new ResolutionDialog(NULL);
  }

  return m_resDlg;
}

void ZStackDoc::showSelectedPuncta()
{
  setPunctaVisible(true);
#if 0
  for (std::set<ZPunctum*>::iterator it = selectedPuncta()->begin();
       it != selectedPuncta()->end(); ++it) {
    setPunctumVisible(*it, true);
  }
#endif
}

void ZStackDoc::selectSwcNodeNeighbor()
{
#if 0
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
#endif
}

void ZStackDoc::updateVirtualStackSize()
{
  if (!hasStackData()) {
    Stack *stack = new Stack;
    double corner[6] = {.0, .0, .0, .0, .0, .0};
    QList<ZSwcTree*> swcList = getSwcList();
    for (int i = 0; i < swcList.size(); i++) {
      double tmpcorner[6];
      swcList[i]->getBoundBox(tmpcorner);
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

bool ZStackDoc::hasDrawable() const
{
  return !m_objectGroup.isEmpty();
}

bool ZStackDoc::hasDrawable(ZStackObject::ETarget target) const
{
  return m_objectGroup.hasObject(target);
}

int ZStackDoc::getStackWidth() const
{
  if (getStack() == NULL) {
    return 0;
  }

  return getStack()->width();
}

int ZStackDoc::getStackHeight() const
{
  if (getStack() == NULL) {
    return 0;
  }

  return getStack()->height();
}

int ZStackDoc::getStackDepth() const
{
  if (getStack() == NULL) {
    return 0;
  }

  return getStack()->depth();
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
    notifyStackModified();
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
    notifyStackModified();
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

QAction* ZStackDoc::getAction(ZActionFactory::EAction item) const
{
  const_cast<ZStackDoc&>(*this).makeAction(item);

  return m_actionMap[item];
}

void ZStackDoc::makeAction(ZActionFactory::EAction item)
{
  if (!m_actionMap.contains(item)) {
    QAction *action = NULL;

    if (item == ZActionFactory::ACTION_UNDO ||
        item == ZActionFactory::ACTION_REDO) {
      action = m_actionFactory->makeAction(item, m_undoStack);
    } else {
      action = m_actionFactory->makeAction(item, this);
    }
    m_actionMap[item] = action;

    //Additional behaviors
    if (action != NULL) {
      switch (item) {
      case ZActionFactory::ACTION_SELECT_DOWNSTREAM:
        connect(action, SIGNAL(triggered()), this, SLOT(selectDownstreamNode()));
        break;
      case ZActionFactory::ACTION_SELECT_UPSTREAM:
        connect(action, SIGNAL(triggered()), this, SLOT(selectUpstreamNode()));
        break;
      case ZActionFactory::ACTION_SELECT_NEIGHBOR_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(selectNeighborSwcNode()));
        break;
      case ZActionFactory::ACTION_SELECT_SWC_BRANCH:
        connect(action, SIGNAL(triggered()), this, SLOT(selectBranchNode()));
        break;
      case ZActionFactory::ACTION_SELECT_CONNECTED_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(selectConnectedNode()));
        break;
      case ZActionFactory::ACTION_SELECT_ALL_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(selectAllSwcTreeNode()));
        break;
      case ZActionFactory::ACTION_RESOLVE_CROSSOVER:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeResolveCrossoverCommand()));
        break;
      case ZActionFactory::ACTION_REMOVE_TURN:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeRemoveTurnCommand()));
        break;
      case ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH:
        connect(action, SIGNAL(triggered()),
                this, SLOT(showSeletedSwcNodeLength()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH:
        connect(action, SIGNAL(triggered()),
                this, SLOT(showSeletedSwcNodeScaledLength()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_DELETE_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeDeleteSwcNodeCommand()));
        break;
      case ZActionFactory::ACTION_DELETE_UNSELECTED_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeDeleteUnselectedSwcNodeCommand()));
        break;
      case ZActionFactory::ACTION_INSERT_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeInsertSwcNode()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_BREAK_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeBreakSwcConnectionCommand()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_CONNECT_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeConnectSwcNodeCommand()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_MERGE_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeMergeSwcNodeCommand()));
        m_singleSwcNodeActionActivator.registerAction(action, false);
        break;
      case ZActionFactory::ACTION_TRANSLATE_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeTranslateSelectedSwcNode()));
        break;
      case ZActionFactory::ACTION_CHANGE_SWC_SIZE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeChangeSelectedSwcNodeSize()));
        break;
      case ZActionFactory::ACTION_SET_SWC_ROOT:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeSetRootCommand()));
        m_singleSwcNodeActionActivator.registerAction(action, true);
        break;
      case ZActionFactory::ACTION_CONNECTED_ISOLATED_SWC:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeConnectIsolatedSwc()));
        break;
      case ZActionFactory::ACTION_RESET_BRANCH_POINT:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeResetBranchPoint()));
        m_singleSwcNodeActionActivator.registerAction(action, true);
        break;
      case ZActionFactory::ACTION_SWC_Z_INTERPOLATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeInterpolateSwcZCommand()));
        break;
      case ZActionFactory::ACTION_SWC_RADIUS_INTERPOLATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeInterpolateSwcRadiusCommand()));
        break;
      case ZActionFactory::ACTION_SWC_POSITION_INTERPOLATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeInterpolateSwcPositionCommand()));
        break;
      case ZActionFactory::ACTION_SWC_INTERPOLATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(executeInterpolateSwcCommand()));
        break;
      case ZActionFactory::ACTION_SWC_SUMMARIZE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(showSwcSummary()));
        break;
      default:
        break;
      }
    }
  }
}

void ZStackDoc::selectDownstreamNode()
{
  std::set<Swc_Tree_Node*> oldSelected = getSelectedSwcNodeSet();

  ZOUT(LTRACE(), 5) << "Select downstream";
  TStackObjectList &objList =
      m_objectGroup.getObjectList(ZStackObject::TYPE_SWC);
  for (TStackObjectList::iterator iter= objList.begin(); iter != objList.end();
       ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    tree->selectDownstreamNode();
  }

  std::set<Swc_Tree_Node*> newSelected = getSelectedSwcNodeSet();
  notifySelectionAdded(oldSelected, newSelected);
#if 0
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
#endif
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

    notifyStackModified();
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
  tree->getBoundBox(corner);
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
//  addSwcTree(tree);

  addObject(tree);


//  emit swcModified();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addObject(m_swcNetwork->getTree(i));
  }
  endObjectModifiedMode();
  notifyObjectModified();

  emit swcNetworkModified();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (size_t i = 0; i < m_swcNetwork->treeNumber(); i++) {
    addObject(m_swcNetwork->getTree(i));
  }
  endObjectModifiedMode();
  notifyObjectModified();

  emit swcNetworkModified();
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

bool ZStackDoc::hasStackPaint() const
{
  return hasStackData() || hasSparseStack();
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
    addObject(zpunctum);
//    emit punctaModified();
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

    if (getPreferredZScale() != 1.0) {
      pos[2] /= getPreferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(),
                              getPreferredZScale(), 1, ws);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(getPreferredZScale());

    addObject(obj);
//    emit chainModified();

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

    if (getPreferredZScale() != 1.0) {
      pos[2] /= getPreferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, NEUROSEG_DEFAULT_H, TZ_PI_4,
                 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_BOTTOM);

    Receptor_Fit_Workspace *ws = New_Receptor_Fit_Workspace();
    Default_Locseg_Fit_Workspace(ws);

    Fit_Local_Neuroseg_W(locseg, mainStack->c_stack(),
                         getPreferredZScale(), ws);

    Kill_Receptor_Fit_Workspace(ws);

    Local_Rpi_Neuroseg rpiseg;
    Local_Rpi_Neuroseg_From_Local_Neuroseg(&rpiseg, locseg);

    ws = New_Receptor_Fit_Workspace();
    Default_Rpi_Locseg_Fit_Workspace(ws);

    Fit_Local_Rpi_Neuroseg_W(&rpiseg, mainStack->c_stack(),
                             getPreferredZScale(), ws);
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
    obj->setZScale(getPreferredZScale());

    addObject(obj);
//    emit chainModified();

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
    obj->setZScale(getPreferredZScale());

    addObject(obj);
//    emit chainModified();

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
    obj->setZScale(getPreferredZScale());

    addObject(obj);
//    emit chainModified();

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

    if (getPreferredZScale() != 1.0) {
      pos[2] /= getPreferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();

    Set_Neuroseg(&(locseg->seg), r, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Chain *locseg_chain = New_Locseg_Chain();
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Chain_Add(locseg_chain, locseg, tr, DL_TAIL);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    obj->setZScale(getPreferredZScale());
    obj->setIgnorable(true);

    addObject(obj);
//    emit chainModified();

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

    QList<ZSwcTree*> swcList = getSwcList();
    foreach (ZSwcTree *tree, swcList) {
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

    if (getPreferredZScale() != 1.0) {
      pos[2] /= getPreferredZScale();
    }

    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Set_Neuroseg(&(locseg->seg), r, 0.0, 11.0, TZ_PI_4, 0.0, 0.0, 0.0, 1.0);

    Set_Neuroseg_Position(locseg, pos, NEUROSEG_CENTER);

    Locseg_Fit_Workspace *ws =
        (Locseg_Fit_Workspace*) getTraceWorkspace()->fit_workspace;
    Local_Neuroseg_Optimize_W(locseg, mainStack->c_stack(c),
                              getPreferredZScale(), 1, ws);

    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Trace_Record_Set_Direction(tr, DL_BOTHDIR);
    Locseg_Node *p = Make_Locseg_Node(locseg, tr);
    Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

    Trace_Workspace_Set_Trace_Status(getTraceWorkspace(), TRACE_NORMAL,
    		TRACE_NORMAL);
    Trace_Locseg(mainStack->c_stack(c), getPreferredZScale(), locseg_chain,
            getTraceWorkspace());
    Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
    Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);

    ZLocsegChain *obj = new ZLocsegChain(locseg_chain);
    if (!obj->isEmpty()) {
      obj->setZScale(getPreferredZScale());
      addObject(obj);
//      emit chainModified();
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

    if (getPreferredZScale() != 1.0) {
      pos[2] /= getPreferredZScale();
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
      obj->setZScale(getPreferredZScale());
      addObject(obj);
//      emit chainModified();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (obj->heldNode() >= 0) {
    removeObject(obj, false);
    ZLocsegChain *chain = new ZLocsegChain(*obj);
    ZLocsegChain *new_chain = chain->cutHeldNode();
    if (new_chain != NULL) {
      addObject(new_chain);
//      emit chainModified();
      if (pResult) {
        pResult->append(new_chain);
      }
    }
    if (chain->isEmpty() == false) {
      addObject(chain);
//      emit chainModified();
      if (pResult) {
        pResult->append(chain);
      }
    } else {
      delete chain;
    }
  }
  endObjectModifiedMode();
  notifyObj3dModified();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (newChain->isEmpty() == false) {
    addObject(newChain);
//    emit chainModified();
    if (pResult) {
      pResult->append(newChain);
    }
  } else {
    delete newChain;
  }

  if (chain->isEmpty() == false) {
    addObject(chain);
//    emit chainModified();
    if (pResult) {
      pResult->append(chain);
    }
  } else {
    delete chain;
  }
  endObjectModifiedMode();
  notifyObjectModified();
}

void ZStackDoc::cutSelectedLocsegChain()
{
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    if (chainList.at(i)->isSelected() == true) {
      cutLocsegChain(chainList.at(i));
      break;
    }
  }
}

void ZStackDoc::breakSelectedLocsegChain()
{
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    if (chainList.at(i)->isSelected() == true) {
      breakLocsegChain(chainList.at(i));
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

void ZStackDoc::autoThreshold()
{
  int thre = 0;
  ZStack *mainStack = getStack();

  if (!mainStack->isVirtual()) {
    notifyProgressStart();
    Stack *stack = mainStack->c_stack();
    double scale = 1.0*stack->width * stack->height * stack->depth * stack->kind /
        (2.0*1024*1024*1024);
    if (scale >= 1.0) {
      scale = std::ceil(std::sqrt(scale + 0.1));
      stack = C_Stack::resize(stack, stack->width/scale, stack->height/scale, stack->depth);
    }

    int conn = 18;
    notifyProgressAdvanced(0.1);
//    m_progressReporter->advance(0.1);
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    notifyProgressAdvanced(0.1);
//    m_progressReporter->advance(0.1);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    notifyProgressAdvanced(0.1);
//    m_progressReporter->advance(0.2);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }
//    m_progressReporter->advance(0.1);
    notifyProgressAdvanced(0.1);

    int *hist = Stack_Hist_M(stack, locmax);
//    m_progressReporter->advance(0.1);
    notifyProgressAdvanced(0.1);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);

#ifdef _DEBUG_2
    std::cout << Int_Histogram_Sum(hist) << std::endl;
    std::cout << (double) Int_Histogram_Sum(hist) / C_Stack::voxelNumber(stack) << std::endl;
#endif

//    m_progressReporter->advance(0.1);
    notifyProgressAdvanced(0.1);

    if ((double) Int_Histogram_Sum(hist) / C_Stack::voxelNumber(stack) <= 1e-5) {
      thre = Stack_Common_Intensity(stack, 0, high);
    } else {
      if (high > low) {
        thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
      } else {
        free(hist);
        hist = Stack_Hist(stack);
        Int_Histogram_Range(hist, &low, &high);
        thre = Int_Histogram_Rc_Threshold(hist, low, high);
      }
    }


//    m_progressReporter->advance(0.1);
    notifyProgressAdvanced(0.1);
    free(hist);

    if (stack != mainStack->c_stack())
      C_Stack::kill(stack);
    notifyProgressEnd();

    emit thresholdChanged(thre);
  }

  //return thre;
}

void ZStackDoc::addSwcTreeP(ZSwcTree *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->forceVirtualRoot();

  m_objectGroup.add(obj, false);

  /*
  if (obj->isSelected()) {
    setSwcSelected(obj, true);
  }
  */
}

#if 0
void ZStackDoc::addSwcTree(ZSwcTree *obj, bool uniqueSource)
{
  if (obj == NULL) {
    return;
  }

  obj->forceVirtualRoot();
  m_objectGroup.add(obj, uniqueSource);
#if 0
  if (uniqueSource) {
    if (!obj->getSource().empty()) {

      QList<ZSwcTree*> treesToRemove;
      QList<ZSwcTree*> swcList = getSwcList();
      for (int i=0; i<m_swcList.size(); i++) {
        if (m_swcList.at(i)->getSource() == obj->getSource()) {
          treesToRemove.push_back(m_swcList.at(i));
        }
      }
      for (int i=0; i<treesToRemove.size(); i++) {
        removeObject(treesToRemove.at(i), true);
      }
    }
  }
#endif

  if (obj->isSelected()) {
    setSwcSelected(obj, true);
  }

#ifdef _DEBUG_2
  obj->useCosmeticPen(true);
  obj->updateHostState(ZSwcTree::NODE_STATE_COSMETIC);
#endif

  processObjectModified(obj);

  /*
  notifySwcModified();
  notifyObjectModified(obj->getRole());
  notifyObjectModified(obj->getType());
  */
}
#endif

void ZStackDoc::addSwcTree(
    ZSwcTree *obj, bool uniqueSource, bool translatingWithStack)
{
  if (obj == NULL) {
    return;
  }

  if (translatingWithStack) {
      obj->translate(getStackOffset());
  }

  addObject(obj, uniqueSource);
//  addSwcTree(obj, uniqueSource);
}

void ZStackDoc::addSwcTree(const QList<ZSwcTree *> &swcList, bool uniqueSource)
{
//  blockSignals(true);
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    addObject(*iter, uniqueSource);
  }
  endObjectModifiedMode();
//  blockSignals(false);

  notifyObjectModified();

  /*
  if (!hasSwc()) {
    notifySwcModified();
    processObjectModified(targetSet);
  }
  */
}

void ZStackDoc::addSparseObject(const QList<ZSparseObject*> &objList)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (QList<ZSparseObject*>::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    addObject(*iter);
  }
  endObjectModifiedMode();
  notifyObj3dModified();
}

void ZStackDoc::addPunctum(const QList<ZPunctum *> &punctaList)
{
//  blockSignals(true);
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  foreach (ZPunctum *punctum, punctaList) {
    addObject(punctum, false);
  }
  endObjectModifiedMode();
  notifyObjectModified();
//  blockSignals(false);

//  notifyPunctumModified();
}

void ZStackDoc::addPunctumFast(const QList<ZPunctum *> &punctaList)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  foreach (ZPunctum *punctum, punctaList) {
    addObjectFast(punctum);
  }
  endObjectModifiedMode();
  notifyObjectModified();
}

void ZStackDoc::addPunctumP(ZPunctum *obj)
{
  if (obj == NULL) {
    return;
  }

  m_objectGroup.add(obj, false);

  /*
  m_punctaList.append(obj);
  m_objectList.append(obj);
*/
  if (obj->isSelected()) {
    setPunctumSelected(obj, true);
  }

//  processObjectModified(obj);
//  notifyPunctumModified();
}

void ZStackDoc::addStackPatchP(ZStackPatch *patch, bool uniqueSource)
{
  if (patch == NULL) {
    return;
  }

  m_objectGroup.add(patch, uniqueSource);

  /*

  if (patch->isSelected()) {
    setSelected(patch, true);
  }

  processObjectModified(patch);
  */

  /*
  if (patch->getTarget() == ZStackObject::TARGET_STACK_CANVAS) {
    emit stackTargetModified();
  }

  notifyStackPatchModified();
  */
}

void ZStackDoc::addObj3dP(ZObject3d *obj)
{
  if (obj == NULL) {
    return;
  }

  m_objectGroup.add(obj, false);
}

void ZStackDoc::addObject3dScanP(ZObject3dScan *obj)
{
  if (obj == NULL) {
    return;
  }

  m_objectGroup.add(obj, false);
}

#define DEFINE_GET_OBJECT_LIST(Function, ObjectClass, OBJECT_TYPE) \
  QList<ObjectClass*> ZStackDoc::Function() const \
  {\
    return m_objectGroup.getObjectList<ObjectClass>();\
  }

QList<ZSwcTree*> ZStackDoc::getSwcList() const
{
  ZOUT(LTRACE(), 5) << "Select swc list";
  return m_objectGroup.getObjectList<ZSwcTree>();
  /*
  QList<ZSwcTree*> treeList;
  for (ZStackObjectGroup::const_iterator iter = m_objectGroup.begin();
       iter != m_objectGroup.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->getType() == ZStackObject::TYPE_SWC) {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
      TZ_ASSERT(tree != NULL, "Null pointer.");
      treeList.append(tree);
    }
  }

  return treeList;
  */
}

DEFINE_GET_OBJECT_LIST(getObj3dList, ZObject3d, TYPE_OBJ3D)
DEFINE_GET_OBJECT_LIST(getStrokeList, ZStroke2d, TYPE_STROKE)
DEFINE_GET_OBJECT_LIST(getLocsegChainList, ZLocsegChain, TYPE_LOCSEG_CHAIN)
DEFINE_GET_OBJECT_LIST(getPunctumList, ZPunctum, TYPE_PUNCTUM)
DEFINE_GET_OBJECT_LIST(getSparseObjectList, ZSparseObject, TYPE_SPARSE_OBJECT)
DEFINE_GET_OBJECT_LIST(getObject3dScanList, ZObject3dScan, TYPE_OBJECT3D_SCAN)
DEFINE_GET_OBJECT_LIST(getDvidLabelSliceList, ZDvidLabelSlice, TYPE_DVID_LABEL_SLICE)
DEFINE_GET_OBJECT_LIST(getDvidTileEnsembleList, ZDvidTileEnsemble, TYPE_DVID_TILE_ENSEMBLE)
DEFINE_GET_OBJECT_LIST(getDvidSparsevolSliceList, ZDvidSparsevolSlice, TYPE_DVID_SPARSEVOL_SLICE)

void ZStackDoc::addSparseObjectP(ZSparseObject *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  m_objectGroup.add(obj, false);

  obj->setRole(ZStackObjectRole::ROLE_SEED);
//  m_playerList.append(new ZSparseObjectPlayer(obj));

//  processObjectModified(obj);

//  emit seedModified();

//  notifySparseObjectModified();
}

void ZStackDoc::addStrokeP(ZStroke2d *obj)
{
  if (obj == NULL) {
    return;
  }

  obj->setTarget(ZStackObject::TARGET_OBJECT_CANVAS);


  m_objectGroup.add(obj, false);
  /*
  if (obj->isSelected()) {
    setSelected(obj, true);
  }

  processObjectModified(obj);
  */

//  notifyObjectModified();
}

void ZStackDoc::addLocsegChain(const QList<ZLocsegChain *> &chainList)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  foreach (ZLocsegChain *chain, chainList) {
    addObject(chain);
//    addLocsegChain(chain);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::addLocsegChainP(ZLocsegChain *obj)
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

  m_objectGroup.add(obj, false);

//  processObjectModified(obj);
  //m_swcObjects.append(obj);
  //m_vrmlObjects.append(obj);
  //m_chainList.append(obj);
  //m_objectList.append(obj);

  getTraceWorkspace()->chain_id++;

  /*
  if (obj->isSelected()) {
    setChainSelected(obj, true);
  }
  */
}

void ZStackDoc::updateLocsegChain(ZLocsegChain *obj)
{
  if (obj != NULL) {
    obj->labelTraceMask(getTraceWorkspace()->trace_mask);
  }
}

void ZStackDoc::exportPuncta(const char *filePath)
{
  QList<ZPunctum*> punctaList = getPunctumList();
  ZPunctumIO::save(filePath, punctaList);
}

ZSwcTree *ZStackDoc::nodeToSwcTree(Swc_Tree_Node *node) const
{
  ZOUT(LTRACE(), 5) << "Obtaining node host:" << node;

  ZSwcTree *host = NULL;

  QList<ZSwcTree*> swcList = getSwcList();
  for (int i=0; i<swcList.size(); ++i) {
    ZOUT(LTRACE(), 5) << "Checking" << swcList[i];
    if (swcList[i]->contains(node)) {
      host = swcList[i];
      break;
    }
  }
  //assert(false);

  ZOUT(LTRACE(), 5) << host;

  return host;
}

void ZStackDoc::exportSvg(const char *filePath)
{
  QList<ZSwcTree*> swcList = getSwcList();
  if (!swcList.isEmpty()) {
    swcList.at(0)->toSvgFile(filePath);
  }
}

void ZStackDoc::exportBinary(const char *prefix)
{
  QList<ZSwcTree*> swcList = getSwcList();
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  if (!chainList.isEmpty()) {
    char *filePath = new char[strlen(prefix) + 10];
    for (int i = 0; i < chainList.size(); i++) {
      sprintf(filePath, "%s%d.tb", prefix, i);
      chainList.at(i)->save(filePath);
    }
    ////   ?
    int startNum = chainList.size();
    for (int i = 0; i < swcList.size(); i++) {
      startNum = swcList.at(i)->saveAsLocsegChains(prefix, startNum);
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
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    stream << chainList.at(i)->getSource().c_str() << '\n';
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
      QList<ZLocsegChain*> chainList = getLocsegChainList();
      for (int i=0; i<chainList.size(); i++) {
        if (chainList.at(i)->getSource().c_str() == file) {
          chainsToRemove.push_back(chainList.at(i));
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
        addObject(chain);
      } else {
        delete chain;
      }
    }
  }
//  emit chainModified();
}

void ZStackDoc::loadSwc(const QString &filePath)
{
  ZSwcTree *tree = new ZSwcTree();
  tree->load(filePath.toLocal8Bit().constData());
//  executeAddObjectCommand(obj);
  addObject(tree);
}

void ZStackDoc::loadLocsegChain(const QString &filePath)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (!filePath.isEmpty()) {
    QList<ZLocsegChain*> chainsToRemove;
    QList<ZLocsegChain*> chainList = getLocsegChainList();
    for (int i=0; i<chainList.size(); i++) {
      if (chainList.at(i)->getSource().c_str() == filePath) {
        chainsToRemove.push_back(chainList.at(i));
      }
    }
    for (QList<ZLocsegChain*>::iterator iter = chainsToRemove.begin();
         iter != chainsToRemove.end(); ++iter) {
      removeObject(*iter, true);
    }
  }

  ZLocsegChain *chain = new ZLocsegChain();

  chain->load(filePath.toLocal8Bit().constData());

  addObject(chain);
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::importSwc(QStringList fileList, LoadObjectOption objopt)
{
  if (fileList.empty())
    return;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (objopt == REPLACE_OBJECT) {
    removeAllObject(true);
  }

  QString file;
  foreach (file, fileList) {
//    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
//      QList<ZSwcTree*> treesToRemove;
//      for (int i=0; i<m_swcList.size(); i++) {
//        if (m_swcList.at(i)->getSource() == file.toStdString()) {
//          treesToRemove.push_back(m_swcList.at(i));
//        }
//      }
//      for (int i=0; i<treesToRemove.size(); i++) {
//        removeObject(treesToRemove.at(i), true);
//      }
//    }

    if (file.endsWith(".swc", Qt::CaseInsensitive)) {
      ZSwcTree *tree = new ZSwcTree();
      tree->load(file.toStdString());
      addObject(tree, true);
    } /*else if (file.endsWith(".json", Qt::CaseInsensitive))  {
      importSynapseAnnotation(file.toStdString(), 0);
    }*/
  }
//  emit swcModified();

  endObjectModifiedMode();

  notifyObjectModified();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

  if (objopt == REPLACE_OBJECT) {
    removeAllObject();
  }

  QString file;
//  blockSignals(true);
  foreach (file, fileList) {
    if (objopt == APPEND_OBJECT) {   // if this file is already loaded, replace it
      QList<ZStackObject*> punctaToRemove = m_objectGroup.findSameSource(
            ZStackObject::TYPE_PUNCTUM, file.toStdString());

      for (QList<ZStackObject*>::iterator iter = punctaToRemove.begin();
           iter != punctaToRemove.end(); ++iter) {
        removeObject(*iter);
      }
      /*
      m_objectGroup.removeObject(
            punctaToRemove.begin(), punctaToRemove.end(), true);
            */
//      for (int i=0; i<m_punctaList.size(); i++) {
//        if (m_punctaList.at(i)->getSource().c_str() == file) {
//          punctaToRemove.push_back(m_punctaList.at(i));
//        }
//      }
//      for (int i=0; i<punctaToRemove.size(); i++) {
//        removeObject(punctaToRemove.at(i), true);
//      }
    }
    QList<ZPunctum*> plist = ZPunctumIO::load(file);
    for (int i=0; i<plist.size(); i++) {
      addObject(plist[i], false);
    }
  }

  endObjectModifiedMode();

  notifyObjectModified();
//  blockSignals(false);
//  emit punctaModified();
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

#if 0
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
#endif

bool ZStackDoc::expandSelectedPuncta()
{
  QList<ZPunctum*> punctaList = getPunctumList();
  QMutableListIterator<ZPunctum*> iter(punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      iter.value()->setRadius(iter.value()->radius() + 1);
    }
  }
  return true;
}

bool ZStackDoc::shrinkSelectedPuncta()
{
  QList<ZPunctum*> punctaList = getPunctumList();
  QMutableListIterator<ZPunctum*> iter(punctaList);
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
  QList<ZPunctum*> punctaList = getPunctumList();
  QMutableListIterator<ZPunctum*> iter(punctaList);
  while (iter.hasNext()) {
    if (iter.next()->isSelected()) {
      Geo3d_Ball *gb = New_Geo3d_Ball();
      gb->center[0] = iter.value()->x();
      gb->center[1] = iter.value()->y();
      gb->center[2] = iter.value()->z();
      gb->r = iter.value()->radius();
      Geo3d_Ball_Mean_Shift(gb, getStack()->c_stack(), 1, 0.5);
      iter.value()->setCenter(gb->center[0], gb->center[1], gb->center[2]);
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
  QList<ZPunctum*> punctaList = getPunctumList();
  for (int i=0; i<punctaList.size(); i++) {
    Geo3d_Ball *gb = New_Geo3d_Ball();
    gb->center[0] = punctaList[i]->x();
    gb->center[1] = punctaList[i]->y();
    gb->center[2] = punctaList[i]->z();
    gb->r = punctaList[i]->radius();
    Geo3d_Ball_Mean_Shift(gb, getStack()->c_stack(), 1, 0.5);
    punctaList[i]->setCenter(gb->center[0], gb->center[1], gb->center[2]);
    Delete_Geo3d_Ball(gb);
  }
  return true;
}

void ZStackDoc::holdClosestSeg(int id, int x, int y, int z)
{
  ZLocsegChain *chain;
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  foreach (chain, chainList) {
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
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  foreach (chain, chainList) {
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
  QList<ZSwcTree*> swcList = getSwcList();
  if (!swcList.isEmpty()) {
    return swcList.at(0)->labelBranch(x, y, z, 5.0);
  }

  return false;
}

#if 0
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
#endif

void ZStackDoc::removeAllObject(bool deleteObject)
{
  {
    QMutexLocker locker(m_objectGroup.getMutex());

    QList<ZStackObject*> &objList = m_objectGroup.getObjectList();

    for (QList<ZStackObject*>::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      bufferObjectModified(*iter);
    }
  }

  m_objectGroup.removeAllObject(deleteObject);
  m_playerList.removeAll();

  notifyObjectModified();

  /*
  while (!m_objectList.isEmpty()) {
    removeLastObject(deleteObject);
  }
  */
}


void ZStackDoc::removeSmallLocsegChain(double thre)
{
  //QMutexLocker locker(&m_mutex);

  QList<ZLocsegChain*> chainList = getLocsegChainList();
  QMutableListIterator<ZLocsegChain*> chainIter(chainList);

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    if (chain->geoLength() < thre) {
      removeObject(chain, true);
    }
  }
  endObjectModifiedMode();

  notifyObjectModified();

//  notifyChainModified();
}

ZSwcTree* ZStackDoc::getSwcTree(size_t index)
{
  const TStackObjectList &objList =
      m_objectGroup.getObjectList(ZStackObject::TYPE_SWC);

  return const_cast<ZSwcTree*>(dynamic_cast<const ZSwcTree*>(objList.at(index)));
}

void ZStackDoc::removeObject(ZStackObject *obj, bool deleteObject)
{
  if (obj != NULL) {
    bufferObjectModified(obj);
    m_playerList.removePlayer(obj);
    m_objectGroup.removeObject(obj, deleteObject);
    notifyObjectModified();
  }
}

QList<ZStackObject*> ZStackDoc::getObjectList(ZStackObjectRole::TRole role) const
{
  return m_objectGroup.getObjectList(role);
}

void ZStackDoc::removeObject(ZStackObjectRole::TRole role, bool deleteObject)
{
  std::set<ZStackObject*> removeSet;
  {
    QMutexLocker locker(m_playerList.getMutex());

    QList<ZDocPlayer*> &playerList = m_playerList.getPlayerList();
    for (QList<ZDocPlayer*>::iterator iter = playerList.begin();
         iter != playerList.end(); /*++iter*/) {
      ZDocPlayer *player = *iter;
      if (player->hasRole(role)) {
        removeSet.insert(player->getData());
        bufferObjectModified(player->getData());
        iter = playerList.erase(iter);
        delete player;
      } else {
        ++iter;
      }
    }
  }

  m_objectGroup.removeObject(removeSet.begin(), removeSet.end(), deleteObject);

  notifyObjectModified();

//  blockSignals(true);
//  for (std::set<ZStackObject*>::iterator iter = removeSet.begin();
//       iter != removeSet.end(); ++iter) {
//    removeObject(*iter, deleteObject);
//  }
//  blockSignals(false);

  /*
  if (!removeSet.empty()) {
    notifyObjectModified();
    notifyPlayerChanged(role);
  }
  */
}

void ZStackDoc::removeObject(const string &source, bool deleteObject)
{
  TStackObjectList objList = m_objectGroup.findSameSource(source);

  removeObjectP(objList.begin(), objList.end(), deleteObject);
}

std::set<ZSwcTree *> ZStackDoc::removeEmptySwcTree(bool deleteObject)
{ 
  //QMutexLocker locker(&m_mutex);

  std::set<ZSwcTree *> emptyTreeSet;

  TStackObjectList objSet = m_objectGroup.take(ZStackObject::isEmptyTree);

//  QSet<ZStackObject::ETarget> targetSet;

  ZStackObjectRole role;
  for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
       ++iter) {
    role.addRole(m_playerList.removePlayer(*iter));

    bufferObjectModified((*iter)->getRole());
//    targetSet.insert((*iter)->getTarget());
    if (deleteObject) {
      delete *iter;
    } else {
      emptyTreeSet.insert(dynamic_cast<ZSwcTree*>(*iter));
    }
  }

  if (!emptyTreeSet.empty()) {
    bufferObjectModified(ZStackObject::TYPE_SWC);
    bufferObjectModified(role);
    notifyObjectModified();
  }

  /*
  if (!emptyTreeSet.empty()) {
    notifySwcModified();
    processObjectModified(targetSet);
    notifyPlayerChanged(role.getRole());
  }
  */

  return emptyTreeSet;
}

std::set<ZSwcTree*> ZStackDoc::getEmptySwcTreeSet() const
{
  std::set<ZSwcTree*> treeSet;

  TStackObjectList objList = m_objectGroup.getObjectList(
        ZStackObject::TYPE_SWC, ZStackObject::isEmptyTree);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    if (tree != NULL) {
      treeSet.insert(tree);
    }
  }

  return treeSet;
}


void ZStackDoc::removeAllSwcTree(bool deleteObject)
{
  //QMutexLocker locker(&m_mutex);

  removeObject(ZStackObject::TYPE_SWC, deleteObject);
  /*
  if (m_objectGroup.removeObject(ZStackObject::TYPE_SWC, deleteObject)) {
    notifySwcModified();
  }
  */
//  blockSignals(true);
//  QMutableListIterator<ZSwcTree*> swcIter(m_swcList);
//  while (swcIter.hasNext()) {
//    ZSwcTree *tree = swcIter.next();
//    removeObject(tree, deleteObject);
//  }
//  blockSignals(false);
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
  //QMutexLocker locker(&m_mutex);

  TStackObjectList objList = m_objectGroup.takeSelected();

//  ZStackObjectRole role;
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
//    role.addRole(m_playerList.removePlayer(*iter));
    bufferObjectModified(*iter);
    m_playerList.removePlayer(*iter);

    if (deleteObject) {
      delete *iter;
    }
  }

  notifyObjectModified();

  /*
  if (!objList.empty()) {
    notifyObjectModified();
    notifyPlayerChanged(role);
  }
  */
}

TStackObjectList ZStackDoc::takeObject(ZStackObject::EType type)
{
  return m_objectGroup.take(type);
}

TStackObjectList ZStackDoc::takeObject(
    ZStackObject::EType type, const string &source)
{
  return m_objectGroup.takeSameSource(type, source);
}

void ZStackDoc::removeObject(ZStackObject::EType type, bool deleteObject)
{
  TStackObjectList objList = m_objectGroup.take(type);
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
//    role.addRole(m_playerList.removePlayer(*iter));
    bufferObjectModified(*iter);
    m_playerList.removePlayer(*iter);

    if (deleteObject) {
      delete *iter;
    }
  }

  notifyObjectModified();
}

void ZStackDoc::removeSelectedPuncta(bool deleteObject)
{
  /*
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
  */

  //QMutexLocker locker(&m_mutex);

  m_objectGroup.removeSelected(ZStackObject::TYPE_PUNCTUM, deleteObject);
}

bool ZStackDoc::pushLocsegChain(ZStackObject *obj)
{
  bool found =false;

  QList<ZLocsegChain*> chainList = getLocsegChainList();
  ZLocsegChain *chain = NULL;
  foreach (chain, chainList) {
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
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    if (chainList.at(i)->isSelected()) {
      pushLocsegChain(chainList.at(i));
    }
  }
}

void ZStackDoc::setPunctumSelected(ZPunctum *punctum, bool select)
{
  if (punctum->isSelected() != select) {
    m_objectGroup.setSelected(punctum, select);
    //punctum->setSelected(select);
    QList<ZPunctum*> selected;
    QList<ZPunctum*> deselected;
    if (select) {
      //m_selectedPuncta.insert(punctum);
      selected.push_back(punctum);
    } else {
      //m_selectedPuncta.erase(punctum);
      deselected.push_back(punctum);
    }
    notifySelectionChanged(selected, deselected);
    //emit punctaSelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectAllPuncta()
{
  //QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;

  TStackObjectSet selectedSet =
      m_objectGroup.getSelectedSet(ZStackObject::TYPE_PUNCTUM);
  for (TStackObjectSet::iterator iter= selectedSet.begin();
       iter != selectedSet.end(); ++iter) {
    deselected.append(dynamic_cast<ZPunctum*>(*iter));
  }

  m_objectGroup.setSelected(ZStackObject::TYPE_PUNCTUM, false);
  /*
  //m_selectedPuncta.clear();
  QList<ZPunctum*> punctaList = getPunctumList();
  for (int i=0; i<punctaList.size(); i++) {
    if (punctaList[i]->isSelected()) {
      punctaList[i]->setSelected(false);
      deselected.push_back(punctaList[i]);
    }
  }
  */
  notifyDeselected(deselected);
}
#if 1
void ZStackDoc::setChainSelected(ZLocsegChain */*chain*/, bool /*select*/)
{
#if 0
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
#endif
}

void ZStackDoc::setChainSelected(const std::vector<ZLocsegChain *> &/*chains*/,
                                 bool /*select*/)
{
#if 0
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
#endif
}

void ZStackDoc::deselectAllChains()
{
#if 0
  QList<ZLocsegChain*> selected;
  QList<ZLocsegChain*> deselected;
  m_selectedChains.clear();
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  for (int i=0; i<chainList.size(); i++) {
    if (chainList[i]->isSelected()) {
      chainList[i]->setSelected(false);
      deselected.push_back(chainList[i]);
    }
  }
  if (deselected.size() > 0) {
    emit chainSelectionChanged(selected, deselected);
  }
#endif
}
#endif
/*
void ZStackDoc::deselectAllStroke()
{
  for (std::set<ZStroke2d*>::iterator iter = m_selectedStroke.begin();
       iter != m_selectedStroke.end(); ++iter) {
    ZStroke2d *stroke = *iter;
    stroke->setSelected(false);
  }
  m_selectedStroke.clear();
}
*/

void ZStackDoc::setSwcSelected(ZSwcTree *tree, bool select)
{
  if (tree != NULL) {
    if (tree->isSelected() != select) {
      tree->setSelected(select);
      QList<ZSwcTree*> selected;
      QList<ZSwcTree*> deselected;

      m_objectGroup.setSelected(tree, select);

      if (select) {
        QList<Swc_Tree_Node*> deselectedSwcTreeNode;

        //m_selectedSwcs.insert(tree);
        selected.push_back(tree);
        // deselect its nodes
//        std::vector<Swc_Tree_Node *> tns;
        for (std::set<Swc_Tree_Node*>::iterator it = tree->getSelectedNode().begin();
             it != tree->getSelectedNode().end(); ++it) {
          deselectedSwcTreeNode.append(*it);
        }
        //setSwcTreeNodeSelected(tns.begin(), tns.end(), false);
        notifyDeselected(deselectedSwcTreeNode);
      } else {
        //m_selectedSwcs.erase(tree);
        deselected.push_back(tree);
      }
      notifySelectionChanged(selected, deselected);
      //emit swcSelectionChanged(selected, deselected);
    }
  }
}

void ZStackDoc::deselectAllSwcs()
{
  //QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;

  //m_selectedSwcs.clear();
  QList<ZSwcTree*> swcList = getSwcList();
  for (int i=0; i<swcList.size(); i++) {
    if (swcList[i]->isSelected()) {
      swcList[i]->setSelected(false);
      deselected.push_back(swcList[i]);
    }
  }

  m_objectGroup.setSelected(ZStackObject::TYPE_SWC, false);

  notifyDeselected(deselected);
}

void ZStackDoc::setSwcTreeNodeSelected(Swc_Tree_Node *tn, bool select)
{
  if (SwcTreeNode::isRegular(tn)) {
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;

    QList<ZSwcTree*> selectedSwc;
    QList<ZSwcTree*> deselectedSwc;

    ZSwcTree *hostTree = nodeToSwcTree(tn);
    if (hostTree != NULL) {
      if (select) {
        if (!hostTree->isNodeSelected(tn)) {
          hostTree->selectNode(tn, true);
          getObjectGroup().setSelected(hostTree, false);
          deselectedSwc.append(hostTree);
          selected.append(tn);
        }
      } else {
        if (hostTree->isNodeSelected(tn)) {
          hostTree->deselectNode(tn);
          deselected.append(tn);
        }
      }

      notifySelectionChanged(selected, deselected);
      notifySelectionChanged(selectedSwc, deselectedSwc);
    }
  }
}

std::set<Swc_Tree_Node*> ZStackDoc::getUnselectedSwcNodeSet() const
{
  std::set<Swc_Tree_Node*> swcNodeSet;

  ZOUT(LTRACE(), 5) << "Get unselected node";
  TStackObjectList objList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZSwcTree* tree = dynamic_cast<const ZSwcTree*>(*iter);
    ZSwcTree::DepthFirstIterator treeIter(tree);
    treeIter.excludeVirtual(true);

    while (treeIter.hasNext()) {
      Swc_Tree_Node *tn = treeIter.next();
      if (!tree->isNodeSelected(tn)) {
        swcNodeSet.insert(tn);
      }
    }
  }

  return swcNodeSet;
}


QList<Swc_Tree_Node*> ZStackDoc::getSelectedSwcNodeList(const ZSwcTree *tree)
{
  QList<Swc_Tree_Node*> swcNodeList;
  for (std::set<Swc_Tree_Node*>::const_iterator
       iter = tree->getSelectedNode().begin();
       iter != tree->getSelectedNode().end(); ++iter) {
    swcNodeList.append(const_cast<Swc_Tree_Node*>(*iter));
  }

  return swcNodeList;
}

QList<Swc_Tree_Node*> ZStackDoc::getSelectedSwcNodeList() const
{
  QList<Swc_Tree_Node*> swcNodeList;
  ZOUT(LTRACE(), 5) << "Get selected node";
  TStackObjectList objList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZSwcTree* tree = dynamic_cast<const ZSwcTree*>(*iter);
    for (std::set<Swc_Tree_Node*>::const_iterator
         iter = tree->getSelectedNode().begin();
         iter != tree->getSelectedNode().end(); ++iter) {
      swcNodeList.append(const_cast<Swc_Tree_Node*>(*iter));
    }
  }

  return swcNodeList;
}

/*
std::set<Swc_Tree_Node*> ZStackDoc::getSelectedSwcTreeNodeSet() const
{
  std::set<Swc_Tree_Node*> swcNodeSet;

  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter){
    ZSwcTree *tree = *iter;
    swcNodeSet.insert(
          tree->getSelectedNode().begin(), tree->getSelectedNode().end());
  }

  return swcNodeSet;
}
*/

QMap<const Swc_Tree_Node *, const ZSwcTree *>
ZStackDoc::getSelectedSwcNodeMap() const
{
  QMap<const Swc_Tree_Node*, const ZSwcTree*> swcMap;

  ZOUT(LTRACE(), 5) << "Get node map";
  TStackObjectList objList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZSwcTree* tree = dynamic_cast<const ZSwcTree*>(*iter);
    const std::set<Swc_Tree_Node*> &nodeSet = tree->getSelectedNode();
    for (std::set<Swc_Tree_Node*>::const_iterator nodeIter = nodeSet.begin();
         nodeIter != nodeSet.end(); ++nodeIter) {
      const Swc_Tree_Node *tn = *nodeIter;
      swcMap[tn] = tree;
    }
  }

  return swcMap;
}

std::set<Swc_Tree_Node*> ZStackDoc::getSelectedSwcNodeSet() const
{
  std::set<Swc_Tree_Node*> swcNodeSet;
  ZOUT(LTRACE(), 5) << "Get selected node set";
  TStackObjectList objList = getObjectList(ZStackObject::TYPE_SWC);

  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZSwcTree* tree = dynamic_cast<const ZSwcTree*>(*iter);
    const std::set<Swc_Tree_Node*> &nodeSet = tree->getSelectedNode();
    swcNodeSet.insert(nodeSet.begin(), nodeSet.end());
  }

  return swcNodeSet;
}

void ZStackDoc::deselectAllSwcTreeNodes()
{
  //QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected = getSelectedSwcNodeList();
  //std::set<Swc_Tree_Node*> swcNodeSet;

  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter){
    ZSwcTree *tree = *iter;
    /*
    swcNodeSet.insert(
          tree->getSelectedNode().begin(), tree->getSelectedNode().end());
          */
    tree->deselectAllNode();
  }

//  for (std::set<Swc_Tree_Node*>::iterator it = nodeSet.begin();
//       it != nodeSet.end(); it++) {
//    deselected.push_back(*it);
//  }
  //m_selectedSwcTreeNodes.clear();
  notifyDeselected(deselected);
}

bool ZStackDoc::isSwcNodeSelected(const Swc_Tree_Node *tn) const
{
  if (tn != NULL) {
    ZOUT(LTRACE(), 5) << "Is node selected?";
    const TStackObjectList &objList =
        getObjectGroup().getObjectList(ZStackObject::TYPE_SWC);
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZSwcTree* tree = dynamic_cast<const ZSwcTree*>(*iter);
      if (tree->isNodeSelected(tn)) {
        return true;
      }
    }
  }

  return false;
}

/*
ZStackViewParam ZStackDoc::getSelectedSwcNodeView() const
{
  if (hasSelectedSwcNode()) {
    ZPoint pt = SwcTreeNode::centroid(getSelectedSwcNodeSet());

  }
}
*/
void ZStackDoc::deselectAllObject(bool recursive)
{
  //m_selectedSwcTreeNodes.clear();
  if (recursive) {
    deselectAllSwcTreeNodes();
  }

  if (recursive) {
    QList<ZDvidLabelSlice*> labelSliceList = getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *labelSlice, labelSliceList) {
      if (labelSlice->isHittable()) {
        labelSlice->deselectAll();
      }
    }
  }

  notifyDeselected(getSelectedObjectList<ZSwcTree>(ZStackObject::TYPE_SWC));
  notifyDeselected(getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM));
  notifyDeselected(getSelectedObjectList<ZLocsegChain>(
                     ZStackObject::TYPE_LOCSEG_CHAIN));

  m_objectGroup.setSelected(false);
}

void ZStackDoc::deselectAllObject(ZStackObject::EType type)
{
  ZOUT(LTRACE(), 5) << "Deselect all object";
  switch (type) {
  case ZStackObject::TYPE_SWC_NODE:
    deselectAllSwcTreeNodes();
    break;
  case ZStackObject::TYPE_DVID_LABEL_SLICE:
  {
    QList<ZDvidLabelSlice*> labelSliceList = getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *labelSlice, labelSliceList) {
      if (labelSlice->isHittable()) {
        labelSlice->deselectAll();
      }
    }
  }
    break;
  default:
  {
    TStackObjectList objList = getObjectList(type);
    m_objectGroup.getSelector()->reset();

    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      if (obj->isSelected()) {
        m_objectGroup.setSelected(obj, false);
      }
    }
    notifySelectionChanged(m_objectGroup.getSelector()->getSelectedObjectSet(type),
                           m_objectGroup.getSelector()->getDeselectedObjectSet(type));
  }
    break;
  }

  //m_selectedSwcTreeNodes.clear();


  if (type == ZStackObject::TYPE_SWC) {
    notifyDeselected(getSelectedObjectList<ZSwcTree>(ZStackObject::TYPE_SWC));
  }

  if (type == ZStackObject::TYPE_PUNCTUM) {
    notifyDeselected(getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM));
  }

  if (type == ZStackObject::TYPE_LOCSEG_CHAIN) {
    notifyDeselected(getSelectedObjectList<ZLocsegChain>(
                       ZStackObject::TYPE_LOCSEG_CHAIN));
  }

//  m_objectGroup.setSelected(false);
}

void ZStackDoc::setPunctumVisible(ZPunctum *punctum, bool visible)
{
  if (punctum->isVisible() != visible) {
    punctum->setVisible(visible);
    emit punctumVisibleStateChanged();
  }
}

void ZStackDoc::setGraphVisible(Z3DGraph *graph, bool visible)
{
  if (graph->isVisible() != visible) {
    graph->setVisible(visible);
    emit graphVisibleStateChanged();
  }
}

void ZStackDoc::setSurfaceVisible(ZCubeArray *cubearray, bool visible)
{
  if (cubearray->isVisible() != visible) {
    cubearray->setVisible(visible);
    emit surfaceVisibleStateChanged();
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

/*
QString ZStackDoc::toString()
{
  return QString("Number of chains: %1").arg(m_chainList.size());
}
*/
QStringList ZStackDoc::toStringList() const
{
  ZStack *mainStack = getStack();

  QStringList list;
  list.append(QString("Number of objects: %1").arg(
                m_objectGroup.getObjectList().size()));
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
      QList<ZLocsegChain*> chainList = getLocsegChainList();
      for (int i = 0; i < chainList.size(); i++) {
        ZLocsegChain *chain = chainList.at(i);
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

/*
void ZStackDoc::addObject(ZStackObject *obj)
{
  m_objectGroup.add(obj);
//  m_objectList.append(obj);
  notifyObjectModified();
}
*/
void ZStackDoc::appendSwcNetwork(ZSwcNetwork &network)
{
  if (m_swcNetwork == NULL) {
    m_swcNetwork = new ZSwcNetwork;
  }
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (size_t i = 0; i < network.treeNumber(); i++) {
    addObject(network.getTree(i));
  }
  endObjectModifiedMode();
  notifyObjectModified();

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

void ZStackDoc::setAutoTraceMinScore(double score)
{
  m_neuronTracer.setMinScore(score, ZNeuronTracer::TRACING_AUTO);
  m_neuronTracer.setMinScore(score + 0.05, ZNeuronTracer::TRACING_SEED);
//  m_neuronTracer.setAutoTraceMinScore(score);
}

void ZStackDoc::setManualTraceMinScore(double score)
{
  m_neuronTracer.setMinScore(score, ZNeuronTracer::TRACING_INTERACTIVE);

//  m_neuronTracer.setManualTraceMinScore(score);
//  m_neuronTracer.setAutoTraceMinScore(score);
//  m_neuronTracer.setSeedMinScore(score);
//  getTraceWorkspace()->min_score = score;
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

void ZStackDoc::mergeAllChain()
{
  QList<ZLocsegChain*> chainList = getLocsegChainList();
  foreach(ZLocsegChain *chain, chainList) {
    chain->eraseTraceMask(getTraceWorkspace()->trace_mask);
  }

  if (chainList.size() > 0) {
     int chain_number = chainList.size();

     /* alloc <chain_array> */
     Neuron_Component *chain_array =
       Make_Neuron_Component_Array(chain_number);

     int chain_number2 = 0;

     for (int i = 0; i < chain_number; i++) {
       if ((chainList.at(i)->length() > 0) &&
           !(chainList.at(i)->isIgnorable())) {
         Set_Neuron_Component(chain_array + chain_number2,
                              NEUROCOMP_TYPE_LOCSEG_CHAIN,
                              chainList.at(i)->data());
         chain_number2++;
       }
     }

     /* reconstruct neuron */
     Connection_Test_Workspace ctw = *getConnectionTestWorkspace();
     ctw.sp_test = FALSE;

     double zscale = chainList.at(0)->zScale();

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

     QMutableListIterator<ZLocsegChain*> chainIter(chainList);
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

QString ZStackDoc::rawDataInfo(
    double cx, double cy, int z) const
{
  QString info;

  int x = iround(cx);
  int y = iround(cy);

  int wx = x;
  int wy = y;
  int wz = z;

//  ZGeometry::shiftSliceAxisInverse(wx, wy, wz, axis);

  if (x >= 0 && y >= 0) {
    std::ostringstream stream;

    stream << "(";
    if (wx >= 0) {
      stream << wx << ", ";
    }
    if (wy >= 0) {
      stream << wy;
    }
    if (wz >= 0) {
      stream << " , " << wz;
    }

    stream << ")";

    info = stream.str().c_str();
//    QString info = QString("%1, %2").arg(wx).arg(wy);

    if (z < 0) {
      info += " (MIP): ";
    } else {
      info += QString(": ");
    }

    if (getStack() != NULL) {
      if (!getStack()->isVirtual()) {
        if (getStack()->channelNumber() == 1) {
          info += QString("%4").arg(getStack()->value(wx, wy, wz));
        } else {
          info += QString("(");
          for (int i=0; i<getStack()->channelNumber(); i++) {
            if (i==0) {
              info += QString("%1").arg(getStack()->value(wx, wy, wz, i));
            } else {
              info += QString(", %1").arg(getStack()->value(wx, wy, wz, i));
            }
          }
          info += QString(")");
        }
      }

      if (stackMask() != NULL) {
        info += " | Mask: ";
        if (stackMask()->channelNumber() == 1) {
          info += QString("%4").arg(stackMask()->value(wx, wy, wz));
        } else {
          info += QString("(");
          for (int i=0; i<stackMask()->channelNumber(); i++) {
            if (i==0) {
              info += QString("%1").arg(stackMask()->value(wx, wy, wz, i));
            } else {
              info += QString(", %1").arg(stackMask()->value(wx, wy, wz, i));
            }
          }
          info += QString(")");
        }
      }

      if (getStack()->hasOffset()) {
        info += QString("; (%1, %2, %3)").
            arg(getStackOffset().getX() + wx).arg(getStackOffset().getY() + wy).
            arg(getStackOffset().getZ() + wz);
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

void ZStackDoc::eraseTraceMask(const ZLocsegChain *chain)
{
  chain->eraseTraceMask(getTraceWorkspace()->trace_mask);
}

void ZStackDoc::setSelected(ZStackObject *obj,  bool selecting)
{
  if (obj != NULL) {
    if (obj->isSelectable()) {
      ZStackObject::EType type = obj->getType();
      TStackObjectSet &selectedSet = getSelected(type);

      m_objectGroup.setSelected(obj, selecting);
      //obj->setSelected(selecting);

      if (selecting) {
        selectedSet.insert(obj);
      } else {
        selectedSet.remove(obj);
      }
    }
  }
}

void ZStackDoc::toggleSelected(ZStackObject *obj)
{
  if (obj != NULL) {
    setSelected(obj, !obj->isSelected());
  }
}

void ZStackDoc::selectObject(ZStackObject *obj, bool appending)
{
  if (!appending) {
    getObjectGroup().deselectAll();
//    m_objectGroup.getSelector()->deselectAll();
  }
  m_objectGroup.setSelected(obj, true);

//  m_objectGroup.getSelector()->selectObject(obj);
  notifySelectionChanged(m_objectGroup.getSelector()->getSelectedSet(),
                         m_objectGroup.getSelector()->getDeselectedSet());
}

bool ZStackDoc::hasSelectedObject() const
{
  return m_objectGroup.hasSelected();
}

TStackObjectSet &ZStackDoc::getSelected(ZStackObject::EType type)
{
  return m_objectGroup.getSelectedSet(type);
  /*
  if (!m_selectedObjectMap.contains(type)) {
    m_selectedObjectMap[type] = std::set<ZStackObject*>();
  }

  return m_selectedObjectMap[type];
  */
}

const TStackObjectSet &ZStackDoc::getSelected(
    ZStackObject::EType type) const
{
  return const_cast<ZStackDoc&>(*this).getSelected(type);
}

bool ZStackDoc::binarize(int threshold)
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (threshold < 0) {
      threshold = 0;
    }

    if (mainStack->binarize(threshold)) {
      notifyStackModified();
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
      notifyStackModified();
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
      notifyStackModified();
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
    notifyStackModified();
    return true;
  }

  return false;
}

bool ZStackDoc::subtractBackground()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    ZStackProcessor::SubtractBackground(mainStack, 0.5, 3);
    notifyStackModified();
    return true;
  }

  return false;
}

bool ZStackDoc::enhanceLine()
{
  ZStack *mainStack = getStack();
  if (mainStack != NULL) {
    if (mainStack->enhanceLine()) {
      notifyStackModified();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::importSynapseAnnotation(const std::string &filePath,
                                        int s)
{
  FlyEm::ZSynapseAnnotationArray synapseArray;
  if (synapseArray.loadJson(filePath)) {
    std::vector<ZPunctum*> puncta;
    switch (s) {
    case 0:
      puncta = synapseArray.toPuncta(10.0);
      break;
    case 1:
      puncta = synapseArray.toTBarPuncta(10.0);
      break;
    case 2:
      puncta = synapseArray.toPsdPuncta(10.0);
      break;
    }

//    = synapseArray.toPuncta(10.0);
//    blockSignals(true);
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
         iter != puncta.end(); ++iter) {
      addObject(*iter);
    }
    endObjectModifiedMode();
    notifyObjectModified();
//    blockSignals(false);

//    notifyPunctumModified();

    //ZSwcTree *tree = synapseArray.toSwcTree();
//    ZStack *mainStack = getStack();
//    if (mainStack != NULL) {
//      tree->flipY(mainStack->height() - 1);
//    }

//    addSwcTree(tree);
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
//  bool swcLoaded = false;
//  bool chainLoaded = false;
  bool networkLoaded = false;
//  bool punctaLoaded = false;
//  bool obj3dScanLoaded = false;
  //bool apoLoaded = false;

//  m_changingSaveState = false;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

  for (QStringList::const_iterator iter = fileList.begin(); iter != fileList.end();
       ++iter) {
    switch (ZFileType::fileType(iter->toStdString())) {
    case ZFileType::SWC_FILE:
    case ZFileType::SYNAPSE_ANNOTATON_FILE:
//      swcLoaded = true;
      break;
    case ZFileType::SWC_NETWORK_FILE:
    case ZFileType::FLYEM_NETWORK_FILE:
//      swcLoaded = true;
      networkLoaded = true;
      break;
    case ZFileType::LOCSEG_CHAIN_FILE:
//      chainLoaded = true;
      break;
    case ZFileType::V3D_APO_FILE:
    case ZFileType::V3D_MARKER_FILE:
    case ZFileType::RAVELER_BOOKMARK:
//      punctaLoaded = true;
      break;
    case ZFileType::OBJECT_SCAN_FILE:
//      obj3dScanLoaded = true;
      break;
    default:
      break;
    }

//    blockSignals(true);
    loadFile(*iter);
//    blockSignals(false);
  }

  endObjectModifiedMode();
  notifyObjectModified();

  if (networkLoaded) {
    emit swcNetworkModified();
  }

//  m_changingSaveState = true;

  /*
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

  if (obj3dScanLoaded) {
    notifyObject3dScanModified();
  }
  */

#ifdef _FLYEM_2
  emit punctaModified();
#endif
}

bool ZStackDoc::loadFile(const std::string filePath)
{
  return loadFile(QString(filePath.c_str()));
}

bool ZStackDoc::loadFile(const char *filePath)
{
  return loadFile(QString(filePath));
}


bool ZStackDoc::loadFile(const QString &filePath)
{
  QFile file(filePath);

  if (!file.exists()) {
    return false;
  }

  ZOUT(LTRACE(), 5) << "Load file: " << filePath;

  bool succ = true;

  m_changingSaveState = false;
  switch (ZFileType::fileType(filePath.toStdString())) {
  case ZFileType::SWC_FILE:
#ifdef _FLYEM_2
    removeAllObject();
#endif
  {
    ZSwcTree *tree = new ZSwcTree();
    tree->load(filePath.toStdString());
    executeAddObjectCommand(tree);
  }
//    loadSwc(filePath);
    break;
  case ZFileType::LOCSEG_CHAIN_FILE:
    loadLocsegChain(filePath);
    break;
  case ZFileType::SWC_NETWORK_FILE:
    loadSwcNetwork(filePath);
    break;
  case ZFileType::OBJECT_SCAN_FILE:
    setTag(NeuTube::Document::FLYEM_BODY);
    if (hasStackData()){
      ZObject3dScan *obj = new ZObject3dScan;
      obj->load(filePath.toStdString());
      int index = m_objectGroup.getObjectList(
            ZStackObject::TYPE_OBJECT3D_SCAN).size() + 1;
      QColor color = m_objColorSheme.getColor(index);
      color.setAlpha(128);
      obj->setColor(color);
      executeAddObjectCommand(obj);
    } else {
      ZSparseObject *sobj = new ZSparseObject;
      sobj->load(filePath.toStdString().c_str());
      addObject(sobj);
      sobj->setColor(255, 255, 255, 255);

      ZIntCuboid cuboid = sobj->getBoundBox();
      ZStack *stack = ZStackFactory::makeVirtualStack(
            cuboid.getWidth(), cuboid.getHeight(), cuboid.getDepth());
      if (stack != NULL) {
        stack->setSource(filePath.toStdString());
        stack->setOffset(cuboid.getFirstCorner());
        loadStack(stack);
      }
    }
    break; //experimenting _DEBUG_
  case ZFileType::DVID_OBJECT_FILE:
    setTag(NeuTube::Document::FLYEM_BODY);
    if (hasStackData()){
      ZObject3dScan *obj = new ZObject3dScan;
      obj->importDvidObject(filePath.toStdString());
      int index = m_objectGroup.getObjectList(
            ZStackObject::TYPE_OBJECT3D_SCAN).size() + 1;
      QColor color = m_objColorSheme.getColor(index);
      color.setAlpha(128);
      obj->setColor(color);
      executeAddObjectCommand(obj);
    } else {
      ZSparseObject *sobj = new ZSparseObject;
      sobj->importDvidObject(filePath.toStdString());
      addObject(sobj);
      sobj->setColor(255, 255, 255, 255);

      ZIntCuboid cuboid = sobj->getBoundBox();
      ZStack *stack = ZStackFactory::makeVirtualStack(
            cuboid.getWidth(), cuboid.getHeight(), cuboid.getDepth());
      stack->setSource(filePath.toStdString());
      stack->setOffset(cuboid.getFirstCorner());
      loadStack(stack);
    }
    break;
  case ZFileType::TIFF_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
  case ZFileType::PNG_FILE:
  case ZFileType::V3D_PBD_FILE:
    readStack(filePath.toStdString().c_str(), false);
    break;
  case ZFileType::FLYEM_NETWORK_FILE:
    importFlyEmNetwork(filePath.toStdString().c_str());
    break;
  case ZFileType::JSON_FILE:
  case ZFileType::SYNAPSE_ANNOTATON_FILE:
    if (!importSynapseAnnotation(filePath.toStdString())) {
      succ = false;
    }
    break;
  case ZFileType::V3D_APO_FILE:
  case ZFileType::V3D_MARKER_FILE:
  case ZFileType::RAVELER_BOOKMARK:
    if (!importPuncta(filePath.toStdString().c_str())) {
      succ = false;
    }
    break;
  default:
    succ = false;
    break;
  }

  m_changingSaveState = true;

  return succ;
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

ZStackObject* ZStackDoc::hitTest(double x, double y, double z)
{
  QMutexLocker locker(m_objectGroup.getMutex());

  ZOUT(LTRACE(), 5) << "Hit test";
  QList<ZStackObject*> sortedObjList = m_objectGroup.getObjectList();
  sort(sortedObjList.begin(), sortedObjList.end(),
       ZStackObject::ZOrderBiggerThan());

  for (QList<ZStackObject*>::iterator iter = sortedObjList.begin();
       iter != sortedObjList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (obj->isHittable()) {
      if (obj->hit(x, y, z)) {
        return obj;
      }
    }
  }

  return NULL;
}

ZStackObject* ZStackDoc::hitTest(double x, double y, NeuTube::EAxis sliceAxis)
{
  QMutexLocker locker(m_objectGroup.getMutex());

  ZOUT(LTRACE(), 5) << "Hit test";
  QList<ZStackObject*> sortedObjList = m_objectGroup.getObjectList();

  sort(sortedObjList.begin(), sortedObjList.end(),
       ZStackObject::ZOrderBiggerThan());

  for (QList<ZStackObject*>::iterator iter = sortedObjList.begin();
       iter != sortedObjList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (obj->isHittable() && obj->isProjectionVisible()) {
      if (obj->hit(x, y, sliceAxis)) {
        return obj;
      }
    }
  }

  return NULL;
}

#if 0
Swc_Tree_Node* ZStackDoc::swcHitTest(double x, double y) const
{
  Swc_Tree_Node *selected = NULL;

  QList<ZSwcTree*> swcList = getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *tree = const_cast<ZSwcTree*>(*iter);
    selected = tree->hitTest(x, y);

    if (selected != NULL) {
      break;
    }
  }

  return selected;
}

Swc_Tree_Node* ZStackDoc::swcHitTest(double x, double y, double z) const
{
  Swc_Tree_Node *selected = NULL;
  const double Margin = 0.5;

  QList<ZSwcTree*> swcList = getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *tree = const_cast<ZSwcTree*>(*iter);
    //if (z < 0) {
      //selected = tree->hitTest(x, y);
    //} else {
      selected = tree->hitTest(x, y, z, Margin);
    //}

    if (selected != NULL) {
      break;
    }
  }

  return selected;
}

Swc_Tree_Node* ZStackDoc::swcHitTest(const ZPoint &pt) const
{
  return swcHitTest(pt.x(), pt.y(), pt.z());
}
#endif

void ZStackDoc::selectSwcTreeNode(
    ZSwcTree *tree, Swc_Tree_Node *tn, bool append)
{
  if (tree != NULL && tn != NULL) {
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;

    if (!append) {
      expandSwcNodeList(&deselected, tree->getSelectedNode(), tn);
    }
    selected.append(tn);

    tree->selectNode(tn, append);

    notifySelectionChanged(selected, deselected);
  }
}

void ZStackDoc::deselectSwcTreeNode(
    ZSwcTree *tree, Swc_Tree_Node *selected)
{
  if (tree != NULL && selected != NULL) {
    tree->deselectNode(selected);
    QList<Swc_Tree_Node*> nodeSet;
    nodeSet.append(selected);
    notifyDeselected(nodeSet);
  }
}

void ZStackDoc::selectSwcTreeNode(Swc_Tree_Node *tn, bool append)
{
  selectSwcTreeNode(nodeToSwcTree(tn), tn, append);
}

void ZStackDoc::deselectSwcTreeNode(Swc_Tree_Node *tn)
{
  deselectSwcTreeNode(nodeToSwcTree(tn), tn);
}

void ZStackDoc::selectHitSwcTreeNode(ZSwcTree *tree, bool append)
{
  if (tree != NULL) {
    tree->selectHitNode(append);
  }
}

void ZStackDoc::deselectHitSwcTreeNode(ZSwcTree *tree)
{
  QList<Swc_Tree_Node*> deselected;
  if (tree != NULL) {
    Swc_Tree_Node *tn = tree->deselectHitNode();
    if (tn != NULL) {
      deselected.append(tn);
    }
  }
  notifyDeselected(deselected);
}

void ZStackDoc::expandSwcNodeList(QList<Swc_Tree_Node*> *swcList,
                                  const std::set<Swc_Tree_Node*> &swcSet)
{
  if (swcList != NULL) {
    for (std::set<Swc_Tree_Node*>::const_iterator iter = swcSet.begin();
         iter != swcSet.end(); ++iter) {
      swcList->append(const_cast<Swc_Tree_Node*>(*iter));
    }
  }
}

void ZStackDoc::expandSwcNodeList(QList<Swc_Tree_Node*> *swcList,
                                  const std::set<Swc_Tree_Node*> &swcSet,
                                  const Swc_Tree_Node *excluded)
{
  if (swcList != NULL) {
    for (std::set<Swc_Tree_Node*>::const_iterator iter = swcSet.begin();
         iter != swcSet.end(); ++iter) {
      if (*iter != excluded) {
        swcList->append(const_cast<Swc_Tree_Node*>(*iter));
      }
    }
  }
}

Swc_Tree_Node *ZStackDoc::selectSwcTreeNode(int x, int y, int z, bool append)
{

  //QList<Swc_Tree_Node*> oldSet = getSelectedSwcNodeList();

  Swc_Tree_Node *hitNode = NULL;
  ZSwcTree *hitTree = NULL;

  QList<ZSwcTree*> treeList = getSwcList();
  foreach (ZSwcTree *tree, treeList) {
    if (tree->hit(x, y, z)) {
      hitTree = tree;
      hitNode = tree->getHitNode();
      break;
    }
  }

  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;

  if (!append) {
    foreach (ZSwcTree *tree, treeList) {
      if (tree != hitTree) {
        expandSwcNodeList(&deselected, tree->getSelectedNode());
      } else {
        expandSwcNodeList(&deselected, tree->getSelectedNode(), hitNode);
      }
      tree->deselectAllNode();
    }
  } else {
    if (hitTree->getSelectedNode().count(hitNode) == 0) {
      selected.append(hitNode);
    }
  }

  hitTree->selectHitNode(append);

  emit swcTreeNodeSelectionChanged(selected, deselected);

  return hitNode;
}

Swc_Tree_Node *ZStackDoc::selectSwcTreeNode(const ZPoint &pt, bool append)
{
  return selectSwcTreeNode(pt.x(), pt.y(), pt.z(), append);
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
      notifyStackModified();
    } else {
      if (mainStack->updateFromSource()) {
        notifyStackModified();
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
  QList<ZSwcTree*> swcList = getSwcList();
  foreach (ZSwcTree *tree, swcList) {
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

void ZStackDoc::notifyObject3dScanModified()
{
  emit object3dScanModified();
}

void ZStackDoc::notifyStackPatchModified()
{
  emit stackPatchModified();
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

void ZStackDoc::notify3DCubeModified()
{
  emit cube3dModified();
}

void ZStackDoc::notifyTodoModified()
{
  emit todoModified();
}

void ZStackDoc::notifyActiveViewModified()
{
  emit activeViewModified();
}

void ZStackDoc::clearObjectModifiedTypeBuffer(bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTypeBufferMutex);
    ZOUT(LTRACE(), 6) << "m_objectModifiedTypeBufferMutex locked";
    m_objectModifiedTypeBuffer.clear();
  } else {
    m_objectModifiedTypeBuffer.clear();
  }
}

void ZStackDoc::clearObjectModifiedTargetBuffer(bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTargetBufferMutex);
    m_objectModifiedTargetBuffer.clear();
  } else {
    m_objectModifiedTargetBuffer.clear();
  }
}

void ZStackDoc::clearObjectModifiedRoleBuffer(bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedRoleBufferMutex);
    m_objectModifiedRoleBuffer.clear();
  } else {
    m_objectModifiedRoleBuffer.clear();
  }
}

void ZStackDoc::notifyObjectModified(bool sync)
{
//  emit objectModified();

  if (getObjectModifiedMode() == OBJECT_MODIFIED_SIGNAL) {
    if (!m_objectModifiedTypeBuffer.empty()) {
      processObjectModified(m_objectModifiedTypeBuffer, sync);
      clearObjectModifiedTypeBuffer(sync);
    }

    if (!m_objectModifiedTargetBuffer.empty()) {
      processObjectModified(m_objectModifiedTargetBuffer, sync);
      clearObjectModifiedTypeBuffer(sync);
    }

    if (!m_objectModifiedRoleBuffer.isNone()) {
      processObjectModified(m_objectModifiedRoleBuffer.getRole(), sync);
      clearObjectModifiedRoleBuffer(sync);
    }
  }
}

void ZStackDoc::bufferObjectModified(ZStackObject::EType type, bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTypeBufferMutex);
    ZOUT(LTRACE(), 6) << "m_objectModifiedTypeBufferMutex locked";
    m_objectModifiedTypeBuffer.insert(type);
  } else {
    m_objectModifiedTypeBuffer.insert(type);
  }
}

void ZStackDoc::bufferObjectModified(const ZStackObjectRole &role, bool sync)
{
  bufferObjectModified(role.getRole(), sync);
}

void ZStackDoc::bufferObjectModified(ZStackObject::ETarget target, bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTargetBufferMutex);
    m_objectModifiedTargetBuffer.insert(target);
  } else {
    m_objectModifiedTargetBuffer.insert(target);
  }
}

void ZStackDoc::bufferObjectModified(
    const QSet<ZStackObject::EType> &typeSet, bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTypeBufferMutex);
    ZOUT(LTRACE(), 6) << "m_objectModifiedTypeBufferMutex locked";
    m_objectModifiedTypeBuffer.unite(typeSet);
  } else {
    m_objectModifiedTypeBuffer.unite(typeSet);
  }
}

void ZStackDoc::bufferObjectModified(
    const QSet<ZStackObject::ETarget> &targetSet, bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedTargetBufferMutex);
    m_objectModifiedTargetBuffer.unite(targetSet);
  } else {
    m_objectModifiedTargetBuffer.unite(targetSet);
  }
}

void ZStackDoc::bufferObjectModified(ZStackObject *obj, bool sync)
{
  bufferObjectModified(obj->getType(), sync);
  bufferObjectModified(obj->getTarget(), sync);
  bufferObjectModified(obj->getRole(), sync);
}

void ZStackDoc::bufferObjectModified(ZStackObjectRole::TRole role, bool sync)
{
  if (sync) {
    QMutexLocker locker(&m_objectModifiedRoleBufferMutex);
    m_objectModifiedRoleBuffer.addRole(role);
  } else {
    m_objectModifiedRoleBuffer.addRole(role);
  }
}

void ZStackDoc::processObjectModified(ZStackObject *obj, bool sync)
{
  processObjectModified(obj->getType(), sync);
  processObjectModified(obj->getTarget(), sync);
  processObjectModified(obj->getRole(), sync);
}

void ZStackDoc::processObjectModified(ZStackObject::ETarget target, bool sync)
{
  switch (getObjectModifiedMode()) {
  case OBJECT_MODIFIED_SIGNAL:
    emit objectModified(target);
    break;
  case OBJECT_MODIFIED_CACHE:
  {
    bufferObjectModified(target, sync);
  }
    break;
  default:
    break;
  }
}

void ZStackDoc::processObjectModified(
    const QSet<ZStackObject::ETarget> &targetSet, bool sync)
{
  switch (getObjectModifiedMode()) {
  case OBJECT_MODIFIED_SIGNAL:
    emit objectModified(targetSet);
    break;
  case OBJECT_MODIFIED_CACHE:
  {
    bufferObjectModified(targetSet, sync);
  }
    break;
  default:
    break;
  }
}

void ZStackDoc::processObjectModified(ZStackObject::EType type, bool sync)
{
  switch (getObjectModifiedMode()) {
  case OBJECT_MODIFIED_SIGNAL:
    notifyObjectModified(type);
    break;
  case OBJECT_MODIFIED_CACHE:
    bufferObjectModified(type, sync);
//    m_objectModifiedTargetBuffer.unite(targetSet);
    break;
  default:
    break;
  }
}

void ZStackDoc::processSwcModified()
{
  processObjectModified(ZStackObject::TYPE_SWC);
  processObjectModified(ZSwcTree::GetDefaultTarget());
}

void ZStackDoc::processObjectModified(const ZStackObjectRole &role, bool sync)
{
  processObjectModified(role.getRole(), sync);
}

void ZStackDoc::processObjectModified(ZStackObjectRole::TRole role, bool sync)
{
  switch (getObjectModifiedMode()) {
  case OBJECT_MODIFIED_SIGNAL:
    notifyPlayerChanged(role);
    break;
  case OBJECT_MODIFIED_CACHE:
    bufferObjectModified(role, sync);
//    m_objectModifiedTargetBuffer.unite(targetSet);
    break;
  default:
    break;
  }
}

void ZStackDoc::notifyObjectModified(ZStackObject::EType type)
{
  switch (type) {
  case ZStackObject::TYPE_LOCSEG_CHAIN:
    notifyChainModified();
    break;
  case ZStackObject::TYPE_OBJ3D:
    notifyObj3dModified();
    break;
  case ZStackObject::TYPE_SWC:
    notifySwcModified();;
    break;
  case ZStackObject::TYPE_PUNCTUM:
    notifyPunctumModified();
    break;
  case ZStackObject::TYPE_STROKE:
    notifyStrokeModified();
    break;
  case ZStackObject::TYPE_SPARSE_OBJECT:
    notifySparseObjectModified();
    break;
  case ZStackObject::TYPE_OBJECT3D_SCAN:
    notifyObject3dScanModified();
    break;
  case ZStackObject::TYPE_3D_GRAPH:
    notify3DGraphModified();
    break;
  case ZStackObject::TYPE_3D_CUBE:
    notify3DCubeModified();
  case ZStackObject::TYPE_FLYEM_TODO_ITEM:
    notifyTodoModified();
    break;
  default:
//    notifyObjectModified();
    break;
  }

  setSaved(type, false);

  customNotifyObjectModified(type);
}


void ZStackDoc::processObjectModified(
    const QSet<ZStackObject::EType> &typeSet, bool sync)
{
  if (sync) {
    QSet<ZStackObject::EType> bufferTypeSet;
    {
      QMutexLocker locker(&m_objectModifiedTypeBufferMutex);
      ZOUT(LTRACE(), 5) << "m_objectModifiedTypeBufferMutex locked";
      bufferTypeSet = typeSet;
    }

    for (QSet<ZStackObject::EType>::const_iterator iter = bufferTypeSet.begin();
         iter != bufferTypeSet.end(); ++iter) {
      ZStackObject::EType type = *iter;
      processObjectModified(type, false);
    }
  } else {
    for (QSet<ZStackObject::EType>::const_iterator iter = typeSet.begin();
         iter != typeSet.end(); ++iter) {
      ZStackObject::EType type = *iter;
      processObjectModified(type, false);
    }
  }
}

bool ZStackDoc::watershed()
{
  ZStack *mainStack = getStack();
  m_progressReporter->start();
  m_progressReporter->advance(0.5);
  if (mainStack != NULL) {
    if (mainStack->watershed()) {
      notifyStackModified();
      return true;
    }
  }
  m_progressReporter->end();

  return false;
}

int ZStackDoc::findLoop(int minLoopSize)
{
  int loopNumber = 0;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
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
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
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
        addObject(obj);
        ++loopNumber;
      }
    }
    endObjectModifiedMode();
    notifyObjectModified();
    m_progressReporter->endSubprogress(0.3);

    m_progressReporter->advance(0.1);

    /*
    if (loopNumber > 0) {
      emit obj3dModified();
    }
    */

    delete graph;
    Kill_Stack(shrinked);
    Kill_Stack(data);

    m_progressReporter->end();
  }
  endObjectModifiedMode();
  notifyObjectModified();

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
      notifyStackModified();
    }

    m_progressReporter->end();
  }
}

void ZStackDoc::executeSwcRescaleCommand(const ZRescaleSwcSetting &setting)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

  ZUndoCommand *allcommand = new ZUndoCommand();
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
    allcommand->setLogMessage(allcommand->text());
    pushUndoCommand(allcommand);
    deprecateTraceMask();
  } else {
    delete allcommand;
  }

  endObjectModifiedMode();
  notifyObjectModified();
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center)
{
  bool succ = false;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  ZUndoCommand *command = NULL;
  QList<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeList();
  if (!nodeSet.empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet.begin());
    if (prevNode != NULL) {
      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], SwcTreeNode::radius(prevNode));
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(this, tn, prevNode);
      }
    }
  }

  if (command != NULL) {
    command->setLogMessage("Extend SWC node");
    pushUndoCommand(command);
    deprecateTraceMask();
    succ = true;
  }
  endObjectModifiedMode();
  notifyObjectModified();

  return succ;
}

bool ZStackDoc::executeSwcNodeExtendCommand(const ZPoint &center, double radius)
{
  bool succ = false;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  ZUndoCommand *command = NULL;
  QList<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeList();
  if (!nodeSet.empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet.begin());
    if (prevNode != NULL) {
//      if (center[0] >= 0 && center[1] >= 0 && center[2] >= 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              center[0], center[1], center[2], radius);
        command  = new ZStackDocCommand::SwcEdit::ExtendSwcNode(
              this, tn, prevNode);
//      }
    }
  }

  if (command != NULL) {
    command->setLogMessage("Extend SWC node");
    pushUndoCommand(command);
    deprecateTraceMask();
    succ = true;
  }
  endObjectModifiedMode();
  notifyObjectModified();

  return succ;
}

bool ZStackDoc::executeSwcNodeSmartExtendCommand(const ZPoint &center)
{
//  QUndoCommand *command = NULL;

  QList<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeList();
 // std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (!nodeSet.empty()) {
    Swc_Tree_Node *prevNode = *(nodeSet.begin());
    if (prevNode != NULL) {
      return executeSwcNodeSmartExtendCommand(
            center, SwcTreeNode::radius(prevNode));
    }
  }

  return false;
}

void ZStackDoc::setStackBc(double factor, double offset, int channel)
{
  if (channel == 0) {
    if (factor != 1.0 || offset != 0.0) {
      m_neuronTracer.setBcAdjust(true);
      m_neuronTracer.setGreyFactor(factor);
      m_neuronTracer.setGrayOffset(offset);
    } else {
      m_neuronTracer.setBcAdjust(false);
    }
  }
}

void ZStackDoc::notify(const ZWidgetMessage &msg)
{
  emit messageGenerated(msg);
}

void ZStackDoc::notify(const QString &msg)
{
  notify(ZWidgetMessage(msg));
}

void ZStackDoc::notifyUpdateLatency(int64_t t)
{
  emit updatingLatency((int) t);
}

bool ZStackDoc::executeSwcNodeSmartExtendCommand(
    const ZPoint &center, double radius)
{
  if (!hasStackData()) {
    return false;
  }

  bool succ = false;
  QString message;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

  ZUndoCommand *command = NULL;

  ZSwcTree *hostTree = NULL;
  ZOUT(LTRACE(), 5) << "Extend swc node";
  const TStackObjectList &objList = getObjectList(ZStackObject::TYPE_SWC);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZSwcTree *tree = const_cast<ZSwcTree*>(
          dynamic_cast<const ZSwcTree*>(*iter));
    if (tree->getSelectedNode().size() == 1) {
      hostTree = tree;
    }
  }
  //std::set<Swc_Tree_Node*> *nodeSet = selectedSwcTreeNodes();
  if (hostTree != NULL) {
    Swc_Tree_Node *prevNode = *(hostTree->getSelectedNode().begin());
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
        /*
        if (GET_APPLICATION_NAME == "Biocytin") {
          m_neuronTracer.setResolution(1, 1, 10);
        }
        */
        m_neuronTracer.setResolution(getResolution().voxelSizeX(),
                                     getResolution().voxelSizeY(),
                                     getResolution().voxelSizeZ());

//        m_neuronTracer.setStackOffset(getStackOffset());

//        if (getTag() == NeuTube::Document::FLYEM_ROI) {
//          m_neuronTracer.useEdgePath(true);
//        }

        if (getTag() == NeuTube::Document::FLYEM_ROI) {
          m_neuronTracer.setEstimatingRadius(false);
        }

        Swc_Tree *branch = m_neuronTracer.trace(
              SwcTreeNode::x(prevNode), SwcTreeNode::y(prevNode),
              SwcTreeNode::z(prevNode), SwcTreeNode::radius(prevNode),
              center.x(), center.y(), center.z(), radius);
        if (branch != NULL) {
          if (Swc_Tree_Has_Branch(branch)) {
            //tracer.updateMask(branch);
            Swc_Tree_Node *root = Swc_Tree_Regular_Root(branch);
            Swc_Tree_Node *begin = SwcTreeNode::firstChild(root);

            Swc_Tree_Node *leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
            }
            ZSwcPath originalPath(root, leaf);
            if (getTag() == NeuTube::Document::BIOCYTIN_STACK) {
              originalPath.smooth(true);
            } else {
              originalPath.smoothRadius(true);
            }
            originalPath.smoothZ();

            if (SwcTreeNode::hasChild(begin)) {
              if (SwcTreeNode::hasSignificantOverlap(begin, prevNode)) {
                begin = SwcTreeNode::firstChild(begin);
              }
            }

            SwcTreeNode::detachParent(begin);
            Kill_Swc_Tree(branch);


            ZSwcPath path(begin, leaf);
            for (ZSwcPath::iterator iter = path.begin(); iter != path.end();
                 ++iter) {
              Swc_Tree_Node *tn = *iter;
              tn->tree_state = prevNode->tree_state;
            }

//            resampler.setDistanceScale(3.0);
//            resampler.setRadiusScale(0.0);

            ZSwcTree tree;
            tree.setDataFromNode(begin);

//            Swc_Tree_Remove_Zigzag(tree.data());
#ifdef _DEBUG_2
            tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif


//            resampler.denseInterpolate(&tree);

            begin = tree.firstRegularRoot();

            leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
              SwcTreeNode::correctTurn(leaf);
            }

            ZSwcResampler resampler;
            resampler.ignoreInterRedundant(true);
            resampler.optimalDownsample(&tree);
            begin = tree.firstRegularRoot();

            int count = 1;
            leaf = begin;
            while (SwcTreeNode::firstChild(leaf) != NULL) {
              leaf = SwcTreeNode::firstChild(leaf);
              ++count;
            }

            if (count == 2) {
              if (SwcTreeNode::hasSignificantOverlap(leaf, begin)) {
                SwcTreeNode::mergeToParent(leaf, SwcTreeNode::MERGE_AVERAGE);
                leaf = begin;
              }
            }

            message = QString("%1 nodes are added").arg(tree.size(begin));

            tree.setData(NULL, ZSwcTree::LEAVE_ALONE);

//            free(branch);

            command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
            new ZStackDocCommand::SwcEdit::AddSwcNode(
                  this, begin, ZStackObjectRole::ROLE_NONE, command);
            std::set<Swc_Tree_Node*> nodeSet;
            nodeSet.insert(leaf);
            //new ZStackDocCommand::SwcEdit::SetSwcNodeSeletion(
           //       this, nodeSet, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, begin, prevNode, false, command);
            new ZStackDocCommand::SwcEdit::SetSwcNodeSeletion(
                  this, hostTree, nodeSet, false, command);
            new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, command);
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
    command->setLogMessage("Extend SWC node (path computation)");
    pushUndoCommand(command);
    deprecateTraceMask();
    notifyStatusMessageUpdated(message);
    succ = true;
  }
  endObjectModifiedMode();
  notifyObjectModified();

  return succ;
}

bool ZStackDoc::executeInterpolateSwcZCommand()
{
  bool succ = false;
  QString message;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (hasSelectedSwcNode()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               nodeSet.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               nodeSet.count(downEnd) == 1) { /* continuation and selected*/
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
      allCommand->setLogMessage("Interpolate SWC node");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("The Z coordinates of %1 node(s) are intepolated").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  endObjectModifiedMode();
  notifyObjectModified();

  notifyStatusMessageUpdated(message);


  return succ;
}

bool ZStackDoc::executeInterpolateSwcPositionCommand()
{
  bool succ = false;
  QString message;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (hasSelectedSwcNode()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               nodeSet.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               nodeSet.count(downEnd) == 1) { /* continuation and selected*/
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
      allCommand->setLogMessage("Interpolate SWC postion");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("The coordinates of %1 node(s) are intepolated").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  endObjectModifiedMode();
  notifyObjectModified();

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeInterpolateSwcCommand()
{
  bool succ = false;
  QString message;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (hasSelectedSwcNode()) {
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               nodeSet.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               nodeSet.count(downEnd) == 1) { /* continuation and selected*/
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
      allCommand->setLogMessage("Interpolate SWC");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("%1 node(s) are interpolated").arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
  }

  endObjectModifiedMode();
  notifyObjectModified();

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeInterpolateSwcRadiusCommand()
{
  bool succ = false;
  QString message;
  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      if (SwcTreeNode::isContinuation(*iter)) {
        Swc_Tree_Node *upEnd = SwcTreeNode::parent(*iter);
        while (SwcTreeNode::isContinuation(upEnd) &&
               nodeSet.count(upEnd) == 1) { /* continuation and selected*/
          upEnd = SwcTreeNode::parent(upEnd);
        }

        Swc_Tree_Node *downEnd = SwcTreeNode::firstChild(*iter);
        while (SwcTreeNode::isContinuation(downEnd) &&
               nodeSet.count(downEnd) == 1) { /* continuation and selected*/
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
      allCommand->setLogMessage("Interpolate SWC radius");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("Radii of %1 node(s) are interpolated.").
          arg(allCommand->childCount());
    } else {
      delete allCommand;
    }

    succ = true;
    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeSwcNodeChangeZCommand(double z)
{
  bool succ = false;
  QString message;
  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    QList<Swc_Tree_Node*> nodeList = getSelectedSwcNodeList();
    for (QList<Swc_Tree_Node*>::iterator iter = nodeList.begin();
         iter != nodeList.end(); ++iter) {
      if (*iter != NULL && SwcTreeNode::z(*iter) != z) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeZ(
              this, *iter, z, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Change Z of Selected Node"));
      allCommand->setLogMessage("Change Z of SWC node");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = QString("Z of %1 node(s) is set to %2").
          arg(allCommand->childCount()).arg(z);
    } else {
      delete allCommand;
    }
    succ = true;
    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeMoveSwcNodeCommand(double dx, double dy, double dz)
{
  bool succ = false;
  QString message;
  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);

    QList<Swc_Tree_Node*> nodeList = getSelectedSwcNodeList();
    for (QList<Swc_Tree_Node*>::iterator iter = nodeList.begin();
         iter != nodeList.end(); ++iter) {
      if (*iter != NULL && (dx != 0 || dy != 0 || dz != 0)) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::translate(&newNode, dx, dy, dz);
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Move Selected Node"));
      allCommand->setLogMessage("Move SWC node");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      message = "Nodes moved.";
    } else {
      delete allCommand;
    }

    succ = true;
    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);
  return succ;
}

bool ZStackDoc::executeTranslateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::center(node + 1) - SwcTreeNode::center(node);
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

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
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
      allCommand->setLogMessage("Translate SWC node");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
      return true;
    }
  }

  return false;
}

bool ZStackDoc::executeChangeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
      ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        Swc_Tree_Node newNode = *(*iter);
        SwcTreeNode::changeRadius(&newNode, dlg.getAddValue(), dlg.getMulValue());
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
      }

      allCommand->setLogMessage("Change SWC node size");
      pushUndoCommand(allCommand);
      deprecateTraceMask();

      endObjectModifiedMode();
      notifyObjectModified();

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

  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);

    QList<Swc_Tree_Node*> nodeList = getSelectedSwcNodeList();
    for (QList<Swc_Tree_Node*>::iterator iter = nodeList.begin();
         iter != nodeList.end(); ++iter) {
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

      allCommand->setLogMessage("Change SWC node size");
      pushUndoCommand(allCommand);
      deprecateTraceMask();

      message = QString("Size(s) of %1 node(s) are changed.").arg(nodeCount);
    } else {
      delete allCommand;
    }
    endObjectModifiedMode();
    notifyObjectModified();

    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::estimateSwcNodeRadius(Swc_Tree_Node *tn, int maxIter)
{
  ZSwcSignalFitter fitter;
  fitter.setBackground(getStackBackground());

  int channel = 0;
  if (getStack()->channelNumber() == 3 &&
      getTag() == NeuTube::Document::BIOCYTIN_STACK) {
    channel = 1;
  }

  bool succ = false;

  for (int iter = 0; iter < maxIter; ++iter) {
    bool currentStat = fitter.fitSignal(tn, getStack(), channel);
    if (currentStat) {
      succ = true;
    } else { //Fitting failed. No need to continue.
      break;
    }
  }

  return succ;
}

void ZStackDoc::estimateSwcRadius(ZSwcTree *tree, int maxIter)
{
  if (tree != NULL) {
    startProgress();
    int count = tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    double step = 1.0 / count / maxIter;
//    ZSwcSignalFitter fitter;
//    fitter.setBackground(getStackBackground());

    for (int iter = 0; iter < maxIter; ++iter) {
      for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
        if (SwcTreeNode::isRegular(tn)) {
          estimateSwcNodeRadius(tn, maxIter);
        }
//        fitter.fitSignal(tn, getStack(), 0);
        advanceProgress(step);
      }
    }
    endProgress();
  }
}

void ZStackDoc::estimateSwcRadius()
{
  QList<ZSwcTree*> swcList = getSwcList();
  foreach (ZSwcTree *tree, swcList){
    estimateSwcRadius(tree);
  }
}

bool ZStackDoc::executeSwcNodeEstimateRadiusCommand()
{
  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    startProgress();

    /*
    ZSwcSignalFitter fitter;
    fitter.setBackground(getStackBackground());

    int channel = 0;
    if (getStack()->channelNumber() == 3 &&
        getTag() == NeuTube::Document::BIOCYTIN_STACK) {
      channel = 1;
    }
    */

    QList<Swc_Tree_Node*> nodeList = getSelectedSwcNodeList();
    double step = 1.0 / nodeList.size();
    for (QList<Swc_Tree_Node*>::iterator iter = nodeList.begin();
         iter != nodeList.end(); ++iter) {
      Swc_Tree_Node newNode = *(*iter);
      bool succ = estimateSwcNodeRadius(&newNode, 1);
//      bool succ = fitter.fitSignal(&newNode, getStack(), channel);
      if (succ) {
        //SwcTreeNode::translate(&newNode, offset.getX(), offset.getY(),
          //                     offset.getZ());
        new ZStackDocCommand::SwcEdit::ChangeSwcNode(
              this, *iter, newNode, allCommand);
        advanceProgress(step);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Node - Estimate Radius"));
      allCommand->setLogMessage("Estimate SWC node radius");
      pushUndoCommand(allCommand);
      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    endProgress();

    endObjectModifiedMode();
    notifyObjectModified();

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
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  if (nodeSet.size() > 1 &&
      /*SwcTreeNode::isAllConnected(*selectedSwcTreeNodes()) &&*/
      isMergable(nodeSet)) {
    message = QString("%1 nodes are merged.").arg(nodeSet.size());
    ZUndoCommand *command = new ZStackDocCommand::SwcEdit::MergeSwcNode(this);
    command->setLogMessage("Merge SWC nodes");
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

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    Swc_Tree_Node *tn = *nodeSet.begin();
    if (!SwcTreeNode::isRoot(tn)) {
      ZUndoCommand *command =
          new ZStackDocCommand::SwcEdit::SetRoot(this, tn);
      command->setLogMessage("Set SWC root");
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

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  if (nodeSet.size() == 1) {
    Swc_Tree_Node *tn = *(nodeSet.begin());
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
      ZUndoCommand *command =
          new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
            this, tn, x, y, z, r);
      command->setLogMessage("Remove turning node");
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

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();

  QString message = "No crossover is detected. Nothing is done";
  if (nodeSet.size() == 1) {
    ZStackDocCommand::SwcEdit::ResolveCrossover *command =
        new ZStackDocCommand::SwcEdit::ResolveCrossover(this);

    command->setLogMessage("Remove crossover");
    pushUndoCommand(command);

    succ = command->isSwcModified();
    if (succ) {
      message = "A crossover is created.";
    }
  } else {
    message = "Nothing done. Exactly one node should be selected.";
  }

  notify(ZWidgetMessage(
           message, NeuTube::MSG_INFORMATION,
           ZWidgetMessage::TARGET_STATUS_BAR));

  return succ;

#if 0
  bool succ = false;
  QString message;

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    Swc_Tree_Node *center = *(nodeSet.begin());
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
                this, iter->first, iter->second, false, command);
          new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->second, root, false, command);
        } else {
          new ZStackDocCommand::SwcEdit::SetParent(
                this, center, root, false, command);
          if (SwcTreeNode::parent(iter->first) == center) {
            new ZStackDocCommand::SwcEdit::SetParent(
                this, iter->first, iter->second, false, command);
          } else {
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, iter->second, iter->first, false, command);
          }
        }

        if (matched.size() * 2 == SwcTreeNode::neighborArray(center).size()) {
          new ZStackDocCommand::SwcEdit::SetParent(
                this, center, NULL, true, command);
          /*
          new ZStackDocCommand::SwcEdit::DeleteSwcNode(
                this, center, root, command);
                */
        }
      }
      pushUndoCommand(command);
      deprecateTraceMask();

      message = "A crossover is created.";
      succ = true;

      endObjectModifiedMode();
      notifyObjectModified();
    } else {
      message = "No crossover is detected. Nothing is done";
    }
  }

  notifyStatusMessageUpdated(message);

  return succ;
#endif
}

bool ZStackDoc::executeWatershedCommand()
{
  if (hasStackData()) {
    ZUndoCommand *command = new ZStackDocCommand::StackProcess::Watershed(this);
    command->setLogMessage("Run watershed");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

void ZStackDoc::executeRemoveRectRoiCommand()
{
  QUndoCommand *command = new QUndoCommand;
  TStackObjectList objList = getObjectGroup().findSameSource(
        ZStackObjectSourceFactory::MakeRectRoiSource());
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    new ZStackDocCommand::ObjectEdit::RemoveObject(this, *iter, command);
  }

  if (command->childCount() > 0) {
    pushUndoCommand(command);
  } else {
    delete command;
  }
}

bool ZStackDoc::executeBinarizeCommand(int thre)
{
  if (hasStackData()) {
    ZUndoCommand *command =
        new ZStackDocCommand::StackProcess::Binarize(this, thre);
    command->setLogMessage("Binarize image");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeBwsolidCommand()
{
  if (hasStackData()) {
    ZUndoCommand *command = new ZStackDocCommand::StackProcess::BwSolid(this);

    command->setLogMessage("Solidfy binary image");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeEnhanceLineCommand()
{
  if (hasStackData()) {
    ZUndoCommand *command =
        new ZStackDocCommand::StackProcess::EnhanceLine(this);
    command->setLogMessage("Enhance line");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeDeleteSwcNodeCommand()
{
  bool succ = false;
  QString message;

  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
//    QSet<ZStackObject::ETarget> targetSet;

    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    TStackObjectList &objList = getObjectList(ZStackObject::TYPE_SWC);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
//      targetSet.insert(tree->getTarget());
      if (tree->hasSelectedNode()) {
        processObjectModified(tree);
        new ZStackDocCommand::SwcEdit::SetSwcNodeSeletion(
              this, tree, std::set<Swc_Tree_Node*>(), false, allCommand);
      }
    }

    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    new ZStackDocCommand::SwcEdit::DeleteSwcNodeSet(this, nodeSet, allCommand);
    new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, allCommand);

    if (allCommand->childCount() > 0) {
      message = QString("%1 node(s) are deleted").arg(nodeSet.size());
      allCommand->setText(QObject::tr("Delete Selected Node"));
//      blockSignals(true);
      allCommand->setLogMessage("Delete SWC node");
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
      deselectAllSwcTreeNodes();
      //m_selectedSwcTreeNodes.clear();
//      blockSignals(false);

//      notifySwcModified();
//      processObjectModified(targetSet);

      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    succ = true;

    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeDeleteUnselectedSwcNodeCommand()
{
  bool succ = false;
  QString message;

  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
//    QSet<ZStackObject::ETarget> targetSet;

    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
//    TStackObjectList &objList = getObjectList(ZStackObject::TYPE_SWC);

    std::set<Swc_Tree_Node*> nodeSet;

    ZOUT(LTRACE(), 5) << "Delete unselected";
    TStackObjectList objList = getObjectList(ZStackObject::TYPE_SWC);

    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZSwcTree* tree = dynamic_cast<ZSwcTree*>(*iter);
      if (tree->hasSelectedNode()) {
        ZSwcTree::DepthFirstIterator treeIter(tree);
        treeIter.excludeVirtual(true);

        while (treeIter.hasNext()) {
          Swc_Tree_Node *tn = treeIter.next();
          if (!tree->isNodeSelected(tn)) {
            nodeSet.insert(tn);
          }
        }
      } else {
        new ZStackDocCommand::SwcEdit::RemoveSwc(this, tree, allCommand);
      }
    }


//    std::set<Swc_Tree_Node*> nodeSet = getUnselectedSwcNodeSet();

    new ZStackDocCommand::SwcEdit::DeleteSwcNodeSet(this, nodeSet, allCommand);
    new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, allCommand);

    if (allCommand->childCount() > 0) {
      message = QString("%1 node(s) are deleted").arg(nodeSet.size());
      allCommand->setLogMessage(QObject::tr("Delete Selected Node"));
//      blockSignals(true);
      pushUndoCommand(allCommand);
#ifdef _DEBUG_2
      m_swcList[0]->print();
#endif
//      deselectAllSwcTreeNodes();
      //m_selectedSwcTreeNodes.clear();
//      blockSignals(false);

//      notifySwcModified();
//      processObjectModified(targetSet);

      deprecateTraceMask();
    } else {
      delete allCommand;
    }

    succ = true;

    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeConnectSwcNodeCommand()
{
  bool succ = false;
  QString message;

  if (getSelectedSwcNodeNumber() > 1) {
    ZUndoCommand *command =  new ZStackDocCommand::SwcEdit::ConnectSwcNode(this);
    command->setLogMessage("Set SWC node");
    pushUndoCommand(command);
    deprecateTraceMask();

    message = "Nodes are connected, except those too far away.";
    succ = true;
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeConnectSwcNodeCommand(Swc_Tree_Node *tn)
{
  if (hasSelectedSwcNode()) {
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          nodeSet, tn);
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  ZUndoCommand *command =
      new ZStackDocCommand::SwcEdit::CompositeCommand(this);

  Swc_Tree_Node *upNode = tn1;
  Swc_Tree_Node *downNode = tn2;

  ZSwcTree *tree1 = nodeToSwcTree(tn1);
  ZSwcTree *tree2 = nodeToSwcTree(tn2);

  if (tree1 != tree2) {
    //Check source
    if (ZFileType::fileType(tree2->getSource()) == ZFileType::SWC_FILE) {
      upNode = tn2;
      downNode = tn1;
    }
  }

  new ZStackDocCommand::SwcEdit::SetRoot(this, downNode, command);
  new ZStackDocCommand::SwcEdit::SetParent(this, downNode, upNode, false, command);
  new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, command);

  command->setLogMessage("Connect SWC node");
  pushUndoCommand(command);
  deprecateTraceMask();

  endObjectModifiedMode();
  notifyObjectModified();

//  processSwcModified();

//  notifySwcModified();
//  processObjectModified(ZSwcTree::GetDefaultTarget());

  message = "Two nodes are connected.";
  notifyStatusMessageUpdated(message);

  return true;
}

bool ZStackDoc::executeSmartConnectSwcNodeCommand()
{
  if (getSelectedSwcNodeNumber() == 2) {
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    std::set<Swc_Tree_Node*>::iterator first = nodeSet.begin();
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

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
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

      ZSwcPath path(root, leaf);
      path.smooth(true);
      path.smoothZ();

      ZUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);
      new ZStackDocCommand::SwcEdit::SetRoot(this, tn2, command);

      if (branch != NULL) {
        SwcTreeNode::detachParent(begin);
        Kill_Swc_Tree(branch);

        Swc_Tree_Node *terminal = SwcTreeNode::parent(leaf);
        SwcTreeNode::detachParent(leaf);
        SwcTreeNode::kill(leaf);

        ZSwcResampler resampler;
        resampler.setDistanceScale(1.5);

        ZSwcTree tree;
        tree.setDataFromNode(begin);

//            Swc_Tree_Remove_Zigzag(tree.data());
        resampler.optimalDownsample(&tree);
        begin = tree.firstRegularRoot();

        leaf = begin;
        while (SwcTreeNode::firstChild(leaf) != NULL) {
          leaf = SwcTreeNode::firstChild(leaf);
          SwcTreeNode::correctTurn(leaf);
        }

        tree.setData(NULL, ZSwcTree::LEAVE_ALONE);

        new ZStackDocCommand::SwcEdit::SetParent(
              this, tn2, terminal, false, command);
        new ZStackDocCommand::SwcEdit::SetParent(
              this, begin, tn1, false, command);
      } else {
        new ZStackDocCommand::SwcEdit::SetParent(
              this, tn2, tn1, false, command);
      }
      new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, command);

      command->setLogMessage("Connect SWC node with path computation");
      pushUndoCommand(command);
      deprecateTraceMask();

//      notifySwcModified();
      message = "Nodes are connected";
      succ = true;
    }
  }

  endObjectModifiedMode();
  notifyObjectModified();

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeBreakSwcConnectionCommand()
{
  bool succ = false;

  QString message;
  if (hasSelectedSwcNode()) {
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    ZStackDocCommand::SwcEdit::CompositeCommand *allCommand =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      if (nodeSet.count(SwcTreeNode::parent(*iter)) > 0) {
        new ZStackDocCommand::SwcEdit::SetParent(
              this, *iter, SwcTreeNode::root(*iter), false, allCommand);
      }
    }

    if (allCommand->childCount() > 0) {
      allCommand->setText(QObject::tr("Break Connection"));
      blockSignals(true);
      allCommand->setLogMessage("Break SWC link");
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

    endObjectModifiedMode();
    notifyObjectModified();
  }

  notifyStatusMessageUpdated(message);

  return succ;
}

bool ZStackDoc::executeBreakForestCommand()
{
  if (!m_objectGroup.getSelectedSet(ZStackObject::TYPE_SWC).empty()) {
    ZUndoCommand *command = new ZStackDocCommand::SwcEdit::BreakForest(this);
    command->setLogMessage("Break SWC forest");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeGroupSwcCommand()
{
  if (m_objectGroup.getSelectedSet(ZStackObject::TYPE_SWC).size() > 1) {
    ZUndoCommand *command = new ZStackDocCommand::SwcEdit::GroupSwc(this);
    command->setLogMessage("Group SWC");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAddSwcBranchCommand(ZSwcTree *tree, double minConnDist)
{
  bool succ = false;

  if (tree != NULL) {
    if (tree->isEmpty()) {
      return false;
    }
    if (hasSwc()) {
      ZUndoCommand *command =
          new ZStackDocCommand::SwcEdit::CompositeCommand(this);

      ZSwcConnector connector;
      connector.setMinDist(Infinity);
      connector.setResolution(getResolution());
      ZSwcForest *forest = tree->toSwcTreeArray();

      const std::vector<ZSwcTree*> &treeArray = getSwcArray();

      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

      for (ZSwcForest::const_iterator iter = forest->begin();
           iter != forest->end(); ++iter) {
        ZSwcTree *subtree = *iter;
        subtree->forceVirtualRoot();

        std::pair<Swc_Tree_Node*, Swc_Tree_Node*> connPair =
            connector.identifyConnection(*subtree, treeArray);
        if (connPair.first != NULL && connPair.second != NULL) {
          ZSwcTree *hostTree = nodeToSwcTree(connPair.second);
          SwcTreeNode::setAsRoot(connPair.first);
          SwcTreeNode::detachParent(connPair.first);
          if (connector.getConnDist() <= minConnDist) {
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, connPair.first, connPair.second, true, command);
          } else {
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, connPair.first, hostTree->root(), true, command);
          }
          processObjectModified(hostTree);
        }
      }

      if (command->childCount() > 0) {
        command->setLogMessage("Add SWC branch");
        pushUndoCommand(command);
        forest->print();
        delete tree;
        delete forest;
        succ = true;
      } else {
        delete command;
      }

      endObjectModifiedMode();
      notifyObjectModified();
    } else {
      succ = executeAddSwcCommand(tree);
    }
  }

  return succ;
}

bool ZStackDoc::executeAddSwcCommand(ZSwcTree *tree)
{
  if (tree != NULL) {
    ZUndoCommand *command = new ZStackDocCommand::SwcEdit::AddSwc(this, tree);
    command->setLogMessage("Add SWC tree");
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

bool ZStackDoc::executeReplaceSwcCommand(ZSwcTree *tree)
{
  ZUndoCommand *command = new ZStackDocCommand::SwcEdit::CompositeCommand(this);

  bool succ = false;


  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);

  {
    QMutexLocker locker(getPlayerList().getMutex());
    const QList<ZDocPlayer*>& playerList = getPlayerList().getPlayerList();
    for (QList<ZDocPlayer*>::const_iterator iter = playerList.begin();
         iter != playerList.end(); ++iter) {
      ZDocPlayer *player = *iter;
      if (player->hasRole(tree->getRole().getRole())) {
        ZSwcTree *roleTree = dynamic_cast<ZSwcTree*>(player->getData());
        if (roleTree != NULL) {
          new ZStackDocCommand::SwcEdit::RemoveSwc(this, roleTree, command);
        }
      }
    }
  }


  if (tree != NULL) {
    new ZStackDocCommand::ObjectEdit::AddObject(this, tree, false, command);
  }

  if (command->childCount() > 0) {
    command->setLogMessage("Replace SWC tree");
    pushUndoCommand(command);
    succ = true;
  } else {
    delete command;
  }
  endObjectModifiedMode();
  notifyObjectModified();

  return succ;
}

bool ZStackDoc::executeAddSwcNodeCommand(const ZPoint &center, double radius,
                                         ZStackObjectRole::TRole role)
{
  if (radius > 0) {
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(center, radius);
    ZStackDocCommand::SwcEdit::AddSwcNode *command = new
        ZStackDocCommand::SwcEdit::AddSwcNode(this, tn, role);
    command->setLogMessage("Add SWC node");
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

void ZStackDoc::addObjectFast(ZStackObject *obj)
{
  if (obj == NULL) {
    return;
  }

  if (obj->isSelected()) {
    setSelected(obj, true);
  }

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  switch (obj->getType()) {
  case ZStackObject::TYPE_SWC:
  {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
    if (tree != NULL) {
      addSwcTreeP(tree);
      if (obj->hasRole(ZStackObjectRole::ROLE_ROI)) {
        tree->useCosmeticPen(true);
        tree->updateHostState();
      }
    }
  }
    break;
  case ZStackObject::TYPE_PUNCTUM:
    addPunctumP(dynamic_cast<ZPunctum*>(obj));
    break;
  case ZStackObject::TYPE_OBJ3D:
    addObj3dP(dynamic_cast<ZObject3d*>(obj));
    break;
  case ZStackObject::TYPE_OBJECT3D_SCAN:
  {
    ZObject3dScan *comObj = dynamic_cast<ZObject3dScan*>(obj);
    if (comObj != NULL) {
      if (obj->hasRole(ZStackObjectRole::ROLE_MASK)) {
        /*
        int index = m_objectGroup.getObjectList(
              ZStackObject::TYPE_OBJECT3D_SCAN).size() + 1;
              */
        QColor color = m_objColorSheme.getColor(abs((int) comObj->getLabel()));
        color.setAlpha(64);
        obj->setColor(color);
      }
      addObject3dScanP(comObj);
    }
  }
    break;
  case ZStackObject::TYPE_LOCSEG_CHAIN:
    addLocsegChainP(dynamic_cast<ZLocsegChain*>(obj));
    break;
  case ZStackObject::TYPE_STROKE:
    addStrokeP(dynamic_cast<ZStroke2d*>(obj));
    break;
  case ZStackObject::TYPE_SPARSE_OBJECT:
    addSparseObjectP(dynamic_cast<ZSparseObject*>(obj));
    break;
  case ZStackObject::TYPE_STACK_PATCH:
    addStackPatchP(dynamic_cast<ZStackPatch*>(obj));
    break;
  default:
    //addObject(obj);
    m_objectGroup.add(obj, false);
    break;
  }

  addPlayer(obj);
  processObjectModified(obj);

  endObjectModifiedMode();

  notifyObjectModified(true);
}

void ZStackDoc::addObject(ZStackObject *obj, bool uniqueSource)
{
  //QMutexLocker locker(&m_mutex);

  if (obj == NULL) {
    return;
  }

  {
    QMutexLocker locker(m_objectGroup.getMutex());

    if (m_objectGroup.containsUnsync(obj)) {
      return;
    }

    TStackObjectList objList;
    //  ZStackObjectRole role;

    if (uniqueSource) {
      if (!obj->getSource().empty()) {
        objList = m_objectGroup.takeSameSourceUnsync(obj->getType(), obj->getSource());
        for (TStackObjectList::iterator iter = objList.begin();
             iter != objList.end(); ++iter) {
          ZStackObject *oldObj = *iter;
          bufferObjectModified(oldObj);
#ifdef _DEBUG_
          std::cout << "Deleting object in ZStackDoc::addObject: " <<  oldObj << std::endl;
#endif
          //      role.addRole(m_playerList.removePlayer(obj));
          delete oldObj;
        }
      }
    }
  }

  addObjectFast(obj);
}

void ZStackDoc::addObject(ZStackObject *obj, int zOrder, bool uniqueSource)
{
  addObject(obj, uniqueSource);
  obj->setZOrder(zOrder);
  //QMutexLocker locker(&m_mutex);

//  m_objectGroup.add(obj, zOrder, uniqueSource);
//  processObjectModified(obj);
//  notifyObjectModified();
}

void ZStackDoc::notifyPlayerChanged(const ZStackObjectRole &role)
{
  notifyPlayerChanged(role.getRole());
}

void ZStackDoc::notifyPlayerChanged(ZStackObjectRole::TRole role)
{
  ZStackObjectRole roleObj(role);
  if (roleObj.hasRole(ZStackObjectRole::ROLE_SEED)) {
//    m_isSegmentationReady = false;
    setSegmentationReady(false);

    emit messageGenerated(
          ZWidgetMessage(ZWidgetMessage::appendTime("Seed modified.")));
    emit seedModified();
  }

  if (roleObj.hasRole(ZStackObjectRole::ROLE_TMP_RESULT)) {

  }

  if (roleObj.hasRole(ZStackObjectRole::ROLE_3DPAINT)) {
    notifyVolumeModified();
  }

  if (roleObj.hasRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR)) {
    notify3DGraphModified();
  }

  if (roleObj.hasRole(ZStackObjectRole::ROLE_ACTIVE_VIEW)) {
    notifyActiveViewModified();
  }
}

void ZStackDoc::addPlayer(ZStackObject *obj)
{
  if (obj != NULL) {
    if (obj->hasRole()) {
      ZDocPlayer *player = NULL;
      switch (obj->getType()) {
      case ZStackObject::TYPE_OBJ3D:
        player = new ZObject3dPlayer(obj);
        break;
      case ZStackObject::TYPE_STROKE:
        player = new ZStroke2dPlayer(obj);
        break;
      case ZStackObject::TYPE_SPARSE_OBJECT:
        player = new ZSparseObjectPlayer(obj);
        break;
      case ZStackObject::TYPE_STACK_BALL:
        player = new ZStackBallPlayer(obj);
        break;
      case ZStackObject::TYPE_OBJECT3D_SCAN:
        player = new ZObject3dScanPlayer(obj);
        break;
      case ZStackObject::TYPE_DVID_LABEL_SLICE:
        player = new ZDvidLabelSlicePlayer(obj);
        break;
      default:
        player = new ZDocPlayer(obj);
        break;
      }

      if (player != NULL) {
        m_playerList.add(player);
        processObjectModified(obj->getRole().getRole());
      }
    }
  }
}

bool ZStackDoc::hasObjectSelected() const
{
  return m_objectGroup.hasSelected() || hasSelectedSwcNode();
}

bool ZStackDoc::executeAddObjectCommand(ZStackObject *obj, bool uniqueSource)
{
  if (obj != NULL) {
    ZStackDocCommand::ObjectEdit::AddObject *command =
        new ZStackDocCommand::ObjectEdit::AddObject(this, obj, uniqueSource);
    command->setLogMessage("Add object:" + obj->className());
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveObjectCommand(ZStackObject *obj)
{
  if (obj != NULL) {
    ZStackDocCommand::ObjectEdit::RemoveObject *command =
        new ZStackDocCommand::ObjectEdit::RemoveObject(this, obj);
    command->setLogMessage("Remove object: " + obj->className());
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRemoveSelectedObjectCommand()
{
  if (hasObjectSelected()) {
    ZStackDocCommand::ObjectEdit::RemoveSelected *command = new
        ZStackDocCommand::ObjectEdit::RemoveSelected(this);
    command->setLogMessage("Remove selected objects");
    pushUndoCommand(command);
    deprecateTraceMask();
    return true;
  }

  return false;
}

void ZStackDoc::setParentDoc(ZSharedPointer<ZStackDoc> parentDoc)
{
  m_parentDoc = parentDoc;
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
  if (m_objectGroup.getSelectedSet(ZStackObject::TYPE_SWC).empty() &&
      m_objectGroup.getSelectedSet(ZStackObject::TYPE_PUNCTUM).empty() &&
      !hasSelectedSwcNode())
    return false;

  ZStackDocCommand::ObjectEdit::MoveSelected *moveSelectedObjectCommand =
      new ZStackDocCommand::ObjectEdit::MoveSelected(this, x, y, z);
  moveSelectedObjectCommand->setPunctaCoordScale(punctaScaleX,
                                                 punctaScaleY,
                                                 punctaScaleZ);
  moveSelectedObjectCommand->setSwcCoordScale(swcScaleX,
                                              swcScaleY,
                                              swcScaleZ);
  moveSelectedObjectCommand->setLogMessage("Move selected objects");
  pushUndoCommand(moveSelectedObjectCommand);

  return true;
}

bool ZStackDoc::executeTraceTubeCommand(double x, double y, double z, int c)
{
  ZUndoCommand *traceTubeCommand =
      new ZStackDocCommand::TubeEdit::Trace(this, x, y, z, c);
  traceTubeCommand->setLogMessage("Trace tube");
  pushUndoCommand(traceTubeCommand);

  return true;
}

bool ZStackDoc::executeTraceSwcBranchCommand(double x, double y)
{
  x -= getStackOffset().getX();
  y -= getStackOffset().getY();
  double z = maxIntesityDepth(x, y) + getStackOffset().getZ();

  return executeTraceSwcBranchCommand(x, y, z);
}

void ZStackDoc::updatePunctaObjsModel(ZPunctum *punctum)
{
  punctaObjsModel()->updateData(punctum);
}

bool ZStackDoc::executeTraceSwcBranchCommand(
    double x, double y, double z)
{
  /*
  QUndoCommand *command =
      new ZStackDocCommand::SwcEdit::TraceSwcBranch(this, x, y, z, c);
  pushUndoCommand(command);
  */

  //ZNeuronTracer tracer;
  m_neuronTracer.setIntensityField(getStack());
  //tracer.setTraceWorkspace(getTraceWorkspace());
  //tracer.setStackOffset(getStackOffset().x(), getStackOffset().y(),
  //                      getStackOffset().z());

  refreshTraceMask();
  ZSwcPath branch = m_neuronTracer.trace(x, y, z);

  if (branch.size() > 1) {
    ZSwcConnector swcConnector;
    swcConnector.useSurfaceDist(true);

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
        ZPoint rootCenter = SwcTreeNode::center(branchRoot);
        ZPoint nbrCenter = SwcTreeNode::center(rootNeighbor);

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
      ZPoint terminalCenter = SwcTreeNode::center(terminal);
      ZPoint nbrCenter = SwcTreeNode::center(terminalNeighbor);

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

    ZSwcResampler resampler;
    resampler.ignoreInterRedundant(true);
    resampler.optimalDownsample(tree);;

    ZSwcPath path(branchRoot, tree->firstLeaf());


    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);



    new ZStackDocCommand::SwcEdit::AddSwc(this, tree, command);

    if (conn.first != NULL) {
      new ZStackDocCommand::SwcEdit::SetParent(
            this, hook, loop, false, command);
      new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, command);
    }

    new ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask(this, path, command);

    command->setLogMessage("Trace SWC branch");
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
  if (!m_objectGroup.getSelectedSet(ZStackObject::TYPE_LOCSEG_CHAIN).empty()) {
    ZUndoCommand *command =
        new ZStackDocCommand::TubeEdit::RemoveSelected(this);
    command->setLogMessage("Remove tube");
    pushUndoCommand(command);
    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceCommand(int traceLevel, bool doResample)
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
  m_neuronTracer.setTraceLevel(traceLevel);
  ZSwcTree *tree = m_neuronTracer.trace(getStack()->c_stack(), doResample);
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
    command->setLogMessage("Run automatic tracing");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeAutoTraceAxonCommand()
{
  if (hasStackData()) {
    ZUndoCommand *command = new ZStackDocCommand::TubeEdit::AutoTraceAxon(this);
    command->setLogMessage("Run automaic axon tracing");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

void ZStackDoc::saveSwc(QWidget *parentWidget)
{
  //Assume there is no empty tree
  QList<ZSwcTree*> swcList = getSwcList();
  if (!swcList.empty()) {
    if (swcList.size() > 1) {
      /*
      report("Save failed", "More than one SWC tree exist.",
             NeuTube::MSG_ERROR);
             */
      emit statusMessageUpdated("Save failed. More than one SWC tree exist.");
    } else {
      ZSwcTree *tree = swcList[0];
      if (tree->hasGoodSourceName()) {
        tree->resortId();
        tree->save(tree->getSource().c_str());
        setSaved(ZStackObject::TYPE_SWC, true);
        emit statusMessageUpdated(QString(tree->getSource().c_str()) + " saved.");
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
//          tree->resortId();
          tree->save(fileName.toStdString().c_str());
          tree->setSource(fileName.toStdString());
          setSaved(ZStackObject::TYPE_SWC, true);
          notifySwcModified();
          QString msg = QString(tree->getSource().c_str()) + " saved.";
          emit statusMessageUpdated(msg);
          emit messageGenerated(ZWidgetMessage(msg, NeuTube::MSG_INFORMATION));
        }
      }
    }
  }
}

std::vector<ZSwcTree*> ZStackDoc::getSwcArray() const
{
  std::vector<ZSwcTree*> swcArray;
  QList<ZSwcTree*> swcList = getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    swcArray.push_back(const_cast<ZSwcTree*>(*iter));
  }

  return swcArray;
}

std::vector<ZStack*> ZStackDoc::projectBiocytinStack(
    Biocytin::ZStackProjector &projector)
{
  projector.setProgressReporter(m_progressReporter);

  LINFO() << QString("Making projection with %1 slabs").
             arg(projector.getSlabNumber());
  ZStack *out = projector.project(getStack(), getStackBackground());
  std::vector<ZStack*> projArray;
  if (out != NULL) {
    projArray.push_back(out);
  }

  return projArray;

#if 0

  ZStack *out = new ZStack(
        getStack()->kind(), getStack()->width(), getStack()->height(),
        projector.getSlabNumber(),
        getStack()->channelNumber());

  std::vector<ZStack*> projArray;
  projArray.push_back(out);

  for (int slabIndex = 0; slabIndex < projector.getSlabNumber(); ++slabIndex) {
    ZStack *proj = projector.project(getStack(), getStackBackground(), true, slabIndex);

    if (proj != NULL) {
      if (proj->channelNumber() == 2) {
        proj->initChannelColors();
        proj->setChannelColor(0, 1, 1, 1);
        proj->setChannelColor(1, 0, 0, 0);
      }

      for (int c = 0; c < proj->channelNumber(); ++c) {
        C_Stack::copyPlaneValue(
              out->data(), proj->c_stack(c)->array, c, slabIndex);
      }

      delete proj;
//      projArray.push_back(proj);
      // ZString filePath(stack()->sourcePath());
      /*
      proj->setSource(
            projector.getDefaultResultFilePath(getStack()->sourcePath(),
                                               ));
                                               */
    }

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

//  return out;
  return projArray;
#endif
}

void ZStackDoc::selectAllSwcTreeNode()
{
//  QList<Swc_Tree_Node*> selected;
//  QList<Swc_Tree_Node*> deselected;

  std::set<Swc_Tree_Node*> oldSelectedSet = getSelectedSwcNodeSet();

  QList<ZSwcTree*> swcList = getSwcList();
  foreach (ZSwcTree *tree, swcList) {
    tree->selectAllNode();
    setSwcSelected(tree, false);
    /*
    tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
      if (SwcTreeNode::isRegular(tn)) {
        if ((m_selectedSwcTreeNodes.insert(tn)).second) {
          selected.push_back(tn);
          // deselect its tree
          setSwcSelected(nodeToSwcTree(tn), false);
        }
      }
    }
    */
  }
  /*
  if (!selected.empty() || !deselected.empty()) {
    emit swcTreeNodeSelectionChanged(selected, deselected);
  }
  */

  std::set<Swc_Tree_Node*> newSelectedSet = getSelectedSwcNodeSet();

  notifySelectionAdded(oldSelectedSet, newSelectedSet);
}

bool ZStackDoc::getLastStrokePoint(int *x, int *y) const
{
  const ZStackObject *obj =
      m_objectGroup.getLastObject(ZStackObject::TYPE_STROKE);
  if (obj != NULL) {
    const ZStroke2d *stroke = dynamic_cast<const ZStroke2d*>(obj);
    return stroke->getLastPoint(x, y);
  }

  return false;
}

bool ZStackDoc::getLastStrokePoint(double *x, double *y) const
{
  const ZStackObject *obj = m_objectGroup.getLastObject(ZStackObject::TYPE_STROKE);
  if (obj != NULL) {
    const ZStroke2d *stroke = dynamic_cast<const ZStroke2d*>(obj);
    return stroke->getLastPoint(x, y);
  }

  return false;
}

bool ZStackDoc::hasSelectedSwc() const
{
  return !m_objectGroup.getSelectedSet(ZStackObject::TYPE_SWC).empty();
}

bool ZStackDoc::hasSelectedSwcNode() const
{
  bool hasSelected = false;
  ZOUT(LTRACE(), 5) << "Has SWC selected?";
  const QList<ZStackObject*>& objList = getObjectList(ZStackObject::TYPE_SWC);

  ZOUT(LTRACE(), 5) << "Object count: " << objList.size();

  foreach (const ZStackObject *obj, objList) {
    const ZSwcTree *tree = dynamic_cast<const ZSwcTree*>(obj);
    if (tree->hasSelectedNode()) {
      hasSelected = true;
      break;
    }
  }

  return hasSelected;
}

int ZStackDoc::getSelectedSwcNodeNumber() const
{
  int n = 0;
  const QList<ZStackObject*>& objList = getObjectList(ZStackObject::TYPE_SWC);
  foreach (const ZStackObject *obj, objList) {
    const ZSwcTree *tree = dynamic_cast<const ZSwcTree*>(obj);
    n += tree->getSelectedNode().size();
  }

  return n;
}

bool ZStackDoc::hasMultipleSelectedSwcNode() const
{
  return getSelectedSwcNodeNumber() > 1;
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
  if (getResolutionDialog()->exec()) {
    resolution[0] = getResolutionDialog()->getXScale();
    resolution[1] = getResolutionDialog()->getYScale();
    resolution[2] = getResolutionDialog()->getZScale();
    showSeletedSwcNodeLength(resolution);
  }
}

void ZStackDoc::showSeletedSwcNodeLength(double *resolution)
{
  double length = 0.0;

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (resolution == NULL) {
    length = SwcTreeNode::segmentLength(nodeSet);
  } else {
    length = SwcTreeNode::scaledSegmentLength(
          nodeSet, resolution[0], resolution[1], resolution[2]);
  }

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (nodeSet.size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
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
  QList<ZSwcTree*> swcList = getSwcList();
  if (swcList.isEmpty()) {
    textStream << "<p>No neuron data.</p>";
  } else {
    textStream << "<p>Overall length of " << swcList.size() << " Neuron(s): ";
    double length = 0.0;
    foreach (ZSwcTree* tree, swcList) {
      length += tree->length();
    }
    textStream << length << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void ZStackDoc::pushUndoCommand(QUndoCommand *command)
{
  m_undoStack->push(command);
}

void ZStackDoc::pushUndoCommand(ZUndoCommand *command)
{
  command->logCommand();
  m_undoStack->push(command);
}

bool ZStackDoc::executeInsertSwcNode()
{
  bool succ = false;

  QString message;
  int insertionCount = 0;
  if (getSelectedSwcNodeNumber() >= 2) {
    beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(*iter);
      if (nodeSet.count(parent) > 0) {
        Swc_Tree_Node *tn = SwcTreeNode::makePointer();
        SwcTreeNode::interpolate(*iter, parent, 0.5, tn);
        new ZStackDocCommand::SwcEdit::SetParent(
              this, tn, parent, false, command);
        new ZStackDocCommand::SwcEdit::SetParent(
              this, *iter, tn, false, command);
        ++insertionCount;
      }
    }
    if (command->childCount() > 0) {
      command->setLogMessage("Insert SWC node");
      pushUndoCommand(command);
      deprecateTraceMask();
      message = QString("%1 nodes inserted.").arg(insertionCount);
      succ = true;
    } else {
      message = QString("Cannont insert a node. "
                        "At least two adjacent nodes should be selected.");
      delete command;
    }

    endObjectModifiedMode();
    notifyObjectModified();
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

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    Swc_Tree_Node *branchPoint = *(nodeSet.begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);
    if (SwcTreeNode::childNumber(masterRoot) > 1) {
      ZUndoCommand *command =
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
            this, closestNode, branchPoint, false, command);

      command->setLogMessage("Set branch point");
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

  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    Swc_Tree_Node *branchPoint = *(nodeSet.begin());
    Swc_Tree_Node *hostRoot = SwcTreeNode::regularRoot(branchPoint);
    Swc_Tree_Node *masterRoot = SwcTreeNode::parent(hostRoot);

    QList<ZSwcTree*> swcList = getSwcList();
    if (SwcTreeNode::childNumber(masterRoot) > 1 || swcList.size() > 1) {
      //ZSwcTree tree;
      //tree.setDataFromNode(masterRoot);
      Swc_Tree tmpTree;
      tmpTree.root = masterRoot;
      Swc_Tree_Iterator_Start(&tmpTree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

      //tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      bool isConnected = false;
      double minDist = Infinity;
      Swc_Tree_Node *closestNode = NULL;
      //for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
      Swc_Tree_Node *tn = NULL;
      while ((tn = Swc_Tree_Next(&tmpTree)) != NULL) {
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
      //tree.setDataFromNode(NULL, ZSwcTree::LEAVE_ALONE);

      foreach (ZSwcTree *buddyTree, swcList) {
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
        ZUndoCommand *command =
            new ZStackDocCommand::SwcEdit::CompositeCommand(this);
        if (!SwcTreeNode::isRoot(closestNode)) {
          new ZStackDocCommand::SwcEdit::SetRoot(this, closestNode, command);
        }
        new ZStackDocCommand::SwcEdit::SetParent(
              this, closestNode, branchPoint, false, command);
        new ZStackDocCommand::SwcEdit::RemoveEmptyTreePost(this, command);

        command->setLogMessage("Connect isolated SWC");
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
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    Swc_Tree_Node *loop = *(nodeSet.begin());
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
          ZUndoCommand *command =
              new ZStackDocCommand::SwcEdit::CompositeCommand(this);

          if (SwcTreeNode::parent(tn) == hook) { //hook is the parent of tn
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, tn, NULL, false, command);
            new ZStackDocCommand::SwcEdit::SetRoot(this, loop, command);
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, loop, hook, false, command);
          } else { //tn is the parent of hook
            new ZStackDocCommand::SwcEdit::SetParent(
                  this, hook, loop, false, command);
          }
          command->setLogMessage("Reset branch point");
          pushUndoCommand(command);
          deprecateTraceMask();
        }
        break;
      }
    }
  }

  return false;
}

bool ZStackDoc::executeMoveAllSwcCommand(double dx, double dy, double dz)
{
  if (hasSwc()) {
    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    foreach (ZSwcTree *tree, getSwcList()) {
      tree->updateIterator();
      for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, tn, SwcTreeNode::x(tn) + dx, SwcTreeNode::y(tn) + dy,
              SwcTreeNode::z(tn) + dz, SwcTreeNode::radius(tn), command);
      }
    }
    command->setLogMessage("Move all SWC");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeScaleAllSwcCommand(
    double sx, double sy, double sz, bool aroundCenter)
{
  if (hasSwc()) {
    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    foreach (ZSwcTree *tree, getSwcList()) {
      tree->updateIterator();
      ZPoint center;
      if (aroundCenter) {
        center = tree->computeCentroid();
      }
      for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
        ZPoint position = SwcTreeNode::center(tn);
        if (aroundCenter) {
          position -= center;
        }
        position *= ZPoint(sx, sy, sz);
        if (aroundCenter) {
          position += center;
        }
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, tn, position.x(), position.y(), position.z(),
              SwcTreeNode::radius(tn), command);
      }
    }
    command->setLogMessage("Scale all SWC");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeScaleSwcNodeCommand(
    double sx, double sy, double sz, const ZPoint &center)
{
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if (!nodeSet.empty()) {
    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    for (std::set<Swc_Tree_Node*>::iterator iter =
         nodeSet.begin(); iter != nodeSet.end();
         ++iter) {
      Swc_Tree_Node *tn = *iter;
      ZPoint position = SwcTreeNode::center(tn);
      position -= center;
      position *= ZPoint(sx, sy, sz);
      position += center;
      new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
            this, tn, position.x(), position.y(), position.z(),
            SwcTreeNode::radius(tn), command);
    }
    command->setLogMessage("Scale SWC node");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRotateSwcNodeCommand(
    double theta, double psi, bool aroundCenter)
{
  std::set<Swc_Tree_Node*> nodeSet = getSelectedSwcNodeSet();
  if ((!nodeSet.empty() && !aroundCenter) ||
      (nodeSet.size() > 1)) {
    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    ZPoint center;
    if (aroundCenter) {
      center = SwcTreeNode::centroid(nodeSet);
    }
    for (std::set<Swc_Tree_Node*>::iterator iter =
         nodeSet.begin(); iter != nodeSet.end();
         ++iter) {
      Swc_Tree_Node *tn = *iter;
      ZPoint position = SwcTreeNode::center(tn);
      if (aroundCenter) {
        position.rotate(theta, psi, center);
      } else {
        position.rotate(theta, psi);
      }
      new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
            this, tn, position.x(), position.y(), position.z(),
            SwcTreeNode::radius(tn), command);
    }
    command->setLogMessage("Rotate SWC node");
    pushUndoCommand(command);

    return true;
  }

  return false;
}

bool ZStackDoc::executeRotateAllSwcCommand(
    double theta, double psi, bool aroundCenter)
{
  if (hasSwc()) {
    ZUndoCommand *command =
        new ZStackDocCommand::SwcEdit::CompositeCommand(this);
    foreach (ZSwcTree *tree, getSwcList()) {
      tree->updateIterator();
      ZPoint center;
      if (aroundCenter) {
        center = tree->computeCentroid();
      }
      for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
        ZPoint position = SwcTreeNode::center(tn);
        if (aroundCenter) {
          position.rotate(theta, psi, center);
        } else {
          position.rotate(theta, psi);
        }
        new ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry(
              this, tn, position.x(), position.y(), position.z(),
              SwcTreeNode::radius(tn), command);
      }
    }
    command->setLogMessage("Rotate all swc");
    pushUndoCommand(command);

    return true;
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

ZIntPoint ZStackDoc::getStackSize() const
{
  ZIntPoint size(0, 0, 0);
  if (hasStack()) {
    size = ZIntPoint(
          getStack()->width(), getStack()->height(), getStack()->depth());
  }
  return size;
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

void ZStackDoc::addData(ZStackDocReader &reader)
{
  reader.getObjectGroup().moveTo(m_objectGroup);

  if (m_objectGroup.hasObject(ZStackObject::TYPE_SWC)) {
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

  if (m_objectGroup.hasObject(ZStackObject::TYPE_LOCSEG_CHAIN)) {
    notifyChainModified();
  }

  if (m_objectGroup.hasObject(ZStackObject::TYPE_PUNCTUM)) {
    notifyPunctumModified();
  }

  if (m_objectGroup.hasObject(ZStackObject::TYPE_SPARSE_OBJECT)) {
    notifySparseObjectModified();
  }

  reader.getPlayerList().moveTo(m_playerList);
}

void ZStackDoc::reloadData(ZStackDocReader &reader)
{
  clearData();
  addData(reader);
}


std::vector<ZStack*> ZStackDoc::createWatershedMask(bool selectedOnly) const
{
  std::vector<ZStack*> maskArray;

  int numberOfSelected = 0;

  ZStackObject *obj = NULL;
//  bool hasSelected = false;
  QMutexLocker locker(m_playerList.getMutex());

  const QList<ZDocPlayer*> &playerList = m_playerList.getPlayerList();
  if (selectedOnly) {
    for (QList<ZDocPlayer*>::const_iterator iter = playerList.begin();
         iter != playerList.end(); ++iter) {
      const ZDocPlayer *player = *iter;
      if (player->hasRole(ZStackObjectRole::ROLE_SEED) &&
          player->getData()->isSelected()) {
        obj = player->getData();
        ++numberOfSelected;
//        hasSelected = true;
//        break;
      }
    }
  }
  if (numberOfSelected == 1) {
    ZIntCuboid box;
    obj->boundBox(&box);
    if (!box.isEmpty()) {
      for (QList<ZDocPlayer*>::const_iterator iter = playerList.begin();
           iter != playerList.end(); ++iter) {
        const ZDocPlayer *player = *iter;
        if (player->hasRole(ZStackObjectRole::ROLE_SEED)) {
          ZStackObject *checkObj = player->getData();
          ZIntCuboid checkBox;
          checkObj->boundBox(&checkBox);
          int boxDist;
          if (!checkBox.isEmpty()) {
            if (box.contains(checkBox.getLastCorner()) ||
                box.contains(checkBox.getFirstCorner())) {
              boxDist = 0;
            } else {
              ZIntPoint cornerDiff =
                  box.getFirstCorner() - checkBox.getFirstCorner();

              boxDist = imax3(std::abs(cornerDiff.getX()),
                              std::abs(cornerDiff.getY()),
                              std::abs(cornerDiff.getZ()));
              cornerDiff = box.getFirstCorner() - checkBox.getLastCorner();
              boxDist = imin2(boxDist, imax3(std::abs(cornerDiff.getX()),
                                             std::abs(cornerDiff.getY()),
                                             std::abs(cornerDiff.getZ())));

              cornerDiff = box.getFirstCorner() - checkBox.getLastCorner();
              boxDist = imin2(boxDist, imax3(std::abs(cornerDiff.getX()),
                                             std::abs(cornerDiff.getY()),
                                             std::abs(cornerDiff.getZ())));

              cornerDiff = box.getLastCorner() - checkBox.getLastCorner();
              boxDist = imin2(boxDist, imax3(std::abs(cornerDiff.getX()),
                                             std::abs(cornerDiff.getY()),
                                             std::abs(cornerDiff.getZ())));

              cornerDiff = box.getLastCorner() - checkBox.getFirstCorner();
              boxDist = imin2(boxDist, imax3(std::abs(cornerDiff.getX()),
                                             std::abs(cornerDiff.getY()),
                                             std::abs(cornerDiff.getZ())));
            }
            if (boxDist < 100) {
              ZStack *stack = player->toStack();
              if (stack != NULL) {
                maskArray.push_back(stack);
              }
            }
          }
        }
      }
    }
  } else {
    for (QList<ZDocPlayer*>::const_iterator iter = playerList.begin();
         iter != playerList.end(); ++iter) {
      const ZDocPlayer *player = *iter;
      if (player->hasRole(ZStackObjectRole::ROLE_SEED) &&
          ((numberOfSelected == 0) || player->getData()->isSelected())) {
        ZStack *stack = player->toStack();
        if (stack != NULL) {
          maskArray.push_back(stack);
        }
      }
    }
  }


  return maskArray;
}

void ZStackDoc::toggleVisibility(ZStackObjectRole::TRole role)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDocPlayer*> playerList = getPlayerList(role);
  for (QList<ZDocPlayer*>::iterator iter = playerList.begin();
       iter != playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    player->getData()->toggleVisible();
    processObjectModified(player->getData()->getTarget());
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZStackDoc::updateWatershedBoundaryObject(ZStack *out, ZIntPoint dsIntv)
{
  if (out != NULL) {
    std::vector<ZObject3dScan*> objArray =
        ZObject3dFactory::MakeObject3dScanPointerArray(*out);
    /*
    ZObject3dArray *objArray = ZObject3dFactory::MakeRegionBoundary(
          *out, ZObject3dFactory::OUTPUT_SPARSE);
          */
    if (dsIntv.getX() > 0 || dsIntv.getY() > 0 || dsIntv.getZ() > 0) {
      for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3dScan *obj = *iter;
        if (obj != NULL) {
          if (!obj->isEmpty()) {
            obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
          }
        }
      }
    }

    QList<ZDocPlayer*> playerList =
        getPlayerList(ZStackObjectRole::ROLE_SEED);
    //foreach (ZStroke2d *stroke, m_strokeList) {
    foreach (const ZDocPlayer *player, playerList) {
      for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3dScan *obj = *iter;
        if (obj != NULL) {
          if (!obj->isEmpty()) {
            if ((int) obj->getLabel() == player->getLabel()) {
              obj->setColor(player->getData()->getColor());
              //ZString objectSource = "localSeededWatershed:Temporary_Border:";
              // objectSource.appendNumber(stroke->getLabel());
              obj->setSource(
                    ZStackObjectSourceFactory::MakeWatershedBoundarySource(
                      player->getLabel()));
              obj->setHittable(false);
              obj->setProjectionVisible(false);
              obj->setRole(ZStackObjectRole::ROLE_TMP_RESULT);
              addObject(obj, true);
            }
          }
        }
      }
    }

    for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      if (!obj->getRole().hasRole(ZStackObjectRole::ROLE_TMP_RESULT)) {
        delete obj;
      }
    }
  }
}

ZDvidSparseStack* ZStackDoc::getDvidSparseStack() const
{
  ZStackObject *obj = getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_DVID_SPARSE_STACK,
        ZStackObjectSourceFactory::MakeSplitObjectSource());
  ZDvidSparseStack *sparseStack = dynamic_cast<ZDvidSparseStack*>(obj);

  return sparseStack;
}

void ZStackDoc::localSeededWatershed()
{
  getProgressSignal()->startProgress("Running local split ...");
  removeObject(ZStackObjectRole::ROLE_TMP_RESULT, true);
//  m_isSegmentationReady = false;
  setSegmentationReady(false);

  ZStackArray seedMask = createWatershedMask(true);
  getProgressSignal()->advanceProgress(0.1);

  if (!seedMask.empty()) {
    ZStackWatershed engine;

    ZStack *signalStack = m_stack;
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      ZOUT(LINFO(), 3) << "Gettting signal stack";
      signalStack = NULL;
      if (m_sparseStack != NULL) {
        signalStack = m_sparseStack->getStack();
        dsIntv = m_sparseStack->getDownsampleInterval();
      } else {
        ZDvidSparseStack *sparseStack = getDvidSparseStack();
        if (sparseStack != NULL) {
          signalStack = sparseStack->getStack(seedMask.getBoundBox());
#ifdef _DEBUG_2
          signalStack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif
          dsIntv = sparseStack->getDownsampleInterval();
        }
      }
    }

    getProgressSignal()->advanceProgress(0.1);

#ifdef _DEBUG_2
    signalStack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif
    if (signalStack != NULL) {

      ZOUT(LINFO(), 3) << "Downsampling seed mask";
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
      getProgressSignal()->advanceProgress(0.1);

//      advanceProgress(0.1);
//      QApplication::processEvents();

      Cuboid_I box;
      seedMask.getBoundBox(&box);
      const int xMargin = 10;
      const int yMargin = 10;
      const int zMargin = 20;
      Cuboid_I_Expand_X(&box, xMargin);
      Cuboid_I_Expand_Y(&box, yMargin);
      Cuboid_I_Expand_Z(&box, zMargin);

      engine.setRange(box);

      ZOUT(LINFO(), 3) << "Running seeded watershed";
      ZStack *out = engine.run(signalStack, seedMask);
      getProgressSignal()->advanceProgress(0.3);

//      advanceProgress(0.1);
//      QApplication::processEvents();

      //objArray = ZObject3dFactory::MakeRegionBoundary(*out);
      //objData = Stack_Region_Border(out->c_stack(), 6, TRUE);

      ZOUT(LINFO(), 3) << "Updating boundary object";
      updateWatershedBoundaryObject(out, dsIntv);
      getProgressSignal()->advanceProgress(0.1);

//      advanceProgress(0.1);
//      QApplication::processEvents();

      // C_Stack::kill(out);
      //delete out;
    } else {
      std::cout << "No signal for local watershed." << std::endl;
    }
  }
  getProgressSignal()->endProgress();
  emit labelFieldModified();
}

void ZStackDoc::seededWatershed()
{
  getProgressSignal()->startProgress("Splitting ...");

  ZOUT(LINFO(), 3) << "Removing old result ...";
  removeObject(ZStackObjectRole::ROLE_TMP_RESULT, true);
//  m_isSegmentationReady = false;
  setSegmentationReady(false);

  getProgressSignal()->advanceProgress(0.1);
  //removeAllObj3d();
  ZStackWatershed engine;

  ZOUT(LINFO(), 3) << "Creating seed mask ...";
  ZStackArray seedMask = createWatershedMask(false);

  getProgressSignal()->advanceProgress(0.1);

  if (!seedMask.empty()) {
    ZStack *signalStack = m_stack;
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      signalStack = NULL;
      ZOUT(LINFO(), 3) << "Retrieving signal stack";
      if (m_sparseStack != NULL) {
        signalStack = m_sparseStack->getStack();
        dsIntv = m_sparseStack->getDownsampleInterval();
      } else {
        ZDvidSparseStack *sparseStack = getDvidSparseStack();
        if (sparseStack != NULL) {
          signalStack = sparseStack->getStack();
          dsIntv = sparseStack->getDownsampleInterval();
        }
      }
    }
    getProgressSignal()->advanceProgress(0.1);

    if (signalStack != NULL) {
      ZOUT(LINFO(), 3) << "Downsampling ..." << dsIntv.toString();
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

#ifdef _DEBUG_2
      seedMask[0]->save(GET_TEST_DATA_DIR + "/test.tif");
      signalStack->save(GET_TEST_DATA_DIR + "/test2.tif");
#endif

      ZStack *out = engine.run(signalStack, seedMask);
      getProgressSignal()->advanceProgress(0.3);

      ZOUT(LINFO(), 3) << "Updating watershed boundary object";
      updateWatershedBoundaryObject(out, dsIntv);
      getProgressSignal()->advanceProgress(0.1);

//      notifyObj3dModified();

      ZOUT(LINFO(), 3) << "Setting label field";
      setLabelField(out);
//      m_isSegmentationReady = true;
      setSegmentationReady(true);

      emit messageGenerated(ZWidgetMessage(
            ZWidgetMessage::appendTime("Split done. Ready to upload.")));
    } else {
      std::cout << "No signal for watershed." << std::endl;
    }
  }
  getProgressSignal()->endProgress();
  emit labelFieldModified();
}

void ZStackDoc::runLocalSeededWatershed()
{
//  startProgress();
//  QApplication::processEvents();

//  localSeededWatershed();

//  getProgressSignal()->startProgress();

  const QString threadId = "localSeededWatershed";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZStackDoc::localSeededWatershed);
    m_futureMap[threadId] = future;
  }

//  QFuture<void> result =
//      QtConcurrent::run(this, &ZStackDoc::localSeededWatershed); //crashed for unknown reason
//  result.waitForFinished();

//  endProgress();
}

void ZStackDoc::runSeededWatershed()
{
  QList<ZDocPlayer*> playerList =
      getPlayerList(ZStackObjectRole::ROLE_SEED);

  ZOUT(LINFO(), 3) << "Retrieving label set";

  QSet<int> labelSet;
  foreach (const ZDocPlayer *player, playerList) {
    labelSet.insert(player->getLabel());
  }

  if (labelSet.size() < 2) {
    ZWidgetMessage message(
          QString("The seed has no more than one label. No split is done"));
    message.setType(NeuTube::MSG_WARNING);

    emit messageGenerated(message);
    return;
  }

  const QString threadId = "seededWatershed";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZStackDoc::seededWatershed);
    m_futureMap[threadId] = future;
  }

//  seededWatershed();
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

QList<const ZDocPlayer *> ZStackDoc::getPlayerList(
    ZStackObjectRole::TRole role) const
{
  return m_playerList.getPlayerList(role);
}

QList<ZDocPlayer *> ZStackDoc::getPlayerList(
    ZStackObjectRole::TRole role)
{
  return m_playerList.getPlayerList(role);
}

void ZStackDoc::ActiveViewObjectUpdater::clearState()
{
  m_excludeSet.clear();
  m_excludeTarget.clear();
  m_updatedTarget.clear();
}

void ZStackDoc::ActiveViewObjectUpdater::SetUpdateEnabled(
    ZSharedPointer<ZStackDoc> doc, ZStackObject::EType type, bool on)
{
  if (doc.get() != NULL) {
    QList<ZDocPlayer *> playerList =
        doc->getPlayerList(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    for (QList<ZDocPlayer *>::iterator iter = playerList.begin();
         iter != playerList.end(); ++iter) {
      ZDocPlayer *player = *iter;
      ZStackObject *obj = player->getData();
      if (obj->getType() == type) {
        player->enableUpdate(on);
      }
    }
  }
}

void ZStackDoc::ActiveViewObjectUpdater::update(const ZStackViewParam &param)
{
  m_updatedTarget.clear();
  if (m_doc.get() != NULL) {
    QList<ZDocPlayer *> playerList =
        m_doc->getPlayerList(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    for (QList<ZDocPlayer *>::iterator iter = playerList.begin();
         iter != playerList.end(); ++iter) {
      ZDocPlayer *player = *iter;
      ZStackObject *obj = player->getData();
      if (!m_excludeSet.contains(obj->getType()) &&
          !m_excludeTarget.contains(obj->getTarget()) &&
          obj->isVisible()) {
        if (player->updateData(param)) {
          m_updatedTarget.insert(obj->getTarget());
          if (obj->getType() == ZStackObject::TYPE_DVID_LABEL_SLICE) {
            ZDvidLabelSlice *labelSlice = dynamic_cast<ZDvidLabelSlice*>(obj);
            if (labelSlice != NULL) {
              m_doc->notifyUpdateLatency(labelSlice->getReadingTime());
            }
          }
        }
      }
    }
  }
}

/*
QSet<ZStackObject::ETarget>
ZStackDoc::updateActiveViewObject(const ZStackViewParam &param)
{
  QSet<ZStackObject::ETarget> targetSet;

  QList<ZDocPlayer *> playerList =
      getPlayerList(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  for (QList<ZDocPlayer *>::iterator iter = playerList.begin();
       iter != playerList.end(); ++iter) {
    ZDocPlayer *player = *iter;
    if (player->getData()->isVisible()) {
      player->updateData(param);
      targetSet.insert(player->getData()->getTarget());
    }
  }

  return targetSet;
}
*/

bool ZStackDoc::hasPlayer(ZStackObjectRole::TRole role) const
{
  return m_playerList.hasPlayer(role);
}

Z3DGraph ZStackDoc::get3DGraphDecoration() const
{
  Z3DGraph graph;
  QList<const ZDocPlayer *> playerList =
      getPlayerList(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
  foreach(const ZDocPlayer *player, playerList) {
    if (player->getData()->isVisible()) {
      graph.append(player->get3DGraph());
    }
  }
//  graph.

  return graph;
}

void ZStackDoc::importSeedMask(const QString &filePath)
{
  ZStack stack;
  stack.load(filePath.toStdString(), false);

  if (stack.kind() == GREY) {
    ZLabelColorTable colorTable;
    std::vector<ZObject3d*> objArray;
    for (int z = 0; z < stack.depth(); ++z) {
      for (int y = 0; y < stack.height(); ++y) {
        for (int x = 0; x < stack.width(); ++x) {
          int x2 = x + stack.getOffset().getX();
          int y2 = y + stack.getOffset().getY();
          int z2 = z + stack.getOffset().getZ();

          int v = stack.getIntValue(x2, y2, z2);
          if (v > 0) {
            if (v >= (int) objArray.size()) {
              objArray.resize(v + 1, NULL);
            }

            if (objArray[v] == NULL) {
              objArray[v] = new ZObject3d;
              objArray[v]->setLabel(v);
              objArray[v]->setColor(colorTable.getColor(v));
            }

            objArray[v]->append(x2, y2, z2);
          }
        }
      }
    }
    if (objArray.size() > 1) {
      ZUndoCommand *command = new ZUndoCommand;
      for (std::vector<ZObject3d*>::iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3d *obj = *iter;
        if (obj != NULL) {
          obj->setRole(ZStackObjectRole::ROLE_SEED);
          new ZStackDocCommand::ObjectEdit::AddObject(
                this, obj, false, command);
        }
      }
      command->setLogMessage("Import seed mask");
      pushUndoCommand(command);

      notifyObj3dModified();
    }
  }
}

void ZStackDoc::clearSelectedSet()
{
  deselectAllSwcTreeNodes();
  m_objectGroup.setSelected(false);
}

ZRect2d ZStackDoc::getRect2dRoi() const
{
  ZRect2d rect;

  ZRect2d *rectObj = dynamic_cast<ZRect2d*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_RECT2D,
          ZStackObjectSourceFactory::MakeRectRoiSource()));
  if (rectObj != NULL) {
    rect = *rectObj;
  }

  return rect;
}

ZIntCuboid ZStackDoc::getCuboidRoi() const
{
  ZIntCuboid box;

  ZRect2d *rectObj = dynamic_cast<ZRect2d*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_RECT2D,
          ZStackObjectSourceFactory::MakeRectRoiSource()));
  if (rectObj != NULL) {
    box.setFirstCorner(
          rectObj->getFirstX(), rectObj->getFirstY(), rectObj->getZ());
    box.setLastCorner(
          rectObj->getLastX(), rectObj->getLastY(), rectObj->getZ());
    if (rectObj->getZSpan() > 0) {
      box.setFirstZ(box.getFirstCorner().getZ() - rectObj->getZSpan());
      box.setLastZ(box.getLastCorner().getZ() + rectObj->getZSpan());
    }
  }

  return box;
}

void ZStackDoc::notifySelectorChanged()
{
  ZStackObjectSelector *selector = m_objectGroup.getSelector();
  if (!selector->isEmpty()) {
    emit objectSelectorChanged(*selector);
  }
}

void ZStackDoc::recordSwcTreeNodeSelection()
{
  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    tree->recordSelection();
  }
}

void ZStackDoc::notifySwcTreeNodeSelectionChanged()
{
  QList<ZSwcTree*> treeList = getSwcList();
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    tree->processSelection();
    const std::set<Swc_Tree_Node*> &selectedSet =
        tree->getNodeSelector().getSelectedSet();
    for (std::set<Swc_Tree_Node*>::iterator iter = selectedSet.begin();
         iter != selectedSet.end(); ++iter) {
      selected.append(*iter);
    }

    const std::set<Swc_Tree_Node*> &deselectedSet =
        tree->getNodeSelector().getSelectedSet();
    for (std::set<Swc_Tree_Node*>::iterator iter = deselectedSet.begin();
         iter != deselectedSet.end(); ++iter) {
      deselected.append(*iter);
    }
  }

  emit swcTreeNodeSelectionChanged(selected, deselected);

//  emit swcTreeNodeSelectionChanged();
}

void ZStackDoc::registerUser(QObject *user)
{
  if (!m_userList.contains(user)) {
    m_userList.append(user);
    connect(user, SIGNAL(destroyed(QObject*)), this, SLOT(removeUser(QObject*)));
#ifdef _DEBUG_2
//    connect(user, SIGNAL(destroyed()), this, SLOT(emptySlot()));
    connect(user, SIGNAL(destroyed(QObject*)), this, SLOT(emptySlot()));
#endif
  }
}

void ZStackDoc::removeUser(QObject *user)
{
  m_userList.removeOne(user);
}

void ZStackDoc::removeAllUser()
{
  m_userList.clear();
}

void ZStackDoc::notifyZoomingToSelectedSwcNode()
{
  emit zoomingToSelectedSwcNode();
}

void ZStackDoc::notifyZoomingTo(double x, double y, double z)
{
  emit zoomingTo(iround(x), iround(y), iround(z));
}

void ZStackDoc::notifyZoomingTo(const ZIntPoint &pt)
{
  emit zoomingTo(pt.getX(), pt.getY(), pt.getZ());
}

template<typename T>
const T* ZStackDoc::getFirstUserByType() const
{
  for (QList<QObject*>::const_iterator iter = m_userList.begin();
       iter != m_userList.end(); ++iter) {
    T *parent = qobject_cast<T*>(*iter);
    if (parent != NULL) {
      return parent;
    }
  }

  return NULL;
}

const ZStackFrame* ZStackDoc::getParentFrame() const
{
  return getFirstUserByType<ZStackFrame>();
}

ZStackFrame* ZStackDoc::getParentFrame()
{
  return const_cast<ZStackFrame*>(
        static_cast<const ZStackDoc&>(*this).getParentFrame());
}

const Z3DWindow* ZStackDoc::getParent3DWindow() const
{
  return getFirstUserByType<Z3DWindow>();
}

Z3DWindow* ZStackDoc::getParent3DWindow()
{
  return const_cast<Z3DWindow*>(
        static_cast<const ZStackDoc&>(*this).getParent3DWindow());
}

const ZStackMvc* ZStackDoc::getParentMvc() const
{
  return getFirstUserByType<ZStackMvc>();
}

ZStackMvc* ZStackDoc::getParentMvc()
{
  return const_cast<ZStackMvc*>(
        static_cast<const ZStackDoc&>(*this).getParentMvc());
}

QString ZStackDoc::getTitle() const
{
  QString title;

  if (getStack() != NULL) {
    title = getStack()->sourcePath().c_str();
  }

  return title;
}

ZStackDoc::EObjectModifiedMode ZStackDoc::getObjectModifiedMode()
{
  QMutexLocker locker(&m_objectModifiedModeMutex);

  if (!m_objectModifiedMode.empty()) {
    return m_objectModifiedMode.top();
  }

  return OBJECT_MODIFIED_SIGNAL;
}

void ZStackDoc::beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode mode)
{
  QMutexLocker locker(&m_objectModifiedModeMutex);

  m_objectModifiedMode.push(mode);
}

void ZStackDoc::endObjectModifiedMode()
{
  QMutexLocker locker(&m_objectModifiedModeMutex);

  m_objectModifiedMode.pop();
}

void ZStackDoc::setVisible(ZStackObject::EType type, bool visible)
{
  TStackObjectList &objList = getObjectList(type);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setVisible(visible);
    bufferObjectModified(obj->getTarget());
  }

  notifyObjectModified();
}

void ZStackDoc::setVisible(ZStackObjectRole::TRole role, bool visible)
{
  ZOUT(LTRACE(), 5) << "Set visible";
  QList<ZDocPlayer*> playerList = getPlayerList(role);
//  TStackObjectList &objList = getObjectList(role);
  for (QList<ZDocPlayer*>::iterator iter = playerList.begin();
       iter != playerList.end(); ++iter) {
    ZStackObject *obj = (*iter)->getData();
    obj->setVisible(visible);
    bufferObjectModified(obj->getTarget());
  }

  notifyObjectModified();
}

void ZStackDoc::setVisible(ZStackObject::EType type, std::string source, bool visible)
{
  ZOUT(LTRACE(), 5) << "Set visible";
  TStackObjectList &objList = getObjectList(type);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (obj->isSameSource(obj->getSource(), source)) {
        obj->setVisible(visible);
    }
    bufferObjectModified(obj->getTarget());
  }

  notifyObjectModified();
}

void ZStackDoc::showSwcFullSkeleton(bool state)
{
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_SWC);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    if (state) {
      tree->addVisualEffect(NeuTube::Display::SwcTree::VE_FULL_SKELETON);
    } else {
      tree->removeVisualEffect(NeuTube::Display::SwcTree::VE_FULL_SKELETON);
    }
    bufferObjectModified(tree->getTarget());
  }

  notifyObjectModified();
}

void ZStackDoc::setResolution(double x, double y, double z, char unit)
{
  m_resolution.setVoxelSize(x, y, z);
  m_resolution.setUnit(unit);
}

void ZStackDoc::setResolution(const Cz_Lsminfo &lsmInfo)
{
  setResolution(lsmInfo.f64VoxelSizeX * 1e6,
                lsmInfo.f64VoxelSizeY * 1e6,
                lsmInfo.f64VoxelSizeZ * 1e6,
                'u');
}

double ZStackDoc::getPreferredZScale() const
{
  return m_resolution.voxelSizeZ() * 2.0 /
      (m_resolution.voxelSizeX() + m_resolution.voxelSizeY());
}

void ZStackDoc::selectSwcNode(const ZRect2d &roi)
{
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_SWC);
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
    expandSwcNodeList(&deselected, tree->getSelectedNode());
    tree->selectNode(roi, false);
    expandSwcNodeList(&selected, tree->getSelectedNode());
  }
  notifySelectionChanged(selected, deselected);
}

/*
void ZStackDoc::processRectRoiUpdateSlot()
{
  processRectRoiUpdate();
}
*/

void ZStackDoc::processRectRoiUpdate(ZRect2d *rect, bool /*appending*/)
{
  if (rect != NULL) {
    rect->setRole(ZStackObjectRole::ROLE_ROI);
    removeObject(rect, false);
    executeAddObjectCommand(rect);
    processObjectModified(rect);
  }
}

void ZStackDoc::removeRect2dRoi()
{
  removeObject(ZStackObjectSourceFactory::MakeRectRoiSource(), true);
}
