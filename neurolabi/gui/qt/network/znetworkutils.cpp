#include "znetworkutils.h"

#include <regex>

#include "neulib/core/utilities.h"

#include "znetbufferreaderthread.h"
#include "znetbufferreader.h"
#include "zjsonobject.h"

namespace {

static ZJsonObject read_json(std::string source)
{
  ZJsonObject obj;
  if (!source.empty()) {
    ZNetBufferReader reader;
    reader.read(source.c_str(), true);
    obj.decode(reader.getBuffer().toStdString(), false);
  }

  return obj;
}

static auto read_json_memo = neulib::Memoize(read_json);
}


ZNetworkUtils::ZNetworkUtils()
{

}

bool ZNetworkUtils::HasHead(const QString &url)
{
  if (url.isEmpty()) {
    return false;
  }

  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::HAS_HEAD);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getResultStatus();
}

QByteArray ZNetworkUtils::Get(const QString &url)
{
  if (url.isEmpty()) {
    return QByteArray();
  }

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
  if (url.isEmpty()) {
    return QByteArray();
  }

  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::POST);
  thread.setPayload(payload);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getData();
}

ZJsonObject ZNetworkUtils::ReadJsonObjectMemo(const std::string& url)
{
  return read_json_memo(url);
}
