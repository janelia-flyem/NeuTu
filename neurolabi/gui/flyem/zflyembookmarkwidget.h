#ifndef ZFLYEMBOOKMARKWIDGET_H
#define ZFLYEMBOOKMARKWIDGET_H

#include <QWidget>

namespace Ui {
class ZFlyEmBookmarkWidget;
}

class ZFlyEmBookmarkWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZFlyEmBookmarkWidget(QWidget *parent = 0);
  ~ZFlyEmBookmarkWidget();

private:
  Ui::ZFlyEmBookmarkWidget *ui;
};

#endif // ZFLYEMBOOKMARKWIDGET_H
