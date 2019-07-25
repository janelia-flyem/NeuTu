#include "znetbufferreader.h"

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

#include "logging/utilities.h"
#include "logging/zlog.h"

ZNetBufferReader::ZNetBufferReader(QObject *parent) : QObject(parent)
{
  _init();
}

void ZNetBufferReader::_init()
{
  m_eventLoop = new QEventLoop(this);
  connect(this, &ZNetBufferReader::readingCanceled,
          this, &ZNetBufferReader::cancelReading);
  connect(this, &ZNetBufferReader::readingDone,
          m_eventLoop, &QEventLoop::quit);
  connect(this, &ZNetBufferReader::checkingStatus,
          this, &ZNetBufferReader::waitForReading);
}

QNetworkAccessManager* ZNetBufferReader::getNetworkAccessManager()
{
   if (m_networkManager == nullptr) {
     m_networkManager = new QNetworkAccessManager(this);
   }

   return m_networkManager;
 }

void ZNetBufferReader::resetNetworkReply()
{
  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }
}

void ZNetBufferReader::connectNetworkReply()
{
  connect(m_networkReply, &QNetworkReply::finished,
          this, &ZNetBufferReader::finishReading);
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void ZNetBufferReader::read(const QString &url, bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << url;
  }

  neutu::LogUrlIO("GET", url);
//  KINFO << "Reading " + url;

  startReading();

  resetNetworkReply();
  QNetworkRequest req = QNetworkRequest(url);
  foreach (const auto &p, m_header) {
    req.setRawHeader(p.first.toUtf8(), p.second.toUtf8());
  }
  m_networkReply = getNetworkAccessManager()->get(req);
  connectNetworkReply();
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));

  waitForReading();
}

void ZNetBufferReader::readPartial(
    const QString &url, int maxSize, bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << url;
  }

  neutu::LogUrlIO("GETP", url);
//  KINFO << "Reading partial: " + url;

  startReading();

  m_maxSize = maxSize;

  resetNetworkReply();

  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
  connectNetworkReply();
  connect(m_networkReply, &QNetworkReply::readyRead,
          this, &ZNetBufferReader::readBufferPartial);


  waitForReading();
}

void ZNetBufferReader::readHead(const QString &url)
{
  startReading();

//  qDebug() << url;
  neutu::LogUrlIO("HEAD", url);
//  KINFO << "HEAD " + url;

  resetNetworkReply();

  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connectNetworkReply();
  connect(m_networkReply, &QNetworkReply::readyRead,
          this, &ZNetBufferReader::readBuffer);

  waitForReading();
}

bool ZNetBufferReader::hasHead(const QString &url)
{
  startReading();

//  qDebug() << url;
  neutu::LogUrlIO("HEAD", url);

  resetNetworkReply();
  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connectNetworkReply();

  waitForReading();

  return m_status == neutu::EReadStatus::OK;
}

void ZNetBufferReader::post(const QString &url, const QByteArray &data)
{
//  KINFO << "POST " + url;
  neutu::LogUrlIO("POST", url);

  startReading();
  QNetworkRequest request(url);
  QString headInfo;
  foreach (const auto &p, m_header) {
    request.setRawHeader(p.first.toUtf8(), p.second.toUtf8());
    headInfo += p.first + ":" + p.second.left(10) + "; ";
  }
  if (!headInfo.isEmpty()) {
    KINFO << "HEADER:" << headInfo;
  }

  resetNetworkReply();
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  m_networkReply = getNetworkAccessManager()->post(request, data);
  connectNetworkReply();
  connect(m_networkReply, &QNetworkReply::readyRead,
          this, &ZNetBufferReader::readBuffer);

  waitForReading();
}

bool ZNetBufferReader::isReadable(const QString &url)
{
  QTimer::singleShot(15000, this, SLOT(handleTimeout()));

  startReading();

//  KINFO << "ZNetBufferReader::isReadable:" << url;

//  KINFO << "Check readable: " + url;
  neutu::LogUrlIO("Check readable", url);

  resetNetworkReply();
  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
  connectNetworkReply();
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(finishReading()));

  waitForReading();

  return m_status == neutu::EReadStatus::OK;
}

void ZNetBufferReader::startReading()
{
  m_isReadingDone = false;
  m_buffer.clear();
  m_status = neutu::EReadStatus::OK;
}

void ZNetBufferReader::endReading(neutu::EReadStatus status)
{
  m_status = status;
  m_isReadingDone = true;

  if (m_networkReply != NULL) {
    QVariant statusCode = m_networkReply->attribute(
          QNetworkRequest::HttpStatusCodeAttribute);

    m_statusCode = statusCode.toInt();
    if (m_statusCode != 200) {
      KWARN << QString("Status code: %1").arg(m_statusCode);
      m_status = neutu::EReadStatus::BAD_RESPONSE;
    }
    m_networkReply->deleteLater();
    m_networkReply = NULL;
  }

  emit readingDone();
}

bool ZNetBufferReader::isReadingDone() const
{
  return m_isReadingDone;
}

void ZNetBufferReader::waitForReading()
{
  if (!isReadingDone()) {
    m_eventLoop->exec();
  }
}

void ZNetBufferReader::handleError(QNetworkReply::NetworkError /*error*/)
{
  if (m_networkReply != NULL) {
    KWARN << m_networkReply->errorString();
  }
  endReading(neutu::EReadStatus::FAILED);
}

void ZNetBufferReader::readBuffer()
{
  m_buffer.append(m_networkReply->readAll());
}

void ZNetBufferReader::readBufferPartial()
{
  m_buffer.append(m_networkReply->readAll());
  if (m_buffer.size() > m_maxSize) {
    endReading(m_status);
  }
}

void ZNetBufferReader::finishReading()
{
  endReading(m_status);
}

void ZNetBufferReader::handleTimeout()
{
  endReading(neutu::EReadStatus::TIMEOUT);
}

void ZNetBufferReader::cancelReading()
{
  endReading(neutu::EReadStatus::CANCELED);
}

neutu::EReadStatus ZNetBufferReader::getStatus() const
{
  return m_status;
}

void ZNetBufferReader::clearBuffer()
{
  m_buffer.clear();
}

bool ZNetBufferReader::hasRequestHeader(const QString &key) const
{
  return m_header.count(key) > 0;
}

void ZNetBufferReader::setRequestHeader(const QString &key, const QString &value)
{
  m_header[key] =value;
}
