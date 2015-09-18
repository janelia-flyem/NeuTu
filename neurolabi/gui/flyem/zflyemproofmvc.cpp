#include "zflyemproofmvc.h"

#include <QFuture>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QMainWindow>
#include <QDesktopWidget>

#include "flyem/zflyemproofdoc.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackpresenter.h"
#include "dialogs/zdviddialog.h"
#include "dvid/zdvidreader.h"
#include "zstackobjectsourcefactory.h"
#include "dvid/zdvidsparsestack.h"
#include "zprogresssignal.h"
#include "zstackviewlocator.h"
#include "widgets/zimagewidget.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyemproofpresenter.h"
#include "zwidgetmessage.h"
#include "dialogs/zspinboxdialog.h"
#include "zdialogfactory.h"
#include "flyem/zflyembodyannotationdialog.h"
#include "zflyembodyannotation.h"
#include "flyem/zflyemsupervisor.h"
#include "dvid/zdvidwriter.h"
#include "zstring.h"
#include "flyem/zpaintlabelwidget.h"
#include "zwidgetfactory.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "flyem/zflyembookmarkannotationdialog.h"
#include "dialogs/flyembodyinfodialog.h"
#include "dialogs/zflyemsplitcommitdialog.h"
#include "flyem/zflyembodywindowfactory.h"
#include "flyem/zflyemmisc.h"
#include "zswcgenerator.h"
#include "zflyembody3ddoc.h"
#include "neutubeconfig.h"
#include "flyem/zflyemexternalneurondoc.h"
#include "zfiletype.h"

ZFlyEmProofMvc::ZFlyEmProofMvc(QWidget *parent) :
  ZStackMvc(parent)
{
  init();
}

ZFlyEmProofMvc::~ZFlyEmProofMvc()
{
  exitCurrentDoc();

  m_bodyViewWindow->close();
//  delete m_coarseBodyWindow;
//  delete m_bodyWindow;
//  delete m_splitWindow;
}

void ZFlyEmProofMvc::init()
{
  m_dvidDlg = new ZDvidDialog(this);
  m_bodyInfoDlg = new FlyEmBodyInfoDialog(this);
  m_supervisor = new ZFlyEmSupervisor(this);
  m_splitCommitDlg = new ZFlyEmSplitCommitDialog(this);

  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");

  initBodyWindow();
}

void ZFlyEmProofMvc::initBodyWindow()
{
  m_bodyViewWindow = new Z3DMainWindow(NULL);
  m_bodyViewWindow->setWindowTitle(QString::fromUtf8("3D Body View"));
  m_bodyViewWindow->setAttribute(Qt::WA_DeleteOnClose, false);

  m_bodyViewers = new Z3DTabWidget(m_bodyViewWindow);
  m_bodyViewers->setAttribute(Qt::WA_DeleteOnClose, false);

  QSizePolicy sizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  m_bodyViewers->setSizePolicy(sizePolicy);

  QVBoxLayout* bvLayout = new QVBoxLayout;

  QLabel *messageLabel = new QLabel;
  bvLayout->addWidget(messageLabel);

  QWidget *toolWidget = new QWidget(m_bodyViewWindow->toolBar);
  bvLayout->addWidget(toolWidget);

  bvLayout->addWidget(m_bodyViewers);

  m_bodyViewWindow->setLayout(bvLayout);
  m_bodyViewWindow->setCentralWidget(m_bodyViewers);
  m_bodyViewWindow->resize(QDesktopWidget().availableGeometry(0).size()*0.7);

  connect(m_bodyViewWindow, SIGNAL(closed()), this, SLOT(closeAllBodyWindow()));

  m_bodyWindowFactory =
      QSharedPointer<ZWindowFactory>(new ZFlyEmBodyWindowFactory);
  m_bodyWindowFactory->setDeleteOnClose(true);
  m_bodyWindowFactory->setControlPanelVisible(false);
  m_bodyWindowFactory->setObjectViewVisible(false);

  m_bodyViewWindow->m_stayOnTopAction =
      m_bodyViewWindow->toolBar->addAction("Pin");
  m_bodyViewWindow->m_stayOnTopAction->setCheckable(true);
   m_bodyViewWindow->m_stayOnTopAction->setChecked(false);
  connect(m_bodyViewWindow->m_stayOnTopAction, SIGNAL(triggered(bool)),
          m_bodyViewWindow, SLOT(stayOnTop(bool)));

  m_bodyViewWindow->toolBar->addSeparator();

  m_bodyViewWindow->resetCameraAction =
      m_bodyViewWindow->toolBar->addAction("X-Y View");
  connect(m_bodyViewWindow->resetCameraAction, SIGNAL(triggered()), m_bodyViewers, SLOT(resetCamera()));

  m_bodyViewWindow->xzViewAction =
      m_bodyViewWindow->toolBar->addAction("X-Z View");
  connect(m_bodyViewWindow->xzViewAction, SIGNAL(triggered()), m_bodyViewers, SLOT(setXZView()));

  m_bodyViewWindow->yzViewAction =
      m_bodyViewWindow->toolBar->addAction("Y-Z View");
  connect(m_bodyViewWindow->yzViewAction, SIGNAL(triggered()), m_bodyViewers, SLOT(setYZView()));


  m_bodyViewWindow->recenterAction =
      m_bodyViewWindow->toolBar->addAction("Center");
  connect(m_bodyViewWindow->recenterAction, SIGNAL(triggered()),
          m_bodyViewers, SLOT(resetCameraCenter()));

  m_bodyViewWindow->toolBar->addSeparator();

  m_bodyViewWindow->showGraphAction =
      m_bodyViewWindow->toolBar->addAction("Graph");
  connect(m_bodyViewWindow->showGraphAction, SIGNAL(toggled(bool)),
          m_bodyViewers, SLOT(showGraph(bool)));
  m_bodyViewWindow->showGraphAction->setCheckable(true);
  m_bodyViewWindow->showGraphAction->setChecked(true);


  m_bodyViewWindow->settingsAction =
      m_bodyViewWindow->toolBar->addAction("ControlSettings");
  connect(m_bodyViewWindow->settingsAction, SIGNAL(toggled(bool)), m_bodyViewers, SLOT(settingsPanel(bool)));
  m_bodyViewWindow->settingsAction->setCheckable(true);
  m_bodyViewWindow->settingsAction->setChecked(false);

  m_bodyViewWindow->objectsAction = m_bodyViewWindow->toolBar->addAction("Objects");
  connect(m_bodyViewWindow->objectsAction, SIGNAL(toggled(bool)), m_bodyViewers, SLOT(objectsPanel(bool)));
  m_bodyViewWindow->objectsAction->setCheckable(true);
  m_bodyViewWindow->objectsAction->setChecked(false);

  //update button status reversely
  connect(m_bodyViewers, SIGNAL(buttonShowGraphToggled(bool)),
          m_bodyViewWindow, SLOT(updateButtonShowGraph(bool)));
  connect(m_bodyViewers, SIGNAL(buttonSettingsToggled(bool)),
          m_bodyViewWindow, SLOT(updateButtonSettings(bool)));
  connect(m_bodyViewers, SIGNAL(buttonObjectsToggled(bool)),
          m_bodyViewWindow, SLOT(updateButtonObjects(bool)));

  connect(m_bodyViewers, SIGNAL(currentChanged(int)), m_bodyViewers, SLOT(updateTabs(int)));

  //
  m_coarseBodyWindow = NULL;
  m_externalNeuronWindow = NULL;
  m_bodyWindow = NULL;
  m_skeletonWindow = NULL;
  m_splitWindow = NULL;
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc)
{
  ZFlyEmProofMvc *frame = new ZFlyEmProofMvc(parent);

  BaseConstruct(frame, doc);

  return frame;
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(const ZDvidTarget &target)
{
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc;
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmProofMvc *mvc =
      ZFlyEmProofMvc::Make(NULL, ZSharedPointer<ZFlyEmProofDoc>(doc));
  mvc->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  mvc->setDvidTarget(target);

  return mvc;
}

void ZFlyEmProofMvc::detachCoarseBodyWindow()
{
  m_coarseBodyWindow = NULL;
}

void ZFlyEmProofMvc::detachBodyWindow()
{
  m_bodyWindow = NULL;
}

void ZFlyEmProofMvc::detachSplitWindow()
{
  m_splitWindow = NULL;
}

void ZFlyEmProofMvc::detachSkeletonWindow()
{
  m_skeletonWindow = NULL;
}

void ZFlyEmProofMvc::detachExternalNeuronWindow()
{
  m_externalNeuronWindow = NULL;
}

void ZFlyEmProofMvc::setWindowSignalSlot(Z3DWindow *window)
{
  if (window != NULL) {
    if (window == m_coarseBodyWindow) {
      connect(window, SIGNAL(destroyed()), this,
              SLOT(detachCoarseBodyWindow()));
      connect(window, SIGNAL(croppingSwcInRoi()),
              this, SLOT(cropCoarseBody3D()));
    } else if (window == m_bodyWindow) {
      connect(window, SIGNAL(destroyed()), this, SLOT(detachBodyWindow()));
    } else if (window == m_splitWindow) {
      connect(window, SIGNAL(destroyed()), this, SLOT(detachSplitWindow()));
    } else if (window == m_externalNeuronWindow) {
      connect(window, SIGNAL(destroyed()),
              this, SLOT(detachExternalNeuronWindow()));
    } else if (window == m_skeletonWindow) {
      connect(window, SIGNAL(destroyed()), this, SLOT(detachSkeletonWindow()));
    }
    connect(window, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
            this->getView(), SLOT(setView(ZStackViewParam)));
  }
}

ZFlyEmBody3dDoc* ZFlyEmProofMvc::makeBodyDoc(
    ZFlyEmBody3dDoc::EBodyType bodyType)
{
  ZFlyEmBody3dDoc *doc = new ZFlyEmBody3dDoc;
  doc->setDvidTarget(getDvidTarget());
//  doc->updateFrame();
  doc->setDataDoc(m_doc);
  doc->setBodyType(bodyType);

  ZWidgetMessage::ConnectMessagePipe(doc, this, false);

  return doc;
}

void ZFlyEmProofMvc::makeCoarseBodyWindow()
{
  ZFlyEmBody3dDoc *doc = makeBodyDoc(ZFlyEmBody3dDoc::BODY_COARSE);
  m_coarseBodyWindow = m_bodyWindowFactory->make3DWindow(doc);
  setWindowSignalSlot(m_coarseBodyWindow);


  if (m_doc->getParentMvc() != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindow(
          m_coarseBodyWindow, m_dvidInfo,
          m_doc->getParentMvc()->getView()->getViewParameter());
  }

  /*
  ZStackDoc *doc = new ZStackDoc;
  doc->setTag(NeuTube::Document::FLYEM_COARSE_BODY);

  getProgressSignal()->startProgress("Showing 3D coarse body ...");

  getProgressSignal()->advanceProgress(0.1);

  m_coarseBodyWindow = m_bodyWindowFactory->make3DWindow(doc);

  setWindowSignalSlot(m_coarseBodyWindow);
  ZFlyEmMisc::Decorate3dBodyWindow(
        m_coarseBodyWindow, m_dvidInfo,
        getView()->getViewParameter());
  m_coarseBodyWindow->setYZView();

  getProgressSignal()->advanceProgress(0.4);

//  update3DBodyView(false, true);

  getProgressSignal()->endProgress();
  */
}

void ZFlyEmProofMvc::makeBodyWindow()
{
  ZFlyEmBody3dDoc *doc = makeBodyDoc(ZFlyEmBody3dDoc::BODY_FULL);
  m_bodyWindow = m_bodyWindowFactory->make3DWindow(doc);
  setWindowSignalSlot(m_bodyWindow);


  if (m_doc->getParentMvc() != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindow(
          m_bodyWindow, m_dvidInfo,
          m_doc->getParentMvc()->getView()->getViewParameter());
  }
}

void ZFlyEmProofMvc::makeSkeletonWindow()
{
  ZFlyEmBody3dDoc *doc = makeBodyDoc(ZFlyEmBody3dDoc::BODY_SKELETON);

  m_skeletonWindow = m_bodyWindowFactory->make3DWindow(doc);
  setWindowSignalSlot(m_skeletonWindow);

  if (m_doc->getParentMvc() != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindow(
          m_skeletonWindow, m_dvidInfo,
          m_doc->getParentMvc()->getView()->getViewParameter());
  }
}

void ZFlyEmProofMvc::makeExternalNeuronWindow()
{
  ZFlyEmExternalNeuronDoc *doc = new ZFlyEmExternalNeuronDoc;
  doc->setDataDoc(m_doc);
  ZWidgetMessage::ConnectMessagePipe(doc, this, false);

  m_externalNeuronWindow = m_bodyWindowFactory->make3DWindow(doc);
  setWindowSignalSlot(m_externalNeuronWindow);
  if (m_doc->getParentMvc() != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindow(
          m_externalNeuronWindow, m_dvidInfo,
          m_doc->getParentMvc()->getView()->getViewParameter());
  }
}

void ZFlyEmProofMvc::mergeCoarseBodyWindow()
{
  if (m_coarseBodyWindow != NULL) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_coarseBodyWindow->getDocument());
    if (doc != NULL){
      doc->mergeBodyModel(*(getCompleteDocument()->getBodyMerger()));
    }
  }
}

void ZFlyEmProofMvc::updateCoarseBodyWindow()
{
  if (m_coarseBodyWindow != NULL) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_coarseBodyWindow->getDocument());
    if (doc != NULL){
      doc->addBodyChangeEvent(bodySet.begin(), bodySet.end());
    }
  }

//  updateCoarseBodyWindow(false, false, false);
}

void ZFlyEmProofMvc::updateCoarseBodyWindowDeep()
{
  if (m_coarseBodyWindow != NULL) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_coarseBodyWindow->getDocument());
    if (doc != NULL){
      doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
      doc->dumpAllSwc();
      doc->addBodyChangeEvent(bodySet.begin(), bodySet.end());
      doc->processEventFunc();
      doc->endObjectModifiedMode();
      doc->notifyObjectModified();
    }
  }
//  updateCoarseBodyWindow(false, false, true);
}

void ZFlyEmProofMvc::updateBodyWindow()
{
  if (m_bodyWindow != NULL) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_bodyWindow->getDocument());
    if (doc != NULL){
      doc->addBodyChangeEvent(bodySet.begin(), bodySet.end());
    }
  }
}

void ZFlyEmProofMvc::updateSkeletonWindow()
{
  if (m_skeletonWindow != NULL) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_skeletonWindow->getDocument());
    if (doc != NULL){
      doc->addBodyChangeEvent(bodySet.begin(), bodySet.end());
    }
  }
}

void ZFlyEmProofMvc::updateCoarseBodyWindowColor()
{
  updateCoarseBodyWindow();
#if 0
  if (m_coarseBodyWindow != NULL) {
    std::set<std::string> currentBodySourceSet;
    std::set<uint64_t> selectedMapped =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_MAPPED);

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      currentBodySourceSet.insert(
            ZStackObjectSourceFactory::MakeFlyEmBodySource(*iter));
    }

    m_coarseBodyWindow->getDocument()->beginObjectModifiedMode(
          ZStackDoc::OBJECT_MODIFIED_CACHE);

    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    QList<ZSwcTree*> bodyList = m_coarseBodyWindow->getDocument()->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = bodyList.begin();
         iter != bodyList.end(); ++iter) {
      ZSwcTree *tree = *iter;

      std::vector<uint64_t> labelArray = ZString(tree->getSource()).toUint64Array();
      if (!labelArray.empty()) {
        uint64_t label = labelArray.back();
        QColor color = labelSlice->getColor(label, NeuTube::BODY_LABEL_ORIGINAL);
        color.setAlpha(255);
        tree->setColor(color);
        m_coarseBodyWindow->getDocument()->processObjectModified(tree);
      }
    }

    m_coarseBodyWindow->getDocument()->endObjectModifiedMode();
    m_coarseBodyWindow->getDocument()->notifyObjectModified();
  }
#endif
}

void ZFlyEmProofMvc::updateCoarseBodyWindow(
    bool showingWindow, bool resettingCamera, bool isDeep)
{
  if (m_coarseBodyWindow != NULL) {
    std::set<std::string> currentBodySourceSet;
    std::set<uint64_t> selectedMapped =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_MAPPED);

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      currentBodySourceSet.insert(
            ZStackObjectSourceFactory::MakeFlyEmBodySource(*iter));
    }

    m_coarseBodyWindow->getDocument()->beginObjectModifiedMode(
          ZStackDoc::OBJECT_MODIFIED_CACHE);

    if (isDeep) {
      m_coarseBodyWindow->getDocument()->removeAllSwcTree(true);
    }

    std::set<std::string> oldBodySourceSet;
    QList<ZSwcTree*> bodyList = m_coarseBodyWindow->getDocument()->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = bodyList.begin();
         iter != bodyList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (currentBodySourceSet.count(tree->getSource()) == 0) {
        m_coarseBodyWindow->getDocument()->removeObject(
              dynamic_cast<ZStackObject*>(tree), true);
      } else {
        oldBodySourceSet.insert(tree->getSource());
      }
    }

    ZDvidReader reader;
    reader.open(getDvidTarget());

//    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    /*
    if (m_doc->getParentMvc() != NULL) {
      ZFlyEmMisc::Decorate3dBodyWindow(
            m_coarseBodyWindow, m_dvidInfo,
            m_doc->getParentMvc()->getView()->getViewParameter());
    }
    */

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      uint64_t label = *iter;
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(label);
      if (oldBodySourceSet.count(source) == 0) {
        ZObject3dScan body;

        QList<uint64_t> bodyList = getCompleteDocument()->getMergedSource(label);
//        bodyList.append(label);

        for (int i = 0; i < bodyList.size(); ++i) {
          body.concat(reader.readCoarseBody(bodyList[i]));
        }

        if (!body.isEmpty()) {
          ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
          if (labelSlice != NULL) {
            body.setColor(labelSlice->getColor(
                            label, NeuTube::BODY_LABEL_MAPPED));
          }

          body.setAlpha(255);
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-m_dvidInfo.getStartBlockIndex());
          tree->rescale(m_dvidInfo.getBlockSize().getX(),
                        m_dvidInfo.getBlockSize().getY(),
                        m_dvidInfo.getBlockSize().getZ());
          tree->translate(m_dvidInfo.getStartCoordinates());
          tree->setSource(source);
          m_coarseBodyWindow->getDocument()->addObject(tree, true);
        }
      }
    }
//    m_bodyWindow->getDocument()->blockSignals(false);
//    m_bodyWindow->getDocument()->notifySwcModified();
    m_coarseBodyWindow->getDocument()->endObjectModifiedMode();
    m_coarseBodyWindow->getDocument()->notifyObjectModified();

    if (showingWindow) {
      m_bodyViewWindow->show();
      m_bodyViewWindow->raise();
    }

    if (resettingCamera) {
      m_coarseBodyWindow->resetCameraCenter();
    }
  }
}

void ZFlyEmProofMvc::updateBodyWindowPlane(
    Z3DWindow *window, const ZStackViewParam &viewParam)
{
  if (window != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindowPlane(window, m_dvidInfo, viewParam);
  }
}

ZFlyEmProofDoc* ZFlyEmProofMvc::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(getDocument().get());
}

ZFlyEmProofPresenter* ZFlyEmProofMvc::getCompletePresenter() const
{
  return qobject_cast<ZFlyEmProofPresenter*>(getPresenter());
}

void ZFlyEmProofMvc::mergeSelected()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->mergeSelected(getSupervisor());
  }
}

void ZFlyEmProofMvc::undo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->undo();
  }
}

void ZFlyEmProofMvc::redo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->redo();
  }
}

void ZFlyEmProofMvc::setSegmentationVisible(bool visible)
{
  m_showSegmentation = visible;
  if (getCompleteDocument() != NULL) {
    QList<ZDvidLabelSlice*> sliceList =
        getCompleteDocument()->getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *slice, sliceList) {
      slice->setVisible(visible);
      if (visible) {
        slice->update(getView()->getViewParameter(NeuTube::COORD_STACK));
      }
    }
  }
  getView()->redrawObject();
}

void ZFlyEmProofMvc::clear()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->clearData();
    getPresenter()->clearData();
//    getView()->imageWidget();
  }

  m_dvidInfo.clear();
}

void ZFlyEmProofMvc::exitCurrentDoc()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->saveCustomBookmark();
    getCompleteDocument()->saveMergeOperation();
  }
}

void ZFlyEmProofMvc::setDvidTargetFromDialog()
{
  getProgressSignal()->startProgress("Loading data ...");
  setDvidTarget(m_dvidDlg->getDvidTarget());
  getProgressSignal()->endProgress();
}

void ZFlyEmProofMvc::setDvidTarget(const ZDvidTarget &target)
{
  if (getCompleteDocument()->getDvidTarget().isValid()) {
    emit messageGenerated(
          ZWidgetMessage("You cannot change the database in this window. "
                         "Please open a new proofread window to load a different database",
                         NeuTube::MSG_WARNING,
                         ZWidgetMessage::TARGET_DIALOG));
    return;
  }

  exitCurrentDoc();

  getProgressSignal()->startProgress("Loading data ...");

  if (getCompleteDocument() != NULL) {
#if 1
//    QByteArray geometry;
    bool isMaximized = false;
    if (getMainWindow() != NULL) {
      isMaximized = getMainWindow()->isMaximized();
//      geometry = parentWidget()->saveGeometry();
    }
#endif

    clear();
    getProgressSignal()->advanceProgress(0.1);
//    getCompleteDocument()->clearData();
    getCompleteDocument()->setDvidTarget(target);
    getCompleteDocument()->beginObjectModifiedMode(
          ZStackDoc::OBJECT_MODIFIED_CACHE);
    getCompleteDocument()->updateTileData();
    getCompleteDocument()->endObjectModifiedMode();
    QList<ZDvidTileEnsemble*> teList =
        getCompleteDocument()->getDvidTileEnsembleList();
    foreach (ZDvidTileEnsemble *te, teList) {
      te->enhanceContrast(getCompletePresenter()->highTileContrast());
      te->attachView(getView());
    }
    getView()->reset(false);
    getProgressSignal()->advanceProgress(0.1);

#if 0
    if (getMainWindow() != NULL) {
//      parentWidget()->hide();
//      parentWidget()->restoreGeometry(geometry);
//      parentWidget()->show();
      if (isMaximized) {
        getMainWindow()->showNormal();
        getMainWindow()->showMaximized();
      }
    }
#endif

    m_splitProject.setDvidTarget(target);
    m_mergeProject.setDvidTarget(target);
    m_mergeProject.syncWithDvid();
    getProgressSignal()->advanceProgress(0.2);

    ZDvidReader reader;
    if (reader.open(target)) {
      m_dvidInfo = reader.readGrayScaleInfo();
    }

    if (getSupervisor() != NULL) {
      getSupervisor()->setDvidTarget(target);
    }

    getCompleteDocument()->downloadSynapse();
    getCompleteDocument()->downloadBookmark();
    getProgressSignal()->advanceProgress(0.5);

    emit dvidTargetChanged(target);
  }
  getProgressSignal()->endProgress();

  emit messageGenerated(ZWidgetMessage("Database loaded.",
                                       NeuTube::MSG_INFORMATION,
                                       ZWidgetMessage::TARGET_STATUS_BAR));
}

void ZFlyEmProofMvc::setDvidTarget()
{
//  m_dvidDlg = new ZDvidDialog(this);
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    setDvidTarget(target);
    /*
    const QString threadId = "setDvidTarget";
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(this, &ZFlyEmProofMvc::setDvidTargetFromDialog);
      m_futureMap[threadId] = future;
    }
    */


  }
}

ZDvidTarget ZFlyEmProofMvc::getDvidTarget() const
{
  if (m_dvidDlg != NULL) {
    return m_dvidDlg->getDvidTarget();
  }

  return ZDvidTarget();
}

void ZFlyEmProofMvc::createPresenter()
{
  if (getDocument().get() != NULL) {
    m_presenter = ZFlyEmProofPresenter::Make(this);
  }
}

void ZFlyEmProofMvc::customInit()
{
  connect(getPresenter(), SIGNAL(bodySplitTriggered()),
          this, SLOT(notifySplitTriggered()));
  connect(getPresenter(), SIGNAL(bodyAnnotationTriggered()),
          this, SLOT(annotateBody()));
  connect(getPresenter(), SIGNAL(bodyCheckinTriggered()),
          this, SLOT(checkInSelectedBody()));
  connect(getPresenter(), SIGNAL(bodyForceCheckinTriggered()),
          this, SLOT(checkInSelectedBodyAdmin()));
  connect(getPresenter(), SIGNAL(bodyCheckoutTriggered()),
          this, SLOT(checkOutBody()));
  connect(getPresenter(), SIGNAL(objectVisibleTurnedOn()),
          this, SLOT(processViewChange()));
  connect(getCompletePresenter(), SIGNAL(goingToBody()),
          this, SLOT(goToBody()));
  connect(getCompletePresenter(), SIGNAL(goingToBodyBottom()),
          this, SLOT(goToBodyBottom()));
  connect(getCompletePresenter(), SIGNAL(goingToBodyTop()),
          this, SLOT(goToBodyTop()));
  //  connect(getCompletePresenter(), SIGNAL(labelSliceSelectionChanged()),
//          this, SLOT(processLabelSliceSelectionChange()));

  connect(getDocument().get(), SIGNAL(activeViewModified()),
          this, SLOT(processViewChange()));
  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          getView(), SLOT(paintObject()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          getView(), SLOT(paintObject()));
  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          &m_mergeProject, SLOT(update3DBodyViewDeep()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          &m_mergeProject, SLOT(update3DBodyViewDeep()));

  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          this, SLOT(updateCoarseBodyWindowColor()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          this, SLOT(updateCoarseBodyWindowColor()));

  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          &m_mergeProject, SLOT(saveMergeOperation()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          &m_mergeProject, SLOT(saveMergeOperation()));

  connect(getCompleteDocument(), SIGNAL(userBookmarkModified()),
          this, SLOT(updateUserBookmarkTable()));
  connect(getCompleteDocument(), SIGNAL(bodyIsolated(uint64_t)),
          this, SLOT(checkInBodyWithMessage(uint64_t)));
  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          getCompleteDocument(), SLOT(deprecateSplitSource()));

  m_mergeProject.getProgressSignal()->connectProgress(getProgressSignal());
  m_splitProject.getProgressSignal()->connectProgress(getProgressSignal());


  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          getView(), SLOT(paintObject()));
  connect(getDocument().get(),
          SIGNAL(objectSelectorChanged(ZStackObjectSelector)),
          this, SLOT(processSelectionChange(ZStackObjectSelector)));
  connect(getCompleteDocument(), SIGNAL(bodySelectionChanged()),
          this, SLOT(updateBodySelection()));


  m_splitProject.setDocument(getDocument());
  connect(&m_splitProject, SIGNAL(locating2DViewTriggered(const ZStackViewParam&)),
          this->getView(), SLOT(setView(const ZStackViewParam&)));
  connect(&m_splitProject, SIGNAL(resultCommitted()),
          this, SLOT(updateSplitBody()));
  /*
  connect(&m_splitProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));
          */

  m_mergeProject.setDocument(getDocument());
  connect(getPresenter(), SIGNAL(labelSliceSelectionChanged()),
          this, SLOT(updateBodySelection()));
  connect(getCompletePresenter(), SIGNAL(highlightingSelected(bool)),
          &m_mergeProject, SLOT(highlightSelectedObject(bool)));
  connect(&m_mergeProject, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
          this->getView(), SLOT(setView(ZStackViewParam)));
  connect(&m_mergeProject, SIGNAL(dvidLabelChanged()),
          this->getCompleteDocument(), SLOT(updateDvidLabelObject()));
  connect(&m_mergeProject, SIGNAL(checkingInBody(uint64_t)),
          this, SLOT(checkInBodyWithMessage(uint64_t)));
  /*
  connect(&m_mergeProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString,bool)));
  connect(getDocument().get(), SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));
          */

  ZWidgetMessage::ConnectMessagePipe(&m_splitProject, this, false);
  ZWidgetMessage::ConnectMessagePipe(&m_mergeProject, this, false);
//  ZWidgetMessage::ConnectMessagePipe(&getDocument().get(), this, false);


  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          this, SLOT(presentBodySplit(uint64_t)));

  connect(getCompletePresenter(), SIGNAL(selectingBodyAt(int,int,int)),
          this, SLOT(xorSelectionAt(int, int, int)));
  connect(getCompletePresenter(), SIGNAL(deselectingAllBody()),
          this, SLOT(deselectAllBody()));
  connect(getCompletePresenter(), SIGNAL(runningSplit()), this, SLOT(runSplit()));
  connect(getCompletePresenter(), SIGNAL(bookmarkAdded(ZFlyEmBookmark*)),
          this, SLOT(annotateBookmark(ZFlyEmBookmark*)));
  connect(getCompletePresenter(), SIGNAL(annotatingBookmark(ZFlyEmBookmark*)),
          this, SLOT(annotateBookmark(ZFlyEmBookmark*)));
  connect(getCompletePresenter(), SIGNAL(mergingBody()),
          this, SLOT(mergeSelected()));
//  connect(getCompletePresenter(), SIGNAL(goingToBody()), this, SLOT());

  disableSplit();


  connect(m_bodyInfoDlg, SIGNAL(bodyActivated(uint64_t)),
          this, SLOT(locateBody(uint64_t)));
  connect(this, SIGNAL(dvidTargetChanged(ZDvidTarget)), m_bodyInfoDlg, SLOT(dvidTargetChanged(ZDvidTarget)));

  /*
  QPushButton *button = new QPushButton(this);
  button->setCheckable(true);
  button->setChecked(true);
  button->setIcon(QIcon(":/images/synapse.png"));
  connect(button, SIGNAL(toggled(bool)),
          this, SLOT(showSynapseAnnotation(bool)));
          */

//  getView()->addHorizontalWidget(button);

//  getView()->addHorizontalWidget(ZWidgetFactory::makeHSpacerItem());

  m_paintLabelWidget = new ZPaintLabelWidget();

  getView()->addHorizontalWidget(m_paintLabelWidget);
  m_paintLabelWidget->hide();
}

void ZFlyEmProofMvc::goToBodyBottom()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    if (!bodySet.empty()) {
      ZIntPoint pt;
      std::set<uint64_t>::const_iterator iter = bodySet.begin();
      pt = reader.readBodyBottom(*iter);
      ++iter;
      for (; iter != bodySet.end(); ++iter) {
        uint64_t bodyId = *iter;
        ZIntPoint tmpPt = reader.readBodyBottom(bodyId);
        if (pt.getZ() > tmpPt.getZ()) {
          pt = tmpPt;
        }
      }
      zoomTo(pt.getX(), pt.getY(), pt.getZ());
    }
  }
}

void ZFlyEmProofMvc::goToBodyTop()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    std::set<uint64_t> bodySet =
        getCompleteDocument()->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    if (!bodySet.empty()) {
      ZIntPoint pt;
      std::set<uint64_t>::const_iterator iter = bodySet.begin();
      pt = reader.readBodyTop(*iter);
      ++iter;
      for (; iter != bodySet.end(); ++iter) {
        uint64_t bodyId = *iter;
        ZIntPoint tmpPt = reader.readBodyTop(bodyId);
        if (pt.getZ() < tmpPt.getZ()) {
          pt = tmpPt;
        }
      }
      zoomTo(pt.getX(), pt.getY(), pt.getZ());
    }
  }
}


void ZFlyEmProofMvc::goToBody()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Body:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<uint64_t> bodyArray = str.toUint64Array();
      if (bodyArray.size() == 1) {
        locateBody(bodyArray[0]);
//        emit locatingBody();
      }
    }
  }
}

void ZFlyEmProofMvc::selectBody()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Body:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<int> bodyArray = str.toIntegerArray();
      if (bodyArray.size() == 1) {
        selectBody((uint64_t) bodyArray[0]);
      }
    }
  }
}

void ZFlyEmProofMvc::processLabelSliceSelectionChange()
{
  ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
  if (labelSlice != NULL){
    std::vector<uint64_t> selected =
        labelSlice->getSelector().getSelectedList();
    if (selected.size() == 1) {
      ZDvidReader reader;
      if (reader.open(getDvidTarget())) {
        uint64_t bodyId = selected.front();
        ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);
        ZWidgetMessage msg("", NeuTube::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_CUSTOM_AREA);
        if (annotation.isEmpty()) {
          msg.setMessage(QString("%1 is not annotated.").arg(selected.front()));
        } else {
          msg.setMessage(annotation.toString().c_str());
        }
        emit messageGenerated(msg);
      }

    }
  }
}


void ZFlyEmProofMvc::processSelectionChange(const ZStackObjectSelector &selector)
{
  const std::vector<ZStackObject*>& objList =
      selector.getSelectedList(ZStackObject::TYPE_FLYEM_BOOKMARK);
  if (!objList.empty()) {
    const ZStackObject *obj = objList.back();
    const ZFlyEmBookmark *bookmark = dynamic_cast<const ZFlyEmBookmark*>(obj);
    if (bookmark != NULL) {
      emit messageGenerated(
            ZWidgetMessage(bookmark->toJsonObject(true).dumpString(0).c_str(),
                           NeuTube::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
    }
  } else {
    emit messageGenerated(
          ZWidgetMessage("---",
                         NeuTube::MSG_INFORMATION,
                         ZWidgetMessage::TARGET_STATUS_BAR));
  }
}

void ZFlyEmProofMvc::runSplitFunc()
{
  getProgressSignal()->startProgress(1.0);
  m_splitProject.runSplit();
  getProgressSignal()->endProgress();
}

void ZFlyEmProofMvc::runSplit()
{
//  getProgressSignal()->startProgress("Running split ...");
  runSplitFunc();
//  getProgressSignal()->endProgress();

#if 0 //has strange crash caused by Make_Stack
  const QString threadId = "runSplit";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(
          this, &ZFlyEmProofMvc::runSplitFunc);
    m_futureMap[threadId] = future;
  }
#endif
//  m_splitProject.runSplit();
}

void ZFlyEmProofMvc::updateBodySelection()
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    const std::set<uint64_t> &selected = slice->getSelectedOriginal();
    m_mergeProject.setSelection(selected, NeuTube::BODY_LABEL_ORIGINAL);
    updateCoarseBodyWindow();
    updateBodyWindow();
    updateSkeletonWindow();
//    m_mergeProject.update3DBodyView();
    if (getCompletePresenter()->isHighlight()) {
      m_mergeProject.highlightSelectedObject(true);
    } else {
      getCompleteDocument()->processObjectModified(slice);
    }
    processLabelSliceSelectionChange();
  }
}

bool ZFlyEmProofMvc::checkInBody(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkIn(bodyId);
  }

  return true;
}

bool ZFlyEmProofMvc::checkInBodyWithMessage(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    if (bodyId > 0) {
      if (getSupervisor()->checkIn(bodyId)) {
        emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        return true;
      } else {
        emit errorGenerated(QString("Failed to unlock body %1.").arg(bodyId));
      }
    }
  }

  return true;
}

bool ZFlyEmProofMvc::checkOutBody(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkOut(bodyId);
  }

  return true;
}

void ZFlyEmProofMvc::checkInSelectedBody()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkIn(bodyId)) {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to unlock body %1.").arg(bodyId));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
}

void ZFlyEmProofMvc::checkInSelectedBodyAdmin()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->isLocked(bodyId)) {
          if (getSupervisor()->checkInAdmin(bodyId)) {
            emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
          } else {
            emit errorGenerated(QString("Failed to unlock body %1.").arg(bodyId));
          }
        } else {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
}

void ZFlyEmProofMvc::checkOutBody()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkOut(bodyId)) {
          emit messageGenerated(QString("Body %1 is locked.").arg(bodyId));
        } else {
          std::string owner = getSupervisor()->getOwner(bodyId);
          if (owner.empty()) {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to lock body %1. Is the librarian sever (%2) ready?").
                    arg(bodyId).arg(getDvidTarget().getSupervisor().c_str()),
                    NeuTube::MSG_ERROR));
          } else {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to lock body %1 because it has been locked by %2").
                    arg(bodyId).arg(owner.c_str()), NeuTube::MSG_ERROR));
          }
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
#if 0
  std::set<uint64_t> bodyIdArray =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED);
  if (bodyIdArray.size() == 1) {
    uint64_t bodyId = *(bodyIdArray.begin());
    if (bodyId > 0) {
      if (getSupervisor() != NULL) {
        if (getSupervisor()->checkOut(bodyId)) {
          emit messageGenerated(QString("Body %1 is locked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to check out body %1.").arg(bodyId));
        }
      }
    }
  }
#endif
}

void ZFlyEmProofMvc::annotateBody()
{
  std::set<uint64_t> bodyIdArray =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
  if (bodyIdArray.size() == 1) {
    uint64_t bodyId = *(bodyIdArray.begin());
    if (bodyId > 0) {
      if (checkOutBody(bodyId)) {
        ZFlyEmBodyAnnotationDialog *dlg = new ZFlyEmBodyAnnotationDialog(this);
        dlg->setBodyId(bodyId);
        ZDvidReader reader;
        if (reader.open(getDvidTarget())) {
          ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);

          if (!annotation.isEmpty()) {
            dlg->loadBodyAnnotation(annotation);
          }
        }

        if (dlg->exec() && dlg->getBodyId() == bodyId) {
          getCompleteDocument()->annotateBody(bodyId, dlg->getBodyAnnotation());
        }

        checkInBodyWithMessage(bodyId);
      } else {
        if (getSupervisor() != NULL) {
          std::string owner = getSupervisor()->getOwner(bodyId);
          if (owner.empty()) {
//            owner = "unknown user";
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to lock body %1. Is the librarian sever (%2) ready?").
                    arg(bodyId).arg(getDvidTarget().getSupervisor().c_str()),
                    NeuTube::MSG_ERROR));
          } else {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to start annotation. %1 has been locked by %2").
                    arg(bodyId).arg(owner.c_str()), NeuTube::MSG_ERROR));
          }
        }
      }
    } else {
      qDebug() << "Unexpected body ID: 0";
    }
  } else {
    QString msg;
    if (getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED).size() == 1) {
      msg = "The annotation cannot be done because "
          "the merged body has not be uploaded.";
    } else {
      msg = "The annotation cannot be done because "
          "one and only one body has to be selected.";
    }
    if (!msg.isEmpty()) {
      emit messageGenerated(ZWidgetMessage(msg, NeuTube::MSG_WARNING));
    }
  }


//  emit messageGenerated(
//        ZWidgetMessage("The function of annotating body is still under testing.",
//                       NeuTube::MSG_WARING));
}

void ZFlyEmProofMvc::notifySplitTriggered()
{
  const std::set<uint64_t> &selected =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);

  if (selected.size() == 1) {
    uint64_t bodyId = *(selected.begin());

    emit launchingSplit(bodyId);
  } else {
    QString msg;
    if (getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED).size() == 1) {
      msg = "The split cannot be launched because "
          "the merged body has not been uploaded.";
    } else {
      msg = "The split cannot be launched because "
          "one and only one body has to be selected.";
    }
    emit messageGenerated(ZWidgetMessage(msg, NeuTube::MSG_WARNING));
  }

  /*
  ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

  if (labelSlice->isVisible()) {
    const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();

    if (selected.size() == 1) {
      uint64_t bodyId = *(selected.begin());

      emit launchingSplit(bodyId);
    } else {
      emit messageGenerated("The split cannot be launched because "
                            "one and only one body has to be selected.");
    }
  }
  */
}

void ZFlyEmProofMvc::launchSplitFunc(uint64_t bodyId)
{
  if (bodyId == 0) {
    emit errorGenerated(QString("Invalid body id: %1").arg(bodyId));
    return;
  }

  if (!getCompleteDocument()->isSplittable(bodyId)) {
    QString msg = QString("%1 is not ready for split.").arg(bodyId);
    emit messageGenerated(
          ZWidgetMessage(msg, NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG));
    emit errorGenerated(msg);
  } else {
    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Launching split ...");

      getCompletePresenter()->setHighlightMode(false);
      m_mergeProject.highlightSelectedObject(false);

      if (body != NULL) {
        if (body->getLabel() != bodyId) {
          body = NULL;
        }
      }

      getProgressSignal()->advanceProgress(0.1);

      ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

      getProgressSignal()->advanceProgress(0.1);

      if (body == NULL) {
        body = reader.readDvidSparseStack(bodyId);
      }

      if (body->isEmpty()) {
        delete body;
        body = NULL;

        QString msg = QString("Invalid body id: %1").arg(bodyId);
        emit messageGenerated(
              ZWidgetMessage(msg, NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG));
        emit errorGenerated(msg);
      } else {
        body->setZOrder(0);
        body->setSource(ZStackObjectSourceFactory::MakeSplitObjectSource());
        body->setMaskColor(labelSlice->getColor(
                             bodyId, NeuTube::BODY_LABEL_ORIGINAL));
        body->setSelectable(false);
        body->getObjectMask()->setLabel(bodyId);
        getDocument()->addObject(body, true);
        m_splitProject.setBodyId(bodyId);

        labelSlice->setVisible(false);
        labelSlice->setHittable(false);
        body->setVisible(true);
        body->setProjectionVisible(false);

        getProgressSignal()->advanceProgress(0.1);

        emit splitBodyLoaded(bodyId);
      }

//      getDocument()->setVisible(ZStackObject::TYPE_PUNCTA, true);

      getProgressSignal()->endProgress();
    }
  }
}

void ZFlyEmProofMvc::updateSplitBody()
{
  if (m_splitProject.getBodyId() > 0) {
#if 0
    uint64_t bodyId = m_splitProject.getBodyId();
    getDocument()->removeObject(
          ZStackObjectSourceFactory::MakeSplitObjectSource(), true);
    m_splitProject.setBodyId(0);
    launchSplit(bodyId);
#endif
    if (m_coarseBodyWindow != NULL) {
      m_coarseBodyWindow->removeRectRoi();
      updateCoarseBodyWindowDeep();
//      updateCoarseBodyWindow(false, false, true);
    }
    updateBodyWindow();
  }
}

void ZFlyEmProofMvc::presentBodySplit(uint64_t bodyId)
{
  enableSplit();

  m_paintLabelWidget->show();
  m_mergeProject.closeBodyWindow();

  m_splitProject.setBodyId(bodyId);
  m_splitProject.downloadSeed();
  emit bookmarkUpdated(&m_splitProject);
  getView()->redrawObject();
}

void ZFlyEmProofMvc::enableSplit()
{
//  m_splitOn = true;
  getCompletePresenter()->enableSplit();
}

void ZFlyEmProofMvc::disableSplit()
{
//  m_splitOn = false;
  getCompletePresenter()->disableSplit();
}

void ZFlyEmProofMvc::launchSplit(uint64_t bodyId)
{
  if (bodyId > 0) {
    if (checkOutBody(bodyId)) {
#ifdef _DEBUG_2
      bodyId = 14742253;
#endif
      const QString threadId = "launchSplitFunc";
      if (!m_futureMap.isAlive(threadId)) {
        m_futureMap.removeDeadThread();
        QFuture<void> future =
            QtConcurrent::run(
              this, &ZFlyEmProofMvc::launchSplitFunc, bodyId);
        m_futureMap[threadId] = future;
      }
    } else {
      std::string owner = getSupervisor()->getOwner(bodyId);
      if (owner.empty()) {
//        owner = "unknown user";
        emit messageGenerated(
              ZWidgetMessage(
                QString("Failed to lock body %1. Is the librarian sever (%2) ready?").
                arg(bodyId).arg(getDvidTarget().getSupervisor().c_str()),
                NeuTube::MSG_ERROR));
      }
      emit messageGenerated(
            ZWidgetMessage(
              QString("Failed to launch split. %1 has been locked by %2").
              arg(bodyId).arg(owner.c_str()), NeuTube::MSG_ERROR));
    }
  }
}

void ZFlyEmProofMvc::exitSplit()
{
  if (getCompletePresenter()->isSplitWindow()) {
    emit messageGenerated("Exiting split ...");
//    emitMessage("Exiting split ...");
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    labelSlice->setVisible(true);
    labelSlice->update(getView()->getViewParameter(NeuTube::COORD_STACK));

    labelSlice->setHittable(true);

    //m_splitProject.clearBookmarkDecoration();
    getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);
//    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_BOOKMARK);

    getDocument()->setVisible(ZStackObject::TYPE_DVID_SPARSE_STACK, false);

    checkInBodyWithMessage(m_splitProject.getBodyId());
//    getDocument()->setVisible(ZStackObject::TYPE_PUNCTA, false);


    /*
    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    if (body != NULL) {
      body->setVisible(false);
    }

    getView()->redrawObject();
    */

    m_paintLabelWidget->hide();

    m_splitProject.clear();

    disableSplit();
  }
}

void ZFlyEmProofMvc::switchSplitBody(uint64_t bodyId)
{
  if (bodyId != m_splitProject.getBodyId()) {
    if (getCompletePresenter()->isSplitWindow()) {
//      exitSplit();
      QMessageBox msgBox;
       msgBox.setText("Changing to another body to split.");
       msgBox.setInformativeText("Do you want to save your seeds?");
       msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
       msgBox.setDefaultButton(QMessageBox::Save);
       int ret = msgBox.exec();
       if (ret != QMessageBox::Cancel) {
         if (ret == QMessageBox::Save) {
           m_splitProject.saveSeed(false);
         }
         m_splitProject.clear();
         getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
         getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);
         getCompleteDocument()->setSelectedBody(bodyId, NeuTube::BODY_LABEL_ORIGINAL);
         launchSplit(bodyId);
       }
    }
  }
}

void ZFlyEmProofMvc::processMessageSlot(const QString &message)
{
  ZJsonObject obj;
  obj.decodeString(message.toStdString().c_str());

  if (obj.hasKey("type")) {
    std::string type = ZJsonParser::stringValue(obj["type"]);
    if (type == "locate_view") {
//      viewRoi(int x, int y, int z, int radius);
    }
  }
}

void ZFlyEmProofMvc::showBodyQuickView()
{
  showFineBody3d();
//  m_splitProject.showBodyQuickView();
}

void ZFlyEmProofMvc::showSplitQuickView()
{
  m_splitProject.showResultQuickView();
}

void ZFlyEmProofMvc::showBody3d()
{
  m_splitProject.showDataFrame3d();
}

void ZFlyEmProofMvc::showSplit3d()
{
  m_splitProject.showResult3d();
}

void ZFlyEmProofMvc::showExternalNeuronWindow()
{
  if (m_externalNeuronWindow == NULL) {
    makeExternalNeuronWindow();
    m_bodyViewers->addWindow(2, m_externalNeuronWindow, "Neuron Reference");
  }
  else
  {
      m_bodyViewers->setCurrentIndex(2);
  }

//  updateCoarseBodyWindow(false, true, false);

  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();
}

void ZFlyEmProofMvc::showCoarseBody3d()
{
  if (m_coarseBodyWindow == NULL) {
    makeCoarseBodyWindow();
    m_bodyViewers->addWindow(0, m_coarseBodyWindow, "Coarse Body View");
    updateCoarseBodyWindow();
    m_coarseBodyWindow->setYZView();
  }
  else
  {
      m_bodyViewers->setCurrentIndex(0);
  }

//  updateCoarseBodyWindow(false, true, false);

  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();

//  m_mergeProject.showCoarseBody3d();
}

void ZFlyEmProofMvc::showFineBody3d()
{
//  m_mergeProject.showBody3d();
  if (m_bodyWindow == NULL) {
    makeBodyWindow();
    m_bodyViewers->addWindow(1, m_bodyWindow, "Body View");
    updateBodyWindow();
    m_bodyWindow->setYZView();
  }
  else
  {
      m_bodyViewers->setCurrentIndex(1);
  }

  m_bodyViewWindow->setCurrentWidow(m_bodyWindow);
  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();
}

void ZFlyEmProofMvc::showSkeletonWindow()
{
//  m_mergeProject.showBody3d();
  if (m_skeletonWindow == NULL) {
    makeSkeletonWindow();
    m_bodyViewers->addWindow(3, m_skeletonWindow, "Skeleton View");
    updateSkeletonWindow();
    m_skeletonWindow->setYZView();
  } else {
    m_bodyViewers->setCurrentIndex(3);
  }

  m_bodyViewWindow->setCurrentWidow(m_skeletonWindow);
  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();
}

/*
void ZFlyEmProofMvc::closeBodyWindow(int index)
{
  closeBodyWindow(m_coarseBodyWindow);
}
*/

void ZFlyEmProofMvc::closeBodyWindow(Z3DWindow *window)
{
  if (window != NULL) {
    window->close();
  }
}

void ZFlyEmProofMvc::closeAllBodyWindow()
{
  closeBodyWindow(m_coarseBodyWindow);
  closeBodyWindow(m_bodyWindow);
  closeBodyWindow(m_splitWindow);
  closeBodyWindow(m_skeletonWindow);
  closeBodyWindow(m_externalNeuronWindow);
}

void ZFlyEmProofMvc::setDvidLabelSliceSize(int width, int height)
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->setMaxSize(width, height);
      getView()->paintObject();
    }
  }
}

void ZFlyEmProofMvc::showFullSegmentation()
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->updateFullView(getView()->getViewParameter());
      getView()->paintObject();
    }
  }
}

void ZFlyEmProofMvc::saveSeed()
{
  m_splitProject.saveSeed(true);
}

void ZFlyEmProofMvc::saveMergeOperation()
{
  getCompleteDocument()->saveMergeOperation();
}

void ZFlyEmProofMvc::commitMerge()
{
  if (ZDialogFactory::Ask("Upload Confirmation",
                          "Do you want to upload the merging result now? "
                          "It cannot be undone. ",
                          this)) {
    mergeCoarseBodyWindow();
    m_mergeProject.uploadResult();
    ZDvidSparseStack *body = getCompleteDocument()->getBodyForSplit();
    if (body != NULL) {
      getDocument()->getObjectGroup().removeObject(body, true);
    }
  }
}

void ZFlyEmProofMvc::commitCurrentSplit()
{
  if (!getDocument()->isSegmentationReady()) {
//    emit messageGenerated("test");
    emit messageGenerated(
          ZWidgetMessage("Failed to save results: The split has not been updated."
                         "Please Run full split (shift+space) first.",
                         NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_TEXT_APPENDING));
    return;
  }


  if (m_splitCommitDlg->exec()) {
    m_splitProject.setMinObjSize(m_splitCommitDlg->getGroupSize());
    m_splitProject.keepMainSeed(m_splitCommitDlg->keepingMainSeed());
    const QString threadId = "ZFlyEmBodySplitProject::commitResult";
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(
            &m_splitProject, &ZFlyEmBodySplitProject::commitResult);
      m_futureMap[threadId] = future;
    }
//    m_splitProject.commitResult();
  }
}

void ZFlyEmProofMvc::zoomTo(int x, int y, int z, int width)
{
  z -= getDocument()->getStackOffset().getZ();

  ZStackViewLocator locator;
  locator.setCanvasSize(getView()->imageWidget()->canvasSize().width(),
                        getView()->imageWidget()->canvasSize().height());

  QRect viewPort = locator.getRectViewPort(x, y, width);
  getPresenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));
  getPresenter()->setViewPortCenter(x, y, z);

  getView()->highlightPosition(x, y, z);
}


void ZFlyEmProofMvc::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZFlyEmProofMvc::zoomTo(int x, int y, int z)
{
  zoomTo(x, y, z, 800);
}

void ZFlyEmProofMvc::syncDvidBookmark()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    TStackObjectList &objList =
        getDocument()->getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(obj);
      if (bookmark != NULL) {
        const QByteArray &bookmarkData = reader.readKeyValue(
              ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK), bookmark->getDvidKey());
        if (!bookmarkData.isEmpty()) {
          ZJsonObject obj;
          obj.decodeString(bookmarkData.data());
          if (obj.hasKey("checked")) {
            bookmark->setChecked(ZJsonParser::booleanValue(obj["checked"]));
          }
        }
      }
    }
  }
}

void ZFlyEmProofMvc::notifyBookmarkUpdated()
{
  syncDvidBookmark();

//  m_splitProject.updateBookmarkDecoration();
//  m_mergeProject.updateBookmarkDecoration();

  emit bookmarkUpdated(&m_mergeProject);
  emit bookmarkUpdated(&m_splitProject);
}

void ZFlyEmProofMvc::notifyBookmarkDeleted()
{
  emit bookmarkDeleted(&m_mergeProject);
  emit bookmarkDeleted(&m_splitProject);
}

void ZFlyEmProofMvc::loadBookmarkFunc(const QString &filePath)
{
  getProgressSignal()->startProgress("Importing bookmarks ...");
  //  m_splitProject.loadBookmark(filePath);

    ZDvidReader reader;
  //  ZFlyEmCoordinateConverter converter;
    if (reader.open(getDvidTarget())) {
  //    ZDvidInfo info = reader.readGrayScaleInfo();
  //    converter.configure(info);
      getProgressSignal()->advanceProgress(0.1);
      notifyBookmarkDeleted();
      getCompleteDocument()->importFlyEmBookmark(filePath.toStdString());
      getProgressSignal()->advanceProgress(0.5);
  //    m_bookmarkArray.importJsonFile(filePath.toStdString(), NULL/*&converter*/);
    }

    notifyBookmarkUpdated();

    getProgressSignal()->advanceProgress(0.3);

    getProgressSignal()->endProgress();
  //  m_bookmarkArray.importJsonFile(filePath);

  //  emit bookmarkUpdated(&m_splitProject);
}

void ZFlyEmProofMvc::loadBookmark(const QString &filePath)
{
  QtConcurrent::run(this, &ZFlyEmProofMvc::loadBookmarkFunc, filePath);
}

void ZFlyEmProofMvc::loadSynapse()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Synapses", "", this);
  if (!fileName.isEmpty()) {
    getCompleteDocument()->loadSynapse(fileName.toStdString());
  }
}

void ZFlyEmProofMvc::loadBookmark()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Bookmarks", "", this);
  if (!fileName.isEmpty()) {
    loadBookmark(fileName);
  }
}

void ZFlyEmProofMvc::openSequencer()
{
  m_bodyInfoDlg->show();
  m_bodyInfoDlg->raise();
}


void ZFlyEmProofMvc::showSynapseAnnotation(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_PUNCTA, visible);
}

void ZFlyEmProofMvc::showBookmark(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_FLYEM_BOOKMARK, visible);
//  m_splitProject.setBookmarkVisible(visible);
//  m_mergeProject.setBookmarkVisible(visible);
}

void ZFlyEmProofMvc::showSegmentation(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_DVID_LABEL_SLICE, visible);
}

void ZFlyEmProofMvc::addSelectionAt(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    uint64_t bodyId = reader.readBodyIdAt(x, y, z);
    if (bodyId > 0) {
      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
        slice->recordSelection();
        slice->addSelection(
              slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
              NeuTube::BODY_LABEL_ORIGINAL);
        slice->processSelection();
      }
      updateBodySelection();
    }
  }
}

void ZFlyEmProofMvc::xorSelectionAt(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    uint64_t bodyId = reader.readBodyIdAt(x, y, z);
    if (bodyId > 0) {
      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
//        uint64_t finalBodyId = getMappedBodyId(bodyId);
        slice->recordSelection();
        slice->xorSelection(
              slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
              NeuTube::BODY_LABEL_MAPPED);
        slice->processSelection();
#if 0
        QList<uint64_t> labelList =
            getCompleteDocument()->getBodyMerger()->getOriginalLabelList(
              finalBodyId);
        slice->xorSelectionGroup(labelList.begin(), labelList.end());
        /*
        foreach (uint64_t label, labelList) {
          slice->xorSelection(label);
        }
        */
#endif
      }
      updateBodySelection();
    }
  }
}

void ZFlyEmProofMvc::deselectAllBody()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->deselectAll();
      updateBodySelection();
    }
  }
}

void ZFlyEmProofMvc::selectSeed()
{
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Label");
  dlg->getButton(ZButtonBox::ROLE_SKIP)->hide();
  dlg->setValue(1);
  if (dlg->exec()) {
    int label = dlg->getValue();
   int nSelected = m_splitProject.selectSeed(label);
   getView()->paintObject();
   emit messageGenerated(QString("%1 seed(s) are selected.").arg(nSelected));
  }
  delete dlg;
}

void ZFlyEmProofMvc::selectAllSeed()
{
  int nSelected = m_splitProject.selectAllSeed();
  getView()->paintObject();
  emit messageGenerated(QString("%1 seed(s) are selected.").arg(nSelected));
}

void ZFlyEmProofMvc::recoverSeed()
{
  if (ZDialogFactory::Ask("Recover Seed", "All current seeds might be lost. "
                          "Do you want to continue?", this)) {
    m_splitProject.recoverSeed();
  }
}

void ZFlyEmProofMvc::exportSeed()
{
  QString fileName = ZDialogFactory::GetSaveFileName("Export Seed", "", this);
  if (!fileName.isEmpty()) {
    m_splitProject.exportSeed(fileName);
  }
}

void ZFlyEmProofMvc::importSeed()
{
  if (ZDialogFactory::Ask("Import Seed",
                          "All current seeds might be lost after import. "
                          "Do you want to continue?", this)) {
    QString fileName = ZDialogFactory::GetOpenFileName("Import Seed", "", this);
    if (!fileName.isEmpty()) {
      m_splitProject.importSeed(fileName);
    }
  }
}

uint64_t ZFlyEmProofMvc::getMappedBodyId(uint64_t bodyId)
{
  return m_mergeProject.getMappedBodyId(bodyId);
}

std::set<uint64_t> ZFlyEmProofMvc::getCurrentSelectedBodyId(
    NeuTube::EBodyLabelType type) const
{
  return m_mergeProject.getSelection(type);
#if 0
  std::set<uint64_t> idSet;

  if (getCompletePresenter()->isHighlight()) {

//    idSet = m_mergeProject.getSelectedBodyId();
  } else {
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    if (labelSlice != NULL) {
      if (labelSlice->isVisible()) {
        idSet = labelSlice->getSelected(type);
      }
    }
  }

  return idSet;
#endif
}

void ZFlyEmProofMvc::locateBody(uint64_t bodyId)
{
  if (!getCompletePresenter()->isSplitWindow()) {
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      ZObject3dScan body = reader.readCoarseBody(bodyId);
      if (body.isEmpty()) {
        emit messageGenerated(
              ZWidgetMessage(QString("Cannot go to body: %1. No such body.").
                             arg(bodyId), NeuTube::MSG_ERROR));
      } else {
        ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

        ZObject3dScan objSlice = body.getMedianSlice();
        ZVoxel voxel = objSlice.getMarker();
//        ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
        ZIntPoint pt(voxel.x(), voxel.y(), voxel.z());
        pt -= dvidInfo.getStartBlockIndex();
        pt *= dvidInfo.getBlockSize();
        pt += ZIntPoint(dvidInfo.getBlockSize().getX() / 2,
                        dvidInfo.getBlockSize().getY() / 2, 0);
        pt += dvidInfo.getStartCoordinates();

        //    std::set<uint64_t> bodySet;
        //    bodySet.insert(bodyId);

        ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
        if (slice != NULL) {
          slice->clearSelection();
          slice->recordSelection();
          slice->addSelection(
                slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
                NeuTube::BODY_LABEL_MAPPED);
          slice->processSelection();
          processLabelSliceSelectionChange();
        }
        updateBodySelection();

        if (!objSlice.isEmpty()) {
          zoomTo(pt);
        } else {
          emit messageGenerated(ZWidgetMessage("Failed to zoom into the body",
                                               NeuTube::MSG_ERROR));
        }
      }
    }
  }
}

void ZFlyEmProofMvc::selectBody(uint64_t bodyId)
{
  ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
  if (slice != NULL) {
    slice->addSelection(bodyId, NeuTube::BODY_LABEL_MAPPED);
  }
  updateBodySelection();
}

void ZFlyEmProofMvc::processViewChangeCustom(const ZStackViewParam &viewParam)
{
  m_mergeProject.update3DBodyViewPlane(viewParam);
  m_splitProject.update3DViewPlane();

  updateBodyWindowPlane(m_coarseBodyWindow, viewParam);
  updateBodyWindowPlane(m_bodyWindow, viewParam);
  updateBodyWindowPlane(m_skeletonWindow, viewParam);
  updateBodyWindowPlane(m_externalNeuronWindow, viewParam);
}

void ZFlyEmProofMvc::recordCheckedBookmark(const QString &key, bool checking)
{
//  ZFlyEmBookmark *bookmark = m_bookmarkArray.findFirstBookmark(key);
  ZFlyEmBookmark *bookmark = getCompleteDocument()->findFirstBookmark(key);
  if (bookmark != NULL) {
    bookmark->setChecked(checking);
    recordBookmark(bookmark);
  }
}

void ZFlyEmProofMvc::recordBookmark(ZFlyEmBookmark *bookmark)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    writer.writeBookmark(*bookmark);
    if (writer.getStatusCode() != 200) {
      emit messageGenerated(ZWidgetMessage("Failed to record bookmark.",
                                           NeuTube::MSG_WARNING));
    }
  }
}

void ZFlyEmProofMvc::processCheckedUserBookmark(ZFlyEmBookmark */*bookmark*/)
{
  getCompleteDocument()->setCustomBookmarkSaveState(false);
}

void ZFlyEmProofMvc::enhanceTileContrast(bool state)
{
  getCompletePresenter()->setHighTileContrast(state);
  getCompleteDocument()->enhanceTileContrast(state);
  /*
  ZDvidTileEnsemble *tile = getCompleteDocument()->getDvidTileEnsemble();
  if (tile != NULL) {
    if (state) {
      tile->addVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    } else {
      tile->removeVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    }
    getCompleteDocument()->processObjectModified(tile->getTarget());
  }
  */
}

ZFlyEmSupervisor* ZFlyEmProofMvc::getSupervisor() const
{
  if (getDvidTarget().isSupervised()) {
    return m_supervisor;
  }

  return NULL;
}

void ZFlyEmProofMvc::annotateBookmark(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    ZFlyEmBookmarkAnnotationDialog dlg(this);
    dlg.setFrom(bookmark);
    if (dlg.exec()) {
      dlg.annotate(bookmark);
      getCompleteDocument()->processBookmarkAnnotationEvent(bookmark);

      updateUserBookmarkTable();
    }
  }
}

void ZFlyEmProofMvc::updateUserBookmarkTable()
{
  emit userBookmarkUpdated(getDocument().get());
}

void ZFlyEmProofMvc::changeColorMap(const QString &option)
{
  if (option == "Name") {
    getCompleteDocument()->useBodyNameMap(true);
  } else {
    getCompleteDocument()->useBodyNameMap(false);
  }
}

void ZFlyEmProofMvc::cropCoarseBody3D()
{
  if (m_coarseBodyWindow != NULL) {
    if (m_coarseBodyWindow->hasRectRoi()) {
      ZDvidReader reader;
      if (reader.open(getDvidTarget())) {
        if (getCompletePresenter()->isSplitOn()) {
          ZObject3dScan body = reader.readCoarseBody(m_splitProject.getBodyId());
          if (body.isEmpty()) {
            emit messageGenerated(
                  ZWidgetMessage(QString("Cannot crop body: %1. No such body.").
                                 arg(m_splitProject.getBodyId()),
                                 NeuTube::MSG_ERROR));
          } else {
            ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
            ZObject3dScan bodyInRoi;
            ZObject3dScan::ConstSegmentIterator iter(&body);
            while (iter.hasNext()) {
              const ZObject3dScan::Segment &seg = iter.next();
              for (int x = seg.getStart(); x <= seg.getEnd(); ++x) {
                ZIntPoint pt(0, seg.getY(), seg.getZ());
                pt.setX(x);
                pt -= dvidInfo.getStartBlockIndex();
                pt *= dvidInfo.getBlockSize();
                pt += ZIntPoint(dvidInfo.getBlockSize().getX() / 2,
                                dvidInfo.getBlockSize().getY() / 2, 0);
                pt += dvidInfo.getStartCoordinates();

                QPointF screenPos = m_coarseBodyWindow->getScreenProjection(
                      pt.getX(), pt.getY(), pt.getZ(), Z3DWindow::LAYER_SWC);
                if (m_coarseBodyWindow->getRectRoi().contains(screenPos.x(), screenPos.y())) {
                  bodyInRoi.addSegment(seg.getZ(), seg.getY(), x, x);
                }
              }
            }
#ifdef _DEBUG_2
            body.save(GET_TEST_DATA_DIR + "/test2.sobj");
            bodyInRoi.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif
            m_splitProject.commitCoarseSplit(bodyInRoi);
            ZFlyEmMisc::SubtractBodyWithBlock(
                  getCompleteDocument()->getBodyForSplit()->getObjectMask(),
                  bodyInRoi, dvidInfo);
            getCompleteDocument()->processObjectModified(
                  getCompleteDocument()->getBodyForSplit());
            getCompleteDocument()->notifyObjectModified();
          }
        } else {
          emit messageGenerated(
                ZWidgetMessage("Must enter split mode to enable crop.",
                               NeuTube::MSG_WARNING));
        }
      }
    }
  }
}

void ZFlyEmProofMvc::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  bool processed = false;
  if (urls.size() == 1) {
    const QUrl &url = urls[0];
    if (ZFileType::fileType(url.path().toStdString()) == ZFileType::JSON_FILE) {
      processed = true; //todo
    }
  }

  if (!processed) {
    foreach (const QUrl &url, urls) {
      if (ZFileType::fileType(url.path().toStdString()) ==
          ZFileType::SWC_FILE) {
        ZSwcTree *tree = new ZSwcTree;
        tree->load(url.path().toStdString());
        tree->setObjectClass(ZStackObjectSourceFactory::MakeFlyEmExtNeuronClass());
        tree->setHittable(false);
        tree->setColor(QColor(255, 0, 0));
        getDocument()->addObject(tree);
      }
    }
  }

#if 0
  //Filter out tiff files
  QList<QUrl> imageUrls;
  QList<QUrl> nonImageUrls;

  foreach (QUrl url, urls) {
    if (ZFileType::isImageFile(url.path().toStdString())) {
      imageUrls.append(url);
    } else {
      nonImageUrls.append(url);
    }
  }
  if (!nonImageUrls.isEmpty()) {
    getDocument()->loadFileList(nonImageUrls);
  }
#endif
}

//void ZFlyEmProofMvc::toggleEdgeMode(bool edgeOn)


