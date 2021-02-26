#include "znetworkutils.h"

#include <regex>
#include <map>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>

#include "neulib/core/utilities.h"

#include "znetbufferreaderthread.h"
#include "znetbufferreader.h"
#include "zjsonobject.h"

namespace {

static ZJsonObject read_json(std::string source)
{
  ZJsonObject obj;
  if (!source.empty()) {
//    ZNetBufferReader reader;
//    reader.read(source.c_str(), true);
    QByteArray buffer = ZNetworkUtils::Get(source.c_str());
    obj.decode(buffer.toStdString(), false);
  }

  return obj;
}

static auto read_json_memo = neulib::Memoize(read_json);
}


ZNetworkUtils::ZNetworkUtils()
{

}

bool ZNetworkUtils::HasHead(const QString &url, int timeout)
{
  if (url.isEmpty()) {
    return false;
  }

  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::HAS_HEAD);
  thread.setUrl(url);
  thread.start();
  if (thread.wait(timeout)) {
    return false;
  }

  return thread.getResultStatus();
}

QByteArray ZNetworkUtils::Options(const QString &url)
{
  if (url.isEmpty()) {
    return QByteArray();
  }

  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::READ_OPTIONS);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getData();
}

QByteArray ZNetworkUtils::OptionsHeader(
    const QString &url, const QByteArray &headerName)
{
  if (url.isEmpty()) {
    return QByteArray();
  }

  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::READ_OPTIONS);
  thread.setUrl(url);
  thread.start();
  thread.wait();

  return thread.getResponseHeader(headerName);
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

bool ZNetworkUtils::IsAvailable(
      const QString &url, znetwork::EOperation op, int timeout)
{
  ZNetBufferReaderThread *thread = new ZNetBufferReaderThread;
  thread->connect(
        thread, &ZNetBufferReaderThread::finished, thread, &QObject::deleteLater);
  thread->setOperation(op, timeout);
  thread->setUrl(url);
  thread->start();
  thread->wait();

  return thread->getResultStatus();
}

namespace {

const std::map<QString, znetwork::EOperation> ZNETWORK_OPERATION_MAP = {
  {"HEAD", znetwork::EOperation::READ_HEAD},
  {"GET", znetwork::EOperation::READ},
  {"POST", znetwork::EOperation::POST},
  {"OPTIONS", znetwork::EOperation::READ_OPTIONS},
  {"HAS_OPTIONS", znetwork::EOperation::HAS_OPTIONS},
  {"HAS_HEAD", znetwork::EOperation::HAS_HEAD}
};

znetwork::EOperation get_operation(const QString &method) {
  auto iter = ZNETWORK_OPERATION_MAP.find(method);
  if (iter != ZNETWORK_OPERATION_MAP.end()) {
    return iter->second;
  }

  return znetwork::EOperation::NONE;
}

}

bool ZNetworkUtils::IsAvailable(
    const QString &url, const QString &method, int timeout)
{
  return IsAvailable(url, get_operation(method), timeout);
}

/*
bool ZNetworkUtils::IsAvailable(
    const QString &url, const QByteArray &method, int timeout)
{
  if (method == "HEAD") {
    return IsAvailable(url, znetwork::EOperation::HAS_HEAD, timeout);
  } else if (method == "OPTIONS") {
    return IsAvailable(url, znetwork::EOperation::HAS_OPTIONS, timeout);
  }

  return IsAvailable(QNetworkRequest(QUrl(url)), method, timeout);
}

bool ZNetworkUtils::IsAvailable(
    const QNetworkRequest &request, const QByteArray &method, int timeout)
{
  QTimer timer;
  timer.setSingleShot(true);

  QNetworkAccessManager manager;
  QNetworkReply *reply = nullptr;

  if (method == "HEAD") {
      reply = manager.head(request);
  } else {
      reply = manager.sendCustomRequest(request, method);
  }

  QEventLoop loop;

  //FIXME: use a separate slot to indicate timeout
  loop.connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
  loop.connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
  timer.start(timeout);
  loop.exec();

  bool succ = false;
  if (timer.isActive()) {
    timer.stop();
    if(reply->error() == QNetworkReply::NoError) {
      int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
      succ = (statusCode >= 200 && statusCode < 300);
    }
  } else {
    reply->abort();
  }
  reply->deleteLater();

  return succ;
}
*/
