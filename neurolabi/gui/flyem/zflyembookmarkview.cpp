#include "zflyembookmarkview.h"

#include <QMenu>
#include <QContextMenuEvent>

#include "zflyembookmarklistmodel.h"

ZFlyEmBookmarkView::ZFlyEmBookmarkView(QWidget *parent) :
  QTableView(parent), m_contextMenu(NULL)
{
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

