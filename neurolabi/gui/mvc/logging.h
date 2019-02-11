#ifndef MVC_LOGGING_H
#define MVC_LOGGING_H

#include <QString>

class ZStackObject;

namespace neutu {

void LogObjectOperation(const QString &action, const ZStackObject *obj);

}

#endif // LOGGING_H
