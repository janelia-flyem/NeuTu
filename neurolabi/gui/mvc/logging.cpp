#include "logging.h"

#include "common/utilities.h"
#include "logging/zlog.h"
#include "zstackobject.h"

void neutu::LogObjectOperation(const QString &action, const ZStackObject *obj)
{
  if (obj) {
    KLOG << ZLog::Info()
         << ZLog::Action(action.toStdString())
         << ZLog::Object("zstackobject", obj->getTypeName(), obj->getSource())
         << ZLog::Level(3);
  }
}
