#ifndef _LOGGING_UTILITIES_H
#define _LOGGING_UTILITIES_H

#include <QString>

#include "common/neutube_def.h"

class ZWidgetMessage;

namespace neutu {
void LogUrlIO(const QString &action, const QString &url);
void LogMessage(const ZWidgetMessage &msg);
//void LogBodyOperation(
//    const QString &action, uint64_t bodyId, neutu::EBodyLabelType labelType);
}

#endif // UTILITIES_H
