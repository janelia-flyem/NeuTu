#include "znetworkutils.h"

#include "znetbufferreaderthread.h"
#include "znetbufferreader.h"

ZNetworkUtils::ZNetworkUtils()
{

}

bool ZNetworkUtils::HasHead(const QString &url)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(ZNetBufferReaderThread::EOperation::HAS_HEAD);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getResultStatus();
}
