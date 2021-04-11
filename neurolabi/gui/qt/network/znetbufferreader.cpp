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

ZNetBufferReader::~ZNetBufferReader()
{
#ifdef _DEBUG_0
  std::cout << __func__ << std::endl;
#endif
}

void ZNetBufferReader::_init()
{
  m_eventLoop = new QEventLoop(this);
//  connect(this, &ZNetBufferReader::readingCanceled,
//          this, &ZNetBufferReader::cancelReading);
  connect(this, &ZNetBufferReader::readingDone,
          m_eventLoop, &QEventLoop::quit);
//  connect(this, &ZNetBufferReader::checkingStatus,
//          this, &ZNetBufferReader::waitForReading);
}

QTimer* ZNetBufferReader::getTimer()
{
  if (m_timer == nullptr) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ZNetBufferReader::handleTimeout);
  }

  return m_timer;
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
  if (m_networkReply) {
    m_networkReply->disconnect(this);
    m_networkReply->abort();
    m_networkReply->deleteLater();
    m_networkReply = nullptr;
  }
}

void ZNetBufferReader::abort()
{
  resetNetworkReply();
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
  for (const auto &p : m_header.toStdMap()) {
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

void ZNetBufferReader::readHead(const QString &url, int timeout)
{
  startReading();

  neutu::LogUrlIO("HEAD", url);

  resetNetworkReply();

  startRequestTimer(timeout);
  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connectNetworkReply();
  connect(m_networkReply, &QNetworkReply::readyRead,
          this, &ZNetBufferReader::readBuffer);

  waitForReading();
}

void ZNetBufferReader::readOptions(const QString &url, int timeout)
{
  startReading();

  neutu::LogUrlIO("OPTIONS", url);

  resetNetworkReply();

  startRequestTimer(timeout);
  m_networkReply = getNetworkAccessManager()->sendCustomRequest(
        QNetworkRequest(url), "OPTIONS");
  connectNetworkReply();
  connect(m_networkReply, &QNetworkReply::readyRead,
          this, &ZNetBufferReader::readBuffer);

  waitForReading();
}

bool ZNetBufferReader::hasHead(const QString &url, int timeout)
{
  startReading();

  neutu::LogUrlIO("HEAD", url);

  resetNetworkReply();
  startRequestTimer(timeout);
  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connectNetworkReply();

  waitForReading();

  return m_status == neutu::EReadStatus::OK;
}

bool ZNetBufferReader::hasOptions(const QString &url, int timeout)
{
  startReading();

  neutu::LogUrlIO("OPTIONS", url);

  resetNetworkReply();
  startRequestTimer(timeout);
  m_networkReply = getNetworkAccessManager()->sendCustomRequest(
        QNetworkRequest(url), "OPTIONS");
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
  for (const auto &p : m_header.toStdMap()) {
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

void ZNetBufferReader::startRequestTimer(int timeout)
{
  if (timeout > 0) {
    getTimer()->stop();
    getTimer()->start(timeout);
#ifdef _DEBUG_0
    std::cout << "Start timer." << std::endl;
#endif
  }
}

void ZNetBufferReader::startReading()
{
//  m_isReadingDone = false;
  cancelReading();
  waitForReading();

  clearBuffer();
  m_status = neutu::EReadStatus::INPROGRESS;
  m_statusCode = 0;
}

QMap<QByteArray, QByteArray> ZNetBufferReader::getResponseHeaderMap() const
{
  QMap<QByteArray, QByteArray> headerMap;
  for (const auto headerPair : m_responseHeader) {
    headerMap[headerPair.first] = headerPair.second;
  }

  return headerMap;
}

QByteArray ZNetBufferReader::getResponseHeader(
    const QByteArray &headerName) const
{
  for (const auto headerPair : m_responseHeader) {
    if (headerPair.first == headerName) {
      return headerPair.second;
    }
  }

  return QByteArray();
}

void ZNetBufferReader::endReading(neutu::EReadStatus status)
{
  if (!isReadingInproress()) { //No need to end reading when it's not in progress
    return;
  }

#ifdef _DEBUG_0
  std::cout << __func__ << ": " << neutu::EnumValue(status) << std::endl;
#endif

  m_status = status;

  if (m_timer) { //delete timer here explicitly to avoid thread confusion
    m_timer->stop();
    m_timer->deleteLater();
    m_timer = nullptr;
  }

//  m_isReadingDone = true;

  if (m_networkReply) {
    m_responseHeader = m_networkReply->rawHeaderPairs();

    if (m_status == neutu::EReadStatus::FINISHED) {
      QVariant statusCode = m_networkReply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute);

      m_statusCode = statusCode.toInt();
      if (m_statusCode < 200 || m_statusCode >= 300) {
        KWARN << QString("Status code: %1").arg(m_statusCode);
        m_status = neutu::EReadStatus::BAD_RESPONSE;
      } else if (m_statusCode == 204) {
        m_status = neutu::EReadStatus::NO_CONTENT;
      } else {
        m_status = neutu::EReadStatus::OK;
      }
    }

    m_networkReply->disconnect(this);
    m_networkReply->deleteLater();
    m_networkReply = nullptr;
  }

  emit readingDone();
}

bool ZNetBufferReader::isReadingDone() const
{
  return m_status != neutu::EReadStatus::INPROGRESS &&
      m_status != neutu::EReadStatus::NONE;
//  return m_isReadingDone;
}

bool ZNetBufferReader::isReadingInproress() const
{
  return m_status == neutu::EReadStatus::INPROGRESS;
}

void ZNetBufferReader::waitForReading()
{
  if (isReadingInproress()) {
    m_eventLoop->exec();
  }
}

void ZNetBufferReader::handleError(QNetworkReply::NetworkError /*error*/)
{
  if (m_networkReply != NULL) {
    KWARN << m_networkReply->errorString();
#ifdef _DEBUG_0
    std::cout << __func__ << ": " << m_networkReply->errorString().toStdString() << std::endl;
#endif
  }
  endReading(neutu::EReadStatus::FAILED);

#ifdef _DEBUG_0
  std::cout << __func__ << ": done" << std::endl;
#endif
}

void ZNetBufferReader::readBuffer()
{
  m_buffer.append(m_networkReply->readAll());
}

void ZNetBufferReader::readBufferPartial()
{
  m_buffer.append(m_networkReply->readAll());
  if (m_buffer.size() > m_maxSize) {
    if (m_networkReply) {
      m_networkReply->abort();
    }
//    endReading(m_status);
  }
}

void ZNetBufferReader::finishReading()
{
  endReading(neutu::EReadStatus::FINISHED);
}

void ZNetBufferReader::handleTimeout()
{
  resetNetworkReply();
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
  m_responseHeader.clear();
}

bool ZNetBufferReader::hasRequestHeader(const QString &key) const
{
  return m_header.count(key) > 0;
}

void ZNetBufferReader::removeRequestHeader(const QString &key)
{
  m_header.remove(key);
}

void ZNetBufferReader::setRequestHeader(const QString &key, const QString &value)
{
  m_header[key] =value;
}
