#include "flyemsplitcontrolform.h"

#include <QMenu>
#include <iostream>
#include "ui_flyemsplitcontrolform.h"
#include "zdialogfactory.h"

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

  ui->commitPushButton->setEnabled(false);
}

void FlyEmSplitControlForm::slotTest()
{
  std::cout << "slot triggered." << std::endl;
}

void FlyEmSplitControlForm::changeSplit()
{
  emit changingSplit((uint64_t) ui->bodyIdSpinBox->value());
}

void FlyEmSplitControlForm::setSplit(uint64_t bodyId)
{
  ui->bodyIdSpinBox->setValue(bodyId);
}

void FlyEmSplitControlForm::commitResult()
{
  if (ZDialogFactory::Ask("Commit Confirmation",
                          "Do you want to upload the splitting results now? "
                          "It cannot be undone.",
                          this)) {
    emit committingResult();
  }
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
