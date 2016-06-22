#include "zdvidbufferreader.h"

#include <exception>
#include <iostream>

#include <QTimer>
#include <QNetworkRequest>
#include <QDebug>
#include <QNetworkReply>

#include "libdvidheader.h"

#include "dvid/zdvidtarget.h"
#include "zsleeper.h"
#include "dvid/zdvidurl.h"
#include "dvid/libdvidheader.h"

ZDvidBufferReader::ZDvidBufferReader(QObject *parent) :
  QObject(parent), m_networkReply(NULL), m_isReadingDone(false),
  m_status(ZDvidBufferReader::READ_NULL), m_tryingCompress(false)
{
  m_networkManager = new QNetworkAccessManager(this);

//#if !defined(_ENABLE_LIBDVIDCPP_)
  m_eventLoop = new QEventLoop(this);
//#else
//  m_eventLoop = NULL;
//#endif

//  m_timer = new QTimer(this);
//  m_timer->setInterval(60000);

//  connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
  connect(this, SIGNAL(readingCanceled()), this, SLOT(cancelReading()));
  connect(this, SIGNAL(readingDone()), m_eventLoop, SLOT(quit()));
  connect(this, SIGNAL(checkingStatus()), this, SLOT(waitForReading()));
}

#if defined(_ENABLE_LIBDVIDCPP_)
void ZDvidBufferReader::setService(
    const ZSharedPointer<libdvid::DVIDNodeService> &service)
{
  m_service = service;
}
#endif

void ZDvidBufferReader::read(
    const QString &url, const QByteArray &payload, const std::string &method,
    bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
//  qDebug() << "Using libdvidcpp";

  ZDvidTarget target;
  target.setFromUrl(url.toStdString());

  if (target.isValid()) {
    try {
      std::string endPoint = ZDvidUrl::GetEndPoint(url.toStdString());
      libdvid::BinaryDataPtr libdvidPayload =
          libdvid::BinaryData::create_binary_data(payload.data(), payload.length());
      libdvid::BinaryDataPtr data;

      libdvid::ConnectionMethod connMeth = libdvid::GET;
      if (method == "POST") {
        connMeth = libdvid::POST;
      } else if (method == "PUT") {
        connMeth = libdvid::PUT;
      }
      if (m_service.get() != NULL) {
        data = m_service->custom_request(
              endPoint, libdvidPayload, connMeth, m_tryingCompress);
      } else {
        libdvid::DVIDNodeService service(
              target.getAddressWithPort(), target.getUuid());
        data = service.custom_request(
            endPoint, libdvidPayload, connMeth, m_tryingCompress);
      }

      m_buffer.append(data->get_data().c_str(), data->length());
      m_status = READ_OK;
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      m_status = READ_FAILED;
    }
  }
#endif
}

void ZDvidBufferReader::read(const QString &url, bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
//  qDebug() << "Using libdvidcpp";

  ZDvidTarget target;
  target.setFromUrl(url.toStdString());

  if (target.isValid()) {
    try {
      libdvid::BinaryDataPtr data;
      std::string endPoint = ZDvidUrl::GetEndPoint(url.toStdString());
      if (m_service.get() != NULL) {
        data = m_service->custom_request(
              endPoint, libdvid::BinaryDataPtr(), libdvid::GET, m_tryingCompress);
      } else {
        libdvid::DVIDNodeService service(
              target.getAddressWithPort(), target.getUuid());
        data = service.custom_request(
              endPoint, libdvid::BinaryDataPtr(), libdvid::GET, m_tryingCompress);
      }

      m_buffer.append(data->get_data().c_str(), data->length());
      m_status = READ_OK;
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      m_status = READ_FAILED;
    }
  } else {
    startReading();

    m_networkReply = m_networkManager->get(QNetworkRequest(url));
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleError(QNetworkReply::NetworkError)));

    waitForReading();
  }

#else
  startReading();

  m_networkReply = m_networkManager->get(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
#endif
}

void ZDvidBufferReader::readQt(const QString &url, bool outputUrl)
{
  if (outputUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

  startReading();

  m_networkReply = m_networkManager->get(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
}

bool ZDvidBufferReader::isReadable(const QString &url)
{
  QTimer::singleShot(5000, this, SLOT(handleTimeout()));

  startReading();

  qDebug() << url;

  m_networkReply = m_networkManager->get(QNetworkRequest(url));

  //return m_networkReply->error() == QNetworkReply::NoError;
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();

  return m_status == READ_OK;
}

bool ZDvidBufferReader::hasHead(const QString &url)
{
  startReading();

  qDebug() << url;

  m_networkReply = m_networkManager->head(QNetworkRequest(url));

  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();

  return m_status == READ_OK;
}

void ZDvidBufferReader::readHead(const QString &url)
{
  startReading();

  qDebug() << url;

  m_networkReply = m_networkManager->head(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
}

void ZDvidBufferReader::startReading()
{
  m_isReadingDone = false;
  m_buffer.clear();
  m_status = READ_OK;
}

bool ZDvidBufferReader::isReadingDone() const
{
  return m_isReadingDone;
}

void ZDvidBufferReader::waitForReading()
{
      /*
  if (m_isReadingDone) {
    return;
  }

  ZSleeper::msleep(10);
  emit checkingStatus();

  */


  if (!isReadingDone()) {
    m_eventLoop->exec();
  }

}

void ZDvidBufferReader::handleError(QNetworkReply::NetworkError /*error*/)
{
  if (m_networkReply != NULL) {
    qDebug() << m_networkReply->errorString();
  }
  endReading(READ_FAILED);
}

void ZDvidBufferReader::readBuffer()
{
  m_buffer.append(m_networkReply->readAll());
}

void ZDvidBufferReader::finishReading()
{
  endReading(m_status);
}

void ZDvidBufferReader::handleTimeout()
{
  endReading(READ_TIMEOUT);
}

void ZDvidBufferReader::cancelReading()
{
  endReading(READ_CANCELED);
}

void ZDvidBufferReader::endReading(EStatus status)
{
  m_status = status;
  m_isReadingDone = true;

  if (m_networkReply != NULL) {
    QVariant statusCode = m_networkReply->attribute(
          QNetworkRequest::HttpStatusCodeAttribute);
#ifdef _DEBUG_
    qDebug() << "Status code: " << statusCode;
#endif
    if (statusCode.toInt() != 200) {
      m_status = READ_BAD_RESPONSE;
    }
    m_networkReply->deleteLater();
    m_networkReply = NULL;
  }

  emit readingDone();
}

ZDvidBufferReader::EStatus ZDvidBufferReader::getStatus() const
{
  return m_status;
}
