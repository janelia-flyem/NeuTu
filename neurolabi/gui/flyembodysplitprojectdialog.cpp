#include "flyembodysplitprojectdialog.h"
#include <QFileDialog>
#include <QProgressDialog>
#include <QtConcurrentRun>

#include "ui_flyembodysplitprojectdialog.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zflyemnewbodysplitprojectdialog.h"
#include "dvid/zdvidreader.h"
#include "zstackskeletonizer.h"
#include "neutubeconfig.h"
#include "zimage.h"
#include "flyem/zflyemneuronimagefactory.h"

FlyEmBodySplitProjectDialog::FlyEmBodySplitProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodySplitProjectDialog)
{
  ui->setupUi(this);

  m_loadBodyDlg = NULL;

  //connect(this, SIGNAL(accepted()), this, SLOT(clear()));
  connect(ui->view2dBodyPushButton, SIGNAL(clicked()),
          this, SLOT(showData2d()));
  connect(ui->view3dBodyPushButton, SIGNAL(clicked()),
          this, SLOT(showData3d()));
  connect(ui->viewSplitPushButton,
          SIGNAL(clicked()), this, SLOT(showResult3d()));
  connect(ui->donePushButton, SIGNAL(clicked()), this, SLOT(clear()));
  connect(ui->loadBodyPushButton, SIGNAL(clicked()), this, SLOT(loadBody()));
  connect(ui->loadBookmarkButton, SIGNAL(clicked()),
          this, SLOT(loadBookmark()));
  connect(ui->bookmarkVisibleCheckBox, SIGNAL(toggled(bool)),
          &m_project, SLOT(showBookmark(bool)));
  connect(ui->quickViewPushButton, SIGNAL(clicked()), this, SLOT(quickView()));

  ui->bookmarkView->setModel(&m_bookmarkList);

  updateWidget();

  connect(ui->bookmarkView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(locateBookmark(QModelIndex)));

  m_project.showBookmark(ui->bookmarkVisibleCheckBox->isChecked());


  m_sideViewScene = new QGraphicsScene(this);
  //m_sideViewScene->setSceneRect(0, 0, ui->sideView->width(), ui->sideView->height());
  ui->sideView->setScene(m_sideViewScene);
  //ui->outputWidget->setText("Load a body to start.");

  //Progress signal-slot
  connect(this, SIGNAL(progressDone()),
          getMainWindow(), SIGNAL(progressDone()));

  connect(this, SIGNAL(messageDumped(QString,bool)),
          this, SLOT(dump(QString,bool)));
}

FlyEmBodySplitProjectDialog::~FlyEmBodySplitProjectDialog()
{
  delete ui;
  //delete m_sideViewScene;
}

void FlyEmBodySplitProjectDialog::closeEvent(QCloseEvent */*event*/)
{
  clear();
}

void FlyEmBodySplitProjectDialog::setDvidTarget(const ZDvidTarget &target)
{
  m_project.setDvidTarget(target);
}

void FlyEmBodySplitProjectDialog::setBodyId(int id)
{
  m_project.setBodyId(id);
}

int FlyEmBodySplitProjectDialog::getBodyId() const
{
  return m_project.getBodyId();
}

void FlyEmBodySplitProjectDialog::clear()
{
  m_project.clear();
  updateWidget();

  //dump("Load a body to start.");
}

void FlyEmBodySplitProjectDialog::shallowClear()
{
  m_project.shallowClear();
  updateWidget();
}

void FlyEmBodySplitProjectDialog::shallowClearResultWindow()
{
  m_project.shallowClearResultWindow();
}

void FlyEmBodySplitProjectDialog::shallowClearDataFrame()
{
  m_project.shallowClearDataFrame();
  updateWidget();
}

void FlyEmBodySplitProjectDialog::showData2d()
{
  dump("Showing body in 2D ...");

  if (m_project.hasDataFrame()) {
    m_project.showDataFrame();
    getMainWindow()->raise();
    dump("Done.", true);
  } else {
    if (getMainWindow()->initBodySplitProject()) {
      updateWidget();
      dump("Done.", true);
    }
  }
}

void FlyEmBodySplitProjectDialog::showData3d()
{
  if (m_project.hasDataFrame()) {
    dump("Showing body in 3D ...");
    m_project.showDataFrame3d();
    dump("done", true);
  }
}

void FlyEmBodySplitProjectDialog::showResult3d()
{
  dump("Showing splitting result ...");
  m_project.showResult3d();
  dump("Done.", true);
}

MainWindow* FlyEmBodySplitProjectDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
}

QProgressDialog* FlyEmBodySplitProjectDialog::getProgressDialog()
{
  return getMainWindow()->getProgressDialog();
}

const ZDvidTarget& FlyEmBodySplitProjectDialog::getDvidTarget() const
{
  return m_project.getDvidTarget();
}

void FlyEmBodySplitProjectDialog::setDataFrame(ZStackFrame *frame)
{
  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClearDataFrame()));

  m_project.setDataFrame(frame);
}

void FlyEmBodySplitProjectDialog::setLoadBodyDialog(
    ZFlyEmNewBodySplitProjectDialog *dlg)
{
  m_loadBodyDlg = dlg;
}

void FlyEmBodySplitProjectDialog::loadBody()
{
  if (m_loadBodyDlg->exec()) {
    setDvidTarget(m_loadBodyDlg->getDvidTarget());
    setBodyId(m_loadBodyDlg->getBodyId());

    //updateSideView();;
    updateWidget();
    updateSideView();

    dump("Body loaded.");
  }
}

void FlyEmBodySplitProjectDialog::quickView()
{
  m_project.quickView();
}

bool FlyEmBodySplitProjectDialog::isBodyLoaded() const
{
  return m_project.getBodyId() > 0;
}

void FlyEmBodySplitProjectDialog::updateButton()
{
  ui->loadBodyPushButton->setEnabled(!isBodyLoaded());
  ui->view2dBodyPushButton->setEnabled(isBodyLoaded());
  ui->quickViewPushButton->setEnabled(isBodyLoaded());
  ui->view3dBodyPushButton->setEnabled(m_project.hasDataFrame());
  ui->viewSplitPushButton->setEnabled(m_project.hasDataFrame());
  ui->commitPushButton->setEnabled(m_project.hasDataFrame());
  ui->donePushButton->setEnabled(isBodyLoaded());
}

void FlyEmBodySplitProjectDialog::updateWidget()
{
  updateButton();
  QString text;
  if (!m_project.getBookmarkArray().isEmpty()) {
    text += QString("<p>%1 bookmarks</p>").arg(m_project.getBookmarkArray().size());
  }

  if (!isBodyLoaded()) {
    text += "<p>No body loaded.</p>";
    dump("Load a body to start.");
  } else {
    text += QString("<p>DVID Server: %1</p><p>Body ID: %2</p>").
          arg(m_project.getDvidTarget().getSourceString().c_str()).
          arg(m_project.getBodyId());
  }
  ui->infoWidget->setText(text);

  updateBookmarkTable();
}

void FlyEmBodySplitProjectDialog::dump(const QString &info, bool appending)
{
  if (appending) {
    ui->outputWidget->append(info);
  } else {
    ui->outputWidget->setText(info);
  }
}

void FlyEmBodySplitProjectDialog::loadBookmark()
{
  QString fileName = getMainWindow()->getOpenFileName("Load Bookmarks", "*.json");
  if (!fileName.isEmpty()) {
    m_project.loadBookmark(fileName);
    updateWidget();

    m_loadBodyDlg->setBodyIdComboBox(m_project.getBookmarkBodySet());

    dump("Bookmarks loaded.");
  }
}

void FlyEmBodySplitProjectDialog::updateBookmarkTable()
{
  const ZFlyEmBookmarkArray &bookmarkArray = m_project.getBookmarkArray();
  m_bookmarkList.clear();
  if (isBodyLoaded()) {
    m_project.clearBookmarkDecoration();

    foreach (ZFlyEmBookmark bookmark, bookmarkArray) {
      if (bookmark.getBodyId() == m_project.getBodyId()) {
        m_bookmarkList.append(bookmark);
      }
    }

    m_project.addBookmarkDecoration(m_bookmarkList.getBookmarkArray());
  }
}

void FlyEmBodySplitProjectDialog::locateBookmark(const QModelIndex &index)
{
  const ZFlyEmBookmark &bookmark = m_bookmarkList.getBookmark(index.row());
  m_project.locateBookmark(bookmark);
}

void FlyEmBodySplitProjectDialog::updateSideView()
{
  startProgress("Generating side view ...");
  getProgressDialog()->setRange(0, 100);

  /*QFuture<void> res = */QtConcurrent::run(
        this, &FlyEmBodySplitProjectDialog::updateSideViewFunc);
}

void FlyEmBodySplitProjectDialog::updateSideViewFunc()
{
  m_sideViewScene->clear();
  QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
  QPixmap pixmap;
  //Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR + "/benchmark/bfork_2d.tif");

  ZFlyEmNeuronImageFactory factory;
  factory.setDownsampleInterval(7, 7, 7);

  getProgressDialog()->setValue(10);

  ZObject3dScan obj;
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    reader.readBody(getBodyId(), &obj);
  }

  getProgressDialog()->setValue(30);

  //obj.load(GET_DATA_DIR + "/benchmark/432.sobj");
  tic();
  Stack *stack = factory.createSurfaceImage(obj);
  ptoc();

  getProgressDialog()->setValue(80);

  int targetWidth = ui->sideView->width() - 10;
  double ratio = double(targetWidth) / C_Stack::width(stack);

  Stack *stack2 = C_Stack::resize(stack, iround(C_Stack::width(stack) * ratio),
                  iround(C_Stack::height(stack) * ratio), 1);
  C_Stack::kill(stack);
  stack = stack2;

  getProgressDialog()->setValue(90);

  ZImage image(C_Stack::width(stack), C_Stack::height(stack));
  image.setData(C_Stack::array8(stack));
  if (!pixmap.convertFromImage(image)) {
    dump("Failed to load the thumbnail.");
  }
  C_Stack::kill(stack);
  thumbnailItem->setPixmap(pixmap);
  //thumbnailItem->setOffset(0, 0);
  m_sideViewScene->addItem(thumbnailItem);

  emit messageDumped("Side view ready.", true);

  emit progressDone();
}

void FlyEmBodySplitProjectDialog::startProgress(const QString &label)
{
  getMainWindow()->getProgressDialog()->setRange(0, 100);
  getMainWindow()->getProgressDialog()->setLabelText(label);
  getMainWindow()->getProgressDialog()->show();
}

void FlyEmBodySplitProjectDialog::on_pushButton_clicked()
{
#ifdef _DEBUG_
  //updateSideView();
  startProgress("Test ...");
  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/benchmark/432.sobj");

  ZFlyEmNeuronImageFactory factory;
  factory.setDownsampleInterval(7, 7, 7);
  tic();
  Stack *stack = factory.createSurfaceImage(obj);
  ptoc();
  C_Stack::kill(stack);
  emit progressDone();
#endif
}
