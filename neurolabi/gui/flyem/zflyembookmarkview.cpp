#include "zflyembookmarkview.h"

#include <QMenu>
#include <QContextMenuEvent>

#include "zflyembookmarklistmodel.h"

ZFlyEmBookmarkView::ZFlyEmBookmarkView(QWidget *parent) :
  QTableView(parent), m_contextMenu(NULL)
{
  init();
}

void ZFlyEmBookmarkView::init()
{
  createMenu();
}

void ZFlyEmBookmarkView::createMenu()
{
  m_contextMenu = new QMenu(this);
  QAction *checkAction = new QAction("Set Checked", this);
  m_contextMenu->addAction(checkAction);
  connect(checkAction, SIGNAL(triggered()), this, SLOT(checkCurrentBookmark()));

  QAction *unCheckAction = new QAction("Uncheck", this);
  m_contextMenu->addAction(unCheckAction);
  connect(unCheckAction, SIGNAL(triggered()),
          this, SLOT(uncheckCurrentBookmark()));
}

void ZFlyEmBookmarkView::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef _DEBUG_2
  std::cout << "Context menu triggered." << std::endl;
#endif

  if (m_contextMenu != NULL) {
    m_contextMenu->popup(event->globalPos());
  }
}

void ZFlyEmBookmarkView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton) {
    QTableView::mousePressEvent(event);
  }
}

void ZFlyEmBookmarkView::keyPressEvent(QKeyEvent *event)
{
  event->ignore();
}

ZFlyEmBookmarkListModel* ZFlyEmBookmarkView::getModel() const
{
  return dynamic_cast<ZFlyEmBookmarkListModel*>(model());
}

void ZFlyEmBookmarkView::checkCurrentBookmark(bool checking)
{
  QItemSelectionModel *sel = selectionModel();
  QModelIndexList selected = sel->selectedIndexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = getModel()->getBookmark(index.row());
    bookmark->setChecked(checking);
    getModel()->update(index.row());

    emit bookmarkChecked(bookmark);
//    emit bookmarkChecked(bookmark.getDvidKey(), checking);
  }
}

void ZFlyEmBookmarkView::checkCurrentBookmark()
{
  checkCurrentBookmark(true);
}

void ZFlyEmBookmarkView::uncheckCurrentBookmark()
{
  checkCurrentBookmark(false);
}
