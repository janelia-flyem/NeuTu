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

#include "flyemsplitcontrolform.h"
#include "dvid/zdvidtarget.h"
#include "zflyemproofmvc.h"
#include "flyemproofcontrolform.h"
#include "flyem/zflyemmessagewidget.h"
#include "zwidgetfactory.h"
#include "zdialogfactory.h"
#include "tz_math.h"
#include "zprogresssignal.h"
#include "zwidgetmessage.h"

ZProofreadWindow::ZProofreadWindow(QWidget *parent) :
  QMainWindow(parent)
{
  init();
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
  setAttribute(Qt::WA_DeleteOnClose);

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  ZDvidTarget target;
//  target.set("http://emdata1.int.janelia.org", "9db", 8500);

  m_mainMvc = ZFlyEmProofMvc::Make(target);
  m_mainMvc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  layout->addWidget(m_mainMvc);

  QVBoxLayout *controlLayout = new QVBoxLayout(this);

  m_controlGroup = new QStackedWidget(this);
  controlLayout->addWidget(m_controlGroup);


  QHBoxLayout *titleLayout = new QHBoxLayout(this);
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

  FlyEmProofControlForm *controlForm = new FlyEmProofControlForm;
  m_controlGroup->addWidget(controlForm);

  FlyEmSplitControlForm *splitControlForm = new FlyEmSplitControlForm;
  m_controlGroup->addWidget(splitControlForm);
  splitControlForm->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_mainMvc->connectControlPanel(controlForm);
  m_mainMvc->connectSplitControlPanel(splitControlForm);

  connect(controlForm, SIGNAL(splitTriggered(uint64_t)),
          this, SLOT(launchSplit(uint64_t)));
  connect(controlForm, SIGNAL(splitTriggered(uint64_t)),
          splitControlForm, SLOT(setSplit(uint64_t)));
  connect(controlForm, SIGNAL(splitTriggered()),
          this, SLOT(launchSplit()));
  connect(splitControlForm, SIGNAL(exitingSplit()),
          this, SLOT(exitSplit()));

  connectMessagePipe(m_mainMvc);

  connect(m_mainMvc, SIGNAL(splitBodyLoaded(uint64_t)),
          this, SLOT(presentSplitInterface(uint64_t)));
  connect(m_mainMvc, SIGNAL(dvidTargetChanged(ZDvidTarget)),
          this, SLOT(updateDvidTargetWidget(ZDvidTarget)));

  setCentralWidget(widget);

  m_progressDlg = new QProgressDialog(this);
  m_progressDlg->setWindowModality(Qt::WindowModal);
  m_progressDlg->setAutoClose(true);
  m_progressDlg->setCancelButton(0);
  m_progressDlg->setAutoClose(false);

  m_progressSignal = new ZProgressSignal(this);
  ZProgressSignal::ConnectProgress(m_mainMvc->getProgressSignal(),
        m_progressSignal);
//  m_progressSignal->connectProgress(m_mainMvc->getProgressSignal());
  m_progressSignal->connectSlot(this);

  createMenu();
  createToolbar();
  statusBar()->showMessage("Load a database to start proofreading");

  m_mainMvc->enhanceTileContrast(m_contrastAction->isChecked());
}

ZProofreadWindow* ZProofreadWindow::Make(QWidget *parent)
{
  return new ZProofreadWindow(parent);
}

void ZProofreadWindow::createMenu()
{
  QMenu *fileMenu = new QMenu("File", this);

  menuBar()->addMenu(fileMenu);

  m_importBookmarkAction = new QAction("Import Bookmarks", this);
  m_importBookmarkAction->setIcon(QIcon(":/images/import_bookmark.png"));
  fileMenu->addAction(m_importBookmarkAction);
  connect(m_importBookmarkAction, SIGNAL(triggered()),
          m_mainMvc, SLOT(loadBookmark()));

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

  m_viewSegmentationAction = new QAction("Segmentation", this);
  m_viewSegmentationAction->setIcon(QIcon(":/images/view_segmentation.png"));
  m_viewSegmentationAction->setCheckable(true);
  m_viewSegmentationAction->setChecked(true);
  connect(m_viewSegmentationAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(showSegmentation(bool)));

  m_contrastAction = new QAction("Enhance Contrast", this);
  m_contrastAction->setCheckable(true);
  m_contrastAction->setChecked(false);
  m_contrastAction->setIcon(QIcon(":images/bc_enhance.png"));
  m_contrastAction->setChecked(true);
  connect(m_contrastAction, SIGNAL(toggled(bool)),
          m_mainMvc, SLOT(enhanceTileContrast(bool)));

  m_viewMenu->addAction(m_viewSynapseAction);
  m_viewMenu->addAction(m_viewBookmarkAction);
  m_viewMenu->addAction(m_viewSegmentationAction);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_contrastAction);


//  menu->addAction(new QAction("test", menu));

  menuBar()->addMenu(m_viewMenu);

//  m_viewMenu->setEnabled(false);

  m_viewSynapseAction->setEnabled(false);
  m_importBookmarkAction->setEnabled(false);
  m_viewBookmarkAction->setEnabled(false);
  m_viewSegmentationAction->setEnabled(false);
}

void ZProofreadWindow::createToolbar()
{
  m_toolBar = new QToolBar(this);
  m_toolBar->setObjectName(QString::fromUtf8("toolBar"));
  m_toolBar->setIconSize(QSize(24, 24));
  addToolBar(Qt::TopToolBarArea, m_toolBar);

  m_toolBar->addAction(m_importBookmarkAction);

  m_toolBar->addSeparator();
  m_toolBar->addAction(m_viewSynapseAction);
  m_toolBar->addAction(m_viewBookmarkAction);
  m_toolBar->addAction(m_viewSegmentationAction);
  m_toolBar->addSeparator();
  m_toolBar->addAction(m_contrastAction);
}

void ZProofreadWindow::presentSplitInterface(uint64_t bodyId)
{
  m_controlGroup->setCurrentIndex(1);
  dump(QString("Body %1 loaded for split.").arg(bodyId), false);
}

void ZProofreadWindow::launchSplit(uint64_t bodyId)
{
//  emit progressStarted("Launching split ...");
  dump("Launching split ...", false);
//  m_progressSignal->advanceProgress(0.1);
//  advanceProgress(0.1);
  m_mainMvc->launchSplit(bodyId);

  /*
  if (m_mainMvc->launchSplit(bodyId)) {
    m_controlGroup->setCurrentIndex(1);
    dump(QString("Body %1 loaded for split.").arg(bodyId), true);
  } else {
    dumpError(QString("Failed to load %1").arg(bodyId), true);
  }
  */
}

void ZProofreadWindow::launchSplit()
{
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Body ID");
  if (dlg->exec()) {
    if (dlg->isSkipped()) {
      m_mainMvc->notifySplitTriggered();
    } else {
      if (dlg->getValue() > 0) {
        launchSplit(dlg->getValue());
      }
    }
  }
}

void ZProofreadWindow::exitSplit()
{
  m_mainMvc->exitSplit();
  m_controlGroup->setCurrentIndex(0);
  dump("Back from splitting mode.", false);
}

void ZProofreadWindow::dump(const QString &message, bool appending)
{
//  qDebug() << message;
  m_messageWidget->dump(message, appending);
}

void ZProofreadWindow::dumpError(const QString &message, bool appending)
{
  m_messageWidget->dumpError(message, appending);
}

void ZProofreadWindow::dump(const ZWidgetMessage &msg)
{
  switch (msg.getTarget()) {
  case ZWidgetMessage::TARGET_TEXT:
  case ZWidgetMessage::TARGET_TEXT_APPENDING:
    dump(msg.toHtmlString(), msg.isAppending());
    break;
  case ZWidgetMessage::TARGET_DIALOG:
    QMessageBox::information(this, "Notice", msg.toHtmlString());
    break;
case ZWidgetMessage::TARGET_STATUS_BAR:
    statusBar()->showMessage(msg.toHtmlString());
    break;
  }
}


void ZProofreadWindow::advanceProgress(double dp)
{
  if (getProgressDialog()->isVisible()) {
    if (getProgressDialog()->value() < getProgressDialog()->maximum()) {
      int range = getProgressDialog()->maximum() - getProgressDialog()->minimum();
      getProgressDialog()->setValue(getProgressDialog()->value() + iround(dp * range));
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
  setWindowTitle(target.getSourceString(false).c_str());

  m_viewSynapseAction->setEnabled(target.isValid());
  m_importBookmarkAction->setEnabled(target.isValid());
  m_viewBookmarkAction->setEnabled(target.isValid());
  m_viewSegmentationAction->setEnabled(target.isValid());

  m_viewMenu->setEnabled(true);
}
