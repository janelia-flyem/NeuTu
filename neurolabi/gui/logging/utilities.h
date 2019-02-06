#ifndef _LOGGING_UTILITIES_H
#define _LOGGING_UTILITIES_H

#include <QString>

class ZWidgetMessage;

namespace neutu {
void LogUrlIO(const QString &action, const QString &url);
void LogMessage(const ZWidgetMessage &msg);
}

#endif // UTILITIES_H
