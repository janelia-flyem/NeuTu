#include "flyemproofcontrolform.h"

#include <QMenu>
#include <QInputDialog>
#include <QSortFilterProxyModel>

#include "ui_flyemproofcontrolform.h"
#include "dialogs/zdviddialog.h"
#include "zstring.h"
#include "neutubeconfig.h"
#include "flyem/zflyembodymergeproject.h"
#include "zstackdoc.h"
#include "zflyembookmarkview.h"

FlyEmProofControlForm::FlyEmProofControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmProofControlForm)
{
  ui->setupUi(this);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  /*
  connect(ui->segmentCheckBox, SIGNAL(clicked(bool)),
          this, SIGNAL(segmentVisibleChanged(bool)));
  ui->segmentCheckBox->hide();
  */
  connect(ui->mergeSegmentPushButton, SIGNAL(clicked()),
          this, SIGNAL(mergingSelected()));
  connect(ui->dvidPushButton, SIGNAL(clicked()),
          this, SIGNAL(dvidSetTriggered()));
  connect(ui->segmentSizePushButton, SIGNAL(clicked()),
          this, SLOT(setSegmentSize()));
  connect(ui->saveMergePushButton, SIGNAL(clicked()),
          this, SIGNAL(savingMerge()));
  connect(ui->splitPushButton, SIGNAL(clicked()),
          this, SIGNAL(splitTriggered()));
  connect(ui->uploadPushButton, SIGNAL(clicked()),
          this, SIGNAL(committingMerge()));

  ui->segmentSizePushButton->hide();
  ui->segmentSizeDecPushButton->setEnabled(false);

  ui->saveMergePushButton->hide();

//  ui->bodyViewPushButton->hide();


  connect(ui->segmentSizeIncPushButton, SIGNAL(clicked()),
          this, SLOT(incSegmentSize()));
  connect(ui->segmentSizeDecPushButton, SIGNAL(clicked()),
          this, SLOT(decSegmentSize()));
  connect(ui->fullViewPushButton, SIGNAL(clicked()),
          this, SLOT(showFullSegmentation()));

  connect(ui->coarseBodyPushButton, SIGNAL(clicked()),
          this, SIGNAL(coarseBodyViewTriggered()));
  connect(ui->bodyViewPushButton, SIGNAL(clicked()),
          this, SIGNAL(bodyViewTriggered()));
  connect(ui->skeletonViewPushButton, SIGNAL(clicked()),
          this, SIGNAL(skeletonViewTriggered()));

  connect(getAssignedBookmarkView(), SIGNAL(locatingBookmark(const ZFlyEmBookmark*)),
          this, SLOT(locateBookmark(const ZFlyEmBookmark*)));
  connect(getAssignedBookmarkView(), SIGNAL(bookmarkChecked(QString,bool)),
          this, SIGNAL(bookmarkChecked(QString, bool)));
  connect(getAssignedBookmarkView(), SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
          this, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)));
  connect(getAssignedBookmarkView(), SIGNAL(removingBookmark(ZFlyEmBookmark*)),
          this, SIGNAL(removingBookmark(ZFlyEmBookmark*)));

  connect(getUserBookmarkView(), SIGNAL(locatingBookmark(const ZFlyEmBookmark*)),
          this, SLOT(locateBookmark(const ZFlyEmBookmark*)));
  connect(getUserBookmarkView(), SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
          this, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)));
  connect(getUserBookmarkView(), SIGNAL(removingBookmark(ZFlyEmBookmark*)),
          this, SIGNAL(removingBookmark(ZFlyEmBookmark*)));
  connect(getUserBookmarkView(), SIGNAL(removingBookmark(QList<ZFlyEmBookmark*>)),
          this, SIGNAL(removingBookmark(QList<ZFlyEmBookmark*>)));
  /*
  connect(ui->userBookmarkView, SIGNAL(bookmarkChecked(QString,bool)),
          this, SIGNAL(bookmarkChecked(QString, bool)));
          */

//  m_userBookmarkProxy = createSortingProxy(&m_userBookmarkList);
  getUserBookmarkView()->setBookmarkModel(&m_userBookmarkList);
  getAssignedBookmarkView()->setBookmarkModel(&m_assignedBookmarkList);
//  m_bookmarkProxy = createSortingProxy(&m_bookmarkList);
//  ui->bookmarkView->setModel(&m_bookmarkList);
//  ui->bookmarkView->setSortingEnabled(true);

  getAssignedBookmarkView()->resizeColumnsToContents();
  getUserBookmarkView()->resizeColumnsToContents();

  createMenu();
}

FlyEmProofControlForm::~FlyEmProofControlForm()
{
  delete ui;
}

QSortFilterProxyModel*
FlyEmProofControlForm::createSortingProxy(ZFlyEmBookmarkListModel *model)
{
  QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(model);
  proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxy->setFilterKeyColumn(-1);

  return proxy;
}

ZFlyEmBookmarkView* FlyEmProofControlForm::getUserBookmarkView() const
{
  return ui->bookmarkWidget->getBookmarkView(
        ZFlyEmBookmarkWidget::SOURCE_USER);
//  return ui->userBookmarkView;
}

ZFlyEmBookmarkView* FlyEmProofControlForm::getAssignedBookmarkView() const
{
  return ui->bookmarkWidget->getBookmarkView(
        ZFlyEmBookmarkWidget::SOURCE_ASSIGNED);
//  return ui->bookmarkView;
}

void FlyEmProofControlForm::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);

  QAction *queryPixelAction = new QAction("Go to Position", this);
  m_mainMenu->addAction(queryPixelAction);
  connect(queryPixelAction, SIGNAL(triggered()), this, SLOT(goToPosition()));

  QAction *queryBodyAction = new QAction("Go to Body", this);
  queryBodyAction->setShortcut(Qt::Key_F1);
  m_mainMenu->addAction(queryBodyAction);
  connect(queryBodyAction, SIGNAL(triggered()), this, SLOT(goToBody()));

  QAction *selectBodyAction = new QAction("Select Body", this);
  selectBodyAction->setShortcut(Qt::Key_F2);
  m_mainMenu->addAction(selectBodyAction);
  connect(selectBodyAction, SIGNAL(triggered()), this, SLOT(selectBody()));

  QMenu *colorMenu = m_mainMenu->addMenu("Color Map");
  QActionGroup *colorActionGroup = new QActionGroup(this);
  QAction *normalColorAction = new QAction("Normal", this);
  normalColorAction->setCheckable(true);
  normalColorAction->setChecked(true);

  m_nameColorAction = new QAction("Name", this);
  m_nameColorAction->setCheckable(true);
  m_nameColorAction->setEnabled(false);

  m_sequencerColorAction = new QAction("Sequencer", this);
  m_sequencerColorAction->setCheckable(true);
  m_sequencerColorAction->setEnabled(true);

  colorActionGroup->addAction(normalColorAction);
  colorActionGroup->addAction(m_nameColorAction);
  colorActionGroup->addAction(m_sequencerColorAction);
  colorActionGroup->setExclusive(true);

  colorMenu->addAction(normalColorAction);
  colorMenu->addAction(m_nameColorAction);
  colorMenu->addAction(m_sequencerColorAction);

  connect(colorActionGroup, SIGNAL(triggered(QAction*)),
          this, SLOT(changeColorMap(QAction*)));

#ifdef _DEBUG_
  QMenu *developerMenu = m_mainMenu->addMenu("Developer");
  QAction *clearMergeAction = new QAction("Clear All Merges", this);
  connect(clearMergeAction, SIGNAL(triggered()),
          this, SLOT(clearBodyMergeStage()));
  developerMenu->addAction(clearMergeAction);

  QAction *exportBodyAction = new QAction("Export Selected Bodies", this);
  connect(exportBodyAction, SIGNAL(triggered()),
          this, SLOT(exportSelectedBody()));
  developerMenu->addAction(exportBodyAction);
#endif
//  colorMenu->setEnabled(false);
}

void FlyEmProofControlForm::exportSelectedBody()
{
  emit exportingSelectedBody();
}

void FlyEmProofControlForm::enableNameColorMap(bool on)
{
  if (m_nameColorAction != NULL) {
    m_nameColorAction->setEnabled(on);
  }
}

void FlyEmProofControlForm::changeColorMap(QAction *action)
{
  if (action != NULL) {
    emit changingColorMap(action->text());
  }
}

void FlyEmProofControlForm::clearBodyMergeStage()
{
  emit clearingBodyMergeStage();
}

void FlyEmProofControlForm::selectBody()
{
  emit selectingBody();
}

void FlyEmProofControlForm::setSegmentSize()
{
//  emit labelSizeChanged(1024, 1024);
}

void FlyEmProofControlForm::incSegmentSize()
{
  ui->segmentSizeIncPushButton->setEnabled(false);
  ui->segmentSizeDecPushButton->setEnabled(true);
  emit labelSizeChanged(1024, 1024);
}

void FlyEmProofControlForm::decSegmentSize()
{
  ui->segmentSizeIncPushButton->setEnabled(true);
  ui->segmentSizeDecPushButton->setEnabled(false);
  emit labelSizeChanged(512, 512);
}

void FlyEmProofControlForm::showFullSegmentation()
{
  emit showingFullSegmentation();
}

void FlyEmProofControlForm::goToBody()
{
  emit goingToBody();
}

void FlyEmProofControlForm::goToPosition()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Coordinates:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<int> coords = str.toIntegerArray();
      if (coords.size() == 3) {
        emit zoomingTo(coords[0], coords[1], coords[2]);
      }
    }
  }
}

void FlyEmProofControlForm::setInfo(const QString &info)
{
  ui->dataInfoWidget->setText(info);
}

void FlyEmProofControlForm::setDvidInfo(const ZDvidTarget &target)
{
  setInfo(target.toJsonObject().dumpString(2).c_str());
}


void FlyEmProofControlForm::removeBookmarkFromTable(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    if (bookmark->isCustom()) {
      m_userBookmarkList.removeBookmark(bookmark);
    } else {
      m_assignedBookmarkList.removeBookmark(bookmark);
    }
  }
}

void FlyEmProofControlForm::updateUserBookmarkTable(ZStackDoc *doc)
{
  m_userBookmarkList.clear();
  if (doc != NULL) {
    const TStackObjectList &objList =
        doc->getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
      if (bookmark != NULL) {
        if (bookmark->isCustom()) {
          m_userBookmarkList.append(bookmark);
        }
      }
    }
  }
  getUserBookmarkView()->sort();
  /*
  m_userBookmarkProxy->sort(m_userBookmarkProxy->sortColumn(),
                            m_userBookmarkProxy->sortOrder());
                            */
//  ui->userBookmarkView->resizeColumnsToContents();
}

void FlyEmProofControlForm::updateBookmarkTable(ZFlyEmBodyMergeProject *project)
{
  if (project != NULL) {
//    const ZFlyEmBookmarkArray &bookmarkArray = project->getBookmarkArray();
    m_assignedBookmarkList.clear();
//    project->clearBookmarkDecoration();

    if (project->getDocument() != NULL) {
      const TStackObjectList &objList =
          project->getDocument()->getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
      //        foreach (ZFlyEmBookmark bookmark, *bookmarkArray) {
      for (TStackObjectList::const_iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
        if (/*bookmark->getBookmarkType() != ZFlyEmBookmark::TYPE_FALSE_MERGE &&*/
            !bookmark->isCustom()) {
          m_assignedBookmarkList.append(bookmark);
        }
      }
    }
    getAssignedBookmarkView()->sort();
    /*
    m_bookmarkProxy->sort(m_bookmarkProxy->sortColumn(),
                          m_bookmarkProxy->sortOrder());
                          */
//    ui->bookmarkView->resizeColumnsToContents();
//    project->addBookmarkDecoration(m_bookmarkList.getBookmarkArray());
  }
}

void FlyEmProofControlForm::clearBookmarkTable(ZFlyEmBodyMergeProject */*project*/)
{
  m_assignedBookmarkList.clear();
}

void FlyEmProofControlForm::locateBookmark(const ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    emit zoomingTo(bookmark->getLocation().getX(),
                   bookmark->getLocation().getY(),
                   bookmark->getLocation().getZ());
  }
}

void FlyEmProofControlForm::locateAssignedBookmark(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = getAssignedBookmarkView()->getBookmark(index);

  locateBookmark(bookmark);
}

void FlyEmProofControlForm::locateUserBookmark(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = getUserBookmarkView()->getBookmark(index);

  locateBookmark(bookmark);
}
