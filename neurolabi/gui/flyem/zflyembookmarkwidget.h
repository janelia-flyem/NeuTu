#ifndef ZFLYEMBOOKMARKWIDGET_H
#define ZFLYEMBOOKMARKWIDGET_H

#include <QWidget>

class ZFlyEmBookmarkListModel;
class ZFlyEmBookmarkView;
class ZFlyEmBookmark;

namespace Ui {
class ZFlyEmBookmarkWidget;
}

class ZFlyEmBookmarkWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZFlyEmBookmarkWidget(QWidget *parent = 0);
  ~ZFlyEmBookmarkWidget();

  enum EBookmarkSource {
    SOURCE_ASSIGNED, SOURCE_USER
  };

  ZFlyEmBookmarkView* getBookmarkView(EBookmarkSource source);

  void setBookmarkModel(ZFlyEmBookmarkListModel *model, EBookmarkSource source);

signals:
  void locatingBookmark(ZFlyEmBookmark *bookmark);


private:
  Ui::ZFlyEmBookmarkWidget *ui;
};

#endif // ZFLYEMBOOKMARKWIDGET_H
