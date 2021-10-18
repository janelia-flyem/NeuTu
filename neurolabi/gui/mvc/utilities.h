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
QString ComposeViewInfo(ZStackView *view, const ZPoint &dataPos);
QString ComposeStackDataInfo(ZStackDoc *doc, const ZIntPoint &pos,
    neutu::mvc::ViewInfoFlags f);
//ZPoint MapWidgetPosToData(const ZStackView *view, const ZPoint &widgetPos);

} //namespace mvc

}
#endif // UTILITIES_H
