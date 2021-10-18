#include "zflyembookmarkview.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QGuiApplication>

#include "logging/zqslog.h"
#include "logging/zlog.h"

#include "../zflyembookmarklistmodel.h"

ZFlyEmBookmarkView::ZFlyEmBookmarkView(QWidget *parent) :
  QTableView(parent)
{
  init();
}

void ZFlyEmBookmarkView::init()
{
  m_bookmarkModel = NULL;
//  m_proxy = NULL;

  setFocusPolicy(Qt::NoFocus);

  m_contextMenu = NULL;
//  createMenu();
  connectSignalSlot();
  m_enableDeletion = true;
}

void ZFlyEmBookmarkView::connectSignalSlot()
{
  connect(this, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(processDouleClick(QModelIndex)));
  connect(this, SIGNAL(clicked(QModelIndex)),
          this, SLOT(processSingleClick(QModelIndex)));

}

void ZFlyEmBookmarkView::processDouleClick(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = getBookmark(index);
  if (bookmark != NULL) {
    emit locatingBookmark(bookmark);
  }
}

void ZFlyEmBookmarkView::processSingleClick(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = getBookmark(index);
  if (bookmark != NULL) {
    KINFO << bookmark->toLogString() + " is clicked";
    if (QGuiApplication::keyboardModifiers() == Qt::AltModifier) {
      QList<ZIntPoint> posList;
      posList.append(bookmark->getCenter().roundToIntPoint());
      emit togglingBodiesAt(posList);
    }
  }
}


void ZFlyEmBookmarkView::setBookmarkModel(
    ZFlyEmBookmarkListModel *model)
{
  m_bookmarkModel = model;
  m_bookmarkModel->setUsed(true);
  resizeColumnsToContents();
  setSortingEnabled(true);
  setModel(model->getProxy());
  horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

  /*
  m_proxy = proxy;
  if (m_proxy != NULL) {
    setSortingEnabled(true);
    setModel(proxy);
  } else {
    setModel(m_bookmarkModel);
    setSortingEnabled(false);
  }
  */

  /*
  if (m_proxy == NULL) {
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
  }

  setModel(m_proxy);
  setSortingEnabled(true);
  m_proxy->setSourceModel(m_bookmarkModel);
  */
}

void ZFlyEmBookmarkView::createMenu()
{
  m_contextMenu = new QMenu(this);

  QAction *bodySelectionAction = new QAction("Select Bodies", this);
  m_contextMenu->addAction(bodySelectionAction);
  connect(bodySelectionAction, SIGNAL(triggered()),
          this, SLOT(selectBodyUnderSelectedBookmark()));
  m_contextMenu->addSeparator();

  QAction *checkAction = new QAction("Set Checked", this);
  m_contextMenu->addAction(checkAction);
  connect(checkAction, SIGNAL(triggered()), this, SLOT(checkCurrentBookmark()));

  QAction *unCheckAction = new QAction("Uncheck", this);
  m_contextMenu->addAction(unCheckAction);
  connect(unCheckAction, SIGNAL(triggered()),
          this, SLOT(uncheckCurrentBookmark()));

  if (m_enableDeletion) {
    QAction *deleteAction = new QAction("Delete Selected", this);
    m_contextMenu->addAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()),
            this, SLOT(deleteSelectedBookmark()));
  }

#ifdef _DEBUG_
  QAction *copyUrlAction = new QAction("Copy URL", this);
  m_contextMenu->addAction(copyUrlAction);
  connect(copyUrlAction, SIGNAL(triggered()),
          this, SLOT(copySelectedBookmarkUrl()));
#endif
}

void ZFlyEmBookmarkView::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef _DEBUG_2
  std::cout << "Context menu triggered." << std::endl;
#endif

  if (m_contextMenu == NULL) {
    createMenu();
  }

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
  return m_bookmarkModel;
//  return qobject_cast<ZFlyEmBookmarkListModel*>(model());
}

QSortFilterProxyModel* ZFlyEmBookmarkView::getProxy() const
{
  if (getModel() == NULL) {
    return NULL;
  }

  return getModel()->getProxy();
}

const ZFlyEmBookmark* ZFlyEmBookmarkView::getBookmark(
    const QModelIndex &viewIndex) const
{
  QModelIndex index = viewIndex;
  if (getProxy() != NULL) {
    index = getProxy()->mapToSource(viewIndex);
  }

  if (getModel() != NULL) {
    return getModel()->getBookmark(index.row());
  }

  return NULL;
}

void ZFlyEmBookmarkView::checkBookmark(ZFlyEmBookmark *bookmark, bool checking)
{
  if (bookmark != NULL) {
    bookmark->setChecked(checking);
    if (checking) {
      KINFO << bookmark->toLogString() + " is checked";
    } else {
      KINFO << bookmark->toLogString() + " is unchecked";
    }
  }
}

void ZFlyEmBookmarkView::checkCurrentBookmark(bool checking)
{
//  QItemSelectionModel *sel = selectionModel();
//  QItemSelection sourceSelection =
//      getProxy()->mapSelectionToSource(sel->selection());

//  QModelIndexList selected = sourceSelection.indexes();


  QModelIndexList selected = getModel()->getSelected(selectionModel());

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = getModel()->getBookmark(index.row());
    checkBookmark(bookmark, checking);

    getModel()->updateRow(index.row());

    emit bookmarkChecked(bookmark);
  }
}

QList<ZFlyEmBookmark*> ZFlyEmBookmarkView::getSelectedBookmark() const
{
  QItemSelectionModel *sel = selectionModel();
  QItemSelection sourceSelection =
      getProxy()->mapSelectionToSource(sel->selection());

  QModelIndexList selected = sourceSelection.indexes();

  QList<ZFlyEmBookmark*> bookmarkList;
  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = getModel()->getBookmark(index.row());

    bookmarkList.append(bookmark);
  }

  return bookmarkList;
}

void ZFlyEmBookmarkView::deleteSelectedBookmark()
{
  /*
  QItemSelectionModel *sel = selectionModel();
  QItemSelection sourceSelection =
      getProxy()->mapSelectionToSource(sel->selection());

  QModelIndexList selected = sourceSelection.indexes();

  QList<ZFlyEmBookmark*> bookmarkList;
  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = getModel()->getBookmark(index.row());

    bookmarkList.append(bookmark);
//    emit removingBookmark(bookmark);
//    getModel()->removeRow(index.row());
  }
  */

  QList<ZFlyEmBookmark*> bookmarkList = getSelectedBookmark();
  if (!bookmarkList.empty()) {
    emit removingBookmark(bookmarkList);
  }
}

void ZFlyEmBookmarkView::selectBodyUnderSelectedBookmark()
{
  QList<ZFlyEmBookmark*> bookmarkList = getSelectedBookmark();
  if (!bookmarkList.empty()) {
    QList<ZIntPoint> posList;
    foreach (ZFlyEmBookmark *bookmark, bookmarkList) {
      posList.append(bookmark->getCenter().roundToIntPoint());
    }
    emit selectingBodyAt(posList, true);
  }
}

void ZFlyEmBookmarkView::copySelectedBookmarkUrl()
{
  QList<ZFlyEmBookmark*> bookmarkList = getSelectedBookmark();
  if (!bookmarkList.empty()) {
    ZFlyEmBookmark *bookmark = bookmarkList.first();
    ZIntPoint center = bookmark->getLocation();
    emit copyingBookmarkUrl(center.getX(), center.getY(), center.getZ());
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

void ZFlyEmBookmarkView::sort()
{
  if (getModel() != NULL) {
    getModel()->sortTable();
  }
}
