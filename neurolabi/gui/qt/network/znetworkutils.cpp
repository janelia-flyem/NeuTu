#include "znetworkutils.h"

#include "znetbufferreaderthread.h"
#include "znetbufferreader.h"

ZNetworkUtils::ZNetworkUtils()
{

}

bool ZNetworkUtils::HasHead(const QString &url)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::HAS_HEAD);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getResultStatus();
}

QByteArray ZNetworkUtils::Get(const QString &url)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::READ);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getData();
}

QByteArray ZNetworkUtils::Post(
    const QString &url, const QByteArray &payload)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::POST);
  thread.setPayload(payload);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getData();
}
