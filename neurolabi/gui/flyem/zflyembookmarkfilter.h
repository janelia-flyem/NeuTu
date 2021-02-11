#ifndef ZFLYEMBOOKMARKFILTER_H
#define ZFLYEMBOOKMARKFILTER_H

#include <functional>
#include <QWidget>

class ZFlyEmBookmarkWidget;

namespace Ui {
class ZFlyEmBookmarkFilter;
}

class ZFlyEmBookmarkFilter : public QWidget
{
    Q_OBJECT

public:
    explicit ZFlyEmBookmarkFilter(QWidget *parent = 0);
    ~ZFlyEmBookmarkFilter();

public slots:
    void onTabChanged(int index);

private slots:
    void filterUpdated(QString text);

private:
    Ui::ZFlyEmBookmarkFilter *ui;
    ZFlyEmBookmarkWidget * m_bookmarkWidget;
    std::function<void(QString)> m_debounceFilterUpdated;
    bool m_retiring = false;
};

#endif // ZFLYEMBOOKMARKFILTER_H
