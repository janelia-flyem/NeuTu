#ifndef FLYEM_LOGGING_H
#define FLYEM_LOGGING_H

#include <QString>

#include "common/neutudefs.h"

namespace flyem {

void LogBodyOperation(
    const QString &action, uint64_t bodyId, neutu::EBodyLabelType labelType);

}

#endif // FLYEM_LOGGING_H
