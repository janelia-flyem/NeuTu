#ifndef _LOGGING_UTILITIES_H
#define _LOGGING_UTILITIES_H

#include <cstdint>
#include <string>

#include <QString>
#include <QByteArray>

#include "common/neutudefs.h"

class ZWidgetMessage;

namespace neutu {
void LogUrlIO(const QString &action, const QString &url);
void LogUrlIO(
    const QString &action, const QString &url, const QByteArray &payload);
void LogMessage(const ZWidgetMessage &msg);
void LogProfileInfo(int64_t duration, const std::string &info);
void LogMessageF(const std::string &str, EMessageType type);
//void LogBodyOperation(
//    const QString &action, uint64_t bodyId, neutu::EBodyLabelType labelType);
}

#endif // UTILITIES_H
