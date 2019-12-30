#ifndef ZFLYEMBOOKMARKVIEW_H
#define ZFLYEMBOOKMARKVIEW_H

#include <QTableView>
#include <QString>

class ZFlyEmBookmarkListModel;
class ZFlyEmBookmark;
class QSortFilterProxyModel;

class ZFlyEmBookmarkView : public QTableView
{
  Q_OBJECT
public:
  explicit ZFlyEmBookmarkView(QWidget *parent = 0);

  ZFlyEmBookmarkListModel* getModel() const;

  void setBookmarkModel(ZFlyEmBookmarkListModel *model);
//  void setProxy(QSortFilterProxyModel *proxy);
  void checkCurrentBookmark(bool checking);

  const ZFlyEmBookmark* getBookmark(const QModelIndex &viewIndex) const;
  QSortFilterProxyModel* getProxy() const;

  void sort();

  void enableDeletion(bool on) {
    m_enableDeletion = on;
  }

signals:
  void bookmarkChecked(QString key, bool checking);
  void bookmarkChecked(ZFlyEmBookmark*);
  void locatingBookmark(const ZFlyEmBookmark*);
  void removingBookmark(ZFlyEmBookmark*);
  void removingBookmark(QList<ZFlyEmBookmark*> bookmarkList);
  void copyingBookmarkUrl(int x, int y, int z);

public slots:
  void checkCurrentBookmark();
  void uncheckCurrentBookmark();
  void deleteSelectedBookmark();

private slots:
  void processDouleClick(const QModelIndex &index);
  void processSingleClick(const QModelIndex &index);
  void copySelectedBookmarkUrl();

private:
  void init();
  void createMenu();
  void connectSignalSlot();
  void checkBookmark(ZFlyEmBookmark *bookmark, bool checking);
  QList<ZFlyEmBookmark*> getSelectedBookmark() const;

protected:
  void contextMenuEvent(QContextMenuEvent *);
  void mousePressEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QMenu *m_contextMenu;
  ZFlyEmBookmarkListModel *m_bookmarkModel;
//  QSortFilterProxyModel* m_proxy;
  bool m_enableDeletion;
};

#endif // ZFLYEMBOOKMARKVIEW_H
