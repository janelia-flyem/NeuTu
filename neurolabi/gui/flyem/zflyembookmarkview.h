#ifndef ZFLYEMBOOKMARKVIEW_H
#define ZFLYEMBOOKMARKVIEW_H

#include <QTableView>
#include <QString>

class ZFlyEmBookmarkListModel;
class ZFlyEmBookmark;
//class QSortFilterProxyModel;

class ZFlyEmBookmarkView : public QTableView
{
  Q_OBJECT
public:
  explicit ZFlyEmBookmarkView(QWidget *parent = 0);

  ZFlyEmBookmarkListModel* getModel() const;

  void checkCurrentBookmark(bool checking);

signals:
  void bookmarkChecked(QString key, bool checking);
  void bookmarkChecked(ZFlyEmBookmark*);

public slots:
  void checkCurrentBookmark();
  void uncheckCurrentBookmark();

private:
  void init();
  void createMenu();

protected:
  void contextMenuEvent(QContextMenuEvent *);
  void mousePressEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QMenu *m_contextMenu;
//  QSortFilterProxyModel* m_proxy;
};

#endif // ZFLYEMBOOKMARKVIEW_H
