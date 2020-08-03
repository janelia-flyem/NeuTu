#include "zproofreadwindow.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QProgressDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QInputDialog>

#include "qfonticon.h"

#include "common/math.h"
#include "neutubeconfig.h"
#include "logging/zlog.h"
#include "logging/utilities.h"
#include "logging/zqslog.h"

#include "widgets/widgets_def.h"
#include "dvid/zdvidtarget.h"
#include "zwidgetfactory.h"
#include "zdialogfactory.h"
#include "zprogresssignal.h"
#include "zwidgetmessage.h"
#include "mvc/zstackpresenter.h"
#include "zactionlibrary.h"

#include "widgets/zflyembookmarkview.h"
#include "zflyemdataloader.h"
#include "flyemsplitcontrolform.h"
#include "zflyemproofmvc.h"
#include "flyemproofcontrolform.h"
#include "zflyemmessagewidget.h"
#include "zflyemproofdoc.h"
#include "zflyemproofpresenter.h"
#include "neuroglancer/zneuroglancerpathparser.h"
#include "flyem/auth/flyemauthtokendialog.h"
#include "flyem/auth/flyemauthtokenhandler.h"
#include "protocols/protocolassignmentdialog.h"

#include "dialogs/flyembodyfilterdialog.h"
#include "dialogs/dvidoperatedialog.h"
#include "dialogs/zstresstestoptiondialog.h"
#include "dialogs/zflyembodyscreenshotdialog.h"
#include "dialogs/zflyembodysplitdialog.h"
#include "dialogs/userfeedbackdialog.h"


ZProofreadWindow::ZProofreadWindow(QWidget *parent) :
  QMainWindow(parent)
{
  init();
}

ZProofreadWindow::~ZProofreadWindow()
{
  delete m_actionLibrary;
}

template <typename T>
void ZProofreadWindow::connectMessagePipe(T *source)
{
  connect(source, SIGNAL(messageGenerated(QString, bool)),
          this, SLOT(dump(QString,bool)));
  connect(source, SIGNAL(errorGenerated(QString, bool)),
          this, SLOT(dumpError(QString,bool)));
  connect(source, SIGNAL(messageGenerated(ZWidgetMessage)),
          this, SLOT(dump(ZWidgetMessage)));
}

void ZProofreadWindow::init()
{
  setFocusPolicy(Qt::ClickFocus);
  setAttribute(Qt::WA_DeleteOnClose);

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(1);
  widget->setLayout(layout);

//  ZDvidTarget target;
//  target.set("http://emdata1.int.janelia.org", "9db", 8500);

  m_mainMvc = ZFlyEmProofMvc::Make(neutu::EAxis::Z);
  m_mainMvc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  layout->addWidget(m_mainMvc);

  installEventFilter(m_mainMvc);

  QVBoxLayout *controlLayout = new QVBoxLayout;

  m_controlGroup = new QStackedWidget(this);
  controlLayout->addWidget(m_controlGroup);


  QHBoxLayout *titleLayout = new QHBoxLayout;
  titleLayout->addWidget(ZWidgetFactory::MakeHorizontalLine(this));
  QLabel *titleLabel = new QLabel("<font color=\"Green\">Message</font>", this);
  titleLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  titleLayout->addWidget(titleLabel);
  titleLayout->addWidget(ZWidgetFactory::MakeHorizontalLine(this));
  controlLayout->addLayout(titleLayout);

  m_messageWidget = new ZFlyEmMessageWidget(this);
  controlLayout->addWidget(m_messageWidget);


  layout->addLayout(controlLayout);

  m_controlGroup->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_controlForm = new FlyEmProofControlForm;
  m_controlForm->getUserBookmarkView()->setBookmarkModel(
        m_mainMvc->getUserBookmarkModel(flyem::EProofreadingMode::NORMAL));
  m_controlForm->getAssignedBookmarkView()->setBookmarkModel(
        m_mainMvc->getAssignedBookmarkModel(flyem::EProofreadingMode::NORMAL));
  m_mainMvc->registerBookmarkView(m_controlForm->getUserBookmarkView());
  m_mainMvc->registerBookmarkView(m_controlForm->getAssignedBookmarkView());
  m_controlForm->getAssignedBookmarkView()->enableDeletion(false);

  m_controlGroup->addWidget(m_controlForm);

  m_splitControlForm = new FlyEmSplitControlForm;
  m_splitControlForm->getUserBookmarkView()->setBookmarkModel(
        m_mainMvc->getUserBookmarkModel(flyem::EProofreadingMode::SPLIT));
  m_splitControlForm->getAssignedBookmarkView()->setBookmarkModel(
        m_mainMvc->getAssignedBookmarkModel(flyem::EProofreadingMode::SPLIT));
  m_mainMvc->registerBookmarkView(m_splitControlForm->getUserBookmarkView());
  m_mainMvc->registerBookmarkView(m_splitControlForm->getAssignedBookmarkView());
  m_splitControlForm->getAssignedBookmarkView()->enableDeletion(false);

  m_controlGroup->addWidget(m_splitControlForm);
  m_splitControlForm->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_mainMvc->connectControlPanel(m_controlForm);
  m_mainMvc->connectSplitControlPanel(m_splitControlForm);

  connect(m_controlForm, SIGNAL(splitTriggered(uint64_t)),
          this, SLOT(launchSplit(uint64_t)));
  connect(m_controlForm, SIGNAL(splitTriggered(uint64_t)),
          m_splitControlForm, SLOT(setSplit(uint64_t)));
  connect(m_controlForm, SIGNAL(splitTriggered()),
          this, SLOT(launchSplit()));
  connect(m_splitControlForm, SIGNAL(exitingSplit()),
          this, SLOT(exitSplit()));

  connectMessagePipe(m_mainMvc);
  connectMessagePipe(m_mainMvc->getDocument().get());

  connect(m_mainMvc, SIGNAL(splitBodyLoaded(uint64_t, neutu::EBodySplitMode)),
          this, SLOT(presentSplitInterface(uint64_t)));
  connect(m_mainMvc, SIGNAL(dvidTargetChanged(ZDvidTarget)),
          this, SLOT(updateDvidTargetWidget(ZDvidTarget)));
  connect(m_mainMvc, SIGNAL(exitingSplit()), this, SLOT(exitSplit()));

  connect(m_mainMvc, SIGNAL(locating2DViewTriggered(int, int, int, int)),
          this, SLOT(showAndRaise()));
  connect(m_mainMvc, SIGNAL(dvidReady()), this, SLOT(postDvidReady()));

  setCentralWidget(widget);

  m_progressDlg = NULL;
#if 0
  m_progressDlg = new QProgressDialog(this);
  m_progressDlg->setWindowModality(Qt::WindowModal);
  m_progressDlg->setAutoClose(true);
  m_progressDlg->setCancelButton(0);
#endif
//  m_progressDlg->setAutoClose(false);

  m_progressSignal = new ZProgressSignal(this);
  ZProgressSignal::ConnectProgress(m_mainMvc->getProgressSignal(),
        m_progressSignal);
//  m_progressSignal->connectProgress(m_mainMvc->getProgressSignal());
  m_progressSignal->connectSlot(this);

  m_actionLibrary = new ZActionLibrary(this);
  createMenu();
  createToolbar();
  statusBar()->showMessage("Load a database to start proofreading");

  connect(m_segSlider, SIGNAL(valueChanged(int)),
          m_mainMvc, SLOT(setLabelAlpha(int)));
  m_mainMvc->setLabelAlpha(m_segSlider->value());
  m_mainMvc->getCompletePresenter()->getSegmentationOpacity = [&]() {
    return m_segSlider->value();
  };

  m_mainMvc->enhanceTileContrast(m_contrastAction->isChecked());
  m_mainMvc->configure();

  createDialog();
  m_flyemDataLoader = new ZFlyEmDataLoader(this);

  m_defaultPal = palette(); //This has to be the last line to avoid crash

  if (GET_FLYEM_CONFIG.getWindowStyleSheet().empty()) {
    setStyleSheet(neutu::GROUP_BOX_STYLE);
  } else {
    setStyleSheet(GET_FLYEM_CONFIG.getWindowStyleSheet().c_str());
  }
}

ZProofreadWindow* ZProofreadWindow::Make(QWidget *parent)
{
  return new ZProofreadWindow(parent);
}

ZProofreadWindow* ZProofreadWindow::Make(QWidget *parent, ZDvidDialog *dvidDlg)
{
  ZProofreadWindow *window = new ZProofreadWindow(parent);
  if (dvidDlg != NULL) {
    window->setDvidDialog(dvidDlg);
  }

  return window;
}

void ZProofreadWindow::createDialog()
{
  m_dvidOpDlg = new DvidOperateDialog(this);
  m_dvidOpDlg->setDvidDialog(m_mainMvc->getDvidDialog());

  m_bodyFilterDlg = new FlyEmBodyFilterDialog(this);
  m_stressTestOptionDlg = new ZStressTestOptionDialog(this);
  m_bodyScreenshotDlg = new ZFlyEmBodyScreenshotDialog(this);
  m_bodySplitDlg = new ZFlyEmBodySplitDialog(this);

  if (!GET_FLYEM_CONFIG.getDefaultAssignmentManager().empty()) {
    m_authTokenDlg = new FlyEmAuthTokenDialog(this);
    connect(m_authTokenDlg, SIGNAL(requestUpdateAuthIcon()), this, SLOT(updateAuthTokenIcon()));
    m_protocolAssignmentDlg = new ProtocolAssignmentDialog(this);
  }
}

void ZProofreadWindow::setDvidDialog(ZDvidDialog *dvidDlg)
{
  m_mainMvc->setDvidDialog(dvidDlg);
}

ZFlyEmProofMvc* ZProofreadWindow::getMainMvc() const
{
  return m_mainMvc;
}

void ZProofreadWindow::stressTestSlot()
{
  if (m_stressTestOptionDlg->exec()) {
    m_mainMvc->stressTest(m_stressTestOptionDlg);
  }
}

void ZProofreadWindow::diagnose()
{
  m_mainMvc->diagnose();
}

void ZProofreadWindow::profile()
{
  m_mainMvc->profile();
}

void ZProofreadWindow::testSlot()
{
  m_mainMvc->testSlot();
}

void ZProofreadWindow::showSettings()
{
  m_mainMvc->showSetting();
}

QProgressDialog* ZProofreadWindow::getProgressDialog()
{
  if (m_progressDlg == NULL) {
    m_progressDlg = new QProgressDialog(this);
    m_progressDlg->setWindowModality(Qt::WindowModal);
    m_progressDlg->setAutoClose(true);
    m_progressDlg->setCancelButton(nullptr);
  }

  return m_progressDlg;
}

void ZProofreadWindow::stressTest()
{
  if (!m_mainMvc->getDvidTarget().isValid()) {
    m_mainMvc->setDvidTarget();
  }
  /*
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "3303", 8500);
  target.setBodyLabelName("bodies3");
  target.setLabelBlockName("labels3");
  target.setSynapseName("mb6_synapses");

  m_mainMvc->setDvidTarget(target);
  m_mainMvc->getPresenter()->setObjectVisible(false);
  */

  stressTestSlot();
}

void ZProofreadWindow::createMenu()
{
  QMenu *fileMenu = new QMenu("File", this);

  menuBar()->addMenu(fileMenu);

  m_loadDvidAction = new QAction("Import Database", this);
  connect(m_loadDvidAction, &QAction::triggered,
          this, &ZProofreadWindow::loadDatabase);
  fileMenu->addAction(m_loadDvidAction);

  m_loadDvidUrlAction = new QAction("Load Database", this);
  connect(m_loadDvidUrlAction, &QAction::triggered,
          this, &ZProofreadWindow::loadDatabaseFromUrl);
  fileMenu->addAction(m_loadDvidUrlAction);

  m_importBookmarkAction = new QAction("Import Bookmarks", this);
  m_importBookmarkAction->setIcon(QIcon(":/images/import_bookmark.png"));
  fileMenu->addAction(m_importBookmarkAction);
  connect(m_importBookmarkAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(loadBookmark()));

  QMenu *exportMenu = new QMenu("Export", this);
  fileMenu->addMenu(exportMenu);
  QAction *exportScreenshotAction = new QAction("Neuron Screenshot", this);
  connect(exportScreenshotAction, SIGNAL(triggered()),
          this, SLOT(exportNeuronScreenshot()));
  exportMenu->addAction(exportScreenshotAction);

  QAction *exportMeshScreenshotAction = new QAction("Neuron Mesh Screenshot", this);
  connect(exportMeshScreenshotAction, SIGNAL(triggered()),
          this, SLOT(exportNeuronMeshScreenshot()));
  exportMenu->addAction(exportMeshScreenshotAction);


  QAction *exportGrayscaleAction = new QAction("Grayscale", this);
  connect(exportGrayscaleAction, SIGNAL(triggered()),
          this, SLOT(exportGrayscale()));
  exportMenu->addAction(exportGrayscaleAction);

  QAction *exportBodyStackAction = new QAction("Bodies with Grayscale", this);
  connect(exportBodyStackAction, SIGNAL(triggered()), this, SLOT(exportBodyStack()));
  exportMenu->addAction(exportBodyStackAction);

  m_viewMenu = new QMenu("View", this);

  m_viewSynapseAction = new QAction("Synapses", this);
  m_viewSynapseAction->setIcon(QIcon(":/images/synapse.png"));
  m_viewSynapseAction->setCheckable(true);
  m_viewSynapseAction->setChecked(true);
  connect(m_viewSynapseAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showSynapseAnnotation(bool)));

  m_viewBookmarkAction = new QAction("Bookmarks", this);
  m_viewBookmarkAction->setIcon(QIcon(":/images/view_bookmark.png"));
  m_viewBookmarkAction->setCheckable(true);
  m_viewBookmarkAction->setChecked(true);
  connect(m_viewBookmarkAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showBookmark(bool)));

  m_viewRoiAction = new QAction("ROI", this);
  m_viewRoiAction->setCheckable(true);
  m_viewRoiAction->setChecked(true);
  m_viewRoiAction->setIcon(QIcon(":/images/view_roi.png"));
  connect(m_viewRoiAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showRoiMask(bool)));

  m_viewSegmentationAction = new QAction("Segmentation", this);
  m_viewSegmentationAction->setIcon(QIcon(":/images/view_segmentation.png"));
  m_viewSegmentationAction->setCheckable(true);
  m_viewSegmentationAction->setChecked(true);
  connect(m_viewSegmentationAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showSegmentation(bool)));

  m_viewTodoAction = new QAction("Todo", this);
  m_viewTodoAction->setIcon(QIcon(":/images/view_todo.png"));
  m_viewTodoAction->setCheckable(true);
  m_viewTodoAction->setChecked(true);
  connect(m_viewTodoAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showTodo(bool)));

  m_contrastAction = new QAction("Enhance Contrast", this);
  m_contrastAction->setCheckable(true);
  m_contrastAction->setChecked(false);
  m_contrastAction->setIcon(QIcon(":images/bc_enhance.png"));
  m_contrastAction->setChecked(true);
  connect(m_contrastAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(enhanceTileContrast(bool)));

  m_smoothAction = new QAction("Smooth Display", this);
  m_smoothAction->setCheckable(true);
  m_smoothAction->setChecked(false);
  m_smoothAction->setIcon(QIcon(":images/smooth.png"));
  m_smoothAction->setChecked(false);
  connect(m_smoothAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(smoothDisplay(bool)));

  m_openSkeletonAction = new QAction("3D Skeletons", this);
  connect(m_openSkeletonAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(showSkeletonWindow()));

  m_openObject3dAction = new QAction("3D Objects", this);
  connect(m_openObject3dAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(showObjectWindow()));

  m_openRoi3dAction = new QAction("3D ROI", this);
  connect(m_openRoi3dAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(showRoi3dWindow()));

//  m_queryTableAction = new QAction("Query Table", this);
//  connect(m_queryTableAction, SIGNAL(triggered()),
//          m_mainMvc, SLOT(showQueryTabel()));

  m_openExtNeuronWindowAction = new QAction("3D Reference Neurons", this);
  m_openExtNeuronWindowAction->setIcon(QIcon(":images/swcpreview.png"));
  connect(m_openExtNeuronWindowAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(showExternalNeuronWindow()));

  QMenu *viewControlMenu = new QMenu("Controls", this);
  QAction *synpasePropertyControlAction = new QAction("Synapses", this);
  connect(synpasePropertyControlAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(showSynapsePropertyDlg()));
  viewControlMenu->addAction(synpasePropertyControlAction);

  m_viewMenu->addAction(m_viewSynapseAction);
  m_viewMenu->addAction(m_viewBookmarkAction);
  m_viewMenu->addAction(m_viewSegmentationAction);
  m_viewMenu->addAction(m_viewTodoAction);
  m_viewMenu->addAction(m_viewRoiAction);
  m_viewMenu->addMenu(viewControlMenu);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_contrastAction);
  m_viewMenu->addAction(m_smoothAction);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_openObject3dAction);
  m_viewMenu->addAction(m_openExtNeuronWindowAction);

//  menu->addAction(new QAction("test", menu));

  menuBar()->addMenu(m_viewMenu);

  m_toolMenu = new QMenu("Tools", this);

  if (m_mainMvc->hasSequencer()) {
    m_openSequencerAction = new QAction("Open Sequencer", this);
    m_openSequencerAction->setIcon(QIcon(":/images/document.png"));
    connect(m_openSequencerAction, SIGNAL(triggered()),
            m_mainMvc, SLOT(openSequencer()));

    m_toolMenu->addAction(m_openSequencerAction);
  }

  m_neuprintAction = new QAction("Open NeuPrint", this);
  m_neuprintAction->setIcon(QIcon(":/images/neuprint.png"));
  connect(m_neuprintAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(openNeuPrint()));
//  m_neuprintAction->setVisible(false);

  // temporarily disable sequencer
  // m_toolMenu->addAction(m_openSequencerAction);

  m_openTodoAction = new QAction("Show Todo List", this);
  m_openTodoAction->setIcon(QIcon(":/images/todo.png"));
  connect(m_openTodoAction, SIGNAL(triggered()), m_mainMvc, SLOT(openTodo()));
  m_toolMenu->addAction(m_openTodoAction);

  m_roiToolAction = new QAction("ROI Tool", this);
  m_roiToolAction->setIcon(QIcon(":images/roi.png"));
  connect(m_roiToolAction, SIGNAL(triggered()), m_mainMvc, SLOT(openRoiTool()));
  m_toolMenu->addAction(m_roiToolAction);

  m_openProtocolsAction = new QAction("Open Protocols", this);
  m_openProtocolsAction->setIcon(QIcon(":/images/protocol.png"));
  connect(m_openProtocolsAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(openProtocol()));
  m_toolMenu->addAction(m_openProtocolsAction);

  m_openAuthDialogAction = new QAction("Open Auth Dialog", this);
  connect(m_openAuthDialogAction, SIGNAL(triggered()), this, SLOT(showAuthTokenDialog()));
  updateAuthTokenIcon();
  m_toolMenu->addAction(m_openAuthDialogAction);

  m_openProtocolAssignmentDialogAction = new QAction("Open Assignment Dialog", this);
  m_openProtocolAssignmentDialogAction->setIcon(QFontIcon::icon(0xf01c, Qt::darkGreen));
  connect(m_openProtocolAssignmentDialogAction, SIGNAL(triggered()),
          this, SLOT(showProtocolAssignmentDialog()));
  m_toolMenu->addAction(m_openProtocolAssignmentDialogAction);

  m_tuneContrastAction = new QAction("Tune Contrast", this);
  connect(m_tuneContrastAction, &QAction::triggered,
          m_mainMvc, &ZFlyEmProofMvc::tuneGrayscaleContrast);
  m_toolMenu->addAction(m_tuneContrastAction);

  m_bodyExplorerAction = new QAction("Explore Bodies", this);
  m_bodyExplorerAction->setIcon(QIcon(":/images/open_dvid.png"));
  connect(m_bodyExplorerAction, SIGNAL(triggered()),
          this, SLOT(exploreBody()));
//  m_toolMenu->addAction(m_bodyExplorerAction);

  QMenu *dvidMenu = new QMenu("DVID", this);
  m_dvidOperateAction = new QAction("Operate", this);
  connect(m_dvidOperateAction, SIGNAL(triggered()),
          this, SLOT(operateDvid()));

  dvidMenu->addAction(m_dvidOperateAction);

  m_toolMenu->addMenu(dvidMenu);

  QAction *recorderAction = new QAction("Recorder", this);
  connect(recorderAction, &QAction::triggered,
          m_mainMvc, &ZFlyEmProofMvc::configureRecorder);
  m_toolMenu->addAction(recorderAction);

  QAction *feedbackAction = m_actionLibrary->getAction(
        ZActionFactory::ACTION_USER_FEEDBACK, this, SLOT(processFeedback()));
  m_toolMenu->addAction(feedbackAction);

  feedbackAction->setVisible(neutu::HasEnv("NEUTU_USER_FEEDBACK", "yes"));

  menuBar()->addMenu(m_toolMenu);

  m_advancedMenu = new QMenu("Advanced", this);
  menuBar()->addMenu(m_advancedMenu);
  QAction *mainWindowAction = new QAction("Show NeuTu Desktop", this);
  connect(mainWindowAction, SIGNAL(triggered()),
          this, SIGNAL(showingMainWindow()));
  m_advancedMenu->addAction(mainWindowAction);

  QAction *stressTestAction = new QAction("Stress Test", this);
  connect(stressTestAction, SIGNAL(triggered()), this, SLOT(stressTestSlot()));
  m_advancedMenu->addAction(stressTestAction);

  QAction *diagnoseAction = new QAction("Diagnose", this);
  connect(diagnoseAction, SIGNAL(triggered()), this, SLOT(diagnose()));
  m_advancedMenu->addAction(diagnoseAction);

  QAction *settingAction = new QAction(" Settings", this);
  connect(settingAction, SIGNAL(triggered()), this, SLOT(showSettings()));
  m_advancedMenu->addAction(settingAction);

//  QAction *profileAction = new QAction("Profile", this);
  QAction *profileAction = m_actionLibrary->getAction(
        ZActionFactory::ACTION_PROFILE, this, SLOT(profile()));
//  profileAction->setEnabled(neutu::HasEnv("NEUTU_PROFILE", "yes"));
//  connect(profileAction, SIGNAL(triggered()), this, SLOT(profile()));
  if (neutu::HasEnv("NEUTU_PROFILE", "yes")) {
    m_advancedMenu->addAction(profileAction);
  }

  QAction *testAction = new QAction("Test", this);
  connect(testAction, SIGNAL(triggered()), this, SLOT(testSlot()));
  m_advancedMenu->addAction(testAction);


//  m_viewMenu->setEnabled(false);

  enableTargetAction(false);
}

void ZProofreadWindow::enableTargetAction(bool on)
{
  m_viewSynapseAction->setEnabled(on);
  m_importBookmarkAction->setEnabled(on);
  m_viewBookmarkAction->setEnabled(on);
  m_viewSegmentationAction->setEnabled(on);
  m_viewRoiAction->setEnabled(on);
  m_viewTodoAction->setEnabled(on);
  if (m_openSequencerAction) {
    m_openSequencerAction->setEnabled(on);
  }
  if (m_neuprintAction) {
    m_neuprintAction->setEnabled(on);
  }
  m_roiToolAction->setEnabled(on);
  m_contrastAction->setEnabled(on);
  m_smoothAction->setEnabled(on);
  m_openTodoAction->setEnabled(on);
  m_openProtocolsAction->setEnabled(on);
  m_tuneContrastAction->setEnabled(on);
  m_loadDvidAction->setEnabled(!on);
  m_loadDvidUrlAction->setEnabled(!on);
  m_openAuthDialogAction->setEnabled(
        on && !GET_FLYEM_CONFIG.getDefaultAuthenticationServer().empty());
  m_openProtocolAssignmentDialogAction->setEnabled(
        on && !GET_FLYEM_CONFIG.getDefaultAssignmentManager().empty());
  m_actionLibrary->getAction(ZActionFactory::ACTION_PROFILE)->setEnabled(!on);
}

void ZProofreadWindow::addSynapseActionToToolbar()
{
  m_verticalToolbar = new QToolBar(this);
  m_verticalToolbar->setIconSize(QSize(24, 24));
  m_verticalToolbar->addSeparator();
  m_verticalToolbar->addWidget(new QLabel("Synapse"));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_ADD_PRE));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_ADD_POST));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_DELETE));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_MOVE));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_LINK));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_UNLINK));
  m_verticalToolbar->addAction(
        m_mainMvc->getCompletePresenter()->getAction(
          ZActionFactory::ACTION_SYNAPSE_HLPSD));

  m_verticalToolbar->addSeparator();
  m_verticalToolbar->addAction(m_mainMvc->getCompletePresenter()->getAction(
                                ZActionFactory::ACTION_ENTER_RECT_ROI_MODE));

  addToolBar(Qt::LeftToolBarArea, m_verticalToolbar);
}

void ZProofreadWindow::createToolbar()
{
  m_mainToolBar = new QToolBar(this);
  m_mainToolBar->setObjectName(QString::fromUtf8("toolBar"));
  m_mainToolBar->setIconSize(QSize(24, 24));
  addToolBar(Qt::TopToolBarArea, m_mainToolBar);

  m_mainToolBar->addAction(m_importBookmarkAction);

  m_mainToolBar->addSeparator();
  m_mainToolBar->addAction(m_viewSynapseAction);
  m_mainToolBar->addAction(m_viewBookmarkAction);
  m_mainToolBar->addAction(m_viewTodoAction);
  m_mainToolBar->addAction(m_viewRoiAction);

  m_mainToolBar->addSeparator();
  m_mainToolBar->addAction(m_viewSegmentationAction);
  QAction *svAction = m_mainMvc->getCompletePresenter()->getAction(
        ZActionFactory::ACTION_TOGGLE_SUPERVOXEL_VIEW);
  m_mainToolBar->addAction(svAction);
  svAction->setVisible(false);

  m_segSlider = new QSlider(Qt::Horizontal, this);
  m_segSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  m_segSlider->setRange(0, 255);
  m_segSlider->setValue(77);
  m_mainToolBar->addWidget(m_segSlider);

  m_mainToolBar->addSeparator();
  m_mainToolBar->addAction(m_contrastAction);
  m_mainToolBar->addAction(m_smoothAction);
  m_mainToolBar->addSeparator();

  QActionGroup *viewAxisGroup = new QActionGroup(this);
  auto addViewAction = [&](neutu::EAxis axis) {
    QAction *action = nullptr;
    switch (axis) {
    case neutu::EAxis::X:
      action = m_mainMvc->getCompletePresenter()->getAction(
                ZActionFactory::ACTION_VIEW_AXIS_X);
      break;
    case neutu::EAxis::Y:
      action = m_mainMvc->getCompletePresenter()->getAction(
                ZActionFactory::ACTION_VIEW_AXIS_Y);
      break;
    case neutu::EAxis::Z:
      action = m_mainMvc->getCompletePresenter()->getAction(
                ZActionFactory::ACTION_VIEW_AXIS_Z);
      break;
    case neutu::EAxis::ARB:
      action = m_mainMvc->getCompletePresenter()->getAction(
                ZActionFactory::ACTION_VIEW_AXIS_ARB);
      break;
    default:
      break;
    }
    if (action) {
      action->setChecked(m_mainMvc->getSliceAxis() == axis);
      viewAxisGroup->addAction(action);
      m_mainToolBar->addAction(action);
    }
  };
  addViewAction(neutu::EAxis::X);
  addViewAction(neutu::EAxis::Y);
  addViewAction(neutu::EAxis::Z);
  addViewAction(neutu::EAxis::ARB);

  m_mainToolBar->addSeparator();

  if (m_openSequencerAction != NULL) {
     m_mainToolBar->addAction(m_openSequencerAction);
  }

  m_mainToolBar->addAction(m_neuprintAction);

  m_mainToolBar->addAction(m_openTodoAction);
  m_mainToolBar->addAction(m_openProtocolsAction);
  m_mainToolBar->addAction(m_roiToolAction);
  m_mainToolBar->addAction(m_openAuthDialogAction);
  m_mainToolBar->addAction(m_openProtocolAssignmentDialogAction);

  m_mainToolBar->addAction(m_mainMvc->getCompletePresenter()->getAction(
        ZActionFactory::ACTION_VIEW_SCREENSHOT));

  m_mainToolBar->addAction(
        m_actionLibrary->getAction(ZActionFactory::ACTION_USER_FEEDBACK));

  addSynapseActionToToolbar();
}

void ZProofreadWindow::presentSplitInterface(uint64_t bodyId)
{
  m_controlGroup->setCurrentIndex(1);

  dump(ZWidgetMessage(
         QString("Body %1 loaded for split.").arg(bodyId),
         neutu::EMessageType::INFORMATION,
         ZWidgetMessage::TARGET_TEXT | ZWidgetMessage::TARGET_KAFKA));
}

void ZProofreadWindow::operateDvid()
{
  m_dvidOpDlg->show();
  m_dvidOpDlg->raise();
}

void ZProofreadWindow::showAuthTokenDialog() {
    m_authTokenDlg->show();
    m_authTokenDlg->raise();
}

void ZProofreadWindow::updateAuthTokenIcon() {
    FlyEmAuthTokenHandler handler;
    if (!handler.hasMasterToken()) {
        // medium dark red, halfway between Qt::red and Qt::darkRed
        m_openAuthDialogAction->setIcon(QFontIcon::icon(0xf023, QColor(192, 0, 0, 255)));
    } else {
        // medium dark yellow, halfway between Qt::yellow and Qt::darkYellow
        m_openAuthDialogAction->setIcon(QFontIcon::icon(0xf084, QColor(192, 192, 0, 255)));
    }
}

void ZProofreadWindow::showProtocolAssignmentDialog() {
    m_protocolAssignmentDlg->show();
    m_protocolAssignmentDlg->raise();
}

void ZProofreadWindow::launchSplit(uint64_t bodyId, neutu::EBodySplitMode mode)
{
  dump("Launching split ...", false);

  m_mainMvc->launchSplit(bodyId, mode);
}

void ZProofreadWindow::launchSplit()
{
//  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);

//  ->setValueLabel("Body ID");
  std::set<uint64_t> bodySet =
      m_mainMvc->getCompleteDocument()->getSelectedBodySet(
        neutu::ELabelSource::ORIGINAL);

  if (!bodySet.empty()) {
    m_bodySplitDlg->setBodyId(*(bodySet.begin()));
  }

  if (m_bodySplitDlg->exec()) {
/*    if (m_bodySplitDlg->isSkipped()) {
      m_mainMvc->notifySplitTriggered();
    } else {*/
      if (m_bodySplitDlg->getBodyId() > 0) {
        neutu::EBodySplitMode mode = neutu::EBodySplitMode::ONLINE;
        if (m_bodySplitDlg->isOfflineSplit()) {
          mode = neutu::EBodySplitMode::OFFLINE;
        }
        launchSplit(m_bodySplitDlg->getBodyId(), mode);
      }
//    }
  }
}

void ZProofreadWindow::exitSplit()
{
  m_mainMvc->exitSplit();
  m_controlGroup->setCurrentIndex(0);
  dump(ZWidgetMessage(
         "Back from body splitting mode.", neutu::EMessageType::INFORMATION,
         ZWidgetMessage::TARGET_TEXT | ZWidgetMessage::TARGET_KAFKA));
}

void ZProofreadWindow::dump(const QString &message, bool appending)
{
  ZWidgetMessage msg(message);
  if (!appending) {
    msg.setTarget(ZWidgetMessage::TARGET_TEXT | ZWidgetMessage::TARGET_KAFKA);
  }
  dump(msg);
//  dump(ZWidgetMessage(message, ))
//  m_messageWidget->dump(message, appending);
}

void ZProofreadWindow::dumpError(const QString &message, bool appending)
{
  ZWidgetMessage msg(message, neutu::EMessageType::ERROR);
  if (!appending) {
    msg.setTarget(ZWidgetMessage::TARGET_TEXT | ZWidgetMessage::TARGET_KAFKA);
  }
  dump(msg);
//  m_messageWidget->dumpError(message, appending);
}

void ZProofreadWindow::dump(const ZWidgetMessage &msg)
{
  neutu::LogMessage(msg);

  if (msg.hasTarget(ZWidgetMessage::TARGET_TEXT)) {
    if (msg.getType() == neutu::EMessageType::ERROR) {
      m_messageWidget->dumpError(msg.toHtmlString(), msg.isAppending());
    } else {
      m_messageWidget->dump(msg.toHtmlString(), msg.isAppending());
    }
  }

  if (msg.hasTarget(ZWidgetMessage::TARGET_DIALOG)) {
    ZDialogFactory::PromptMessage(msg, this);
  }

  if (msg.hasTarget(ZWidgetMessage::TARGET_STATUS_BAR)) {
    statusBar()->showMessage(msg.toHtmlString());
  }

  if (msg.hasTarget(ZWidgetMessage::TARGET_CUSTOM_AREA)) {
    m_mainMvc->dump(msg.toHtmlString());
  }

#if 0
  switch (msg.getTarget()) {
  case ZWidgetMessage::TARGET_TEXT:
  case ZWidgetMessage::TARGET_TEXT_APPENDING:
    dump(msg.toHtmlString(), msg.isAppending(), true);
    break;
  case ZWidgetMessage::TARGET_DIALOG:
    QMessageBox::information(this, "Notice", msg.toHtmlString());
    break;
  case ZWidgetMessage::TARGET_STATUS_BAR:
    statusBar()->showMessage(msg.toHtmlString());
    break;
  case ZWidgetMessage::TARGET_CUSTOM_AREA:
    m_mainMvc->dump(msg.toHtmlString());
    break;
  default:
    break;
  }
#endif


//  logMessage(msg);
  //Record message in files
//  switch (msg.getType()) {
//  case neutube::EMessageType::INFORMATION:
//    LINFO() << msg.toPlainString();
//    break;
//  case neutube::EMessageType::WARNING:
//    LWARN() << msg.toPlainString();
//    break;
//  case neutube::EMessageType::ERROR:
//    LERROR() << msg.toPlainString();
//    break;
//  case neutube::EMessageType::DEBUG:
//    LDEBUG() << msg.toPlainString();
//    break;
//  }
}

void ZProofreadWindow::closeEvent(QCloseEvent */*event*/)
{
  emit proofreadWindowClosed();
}


void ZProofreadWindow::advanceProgress(double dp)
{
  if (getProgressDialog()->isVisible()) {
    if (getProgressDialog()->value() < getProgressDialog()->maximum()) {
      int range = getProgressDialog()->maximum() - getProgressDialog()->minimum();
      getProgressDialog()->setValue(
            getProgressDialog()->value() + neutu::iround(dp * range));
    }
  }
}

void ZProofreadWindow::startProgress()
{
  getProgressDialog()->show();
}

void ZProofreadWindow::startProgress(const QString &title, int nticks)
{
  initProgress(nticks);
  getProgressDialog()->setLabelText(title);
  getProgressDialog()->show();
}

void ZProofreadWindow::startProgress(const QString &title)
{
  startProgress(title, 100);
}

void ZProofreadWindow::endProgress()
{
  getProgressDialog()->reset();
  getProgressDialog()->close();
}

void ZProofreadWindow::initProgress(int nticks)
{
  getProgressDialog()->setRange(0, nticks);
}

void ZProofreadWindow::updateDvidTargetWidget(const ZDvidTarget &target)
{
  setWindowTitle(
        (target.getName() + " @ " + target.getSourceString(false, 5)).c_str());

  enableTargetAction(target.isValid());
  if (target.hasSupervoxel()) {
    m_mainMvc->getCompletePresenter()->getAction(
            ZActionFactory::ACTION_TOGGLE_SUPERVOXEL_VIEW)->setVisible(true);
  }

  m_viewMenu->setEnabled(true);

  if (target.readOnly()) {
    m_roiToolAction->setVisible(false);
    m_openProtocolsAction->setVisible(false);
    m_openTodoAction->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_ADD_PRE)->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_ADD_POST)->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_DELETE)->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_MOVE)->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_LINK)->setVisible(false);
    m_mainMvc->getCompletePresenter()->getAction(
      ZActionFactory::ACTION_SYNAPSE_UNLINK)->setVisible(false);
    setWindowIcon(QFontIcon::icon(0xf023, QColor(205, 127, 50, 128)));
  }

//  m_neuprintAction->setVisible(m_mainMvc->hasNeuPrint());

//  m_toolBar->hide();
//  m_toolBar->show();
//  addToolBar(Qt::TopToolBarArea, m_toolBar);
//  m_toolBar->show();
}

void ZProofreadWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void ZProofreadWindow::logMessage(const QString &msg)
{
  KLog() << ZLog::Info() << ZLog::Description(msg.toStdString());
//  LINFO() << msg;
}

void ZProofreadWindow::logError(const QString &msg)
{
  KLog() << ZLog::Error() << ZLog::Description(msg.toStdString());
//  LINFO() << msg;
}

void ZProofreadWindow::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::ActivationChange) {
    displayActiveHint(isActiveWindow());
  }
}

void ZProofreadWindow::keyPressEvent(QKeyEvent *event)
{
  event->ignore();
}

void ZProofreadWindow::exploreBody()
{
  bool continueLoading = false;
  ZDvidTarget target;
  if (m_mainMvc->getDvidDialog()->exec()) {
    target = m_mainMvc->getDvidDialog()->getDvidTarget();
    if (!target.isValid()) {
      ZDialogFactory::Warn("Invalid DVID", "Invalid DVID server.", this);
    } else {
      continueLoading = true;
    }
  }

  if (continueLoading && m_bodyFilterDlg->exec()) {
    ZDvidFilter dvidFilter = m_bodyFilterDlg->getDvidFilter();
    dvidFilter.setDvidTarget(target);
    m_flyemDataLoader->loadDataBundle(dvidFilter);
  }
}

void ZProofreadWindow::exportGrayscale()
{
  m_mainMvc->exportGrayscale();
}

void ZProofreadWindow::exportBodyStack()
{
  m_mainMvc->exportBodyStack();
}

void ZProofreadWindow::exportNeuronScreenshot()
{
  if (m_bodyScreenshotDlg->exec()) {
    std::vector<uint64_t> bodyIdArray = m_bodyScreenshotDlg->getBodyIdArray();
    /*
    bodyIdArray.push_back(95963649);
    bodyIdArray.push_back(131229029);
    bodyIdArray.push_back(134974661);
    */
    m_mainMvc->exportNeuronScreenshot(
          bodyIdArray, m_bodyScreenshotDlg->getFrameWidth(),
          m_bodyScreenshotDlg->getFrameHeight(),
          m_bodyScreenshotDlg->getOutputPath());
  }
}

void ZProofreadWindow::exportNeuronMeshScreenshot()
{
  if (m_bodyScreenshotDlg->exec()) {
    std::vector<uint64_t> bodyIdArray = m_bodyScreenshotDlg->getBodyIdArray();
    m_mainMvc->exportNeuronMeshScreenshot(
          bodyIdArray, m_bodyScreenshotDlg->getFrameWidth(),
          m_bodyScreenshotDlg->getFrameHeight(),
          m_bodyScreenshotDlg->getOutputPath());
  }
}

void ZProofreadWindow::displayActiveHint(bool on)
{
//  setAutoFillBackground(on);
#if 1
  if (on) {
    setPalette(m_defaultPal);
  } else {
    QPalette pal(m_defaultPal);
//    QColor color = pal.background().color();
//    color.setAlpha(200);
    pal.setColor(QPalette::Background, QColor(200, 164, 164, 200));
    setPalette(pal);
  }
#if 0
  (palette());

  // set black background
  Pal.setColor(QPalette::Background, Qt::black);
  m_pMyWidget->setAutoFillBackground(true);
  m_pMyWidget->setPalette(Pal);

  if (on) {
//    setWindowOpacity(1.0);
    setStyleSheet("background-color:black;");
  } else {
//    setWindowOpacity(0.8);
    setStyleSheet("background-color:blue;");
  }

  setPalette(pal);
#endif
#endif
}

void ZProofreadWindow::showAndRaise()
{
  if (windowState() & Qt::WindowMinimized) {
    showNormal();
  } else {
    show();
  }
  activateWindow();
  raise();
}

void ZProofreadWindow::loadDatabaseFromName(const QString &name)
{
  if (!name.isEmpty()) {
    getMainMvc()->setDvidFromName(name.toStdString());
  }
}

void ZProofreadWindow::loadDatabase()
{
  QString filename = ZDialogFactory::GetOpenFileName(
        "DVID Settings", "", this);
  if (!filename.isEmpty()) {
    m_mainMvc->setDvidFromJson(filename.toStdString());
  }
}

void ZProofreadWindow::loadDatabaseFromUrl()
{
  QString text = QInputDialog::getMultiLineText(
        this, "Load Database", "URL/JSON").trimmed();

  if (text.startsWith("{")) {
    m_mainMvc->setDvidFromJsonObject(text.toStdString());
  } else if (!text.isEmpty()) {
    m_mainMvc->setDvidFromUrl(text);
  }

  /*
  static QInputDialog *dlg = new QInputDialog(this);

  dlg->setOption(QInputDialog::UsePlainTextEditForTextInput);
  dlg->text
  if (dlg->exec()) {
    QString url = dlg->textValue();
    if (!url.isEmpty()) {
      m_mainMvc->setDvidFromUrl(url);
    }
  }
  */

}

void ZProofreadWindow::postDvidReady()
{
  getMainMvc()->updateRoiWidget();
}

void ZProofreadWindow::processFeedback()
{
  UserFeedbackDialog dlg;
  if (dlg.exec()) {
    dlg.send([this](const QString &msg) {
      this->dump(
            ZWidgetMessage(msg, neutu::EMessageType::INFORMATION,
                           ZWidgetMessage::TARGET_TEXT_APPENDING));
    });
  }
}


