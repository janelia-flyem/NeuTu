#include "flyemsplitcontrolform.h"

#include <QMenu>
#include <QInputDialog>
#include <iostream>

#include "qt/gui/utilities.h"
#include "flyem/zflyembookmarkwidget.h"
#include "flyem/zflyembookmarkview.h"
#include "ui_flyemsplitcontrolform.h"
#include "zdialogfactory.h"
#include "zstring.h"
#include "zflyembodysplitproject.h"
#include "zstackdoc.h"
#include "neutubeconfig.h"

FlyEmSplitControlForm::FlyEmSplitControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmSplitControlForm)
{
  ui->setupUi(this);

  neutu::SetHtmlIcon(ui->coarseBodyViewPushButton, "<font color=red><b>&#9711;</b></font>");
  neutu::SetHtmlIcon(ui->quickViewPushButton, "<font color=red><b>&#9711;&#9711;&#9711;</b></font>");
  neutu::SetHtmlIcon(ui->meshPushButton, "<font color=green>&#9650;&#9650;</font>");
  neutu::SetHtmlIcon(ui->viewResultQuickPushButton,
                     "<font color=red>&#9700;</font><font color=green>&#9701;</font>");

  setupWidgetBehavior();
}

FlyEmSplitControlForm::~FlyEmSplitControlForm()
{
  delete ui;
}

ZFlyEmBookmarkView* FlyEmSplitControlForm::getAssignedBookmarkView() const
{
  return ui->bookmarkWidget->getBookmarkView(
        ZFlyEmBookmarkWidget::SOURCE_ASSIGNED);
}

ZFlyEmBookmarkView* FlyEmSplitControlForm::getUserBookmarkView() const
{
  return ui->bookmarkWidget->getBookmarkView(
        ZFlyEmBookmarkWidget::SOURCE_USER);
}

void FlyEmSplitControlForm::setupWidgetBehavior()
{
  connect(ui->exitPushButton, SIGNAL(clicked()), this, SIGNAL(exitingSplit()));
  connect(ui->quickViewPushButton, SIGNAL(clicked()),
          this, SIGNAL(quickViewTriggered()));
  connect(ui->viewResultQuickPushButton, SIGNAL(clicked()),
          this, SIGNAL(splitQuickViewTriggered()));
  connect(ui->view3dBodyPushButton, SIGNAL(clicked()),
          this, SIGNAL(bodyViewTriggered()));
  connect(ui->coarseBodyViewPushButton, SIGNAL(clicked()),
          this, SIGNAL(coarseBodyViewTriggered()));
  connect(ui->meshPushButton, SIGNAL(clicked()),
          this, SIGNAL(meshViewTriggered()));
//  connect(ui->viewSplitPushButton, SIGNAL(clicked()),
//          this, SIGNAL(splitViewTriggered()));
  connect(ui->loadBodyPushButton, SIGNAL(clicked()),
          this, SLOT(changeSplit()));
//  connect(ui->loadBodyPushButton, SIGNAL(clicked()), this, SLOT(slotTest()));
  connect(ui->saveSeedPushButton, SIGNAL(clicked()),
          this, SIGNAL(savingSeed()));
  connect(ui->commitPushButton, SIGNAL(clicked()),
          this, SLOT(commitResult()));
  connect(ui->bodyIdSpinBox, SIGNAL(valueConfirmed(int)),
          this, SLOT(changeSplit()));
//  connect(ui->loadBookmarkButton, SIGNAL(clicked()),
//          this, SLOT(loadBookmark()));
  connect(getUserBookmarkView(), SIGNAL(locatingBookmark(const ZFlyEmBookmark*)),
          this, SLOT(locateBookmark(const ZFlyEmBookmark*)));
//  connect(getUserBookmarkView(), SIGNAL(bookmarkChecked(QString,bool)),
//          this, SIGNAL(bookmarkChecked(QString, bool)));
//  connect(getUserBookmarkView(), SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
//          this, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)));

//  connect(getAssignedBookmarkView(), SIGNAL(bookmarkChecked(QString,bool)),
//          this, SIGNAL(bookmarkChecked(QString, bool)));
//  connect(getAssignedBookmarkView(), SIGNAL(bookmarkChecked(ZFlyEmBookmark*)),
//          this, SIGNAL(bookmarkChecked(ZFlyEmBookmark*)));
  connect(getAssignedBookmarkView(), SIGNAL(locatingBookmark(const ZFlyEmBookmark*)),
          this, SLOT(locateBookmark(const ZFlyEmBookmark*)));


//  connect(ui->synapsePushButton, SIGNAL(clicked()),
//          this, SIGNAL(loadingSynapse()));

  ui->viewSplitPushButton->setEnabled(false);
//  ui->loadBookmarkButton->hide();
//  ui->synapsePushButton->hide();
  ui->view3dBodyPushButton->hide();
  ui->viewSplitPushButton->hide();
//  ui->sideView->hide();
//  ui->sideViewLabel->hide();

//  ui->commitPushButton->setEnabled(false);
  createMenu();
}

void FlyEmSplitControlForm::slotTest()
{
  std::cout << "slot triggered." << std::endl;
}

void FlyEmSplitControlForm::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);

  QAction *queryPixelAction = new QAction("Go to Position", this);
  queryPixelAction->setShortcut(Qt::Key_F3);
  m_mainMenu->addAction(queryPixelAction);
  connect(queryPixelAction, SIGNAL(triggered()), this, SLOT(goToPosition()));

  QAction *loadSplitTaskAction = new QAction("Load Split Task", this);
  m_mainMenu->addAction(loadSplitTaskAction);
  connect(loadSplitTaskAction, SIGNAL(triggered()), this, SLOT(loadSplitTask()));

  QAction *saveTaskAction = new QAction("Save Split Task", this);
  m_mainMenu->addAction(saveTaskAction);
  connect(saveTaskAction, SIGNAL(triggered()), this, SLOT(saveTask()));

  QAction *loadSplitAction = new QAction("Load Split Result", this);
  m_mainMenu->addAction(loadSplitAction);
  connect(loadSplitAction, SIGNAL(triggered()), this, SLOT(loadSplitResult()));

  QAction *uploadSplitAction = new QAction("Upload Split Result", this);
  m_mainMenu->addAction(uploadSplitAction);
  connect(uploadSplitAction, SIGNAL(triggered()), this, SLOT(uploadSplitResult()));

  QMenu *seedMenu = m_mainMenu->addMenu("Seed");
  QAction *recoverSeedAction = new QAction("Recover", this);
  seedMenu->addAction(recoverSeedAction);
  connect(recoverSeedAction, SIGNAL(triggered()), this, SLOT(recoverSeed()));

  QAction *selectSeedAction = new QAction("Select by Label", this);
  seedMenu->addAction(selectSeedAction);
  connect(selectSeedAction, SIGNAL(triggered()), this, SLOT(selectSeed()));

  QAction *selectAllSeedAction = new QAction("Select All", this);
  seedMenu->addAction(selectAllSeedAction);
  connect(selectAllSeedAction, SIGNAL(triggered()), this, SLOT(selectAllSeed()));

  QAction *setMainSeedAction = new QAction("Set Main Label", this);
  seedMenu->addAction(setMainSeedAction);
  connect(setMainSeedAction, SIGNAL(triggered()), this, SLOT(setMainSeed()));

  QAction *exportSeedAction = new QAction("Export", this);
  seedMenu->addAction(exportSeedAction);
  connect(exportSeedAction, SIGNAL(triggered()), this, SLOT(exportSeed()));

  QAction *importSeedAction = new QAction("Import", this);
  seedMenu->addAction(importSeedAction);
  connect(importSeedAction, SIGNAL(triggered()), this, SLOT(importSeed()));

  QAction *crop3DAction = new QAction("Coarse Body Crop", this);
  m_mainMenu->addAction(crop3DAction);
  connect(crop3DAction, SIGNAL(triggered()), this, SLOT(cropCoarseBody3D()));

  QAction *body3DAction = new QAction("Show 3D Grayscale", this);
  m_mainMenu->addAction(body3DAction);
  connect(body3DAction, SIGNAL(triggered()), this, SLOT(showBodyGrayscale()));
}

void FlyEmSplitControlForm::saveTask()
{
  emit savingTask();
}

void FlyEmSplitControlForm::loadSplitResult()
{
  emit loadingSplitResult();
}

void FlyEmSplitControlForm::uploadSplitResult()
{
  emit uploadingSplitResult();
}

void FlyEmSplitControlForm::loadSplitTask()
{
  emit loadingSplitTask();
}

void FlyEmSplitControlForm::checkCurrentBookmark(bool checking)
{
  getAssignedBookmarkView()->checkCurrentBookmark(checking);
  /*
  QItemSelectionModel *sel = getAssignedBookmarkView()->selectionModel();
  QModelIndexList selected = sel->selectedIndexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = m_assignedBookmarkList.getBookmark(index.row());
    bookmark->setChecked(checking);
    m_assignedBookmarkList.update(index.row());
  }
  */
}

void FlyEmSplitControlForm::checkCurrentBookmark()
{
  checkCurrentBookmark(true);
}

void FlyEmSplitControlForm::uncheckCurrentBookmark()
{
  checkCurrentBookmark(false);
}

void FlyEmSplitControlForm::locateBookmark(const ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    emit zoomingTo(bookmark->getLocation().getX(),
                   bookmark->getLocation().getY(),
                   bookmark->getLocation().getZ());
  }
}



void FlyEmSplitControlForm::changeSplit()
{
  emit changingSplit((uint64_t) ui->bodyIdSpinBox->value());
}

void FlyEmSplitControlForm::setSplit(uint64_t bodyId)
{
  ui->bodyIdSpinBox->setValue(bodyId);
  m_currentBodyId = bodyId;
}

void FlyEmSplitControlForm::goToPosition()
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

void FlyEmSplitControlForm::recoverSeed()
{
  emit recoveringSeed();
}

void FlyEmSplitControlForm::exportSeed()
{
  emit exportingSeed();
}

void FlyEmSplitControlForm::importSeed()
{
  emit importingSeed();
}

void FlyEmSplitControlForm::cropCoarseBody3D()
{
  emit croppingCoarseBody3D();
}

void FlyEmSplitControlForm::showBodyGrayscale()
{
  emit showingBodyGrayscale();
}

void FlyEmSplitControlForm::selectSeed()
{
  emit selectingSeed();
}

void FlyEmSplitControlForm::selectAllSeed()
{
  emit selectingAllSeed();
}

void FlyEmSplitControlForm::setMainSeed()
{
  emit settingMainSeed();
}

void FlyEmSplitControlForm::commitResult()
{
  emit committingResult();
}

#if 0
void FlyEmSplitControlForm::clearBookmarkTable(ZFlyEmBodySplitProject */*project*/)
{
  m_assignedBookmarkList.clear();
}

void FlyEmSplitControlForm::updateBookmarkTable(ZFlyEmBodySplitProject *project)
{
  if (project != NULL) {
    if (project->getDocument() != NULL) {
      ZOUT(LINFO(), 3) << "Update bookmark table for split project";
      m_assignedBookmarkList.clear();
      m_userBookmarkList.clear();
      const TStackObjectList &objList = project->getDocument()->
          getObjectList(ZStackObject::EType::TYPE_FLYEM_BOOKMARK);
      for (TStackObjectList::const_iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
        //        if (bookmark->getBodyId() == project->getBodyId()) {
        if (bookmark->isCustom()) {
          m_userBookmarkList.append(bookmark);
        } else if (bookmark->getBookmarkType() == ZFlyEmBookmark::EBookmarkType::TYPE_FALSE_MERGE) {
          if (bookmark->getBodyId() == project->getBodyId()) {
            m_assignedBookmarkList.append(bookmark);
          }
        }
      }
      ZOUT(LINFO(), 3) << "Bookmark updated";
//      }
    }
  }
}
#endif

void FlyEmSplitControlForm::loadBookmark()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Bookmarks", "", this);
  if (!fileName.isEmpty()) {
    emit loadingBookmark(fileName);
  }
}

void FlyEmSplitControlForm::updateBodyWidget(uint64_t bodyId)
{
  setSplit(bodyId);
  QString text;
  if (bodyId == 0) {
    text += "<p>No body loaded.</p>";
  } else {
    text += QString("<p><i>Body ID</i>: %2</p>").arg(bodyId);
    text += QString("<p><i>Split workflow</i>: Paint seeds -> Compute splits -> Save splits <p>"
                    "<p>Instructions: Once seeds are painted, "
                    "you can use hotkey <b>'Space'</b> (small range), "
                    "<b>'Shift+Space'</b> (medium range)"
                    " or <b>Alt+Space</b> (full range) to run the split, "
                    "which grows regions from the seeds. The red label defines "
                    "the main region."
                    "<p><i>Tip</i>: Painting seeds on <font color=green>brighter regions</font> "
                    "tends to generate better results.</p>");
  }
  ui->infoWidget->setText(text);
}

#if 0
void FlyEmSplitControlForm::updateUserBookmarkTable(ZStackDoc *doc)
{
  ZOUT(LINFO(), 3) << "Updating user bookmark table for split control form";

  m_userBookmarkList.clear();
  if (doc != NULL) {
    const TStackObjectList &objList =
        doc->getObjectList(ZStackObject::EType::TYPE_FLYEM_BOOKMARK);
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
  ZOUT(LINFO(), 3) << "Sorting bookmark view";

  getUserBookmarkView()->sort();
}
#endif
