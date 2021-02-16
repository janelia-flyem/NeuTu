#ifndef ZFLYEMBOOKMARKWIDGET_H
#define ZFLYEMBOOKMARKWIDGET_H

#include <QWidget>

class ZFlyEmBookmarkListModel;
class ZFlyEmBookmarkView;
class ZFlyEmBookmark;
class QSortFilterProxyModel;

namespace Ui {
class ZFlyEmBookmarkWidget;
}

class ZFlyEmBookmarkWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZFlyEmBookmarkWidget(QWidget *parent = 0);
  ~ZFlyEmBookmarkWidget();

  enum class EBookmarkSource {
    ASSIGNED, USER
  };

  ZFlyEmBookmarkView* getBookmarkView(EBookmarkSource source);

  void setBookmarkModel(ZFlyEmBookmarkListModel *model, EBookmarkSource source);  
  EBookmarkSource getCurrentSource();

signals:
  void locatingBookmark(ZFlyEmBookmark *bookmark);
  void importingUserBookmark();
  void exportingUserBookmark();
  void importingAssignedBookmark();
  void exportingAssignedBookmark();

private slots:
  void importBookmark();
  void exportBookmark();
  void onTabChanged(int index);

private:
  Ui::ZFlyEmBookmarkWidget *ui;
};

#endif // ZFLYEMBOOKMARKWIDGET_H
