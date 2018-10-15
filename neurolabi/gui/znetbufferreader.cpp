#include "znetbufferreader.h"

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

ZNetBufferReader::ZNetBufferReader(QObject *parent) : QObject(parent)
{
  _init();
}

void ZNetBufferReader::_init()
{
  m_networkManager = new QNetworkAccessManager(this);

  m_eventLoop = new QEventLoop(this);
  connect(this, &ZNetBufferReader::readingCanceled,
          this, &ZNetBufferReader::cancelReading);
  connect(this, &ZNetBufferReader::readingDone,
          m_eventLoop, &QEventLoop::quit);
  connect(this, &ZNetBufferReader::checkingStatus,
          this, &ZNetBufferReader::waitForReading);
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

  startReading();

  resetNetworkReply();
  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
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

  qDebug() << url;

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

  qDebug() << url;

  resetNetworkReply();
  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connectNetworkReply();

  waitForReading();

  return m_status == neutube::EReadStatus::OK;
}

void ZNetBufferReader::post(const QString &url, const QByteArray &data)
{
  startReading();
  QNetworkRequest request(url);
  resetNetworkReply();
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  m_networkReply = m_networkManager->post(request, data);
  connectNetworkReply();
  waitForReading();
}

bool ZNetBufferReader::isReadable(const QString &url)
{
  QTimer::singleShot(15000, this, SLOT(handleTimeout()));

  startReading();

  qDebug() << url;


  resetNetworkReply();
  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
  connectNetworkReply();
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(finishReading()));

  waitForReading();

  return m_status == neutube::EReadStatus::OK;
}

void ZNetBufferReader::startReading()
{
  m_isReadingDone = false;
  m_buffer.clear();
  m_status = neutube::EReadStatus::OK;
}

void ZNetBufferReader::endReading(neutube::EReadStatus status)
{
  m_status = status;
  m_isReadingDone = true;

  if (m_networkReply != NULL) {
    QVariant statusCode = m_networkReply->attribute(
          QNetworkRequest::HttpStatusCodeAttribute);
#ifdef _DEBUG_
    qDebug() << "Status code: " << statusCode;
#endif
    m_statusCode = statusCode.toInt();
    if (m_statusCode != 200) {
      m_status = neutube::EReadStatus::BAD_RESPONSE;
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
    qDebug() << m_networkReply->errorString();
  }
  endReading(neutube::EReadStatus::FAILED);
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
  endReading(neutube::EReadStatus::TIMEOUT);
}

void ZNetBufferReader::cancelReading()
{
  endReading(neutube::EReadStatus::CANCELED);
}

neutube::EReadStatus ZNetBufferReader::getStatus() const
{
  return m_status;
}

void ZNetBufferReader::clearBuffer()
{
  m_buffer.clear();
}

