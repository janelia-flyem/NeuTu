#include "utilities.h"

#include "zlog.h"

namespace neutu {

void LogUrlIO(const QString &action, const QString &url)
{
  KLOG << ZLog::Info() << ZLog::Tag("action", action.toStdString())
       << ZLog::Tag("url", url.toStdString());
}

}
