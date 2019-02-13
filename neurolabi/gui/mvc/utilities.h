#ifndef MVC_UTILITIES_H
#define MVC_UTILITIES_H

#include <QString>
#include <QPoint>

#include "mvcdef.h"

class ZStackView;
class ZStackDoc;
class ZIntPoint;
class ZPoint;

namespace neutu {

namespace mvc {
QString ComposeViewInfo(ZStackView *view, const QPoint &widgetPos);
QString ComposeStackDataInfo(
    ZStackDoc *doc, double cx, double cy, int z, neutu::EAxis axis,
    neutu::mvc::ViewInfoFlags f);
ZPoint MapWidgetPosToData(const ZStackView *view, const ZPoint &widgetPos);

} //namespace mvc

}
#endif // UTILITIES_H
