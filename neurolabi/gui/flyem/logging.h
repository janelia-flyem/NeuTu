#ifndef FLYEM_LOGGING_H
#define FLYEM_LOGGING_H

#include <QString>

#include "common/neutube_def.h"

namespace flyem {

void LogBodyOperation(const QString &action, uint64_t bodyId, flyem::EBodyLabelType);

}

#endif // FLYEM_LOGGING_H
