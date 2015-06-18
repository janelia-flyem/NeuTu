#include "flyemsplitcontrolform.h"

#include <QMenu>
#include <QInputDialog>
#include <iostream>
#include "ui_flyemsplitcontrolform.h"
#include "zdialogfactory.h"
#include "zstring.h"

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
    const ZFlyEmBookmarkArray &bookmarkArray = project->getBookmarkArray();
    m_bookmarkList.clear();
    if (project->getBodyId() > 0) {
      project->clearBookmarkDecoration();

      foreach (ZFlyEmBookmark bookmark, bookmarkArray) {
        if (bookmark.getBodyId() == project->getBodyId()) {
          m_bookmarkList.append(bookmark);
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
  QString fileName = ZDialogFactory::GetFileName("Load Bookmarks", "", this);
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
