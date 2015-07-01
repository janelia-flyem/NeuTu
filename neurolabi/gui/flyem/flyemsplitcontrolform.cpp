#include "flyemsplitcontrolform.h"

#include <QMenu>
#include <QInputDialog>
#include <iostream>
#include "ui_flyemsplitcontrolform.h"
#include "zdialogfactory.h"
#include "zstring.h"
#include "zflyembodysplitproject.h"

FlyEmSplitControlForm::FlyEmSplitControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmSplitControlForm)
{
  ui->setupUi(this);

  ui->bookmarkView->setModel(&m_bookmarkList);
  setupWidgetBehavior();
}

FlyEmSplitControlForm::~FlyEmSplitControlForm()
{
  delete ui;
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
  connect(ui->viewSplitPushButton, SIGNAL(clicked()),
          this, SIGNAL(splitViewTriggered()));
  connect(ui->loadBodyPushButton, SIGNAL(clicked()),
          this, SLOT(changeSplit()));
//  connect(ui->loadBodyPushButton, SIGNAL(clicked()), this, SLOT(slotTest()));
  connect(ui->saveSeedPushButton, SIGNAL(clicked()),
          this, SIGNAL(savingSeed()));
  connect(ui->commitPushButton, SIGNAL(clicked()),
          this, SLOT(commitResult()));
  connect(ui->bodyIdSpinBox, SIGNAL(valueConfirmed(int)),
          this, SLOT(changeSplit()));
  connect(ui->loadBookmarkButton, SIGNAL(clicked()),
          this, SLOT(loadBookmark()));
  connect(ui->bookmarkView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(locateBookmark(QModelIndex)));
  connect(ui->synapsePushButton, SIGNAL(clicked()),
          this, SIGNAL(loadingSynapse()));

  ui->viewSplitPushButton->setEnabled(false);

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
  m_mainMenu->addAction(queryPixelAction);
  connect(queryPixelAction, SIGNAL(triggered()), this, SLOT(goToPosition()));

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

  QAction *exportSeedAction = new QAction("Export", this);
  seedMenu->addAction(exportSeedAction);
  connect(exportSeedAction, SIGNAL(triggered()), this, SLOT(exportSeed()));

  QAction *importSeedAction = new QAction("Import", this);
  seedMenu->addAction(importSeedAction);
  connect(importSeedAction, SIGNAL(triggered()), this, SLOT(importSeed()));

  m_bookmarkContextMenu = new QMenu(this);
  QAction *checkAction = new QAction("Set Checked", this);
  m_bookmarkContextMenu->addAction(checkAction);
  connect(checkAction, SIGNAL(triggered()), this, SLOT(checkCurrentBookmark()));

  QAction *unCheckAction = new QAction("Uncheck", this);
  m_bookmarkContextMenu->addAction(unCheckAction);
  connect(unCheckAction, SIGNAL(triggered()),
          this, SLOT(uncheckCurrentBookmark()));


  ui->bookmarkView->setContextMenu(m_bookmarkContextMenu);
}

void FlyEmSplitControlForm::checkCurrentBookmark(bool checking)
{
  QItemSelectionModel *sel = ui->bookmarkView->selectionModel();
  QModelIndexList selected = sel->selectedIndexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark &bookmark = m_bookmarkList.getBookmark(index.row());
    bookmark.setChecked(checking);
    m_bookmarkList.update(index.row());
  }
}

void FlyEmSplitControlForm::checkCurrentBookmark()
{
  QItemSelectionModel *sel = ui->bookmarkView->selectionModel();
  QModelIndexList selected = sel->selectedIndexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark &bookmark = m_bookmarkList.getBookmark(index.row());
    bookmark.setChecked(true);
    m_bookmarkList.update(index.row());
  }
}

void FlyEmSplitControlForm::uncheckCurrentBookmark()
{
  checkCurrentBookmark(false);
}


void FlyEmSplitControlForm::changeSplit()
{
  emit changingSplit((uint64_t) ui->bodyIdSpinBox->value());
}

void FlyEmSplitControlForm::setSplit(uint64_t bodyId)
{
  ui->bodyIdSpinBox->setValue(bodyId);
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

void FlyEmSplitControlForm::selectSeed()
{
  emit selectingSeed();
}

void FlyEmSplitControlForm::selectAllSeed()
{
  emit selectingAllSeed();
}

void FlyEmSplitControlForm::commitResult()
{
  emit committingResult();
}

void FlyEmSplitControlForm::updateBookmarkTable(ZFlyEmBodySplitProject *project)
{
  if (project != NULL) {
//    const ZFlyEmBookmarkArray &bookmarkArray = project->getBookmarkArray();
    m_bookmarkList.clear();
    if (project->getBodyId() > 0) {
      project->clearBookmarkDecoration();

      const ZFlyEmBookmarkArray *bookmarkArray = project->getBookmarkArray();
      if (bookmarkArray != NULL) {
//        foreach (ZFlyEmBookmark bookmark, *bookmarkArray) {
        for (ZFlyEmBookmarkArray::const_iterator iter = bookmarkArray->begin();
             iter != bookmarkArray->end(); ++iter) {
          const ZFlyEmBookmark &bookmark = *iter;
          if (bookmark.getBodyId() == project->getBodyId() &&
              bookmark.getType() == ZFlyEmBookmark::TYPE_FALSE_MERGE) {
            m_bookmarkList.append(bookmark);
          }
        }
      }

      project->addBookmarkDecoration(m_bookmarkList.getBookmarkArray());
    }
  }
}

void FlyEmSplitControlForm::locateBookmark(const QModelIndex &index)
{
  const ZFlyEmBookmark &bookmark = m_bookmarkList.getBookmark(index.row());

  emit zoomingTo(bookmark.getLocation().getX(),
                 bookmark.getLocation().getY(),
                 bookmark.getLocation().getZ());
}

void FlyEmSplitControlForm::loadBookmark()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Bookmarks", "", this);
  if (!fileName.isEmpty()) {
    emit loadingBookmark(fileName);
  }
}

void FlyEmSplitControlForm::updateBodyWidget(uint64_t bodyId)
{
  ui->bodyIdSpinBox->setValue(bodyId);
  QString text;
  if (bodyId == 0) {
    text += "<p>No body loaded.</p>";
  } else {
    text += QString("<p>Body ID: %2</p>").arg(bodyId);
  }
  ui->infoWidget->setText(text);
}

