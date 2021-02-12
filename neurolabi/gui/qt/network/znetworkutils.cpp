#include "znetworkutils.h"

#include <regex>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>

#include "http/HTTPRequest.hpp"
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
