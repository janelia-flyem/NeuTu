#include "zflyembookmarkfilter.h"
#include "ui_zflyembookmarkfilter.h"

#include <QSortFilterProxyModel>

#include "zflyembookmarkview.h"
#include "zflyembookmarkwidget.h"

ZFlyEmBookmarkFilter::ZFlyEmBookmarkFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZFlyEmBookmarkFilter)
{
    ui->setupUi(this);

    // this is a little squirrely, but easier than the alternatives
    m_bookmarkWidget = (ZFlyEmBookmarkWidget *) parent;

    connect(ui->clearButton, SIGNAL(clicked(bool)), ui->filterEdit, SLOT(clear()));
    connect(ui->filterEdit, SIGNAL(textChanged(QString)), this, SLOT(filterUpdated(QString)));
}

void ZFlyEmBookmarkFilter::onTabChanged(int /*index*/) {
    // update filter when tab changes (because you're filtering a different list)
    filterUpdated(ui->filterEdit->text());
}

void ZFlyEmBookmarkFilter::filterUpdated(QString text) {
    ZFlyEmBookmarkView * view = m_bookmarkWidget->getBookmarkView(m_bookmarkWidget->getCurrentSource());
    QSortFilterProxyModel * proxy = view->getProxy();

    // at this point you have options; you could do a regex filter (could even add a check box
    //  to enable/disable that); could implement a simpler wildcard filter; could parse the
    //  string and define a mini-language that let you eg filter on specific fields (columns)
    //  (not sure how hard that would be)

    // as a first try, do a simple text filter on all columns:
    proxy->setFilterFixedString(text);

}

ZFlyEmBookmarkFilter::~ZFlyEmBookmarkFilter()
{
    delete ui;
}
