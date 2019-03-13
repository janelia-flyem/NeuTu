#include "zstackframe.h"
#include <QUndoCommand>
#include <iostream>
#include <QTimer>
#include <QtConcurrentRun>
#include <QProgressDialog>

#include "tz_error.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zlocsegchain.h"
//#include "tz_xml_utils.h"
#include "tz_string.h"
#include "ztraceproject.h"
#include "zstack.hxx"
#include "zcurve.h"
#include "z3dwindow.h"
#include "zstackfile.h"
#include "zdoublevector.h"
#include "zfiletype.h"
#include "zobjsmanagerwidget.h"
#include "neutubeconfig.h"
#include "zstackviewlocator.h"
#include "zstackstatistics.h"
#include "tz_stack_bwmorph.h"
#include "zobject3dscan.h"
#include "tz_stack_math.h"
#include "z3dcompositor.h"
#include "zstring.h"
#include "biocytin/zbiocytinfilenameparser.h"
#include "zpunctumio.h"
#include "ztilegraphicsitem.h"
#include "ztileinfo.h"
#include "mainwindow.h"
#include "z3dcanvas.h"
#include "zwindowfactory.h"
#include "zobject3dscanarray.h"
#include "zsparseobject.h"
#include "zmessage.h"
#include "zmessagemanager.h"
#include "zobject3dfactory.h"
#include "zmeshfactory.h"
#include "z3dmeshfilter.h"
#include "qt/core/qthelper.h"
#include "zneurontracer.h"

#include "widgets/zimagewidget.h"

#include "zdialogfactory.h"
#include "dialogs/zstackframesettingdialog.h"
#include "dialogs/zautotracedialog.h"
#include "dialogs/flyemskeletonizationdialog.h"
#include "zstackskeletonizer.h"
#include "zwindowfactory.h"

#ifdef _DEBUG_2
#include "dvid/zdvidgrayslicescrollstrategy.h"
#include "dvid/zdvidgrayslice.h"
#endif

#include <QMdiArea>
#include <QMessageBox>
#include <QMimeData>

using namespace std;

ZStackFrame::ZStackFrame(QWidget *parent, Qt::WindowFlags flags) :
  QMdiSubWindow(parent, flags), m_parentFrame(NULL),
  m_tile(NULL), m_traceProject(NULL), m_isClosing(false),
  m_isWidgetReady(false), m_messageManager(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  setAcceptDrops(true);
  m_settingDlg = new ZStackFrameSettingDialog(this);
  m_settingDlg->setFromTracingConfig(ZNeuronTracerConfig::getInstance());
//  m_settingDlg->setTracingParameter();
//  m_manageObjsDlg = NULL;

  //m_presenter = new ZStackPresenter(this);
  //m_view = new ZStackView(this);
  qDebug() << m_doc.get();
//  m_presenter = NULL;
//  m_view = NULL;

  m_testTimer = new QTimer(this);
  connect(m_testTimer, SIGNAL(timeout()), this, SLOT(testSlot()));
  connect(this, &ZStackFrame::progressDone,
          this, &ZStackFrame::endProgress);
  /*
  if (preparingModel) {
    constructFrame();
  }
  */
}

#if 0
ZStackFrame::ZStackFrame(QWidget *parent, ZSharedPointer<ZStackDoc> doc) :
  QMdiSubWindow(parent), m_parentFrame(NULL),
  m_tile(NULL), m_traceProject(NULL), m_isClosing(false),
  m_3dWindow(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  setAcceptDrops(true);
  m_settingDlg = new SettingDialog(this);
  m_manageObjsDlg = NULL;

  m_presenter = NULL;
  m_view = NULL;
  constructFrame(doc);

#if defined(_QT5_) && defined(Q_OS_WIN)
  showMaximized();
  showNormal();
#endif
}
#endif

ZStackFrame::~ZStackFrame()
{
#ifdef _DEBUG_
  std::cout << "Frame " << this << " deconstructed" << std::endl;
#endif

  clear();
}

void ZStackFrame::BaseConstruct(
    ZStackFrame *frame, ZSharedPointer<ZStackDoc> doc)
{
  if (frame != NULL) {
    frame->constructFrame(doc);

  #if defined(_QT5_) && defined(Q_OS_WIN)
    frame->showMaximized();
    frame->showNormal();
  #endif
  }
}

void ZStackFrame::BaseConstruct(ZStackFrame *frame, ZStackDoc *doc)
{
  ZSharedPointer<ZStackDoc> docPtr(doc);
  BaseConstruct(frame, docPtr);
}

ZStackFrame*
ZStackFrame::Make(QMdiArea *parent, neutu::Document::ETag docTag)
{
  ZSharedPointer<ZStackDoc> doc =
      ZSharedPointer<ZStackDoc>(new ZStackDoc(NULL));
  doc->setTag(docTag);

  return Make(parent, doc);
}

ZStackFrame*
ZStackFrame::Make(QMdiArea *parent)
{
  return Make(parent, ZSharedPointer<ZStackDoc>(new ZStackDoc(NULL)));
}


ZStackFrame*
ZStackFrame::Make(QMdiArea *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZStackFrame *frame = new ZStackFrame(parent);

  BaseConstruct(frame, doc);

  if (parent != NULL) {
    frame->enableMessageManager();
  }

  return frame;
}

/*
ZMessageManager* ZStackFrame::getMessageManager()
{
  if (m_messageManager != NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }

  return m_messageManager;
}
*/
/*
void ZStackFrame::constructFrame()
{
  createView();
  createDocument();
  createPresenter();

  setView(m_view);
  m_view->prepareDocument();
  m_presenter->prepareView();

  customizeWidget();
}
*/
void ZStackFrame::constructFrame(ZSharedPointer<ZStackDoc> doc)
{
  dropDocument(ZSharedPointer<ZStackDoc>(doc));
  createView();
  createPresenter();
//  presenter()->createActions();

  updateDocument();

  if (document()->getTag() == neutu::Document::ETag::BIOCYTIN_PROJECTION) {
    m_presenter->setViewMode(ZInteractiveContext::VIEW_OBJECT_PROJECT);
  }
  setView(m_view);
  m_view->prepareDocument();
  m_presenter->prepareView();
//  m_presenter->createActions();

  if (doc.get() != NULL) {
    customizeWidget();
  }

#ifdef _DEBUG_2
  ZDvidGraySlice *slice = new ZDvidGraySlice;
  slice->setZoom(1);
  ZDvidGraySliceScrollStrategy *scrollStrategy =
      new ZDvidGraySliceScrollStrategy;
  scrollStrategy->setGraySlice(slice);
  view()->setScrollStrategy(scrollStrategy);
#endif
}


/*
void ZStackFrame::detach3DWindow()
{
  m_3dWindow = NULL;
}

void ZStackFrame::close3DWindow()
{
  if (m_3dWindow != NULL) {
    m_3dWindow->close();
//    delete m_3dWindow;
  }

  m_3dWindow = NULL;
}
*/

void ZStackFrame::createDocument()
{
  setDocument(ZSharedPointer<ZStackDoc>(new ZStackDoc(NULL)));
}

void ZStackFrame::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }

  view()->enableMessageManager();
}

void ZStackFrame::createPresenter()
{
  if (m_presenter == NULL) {
    m_presenter = ZStackPresenter::Make(this);
  }
}

void ZStackFrame::createView()
{
  if (m_view == NULL) {
    m_view = new ZStackView(this);
  }
}

void ZStackFrame::addDocData(ZStackDocReader &reader)
{
  if (m_doc.get() == NULL) {
    createDocument();
  }

  m_doc->addData(reader);
  updateTraceConfig();

  /*
  m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                              xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                       zResolution(), unit(),
                                       reconstructDistThre(),
                                       reconstructSpTest(),
                                       crossoverTest());
                                       */

  if (m_doc->hasStackData()) {
    if (m_doc->getStack()->kind() != GREY) {
      m_presenter->optimizeStackBc();
    }
    m_view->reset();
  }

  setWindowTitle(m_doc->stackSourcePath().c_str());
}

void ZStackFrame::autoTrace()
{
  int channelNumber = document()->getStack()->channelNumber();
  getAutoTraceDlg()->setChannelCount(channelNumber);
  if (getAutoTraceDlg()->exec()) {
//    setEnabled(false);
    startProgress("Tracing");
    QtConcurrent::run(this, &ZStackFrame::autoTraceFunc);
  }
}

void ZStackFrame::consumeDocument(ZStackDoc *doc)
{
  ZSharedPointer<ZStackDoc> docPtr(doc);
  setDocument(docPtr);
}

template <typename T>
void ZStackFrame::updateDocSignalSlot(T connectAction)
{
  connectAction(m_doc.get(), SIGNAL(locsegChainSelected(ZLocsegChain*)),
      this, SLOT(setLocsegChainInfo(ZLocsegChain*)), Qt::AutoConnection);

  if (m_doc->getTag() != neutu::Document::ETag::BIOCYTIN_PROJECTION) {
    connectAction(m_doc.get(), SIGNAL(zoomingToSelectedSwcNode()),
                  this, SLOT(zoomToSelectedSwcNodes()), Qt::AutoConnection);
  }

  connectAction(m_doc.get(), SIGNAL(stackLoaded()), this, SIGNAL(stackLoaded()),
                Qt::AutoConnection);

  connectAction(m_doc.get(), SIGNAL(stackRangeChanged()),
                m_view, SLOT(updateStackRange()), Qt::DirectConnection);
  connectAction(m_doc.get(), SIGNAL(stackModified(bool)),
                m_view, SLOT(processStackChange(bool)), Qt::AutoConnection);

  connectAction(m_doc.get(), SIGNAL(stackModified(bool)),
          m_presenter, SLOT(updateStackBc()), Qt::AutoConnection);
//  connectAction(m_doc.get(), SIGNAL(stackModified(bool)),
//          m_view, SLOT(redraw()));
//  connectAction(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));
//  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObject::ETarget)),
//          m_view, SLOT(paintObject(ZStackObject::ETarget)), Qt::AutoConnection);
//  connectAction(m_doc.get(), SIGNAL(objectModified()), m_view, SLOT(paintObject()));
//  connectAction(m_doc.get(), SIGNAL(objectModified(QSet<ZStackObject::ETarget>)),
//          m_view, SLOT(paintObject(QSet<ZStackObject::ETarget>)), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(cleanChanged(bool)),
          this, SLOT(changeWindowTitle(bool)), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(holdSegChanged()),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(),
                SIGNAL(chainSelectionChanged(QList<ZLocsegChain*>,
                                             QList<ZLocsegChain*>)),
                m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(
            swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,
                                        QList<Swc_Tree_Node*>)),
          this, SLOT(updateSwcExtensionHint()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(swcTreeNodeSelectionChanged(
                                QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(objectSelectionChanged(
                                QList<ZStackObject*>,QList<ZStackObject*>)),
          m_view, SLOT(paintObject(QList<ZStackObject*>,QList<ZStackObject*>)),
                Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(chainVisibleStateChanged(ZLocsegChain*,bool)),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(swcVisibleStateChanged(ZSwcTree*,bool)),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(punctumVisibleStateChanged()),
          m_view, SLOT(paintObject()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),
          this, SLOT(notifyUser(QString)), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(stackTargetModified()),
                m_view, SLOT(paintStack()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(thresholdChanged(int)),
                m_view, SLOT(setThreshold(int)), Qt::AutoConnection);
  connectAction(m_view, SIGNAL(viewChanged(ZStackViewParam)),
          this, SLOT(notifyViewChanged(ZStackViewParam)), Qt::AutoConnection);

  connectAction(m_view, SIGNAL(changingSetting()),
                this, SLOT(showSetting()), Qt::AutoConnection);

  connectAction(m_view, SIGNAL(closingChildFrame()),
                this, SLOT(closeAllChildFrame()), Qt::AutoConnection);
  connectAction(m_doc.get(), SIGNAL(objectModified(ZStackObjectInfoSet)),
                m_presenter, SLOT(processObjectModified(ZStackObjectInfoSet)),
                Qt::AutoConnection);
  connectAction(m_view, SIGNAL(autoTracing()), this, SLOT(autoTrace()),
                Qt::AutoConnection);
}

template <typename T>
void ZStackFrame::updateSignalSlot(T connectAction)
{
  updateDocSignalSlot(connectAction);
  connectAction(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()),
                Qt::AutoConnection);
//  connectAction(this, SIGNAL(closed(ZStackFrame*)), this, SLOT(closeAllChildFrame()));
//  connectAction(m_view, SIGNAL(currentSliceChanged(int)),
//          m_presenter, SLOT(processSliceChangeEvent(int)));
}

void ZStackFrame::connectSignalSlot()
{
  updateSignalSlot(neutu::ConnectFunc);
//  UPDATE_SIGNAL_SLOT(connect);
}


void ZStackFrame::disconnectAll()
{
  updateSignalSlot(neutu::DisconnectFunc);
//  UPDATE_SIGNAL_SLOT(disconnect);
}

void ZStackFrame::dropDocument(ZSharedPointer<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    if (m_doc) {
      updateSignalSlot(neutu::DisconnectFunc);
      m_doc->removeUser(this);
//      UPDATE_DOC_SIGNAL_SLOT(disconnect);
    }
    m_doc = doc;
    m_doc->registerUser(this);
//    m_doc->setParentFrame(this);
  }
}

void ZStackFrame::updateDocument()
{
  updateSignalSlot(neutu::ConnectFunc);

  updateTraceConfig();

  if (m_doc->hasStackData()) {
    /*
    m_doc->updateTraceWorkspace(traceEffort(), traceMasked(),
                                xResolution(), yResolution(), zResolution());
    m_doc->updateConnectionTestWorkspace(xResolution(), yResolution(),
                                         zResolution(), unit(),
                                         reconstructDistThre(),
                                         reconstructSpTest(),
                                         crossoverTest());
                                         */

    if (m_presenter != NULL && (m_doc->getStack()->kind() != GREY)) {
      m_presenter->optimizeStackBc();
    }
  }

  if (m_doc->hasStack()) {
    if (m_view != NULL) {
      m_view->reset();
    }
  }

  setWindowTitle(m_doc->stackSourcePath().c_str());

  m_progressReporter.setProgressBar(m_view->progressBar());
  m_doc->setProgressReporter(&m_progressReporter);
}

void ZStackFrame::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  if (m_doc.get() != doc.get()) {
    dropDocument(doc);
    updateDocument();
  }
}

void ZStackFrame::takeScreenshot(const QString &filename)
{
  if (m_view != NULL)
    m_view->takeScreenshot(filename);
}

void ZStackFrame::updateSwcExtensionHint()
{
  if (m_presenter != NULL) {
    m_presenter->updateSwcExtensionHint();
  }
}

void ZStackFrame::clearData()
{
  document()->clearData();
  presenter()->clearData();
}
#if 1
void ZStackFrame::clear()
{
  disconnectAll();
  detachParentFrame();
  removeAllChildFrame();

//  document()->setParentFrame(NULL);
  document()->setProgressReporter(NULL);

  // will be deleted by parent (this), so don't need, otherwise will crash
  /*
  if (m_view != NULL) {
    delete m_view;
    m_view = NULL;
  }

  if (m_presenter != NULL) {
    delete m_presenter;
    m_presenter = NULL;
  }

  if (m_settingDlg != NULL) {
    delete m_settingDlg;
    m_settingDlg = NULL;
  }
  */
  if (m_tile != NULL) {
      delete m_tile;
      m_tile = NULL;
  }
}
#endif

ZAutoTraceDialog* ZStackFrame::getAutoTraceDlg()
{
  if (m_autoTraceDlg == nullptr) {
    m_autoTraceDlg = new ZAutoTraceDialog(this);
  }

  return m_autoTraceDlg;
}

void ZStackFrame::loadStack(Stack *stack, bool isOwner)
{
  Q_ASSERT(m_doc.get() != NULL);
  m_doc->loadStack(stack, isOwner);
  prepareDisplay();
}

void ZStackFrame::loadStack(ZStack *stack)
{
  Q_ASSERT(m_doc.get() != NULL);
  m_doc->loadStack(stack);
  prepareDisplay();
}

void ZStackFrame::prepareDisplay()
{
  setWindowTitle(document()->stackSourcePath().c_str());
  m_statusInfo =  QString("%1 loaded").arg(document()->stackSourcePath().c_str());
//  if (m_doc->getStack()->kind() != GREY) {
//    m_presenter->optimizeStackBc();
//  }
//  m_view->reset();
}

void ZStackFrame::setupDisplay()
{
  prepareDisplay();

  ZOUT(LTRACE(), 5) << "ready(this) emitted";

  //To prevent strange duplcated signal emit
  disconnect(this, SIGNAL(stackLoaded()), this, SLOT(setupDisplay()));

  emit ready(this);
}

void ZStackFrame::setSizeHintOption(neutu::ESizeHintOption option)
{
  if (view() != NULL) {
    view()->setSizeHintOption(option);
  }
}

int ZStackFrame::readStack(const char *filePath)
{
  Q_ASSERT(m_doc.get() != NULL);

  switch (ZFileType::FileType(filePath)) {
  case ZFileType::EFileType::SWC:
    m_doc->readSwc(filePath);
    if (!m_doc->hasSwc()) {
      return ERROR_IO_READ;
    }

#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::EFileType::V3D_APO:
  case ZFileType::EFileType::V3D_MARKER:
    m_doc->importPuncta(filePath);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::EFileType::LOCSEG_CHAIN: {
    QStringList files;
    files.push_back(filePath);
    m_doc->importLocsegChain(files);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  }
  case ZFileType::EFileType::SWC_NETWORK:
    m_doc->loadSwcNetwork(filePath);
#ifdef _DEBUG_
    cout << "emit stackLoaded()" << endl;
#endif
    emit stackLoaded();
    break;
  case ZFileType::EFileType::JSON:
    if (!m_doc->importSynapseAnnotation(filePath, 0)) {
      return ERROR_IO_READ;
    }
    break;
  default:
    m_doc->readStack(filePath);
    break;
  }

  return SUCCESS;
}

int ZStackFrame::importImageSequence(const char *filePath)
{
  Q_ASSERT(m_doc.get() != NULL);

  if (m_doc->importImageSequence(filePath)) {
    if (!m_doc->hasStackData()) {
      return ERROR_IO_READ;
    }
  }

  setWindowTitle(filePath);
  m_statusInfo =  QString("%1 loaded").arg(filePath);
  if (m_doc->getStack()->kind() != GREY) {
    m_presenter->optimizeStackBc();
  }
  m_view->reset();

  return SUCCESS;
}

Z3DWindow* ZStackFrame::viewSegmentationMesh()
{
  Z3DWindow *window = NULL;

  QList<ZStackObject*> objList =
      document()->getObjectList(ZStackObjectRole::ROLE_SEGMENTATION);
  if (!objList.isEmpty()) {
    QList<ZMesh*> meshList;
    foreach (ZStackObject *obj, objList) {
      ZObject3dScan *region = dynamic_cast<ZObject3dScan*>(obj);
      if (region != NULL) {
        ZMesh *mesh = ZMeshFactory::MakeMesh(*region, 0, 3, false);
        if (mesh != NULL) {
          if (!mesh->empty()) {
            mesh->setColor(region->getColor());
            mesh->pushObjectColor();
            meshList.append(mesh);
          }
        }
      }
    }
    if (!meshList.isEmpty()) {
      ZStackFrame *newFrame = ZStackFrame::Make(NULL);
      newFrame->document()->addMesh(meshList);
      window = ZWindowFactory::Open3DWindow(newFrame);
      if (window != NULL) {
        window->getMeshFilter()->setColorMode("Mesh Color");
      }
      delete newFrame;
    }
  }

  return window;
}

void ZStackFrame::saveTraceProject(const QString &filePath,
                                     const QString &output,
                                     const QString &prefix)
{
  QFile file(filePath);
  file.open(QIODevice::WriteOnly);
  QTextStream stream(&file);
  stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  stream << "<trace version=\"1.0\">\n";
  stream << "<data>\n";
  stream << "<image type=\"tif\">\n";
  stream << "<url>" << document()->getStack()->source().firstUrl().c_str()
         << "</url>\n";
  stream << "<resolution>" << "<x>" << xResolution() << "</x>"
      << "<y>" << yResolution() << "</y>" << "<z>" << zResolution() << "</z>"
      << "</resolution>\n";
  stream << "</image>\n";
  stream << "</data>\n";

  if (!(output.isEmpty()) || !(prefix.isEmpty())) {
    stream << "<output>\n";
    if (!output.isEmpty()) {
      stream << "<workdir>" << output << "</workdir>\n";
    }
    if (!prefix.isEmpty()) {
      stream << "<tube>" << prefix << "</tube>\n";
    }
    stream << "</output>\n";
  }
  stream << "</trace>\n";
  file.close();
}

void ZStackFrame::setView(ZStackView *view)
{
  setWidget(view);
}

void ZStackFrame::startProgress(const QString &title)
{
  getProgressDialog()->setLabelText(title);
  getProgressDialog()->setValue(1);
  getProgressDialog()->show();
}

void ZStackFrame::updateTraceConfig()
{
  m_doc->updateTraceWorkspaceResolution(
        xResolution(), yResolution(), zResolution());
  m_doc->updateConnectionTestWorkspace(
        xResolution(), yResolution(), zResolution(), 'p',
        m_settingDlg->getMaxEucDist(), m_settingDlg->getSpTest(),
        m_settingDlg->getCrossoverTest());
  m_settingDlg->updateTracingConfig(m_doc->getNeuronTracer().getConfigRef());
}

void ZStackFrame::autoTraceFunc()
{
  ZQtBarProgressReporter reporter;
  reporter.setProgressBar(getProgressBar());

  ZProgressReporter *oldReporter = document()->getProgressReporter();
  document()->setProgressReporter(&reporter);

  document()->getNeuronTracer().setDiagnosis(m_autoTraceDlg->diagnosis());
  document()->getNeuronTracer().setOverTrace(m_autoTraceDlg->overTracing());
  document()->getNeuronTracer().setSeedScreening(m_autoTraceDlg->screenSeed());
  executeAutoTraceCommand(getAutoTraceDlg()->getTraceLevel(),
                          getAutoTraceDlg()->resampling(),
                          getAutoTraceDlg()->getChannel());
  document()->getNeuronTracer().setDiagnosis(false);
  document()->setProgressReporter(oldReporter);
  emit progressDone();
}

void ZStackFrame::closeEvent(QCloseEvent *event)
{
  if (m_doc->isSavingRequired()) {
    int ans =  QMessageBox::question(
          this, tr("Confirm"), tr("There might be unsaved changes. Close anyway?"),
          QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (ans == QMessageBox::Ok) {
      m_isClosing = true;
      event->accept();

      qDebug() << "emit closed(this)";
      emit closed(this);
    } else {
      event->ignore();
    }
  } else {
    m_isClosing = true;
    event->accept();

    qDebug() << "emit closed(this)";
    emit closed(this);
  }

//  close3DWindow();
}

void ZStackFrame::resizeEvent(QResizeEvent *event)
{
  QMdiSubWindow::resizeEvent(event);

  qDebug() << "emit infoChanged()";
  emit infoChanged();
}

void ZStackFrame::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void ZStackFrame::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();

  //Filter out tiff files
  QList<QUrl> imageUrls;
  QList<QUrl> nonImageUrls;

  foreach (QUrl url, urls) {
    if (ZFileType::isImageFile(neutu::GetFilePath(url).toStdString())) {
      imageUrls.append(url);
    } else {
      nonImageUrls.append(url);
    }
  }

  if (!imageUrls.isEmpty()) {
    MainWindow *mainWindow = getMainWindow();
    if (mainWindow != NULL) {
      QStringList fileList;
      foreach (QUrl url, imageUrls) {
        fileList.append(neutu::GetFilePath(url));
      }
      mainWindow->openFile(fileList);
    }
  }

  if (!nonImageUrls.isEmpty()) {
    load(nonImageUrls);
    if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
      ZWindowFactory::Open3DWindow(this, Z3DView::EInitMode::EXCLUDE_VOLUME);
//      open3DWindow(Z3DWindow::INIT_EXCLUDE_VOLUME);
    }
  }
}

QProgressDialog* ZStackFrame::getProgressDialog()
{
  if (m_progress == NULL) {
    m_progress = new QProgressDialog(this);
    m_progress->setRange(0, 100);
    m_progress->setWindowModality(Qt::WindowModal);
    m_progress->setAutoClose(true);
    //m_progress->setWindowFlags(Qt::Dialog|Qt::WindowStaysOnTopHint);
    m_progress->setCancelButton(0);
  }

  return m_progress;
}

QProgressBar* ZStackFrame::getProgressBar()
{
  return getProgressDialog()->findChild<QProgressBar*>();
}

void ZStackFrame::endProgress()
{
  getProgressDialog()->reset();
}

void ZStackFrame::setViewInfo(const QString &info)
{
  view()->setInfo(info);
}

void ZStackFrame::setViewInfo()
{
  view()->setInfo();
}

QString ZStackFrame::briefInfo() const
{
  return m_statusInfo;
}

QString ZStackFrame::info() const
{
  if ((document()) && view() != NULL) {
    QString info = document()->getStack()->sourcePath().c_str();
    info +=
      QString("\n %1 x %2 => %3 x %4").arg(document()->getStack()->width()).
      arg(document()->getStack()->height()).
      arg(view()->imageWidget()->screenSize().width()).
      arg(view()->imageWidget()->screenSize().height());
    //info += QString("\n zoom ratio: %1").arg(presenter()->zoomRatio());
    //info += QString("\n") + document()->toString();
    info += QString("\n") + m_statusInfo;
    return info;
  } else {
    return QString("");
  }
}

void ZStackFrame::showSetting()
{
  synchronizeSetting();
  if (m_settingDlg->exec() == QDialog::Accepted) {
    synchronizeDocument();
  }
}

void ZStackFrame::showManageObjsDialog()
{
  if (m_manageObjsDlg) {
    m_manageObjsDlg->raise();
    m_manageObjsDlg->show();
  } else {
    m_manageObjsDlg = new QDialog(this);
    m_manageObjsDlg->setWindowTitle("Objects");
    m_manageObjsDlg->setSizeGripEnabled(true);
    ZObjsManagerWidget* omw = new ZObjsManagerWidget(m_doc.get(), NULL);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(omw);
    m_manageObjsDlg->setLayout(layout);

    m_manageObjsDlg->show();
  }
}

double ZStackFrame::xResolution()
{
  return m_settingDlg->getXScale();
}

double ZStackFrame::yResolution()
{
  return m_settingDlg->getYScale();
}

double ZStackFrame::zResolution()
{
  return m_settingDlg->getZScale();
}

/*
double ZStackFrame::xReconstructScale()
{
  return m_settingDlg->xScale();
}

double ZStackFrame::zReconstructScale()
{
  return m_settingDlg->zScale();
}
*/

#if 0
int ZStackFrame::traceEffort()
{
  return m_settingDlg->traceEffort();
}

bool ZStackFrame::traceMasked()
{
  return m_settingDlg->traceMasked();
}

double ZStackFrame::autoTraceMinScore()
{
  return m_settingDlg->autoTraceMinScore();
}

double ZStackFrame::manualTraceMinScore()
{
  return m_settingDlg->manualTraceMinScore();
}

char ZStackFrame::unit()
{
  return m_settingDlg->unit();
}

double ZStackFrame::reconstructDistThre()
{
  return m_settingDlg->distThre();
}

int ZStackFrame::reconstructRootOption()
{
  return m_settingDlg->rootOption();
}

BOOL ZStackFrame::reconstructSpTest()
{
  if (m_settingDlg->reconstructEffort() == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

bool ZStackFrame::crossoverTest()
{
  return m_settingDlg->crossoverTest();
}

bool ZStackFrame::singleTree()
{
  return m_settingDlg->singleTree();
}

bool ZStackFrame::removeOvershoot()
{
  return m_settingDlg->removeOvershoot();
}

void ZStackFrame::setResolution(const double *res)
{
  m_settingDlg->setResolution(res[0], res[1], res[2]);
}
#endif

void ZStackFrame::addDecoration(ZStackObject *obj)
{
  presenter()->addDecoration(obj, true);
}

void ZStackFrame::clearDecoration()
{
  presenter()->removeAllDecoration();

  updateView();
}

void ZStackFrame::setBc(double greyScale, double greyOffset, int channel)
{
  presenter()->setStackBc(greyScale, greyOffset, channel);
  document()->setStackBc(greyScale, greyOffset, channel);
}

void ZStackFrame::synchronizeSetting()
{
  ZResolution res = m_doc->getResolution();
  m_settingDlg->setScale(res.voxelSizeX(), res.voxelSizeY(), res.voxelSizeZ());
//  m_settingDlg->setResolution(document()->getResolution());
//  m_settingDlg->setUnit(document()->getResolution().unit());
  m_settingDlg->setBackground(document()->getStackBackground());
}

void ZStackFrame::synchronizeDocument()
{
  document()->setResolution(m_settingDlg->getXScale(),
                            m_settingDlg->getYScale(),
                            m_settingDlg->getZScale(),
                            'p');
  if (hasProject()) {
    document()->setWorkdir(m_traceProject->workspacePath().toLocal8Bit().constData());
  }

  updateTraceConfig();
  m_doc->setStackBackground(m_settingDlg->getBackground());
}

void ZStackFrame::setLocsegChainInfo(ZLocsegChain *chain, QString prefix,
                                     QString suffix)
{
  m_statusInfo += prefix;
  if (chain != NULL) {
    if (chain->heldNode() < 0) {
      m_statusInfo = QString("Chain %1 %2; %3 segments;")
                     .arg(chain->id())
                     .arg(chain->getSource().c_str())
                     .arg(chain->length());
    } else {
      m_statusInfo =  QString("Chain %1 [%2] %3").arg(chain->id()).
                      arg(chain->heldNode()).arg(chain->getSource().c_str());
    }
  }
  m_statusInfo += suffix;

  updateInfo();
}

void ZStackFrame::changeWindowTitle(bool clean)
{
  QString title = windowTitle();
  if (clean) {
    if (title.endsWith(" *")) {
      title.resize(title.size()-2);
      setWindowTitle(title);
    }
  } else {
    if (!title.endsWith(" *")) {
      title += " *";
      setWindowTitle(title);
    }
  }
}

void ZStackFrame::processKeyEvent(QKeyEvent *event)
{
  if (m_presenter != NULL) {
    if (!m_presenter->processKeyPressEvent(event)) {
      emit keyEventEmitted(event);
    }
  }
}

void ZStackFrame::keyPressEvent(QKeyEvent *event)
{
  processKeyEvent(event);
}

void ZStackFrame::updateInfo()
{
  qDebug() << "emit infoChanged()";
  emit infoChanged();
}

ZCurve ZStackFrame::curveToPlot(PlotSettings *settings, int option) const
{
  switch (option) {
  case 0:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 500.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_DOT);

  case 1:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 1.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_CORRCOEF);

  case 2:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 255.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_MEAN_SIGNAL);

  case 3:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 255.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_OUTER_SIGNAL);

  default:
    if (settings != NULL) {
      settings->minY = 0.0;
      settings->maxY = 1.0;
      settings->adjust();
    }
    return document()->locsegProfileCurve(STACK_FIT_CORRCOEF);
  }
}

QStringList ZStackFrame::toStringList() const
{
  QStringList list;

  list += view()->toStringList();
  list += presenter()->toStringList();
  list += document()->toStringList();

  return list;
}


void ZStackFrame::updateView()
{
  m_view->redraw(ZStackView::EUpdateOption::QUEUED);
}

void ZStackFrame::undo()
{
  m_doc->undoStack()->undo();
}
void ZStackFrame::redo()
{
  m_doc->undoStack()->redo();
}

void ZStackFrame::pushUndoCommand(QUndoCommand *command)
{
  m_doc->undoStack()->push(command);
}

void ZStackFrame::pushBinarizeCommand()
{
  m_doc->executeBinarizeCommand(view()->getIntensityThreshold());
  /*
  QUndoCommand *cmd = new ZStackDocBinarizeCommand(
        document().get(), view()->getIntensityThreshold());
  pushUndoCommand(cmd);
  */
}

void ZStackFrame::pushBwsolidCommand()
{
  m_doc->executeBwsolidCommand();
  /*
  QUndoCommand *cmd = new ZStackDocBwSolidCommand(document().get());
  pushUndoCommand(cmd);
  */
}

void ZStackFrame::pushEnhanceLineCommand()
{
  m_doc->executeEnhanceLineCommand();
  //pushUndoCommand(new ZStackDocEnhanceLineCommand(document().get()));
}

void ZStackFrame::executeSwcRescaleCommand(const ZRescaleSwcSetting &setting)
{
  document()->executeSwcRescaleCommand(setting);
}

void ZStackFrame::executeAutoTraceCommand(int traceLevel, bool doResample, int c)
{
  document()->executeAutoTraceCommand(traceLevel, doResample, c);
}

void ZStackFrame::executeAutoTraceAxonCommand()
{
  document()->executeAutoTraceAxonCommand();
}

void ZStackFrame::executeWatershedCommand()
{
  document()->executeWatershedCommand();
}

void ZStackFrame::executeAddObjectCommand(ZStackObject *obj)
{
  document()->executeAddObjectCommand(obj);
}

double ZStackFrame::displayGreyMin(int c) const
{
  if (presenter()->greyScale(c) == 0.0) {
    return 0.0;
  }

  return -presenter()->greyOffset(c) / presenter()->greyScale(c);
}

double ZStackFrame::displayGreyMax(int c) const
{
  if (presenter()->greyScale(c) == 0.0) {
    return 0.0;
  }

  return (255.0 - presenter()->greyOffset(c)) / presenter()->greyScale(c);
}

void ZStackFrame::saveProjectAs(const QString &path)
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  document()->setWorkdir(path);
  m_traceProject->saveAs(path);
  m_doc->undoStack()->setClean();
}

void ZStackFrame::saveProject()
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  view()->progressBar()->setRange(0, 100);
  view()->progressBar()->show();
  QApplication::processEvents();
  m_traceProject->save();
  view()->progressBar()->setValue(view()->progressBar()->maximum());
  QApplication::processEvents();
  view()->progressBar()->hide();
  view()->setInfo("Project saved.");
  m_doc->undoStack()->setClean();
}

void ZStackFrame::importSwcAsReference(const QStringList &pathList)
{
  if (m_traceProject == NULL) {
    m_traceProject = new ZTraceProject(this);
  }

  document()->importSwc(pathList, ZStackDoc::LoadObjectOption::APPEND_OBJECT);
  foreach (QString path, pathList) {
    m_traceProject->addDecoration(path, "swc");
  }
}

bool ZStackFrame::isReadyToSave() const
{
  if (!hasProject()) {
    return false;
  }

  if (m_traceProject->workspacePath().isEmpty()) {
    return false;
  }

  return true;
}

/*
void ZStackFrame::exportSwc(const QString &filePath)
{
  document()->exportSwc(filePath.toStdString().c_str());

  if (document()->getStack()->isSwc()) {
    m_doc->undoStack()->setClean();
    setWindowTitle(filePath);
  }
}
*/

void ZStackFrame::exportPuncta(const QString &filePath)
{
  document()->exportPuncta(filePath.toStdString().c_str());
}

void ZStackFrame::exportTube(const QString &filePath)
{
  document()->exportBinary(filePath.toStdString().c_str());
}

void ZStackFrame::exportChainFileList(const QString &filePath)
{
  document()->exportChainFileList(filePath.toStdString().c_str());
}
/*
ZStack* ZStackFrame::getObjectMask()
{
  return view()->getObjectMask(1);
}

ZStack* ZStackFrame::getObjectMask(neutube::EColor color)
{
  return view()->getObjectMask(color, 1);
}
*/
ZStack* ZStackFrame::getStrokeMask()
{
  return view()->getStrokeMask(1);
}

ZStack* ZStackFrame::getStrokeMask(neutu::EColor color)
{
  return view()->getStrokeMask(color);
}

void ZStackFrame::exportObjectMask(const QString &filePath)
{
  view()->exportObjectMask(filePath.toStdString());
}

void ZStackFrame::exportObjectMask(
    neutu::EColor color, const QString &filePath)
{
  view()->exportObjectMask(color, filePath.toStdString());
}


void ZStackFrame::saveStack(const QString &filePath)
{
  if (document()->hasStackData()) {
    document()->getStack()->save(filePath.toStdString());
    document()->setStackSource(filePath.toStdString().c_str());
  } else {
    QList<ZSparseObject*> objList = document()->getSparseObjectList();
    ZObject3dScanArray objArray;
    foreach (ZSparseObject *obj, objList) {
      objArray.append(dynamic_cast<ZObject3dScan*>(obj));
    }
    ZStack *stack = objArray.toStackObject();
    objArray.shallowClear();

    if (stack != NULL) {
      stack->save(filePath.toStdString().c_str());
      delete stack;
    }
  }
}

void ZStackFrame::displayActiveDecoration(bool enabled)
{
  m_view->displayActiveDecoration(enabled);
}

QRect ZStackFrame::getViewGeometry() const
{
  QRect rect = view()->geometry();
  rect.moveTo(view()->mapToGlobal(rect.topLeft()));

  return rect;
}

ZInteractiveContext::ViewMode ZStackFrame::getViewMode() const
{
  return presenter()->interactiveContext().viewMode();
}

void ZStackFrame::setViewMode(ZInteractiveContext::ViewMode mode)
{
  presenter()->setViewMode(mode);
}

void ZStackFrame::setObjectDisplayStyle(ZStackObject::EDisplayStyle style)
{
  presenter()->setObjectStyle(style);
}

void ZStackFrame::setViewPortCenter(int x, int y, int z)
{
  view()->setViewPortCenter(x, y, z, neutu::EAxisSystem::NORMAL);
//  presenter()->setViewPortCenter(x, y, z);
}

void ZStackFrame::viewRoi(int x, int y, int z, int radius)
{
//  x -= document()->getStackOffset().getX();
//  y -= document()->getStackOffset().getY();
//  z -= document()->getStackOffset().getZ();
  zgeom::shiftSliceAxis(x, y, z, view()->getSliceAxis());

  ZStackViewLocator locator;
  locator.setCanvasSize(view()->imageWidget()->canvasSize().width(),
                        view()->imageWidget()->canvasSize().height());
  QRect viewPort = locator.getLandmarkViewPort(x, y, radius);
  presenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));

  view()->setViewPortCenter(x, y, z, neutu::EAxisSystem::SHIFTED);

//  presenter()->setViewPortCenter(x, y, z);
}

void ZStackFrame::hideObject()
{
  presenter()->setObjectVisible(false);
}

void ZStackFrame::showObject()
{
  presenter()->setObjectVisible(true);
}

#if 0
Z3DWindow* ZStackFrame::open3DWindow(Z3DWindow::EInitMode mode)
{
  if (Z3DApplication::app() == NULL) {
    ZDialogFactory::Notify3DDisabled(this);

    return NULL;
  }

  ZSharedPointer<ZStackDoc> doc = document();
  if (doc->getTag() == NeuTube::Document::BIOCYTIN_PROJECTION) {
    doc = doc->getParentDoc();
  }

  Z3DWindow *window = doc->getParent3DWindow();

  if (window == NULL) {
    if (getMainWindow() != NULL) {
      getMainWindow()->startProgress("Opening 3D View ...", 0);
    }

    ZWindowFactory factory;
    //factory.setParentWidget(parent);
    window = factory.make3DWindow(doc, mode);
    QString title = windowTitle();
    if (title.endsWith(" *")) {
      title.resize(title.size()-2);
    }
    window->setWindowTitle(title);

    doc->registerUser(window);

    connect(this, SIGNAL(closed(ZStackFrame*)), window, SLOT(close()));
//    connect(window, SIGNAL(destroyed()), this, SLOT(detach3DWindow()));
    if (getMainWindow() != NULL) {
      getMainWindow()->endProgress();
    }
  }

  if (window != NULL) {
    window->show();
    window->raise();
  } else {
    if (Z3DApplication::app() != NULL) {
      QMessageBox::critical(this, tr("3D functions are disabled"),
                            Z3DApplication::app()->getErrorMessage());
    }
  }

  return window;
}
#endif

void ZStackFrame::load(const QList<QUrl> &urls)
{
  m_doc->loadFileList(urls);
}

void ZStackFrame::load(const QStringList &fileList)
{
  m_doc->loadFileList(fileList);
}

void ZStackFrame::load(const QString &filePath)
{
  m_doc->loadFile(filePath);
}

void ZStackFrame::load(const std::string &filePath)
{
  m_doc->loadFile(filePath.c_str());
}

QAction* ZStackFrame::getBodySplitAction()
{
  QAction *action = NULL;
  if (getMainWindow() != NULL) {
    action = getMainWindow()->getBodySplitAction();
  }

  return action;
}

MainWindow* ZStackFrame::getMainWindow()
{
  MainWindow *mainwin = NULL;
  QObject *parentObject = parent();
  if (parentObject != NULL) {
    parentObject = parentObject->parent();
    if (parentObject != NULL) {
      parentObject = parentObject->parent();
    }

    mainwin = qobject_cast<MainWindow*>(parentObject);
  }

  return mainwin;
}

/*
void ZStackFrame::importStackMask(const string &filePath)
{
  ZStackFile file;
  file.import(filePath);
  setStackMask(file.readStack());
}

void ZStackFrame::setStackMask(ZStack *stack)
{
  m_doc->setStackMask(stack);
  updateView();
}
*/

ZStackFrame* ZStackFrame::spinoffStackSelection(const vector<int> &selected)
{
  if (document()->hasStackData()) {
    int channelNumber = document()->getStack()->channelNumber();
    std::vector<std::vector<double> > selectedColor =
        ZDoubleVector::reshape(selected, channelNumber);

    return spinoffStackSelection(selectedColor);
  }

  return NULL;
}

ZStackFrame* ZStackFrame::spinoffStackSelection(
    const vector<vector<double> > &selected)
{
  ZStackFrame *frame = NULL;

  if (m_doc->hasStackData()) {
    //frame = new ZStackFrame();
    frame = Make(NULL);
    ZStack *substack = m_doc->getStack()->createSubstack(selected);

    frame->document()->loadStack(substack);
    frame->view()->reset();
  }

  return frame;
}

void ZStackFrame::invertStack()
{
  if (m_doc->hasStackData()) {
    m_doc->invert();
  }
}

void ZStackFrame::subtractBackground()
{
  if (m_doc->hasStackData()) {
    m_doc->subtractBackground();
  }
}

void ZStackFrame::detachParentFrame()
{
  if (m_parentFrame != NULL) {
    m_parentFrame->removeChildFrame(this);
    m_parentFrame = NULL;
  }
}

void ZStackFrame::removeChildFrame(ZStackFrame *frame)
{
  if (m_childFrameList.removeOne(frame)) {
    frame->m_parentFrame = NULL;
  }
}

void ZStackFrame::removeAllChildFrame()
{
  foreach (ZStackFrame *childFrame, m_childFrameList) {
    childFrame->m_parentFrame = NULL;
  }
  m_childFrameList.clear();
}

void ZStackFrame::closeAllChildFrame()
{
  foreach (ZStackFrame *childFrame, m_childFrameList) {
    childFrame->close();
  }
  m_childFrameList.clear();
}

void ZStackFrame::setParentFrame(ZStackFrame *frame)
{
  detachParentFrame();
  m_parentFrame = frame;
  if (m_parentFrame != NULL) {
    m_parentFrame->m_childFrameList.append(this);
  }
}

void ZStackFrame::findLoopInStack()
{
  document()->findLoop();
}

void ZStackFrame::bwthin()
{
  document()->bwthin();
}

void ZStackFrame::importSeedMask(const QString &filePath)
{
  document()->importSeedMask(filePath);
}

void ZStackFrame::importMask(const QString &filePath)
{
  ZStack *stack = NULL;
  if (ZFileType::FileType(filePath.toStdString()) == ZFileType::EFileType::PNG) {
    QImage image;
    image.load(filePath);
    stack = new ZStack(GREY, image.width(), image.height(), 1, 1);

    size_t offset = 0;
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        QRgb rgb = image.pixel(x, y);
        if (qRed(rgb) == 255 && qGreen(rgb) == 255 && qBlue(rgb) == 255) {
          stack->array8()[offset] = 0;
        } else {
          if (image.hasAlphaChannel()) {
            stack->array8()[offset] = qAlpha(rgb);
          } else {
            stack->array8()[offset] = qRed(rgb);
          }
        }
        ++offset;
      }
    }
  } else {
    ZStackFile stackFile;
    stackFile.import(filePath.toStdString());
    stack = stackFile.readStack();
  }

  if (stack != NULL) {
    if (stack->channelNumber() == 1 && stack->kind() == GREY) {
      ZObject3dScan *obj = ZObject3dFactory::MakeObject3dScan(*stack, NULL);
      obj->setSource(stack->sourcePath());
      obj->setColor(QColor(255, 0, 0, 128));
//      if (obj->loadStack(stack->c_stack(0))) {
      if (!obj->isEmpty()) {
        obj->translate(document()->getStackOffset());
        /*
        obj->translate(iround(document()->getStackOffset().getX()),
                       iround(document()->getStackOffset().getY()),
                       iround(document()->getStackOffset().getZ()));
                       */
        executeAddObjectCommand(obj);
      } else {
        delete obj;
        report("Loading mask failed", "Cannot convert the image into mask",
               neutu::EMessageType::ERROR);
      }
    } else {
      report("Loading mask failed", "Must be single 8-bit image",
             neutu::EMessageType::ERROR);
    }
    delete stack;
  }
}

void ZStackFrame::importSwc(const QString &filePath)
{
  load(filePath);
}

void ZStackFrame::importSobj(const QStringList &fileList)
{
  ZStackFile stackFile;
  vector<string> fileArray;
  foreach(QString file, fileList) {
    fileArray.push_back(file.toStdString());
  }

  stackFile.importSobjList(fileArray);

  ZStack *stack = stackFile.readStack();
  loadStack(stack);
}

void ZStackFrame::importPointList(const QString &filePath)
{
  QList<ZPunctum*> puncta = ZPunctumIO::load(filePath);
  if (!puncta.isEmpty()) {
    document()->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
    foreach (ZPunctum* punctum, puncta) {
      document()->addObject(punctum);
    }
    document()->endObjectModifiedMode();
    document()->processObjectModified();
//    document()->notifyPunctumModified();
  }
}

void ZStackFrame::autoBcAdjust()
{
  document()->startProgress();
  for (int i = 0; i < document()->getStack()->channelNumber(); ++i) {
    document()->advanceProgress(0.1);
    double lower, upper;
    ZStackStatistics::getGreyMapHint(*document()->getStack(), i,
                                     &lower, &upper);
    document()->advanceProgress(0.2);
    double scale = 1.0;
    double offset = 0.0;
    if (upper > lower) {
      scale = 255.0 / (upper - lower);
    }

    offset = -scale * lower;

    setBc(scale, offset, i);
  }
  document()->endProgress();
  updateView();
}

void ZStackFrame::loadRoi(bool isExclusive)
{
  if (!document()->stackSourcePath().empty()) {
    ZString sourcePath = document()->stackSourcePath();

    ZString suffix =
        ZBiocytinFileNameParser::getSuffix(ZBiocytinFileNameParser::ESuffixRole::ROI);

    sourcePath = sourcePath.dirPath() + ZString::FileSeparator +
        ZBiocytinFileNameParser::getCoreName(sourcePath);

    QFileInfo fileInfo((sourcePath + suffix + ".tif").c_str());
    if (!fileInfo.exists()) {
      fileInfo.setFile(
            (sourcePath + static_cast<const ZString&>(suffix).lower() + ".tif").c_str());
    } else if (!fileInfo.exists()) {
      fileInfo.setFile(
            (sourcePath + static_cast<const ZString&>(suffix).upper() + ".tif").c_str());
    }

    if (fileInfo.exists()) {
      loadRoi(fileInfo.absoluteFilePath(), isExclusive);
    }
  }
}

void ZStackFrame::loadRoi(const QString &filePath, bool isExclusive)
{
  ZStackFile stackFile;
  stackFile.import(filePath.toStdString());

  ZStack *stack = stackFile.readStack();

  if (stack->kind() != GREY) {
    std::cerr << "Ignore invalid ROI" <<std::endl;
    delete stack;
    stack = NULL;
  }

  if (stack != NULL) {
    Stack *mask = C_Stack::clone(stack->c_stack());

    ZObject3dScan *obj = new ZObject3dScan;

    Stack_Binarize(mask);
    Stack_Not(mask, mask);

    obj->loadStack(mask);

#ifdef _DEBUG_2
    obj->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

    //obj->print();
    obj->duplicateSlice(document()->getStack()->depth());

    obj->setColor(16, 16, 16, 64);

    obj->setTarget(ZStackObject::ETarget::OBJECT_CANVAS);
    if (isExclusive) {
      clearDecoration();
    }
    obj->addVisualEffect(neutu::display::SparseObject::VE_FORCE_SOLID);
    addDecoration(obj);
    updateView();

    C_Stack::kill(mask);

    delete stack;
  }
}

void ZStackFrame::zoomToSelectedSwcNodes()
{
  if (document()->hasSelectedSwcNode()) {
    std::set<Swc_Tree_Node*> nodeSet = document()->getSelectedSwcNodeSet();
    ZCuboid cuboid = SwcTreeNode::boundBox(nodeSet);
    ZPoint center = cuboid.center();

    //check which stack the selected points belong to.
    //If needed, load the corresponding stack.
    if (getTileManager() != NULL) {
      ZTileInfo tile = getTileManager()->getSelectedTileItem()->getTileInfo();
      QRect bound= QRect(tile.getOffset().x(),tile.getOffset().y(),tile.getWidth(),tile.getHeight());
      if (!bound.contains(center.x(),center.y())) {
        QList<QGraphicsItem*> itemList = getTileManager()->items();
        foreach (QGraphicsItem *item, itemList) {
          ZTileGraphicsItem *zitem = dynamic_cast<ZTileGraphicsItem*>(item);
          if (zitem != NULL) {
            //check whether this item contains the selected points.
            tile = zitem->getTileInfo();
            bound = QRect(tile.getOffset().x(),tile.getOffset().y(),tile.getWidth(),tile.getHeight());
            if (bound.contains(center.x(),center.y())) {
              getTileManager()->selectItem(zitem);
              break;
            }
          }
        }
      }
    }
    int cx, cy, cz;
#if 0
    center.translate(-document()->getStackOffset().getX(),
                     -document()->getStackOffset().getY(),
                     -document()->getStackOffset().getZ());
#endif
    //-= document()->getStackOffset();
    cx = iround(center.x());
    cy = iround(center.y());
    cz = iround(center.z());
    int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
    viewRoi(cx, cy, cz, radius);
  }
}

void ZStackFrame::notifyUser(const QString &message)
{
  if (!message.isEmpty()) {
    m_statusInfo = message;

    emit infoChanged();
  }
}

void ZStackFrame::locateSwcNodeIn3DView()
{
  if (document()->hasSelectedSwcNode()) {
    Z3DWindow *window = document()->getParent3DWindow();
    if (!window) {
      window = ZWindowFactory::Open3DWindow(this);
//      window = open3DWindow();
    }
    //QApplication::processEvents();
    window->zoomToSelectedSwcNodes();
    window->raise();
  }
}

void ZStackFrame::runSeededWatershed()
{
  emit splitStarted();
  document()->runSeededWatershed();
}

void ZStackFrame::makeSwcProjection(ZStackDoc *doc)
{
    if (doc == NULL) return;
    QList<ZSwcTree*> swclist= doc->getSwcList();
    foreach (ZSwcTree* swc, swclist) {
        ZSwcTree *swcClone = swc->clone();
        std::vector<Swc_Tree_Node*> nodes = swcClone->getSwcTreeNodeArray();
        foreach (Swc_Tree_Node *nd, nodes) {
            SwcTreeNode::setZ(nd, 0);
        }
        this->presenter()->addDecoration(swcClone);
    }
}

ZStackObject::EDisplayStyle ZStackFrame::getObjectStyle() const
{
  return m_presenter->objectStyle();
}

void ZStackFrame::setObjectStyle(ZStackObject::EDisplayStyle style)
{
  m_presenter->setObjectStyle(style);
}

void ZStackFrame::createMainWindowActions()
{
  m_presenter->createMainWindowActions();
}

void ZStackFrame::notifyViewChanged(const ZStackViewParam &param)
{
#ifdef _DEBUG_2
  std::cout << "Signal: ZStackFrame::notifyViewChanged" << std::endl;
#endif

  emit viewChanged(param);
}

void ZStackFrame::setView(const ZStackViewParam &param)
{
  view()->setView(param);
//  raise();
}

void ZStackFrame::customizeWidget()
{
  if (!m_isWidgetReady) {
    view()->customizeWidget();
    m_isWidgetReady = true;
  }
}

void ZStackFrame::testSlot()
{
  QtConcurrent::run(document().get(), &ZStackDoc::test);
}

void ZStackFrame::stressTest()
{
  if (m_testTimer->isActive()) {
    m_testTimer->stop();
  } else {
    m_testTimer->start(500);
  }
}

void ZStackFrame::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
  switch (message->getType()) {
  case ZMessage::TYPE_3D_VIS:
  {
    ZStackFrame *frame = qobject_cast<ZStackFrame*>(host);
    if (frame != NULL) {
      if (frame->document()->getTag() == neutu::Document::ETag::BIOCYTIN_STACK ||
          frame->document()->getTag() == neutu::Document::ETag::BIOCYTIN_PROJECTION) {
        ZWindowFactory::Open3DWindow(frame, Z3DView::EInitMode::EXCLUDE_VOLUME);
//        frame->open3DWindow(Z3DWindow::INIT_EXCLUDE_VOLUME);
      } else {
        ZWindowFactory::Open3DWindow(frame);
//        frame->open3DWindow();
      }
    }
    message->deactivate();
  }
    break;
  default:
    break;
  }
}

void ZStackFrame::skeletonize()
{
  FlyEmSkeletonizationDialog dlg;
  if (dlg.exec() == QDialog::Accepted) {
    ZStack *stack = document()->getStack();
    //stack->binarize();

    Stack *stackData = stack->c_stack();

    //      ZStack backupStack;
    ZObject3dScan *obj = NULL;
    if (stackData == NULL) {
      ZOUT(LTRACE(), 5) << "Skeletonization";
      QList<ZSparseObject*> objList =
          document()->getObjectList<ZSparseObject>();
      if (!objList.isEmpty()) {
        obj = new ZObject3dScan;
        for (QList<ZSparseObject*>::iterator iter = objList.begin();
             iter != objList.end(); ++iter) {
          obj->concat(*(*iter));
        }
      }
    }

    ZSwcTree *wholeTree = NULL;
    if (stackData != NULL || obj != NULL) {
      ZStackSkeletonizer skeletonizer;
      skeletonizer.setDownsampleInterval(dlg.getXInterval(), dlg.getYInterval(),
                                         dlg.getZInterval());
      skeletonizer.setProgressReporter(document()->getProgressReporter());
      skeletonizer.setRebase(true);
      if (dlg.isExcludingSmallObj()) {
        skeletonizer.setMinObjSize(dlg.sizeThreshold());
      } else {
        skeletonizer.setMinObjSize(0);
      }

      double distThre = -1.0;
      if (!dlg.isConnectingAll()) {
        distThre = dlg.distanceThreshold();
      }
      skeletonizer.setDistanceThreshold(distThre);

      //skeletonizer.setResolution(1, 3);

      skeletonizer.setLengthThreshold(dlg.lengthThreshold());
      skeletonizer.setKeepingSingleObject(dlg.isKeepingShortObject());


      if (dlg.isLevelChecked()) {
        skeletonizer.setLevel(dlg.level());
        skeletonizer.setLevelOp(dlg.getLevelOp());
        //skeletonizer.useOriginalSignal(true);
      }

      if (stackData != NULL) {
        wholeTree = skeletonizer.makeSkeleton(stackData);
      } else {
        wholeTree = skeletonizer.makeSkeleton(*obj);
        delete obj;
      }

      wholeTree->translate(document()->getStackOffset());
    }

    if (wholeTree != NULL) {
      wholeTree->addComment("skeleton");
      executeAddObjectCommand(wholeTree);
      ZWindowFactory::Open3DWindow(this, Z3DView::EInitMode::EXCLUDE_VOLUME);
      //        frame->open3DWindow(Z3DWindow::INIT_EXCLUDE_VOLUME);
    } else {
      ZDialogFactory::Error("Skeletonization failed", "No SWC tree generated.",
                            this);
    }

  }
}
