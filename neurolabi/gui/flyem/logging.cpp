#include "logging.h"

#include "logging/zlog.h"

void flyem::LogBodyOperation(
    const QString &action, uint64_t bodyId, neutu::EBodyLabelType labelType)
{
  KLOG << ZLog::Info()
       << ZLog::Action(action.toStdString())
       << ZLog::Object(neutu::ToString(labelType), "", std::to_string(bodyId));
}
