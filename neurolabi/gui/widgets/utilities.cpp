#include "utilities.h"

#include <QWidget>

namespace zwidget {

void HideLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        QWidget * widget = nullptr;
        QLayout *sublayout = nullptr;

        if ((sublayout = item->layout())) {
            HideLayout(sublayout);
        } else if ((widget = item->widget())) {
            widget->hide(); /*delete widget;*/
        } else {
            delete item;
        }
    }
}

}
