#include "flyembodysplitprojectdialog.h"
#include <QFileDialog>

#include "ui_flyembodysplitprojectdialog.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zflyemnewbodysplitprojectdialog.h"

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

  ui->bookmarkView->setModel(&m_bookmarkList);

  updateWidget();

  connect(ui->bookmarkView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(locateBookmark(QModelIndex)));

  m_project.showBookmark(ui->bookmarkVisibleCheckBox->isChecked());
  //ui->outputWidget->setText("Load a body to start.");
}

FlyEmBodySplitProjectDialog::~FlyEmBodySplitProjectDialog()
{
  delete ui;
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
  } else {
    getMainWindow()->initBodySplitProject();
  }
  updateWidget();
  dump("Done.", true);
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
  dump("Done.");
}

MainWindow* FlyEmBodySplitProjectDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
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

    updateWidget();

    dump("Body loaded.");
  }
}

bool FlyEmBodySplitProjectDialog::isBodyLoaded() const
{
  return m_project.getBodyId() > 0;
}

void FlyEmBodySplitProjectDialog::updateButton()
{
  ui->loadBodyPushButton->setEnabled(!isBodyLoaded());
  ui->view2dBodyPushButton->setEnabled(isBodyLoaded());
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
    dump("Loading bookmarks ...");

    m_project.loadBookmark(fileName);
    updateWidget();

    m_loadBodyDlg->setBodyIdComboBox(m_project.getBookmarkBodySet());

    dump("Done.");
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
