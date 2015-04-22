#ifndef ZFLYEMBOOKMARKVIEW_H
#define ZFLYEMBOOKMARKVIEW_H

#include <QTableView>

class ZFlyEmBookmarkListModel;

class ZFlyEmBookmarkView : public QTableView
{
  Q_OBJECT
public:
  explicit ZFlyEmBookmarkView(QWidget *parent = 0);
  inline void setContextMenu(QMenu *menu) { m_contextMenu = menu; }

  ZFlyEmBookmarkListModel* getModel() const;

signals:

public slots:

protected:
  void contextMenuEvent(QContextMenuEvent *);
  void mousePressEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  QMenu *m_contextMenu;
};

#endif // ZFLYEMBOOKMARKVIEW_H
