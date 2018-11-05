#include "zdvidbufferreader.h"

#include <exception>
#include <iostream>

//#include <QTimer>
//#include <QNetworkRequest>
#include <QDebug>
//#include <QNetworkReply>
//#include <QCoreApplication>

#include "libdvidheader.h"

#include "dvid/zdvidtarget.h"
#include "zsleeper.h"
#include "dvid/zdvidurl.h"
#include "dvid/libdvidheader.h"
#include "flyem/zflyemmisc.h"
#include "zdvidutil.h"
#include "znetbufferreader.h"

ZDvidBufferReader::ZDvidBufferReader()
{
  _init();
}

void ZDvidBufferReader::_init()
{
//  m_networkReply = NULL;
  m_isReadingDone = false;
  m_status = neutube::EReadStatus::NONE;
  m_tryingCompress = false;

  //Make sure that the reader is processed in the main event loop
//  moveToThread(QCoreApplication::instance()->thread());

//  m_eventLoop = new QEventLoop(this);
//  m_eventLoop->moveToThread(QCoreApplication::instance()->thread());
//  m_eventLoop->setParent(this);

//#if !defined(_ENABLE_LIBDVIDCPP_)

//#else
//  m_eventLoop = NULL;
//#endif

//  m_timer = new QTimer(this);
//  m_timer->setInterval(60000);

//  connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
//  connect(this, SIGNAL(readingCanceled()), this, SLOT(cancelReading()));
//  connect(this, SIGNAL(readingDone()), m_eventLoop, SLOT(quit()));
//  connect(this, SIGNAL(checkingStatus()), this, SLOT(waitForReading()));

  m_maxSize = 0;


}

#if defined(_ENABLE_LIBDVIDCPP_)
void ZDvidBufferReader::setService(
    const ZSharedPointer<libdvid::DVIDNodeService> &service)
{
  m_service = service;
}

#if 0
QNetworkAccessManager* ZDvidBufferReader::getNetworkAccessManager()
{
  if (m_networkManager == NULL) {
    m_networkManager = new QNetworkAccessManager(this);
//    m_networkManager->moveToThread(QCoreApplication::instance()->thread());
//    m_networkManager->setParent(this);
  }

  return m_networkManager;
}
#endif

void ZDvidBufferReader::setService(const ZDvidTarget &target)
{
  m_service = ZDvid::MakeDvidNodeService(target);
}

#endif

void ZDvidBufferReader::clearBuffer()
{
  m_buffer.clear();
}

void ZDvidBufferReader::clear()
{
  m_service.reset();
  m_statusCode = 0;
  clearBuffer();
}

void ZDvidBufferReader::read(
    const QString &url, const QByteArray &payload, const std::string &method,
    bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << "Reading " << url;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
//  qDebug() << "Using libdvidcpp";

  ZDvidTarget target;
  target.setFromUrl(url.toStdString());

  if (target.isValid()) {
    try {
      std::string endPoint = ZDvidUrl::GetPath(url.toStdString());
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
#if 0
        libdvid::DVIDNodeService service(
              target.getAddressWithPort(), target.getUuid());
#endif
        ZSharedPointer<libdvid::DVIDNodeService> service =
            ZDvid::MakeDvidNodeService(target);
        data = service->custom_request(
            endPoint, libdvidPayload, connMeth, m_tryingCompress);
      }

      m_buffer.append(data->get_data().c_str(), data->length());
      m_status = neutube::EReadStatus::OK;
      m_statusCode = 200;
    } catch (libdvid::DVIDException &e) {
      STD_COUT << e.what() << std::endl;
      m_status = neutube::EReadStatus::FAILED;
      m_statusCode = e.getStatus();
    }
  }
#endif
}

void ZDvidBufferReader::readFromPath(const QString &path, bool outputingUrl)
{
  m_statusCode = 0;

  if (path.isEmpty()) {
    return;
  }

  if (outputingUrl) {
    qDebug() << path;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    libdvid::BinaryDataPtr data;
    if (m_service.get() != NULL) {
      data = m_service->custom_request(
            path.toStdString(), libdvid::BinaryDataPtr(),
            libdvid::GET, m_tryingCompress);
    }

    m_buffer.append(data->get_data().c_str(), data->length());
    m_status = neutube::EReadStatus::OK;
    m_statusCode = 200;
  } catch (libdvid::DVIDException &e) {
    STD_COUT << e.what() << std::endl;
    m_statusCode = e.getStatus();
    m_status = neutube::EReadStatus::FAILED;
  }
#endif
}

void ZDvidBufferReader::read(const QString &url, bool outputingUrl)
{
  m_statusCode = 0;

  if (url.isEmpty()) {
    return;
  }

  if (outputingUrl) {
    qDebug() << "Reading" << url;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvidTarget target;
  target.setFromUrl(url.toStdString());

  if (target.isValid()) {
    try {
      libdvid::BinaryDataPtr data;
      std::string endPoint = ZDvidUrl::GetPath(url.toStdString());
      if (m_service.get() != NULL) {
        data = m_service->custom_request(
              endPoint, libdvid::BinaryDataPtr(), libdvid::GET, m_tryingCompress);
      } else {
        ZSharedPointer<libdvid::DVIDNodeService> service =
            ZDvid::MakeDvidNodeService(target);
        data = service->custom_request(
              endPoint, libdvid::BinaryDataPtr(), libdvid::GET, m_tryingCompress);
      }
      qDebug() << "Reading done:" << url;

      m_buffer.append(data->get_data().c_str(), data->length());
      m_status = neutube::EReadStatus::OK;
      m_statusCode = 200;
    } catch (libdvid::DVIDException &e) {
      STD_COUT << "Exception: " << e.what() << std::endl;
      m_statusCode = e.getStatus();
      m_status = neutube::EReadStatus::FAILED;
    } catch (std::exception &e) {
      STD_COUT << "Any exception: " << e.what() << std::endl;
      m_statusCode = 0;
      m_status = neutube::EReadStatus::FAILED;
    }
  } else {
#if 0
    startReading();

    if (m_networkReply != NULL) {
      m_networkReply->disconnect();
      m_networkReply->deleteLater();
    }

    m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(handleError(QNetworkReply::NetworkError)));

    waitForReading();
#endif

    ZNetBufferReader bufferReader;
    bufferReader.read(url, false);
    m_buffer = bufferReader.getBuffer();
  }

#else
  startReading();

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }


  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
#endif

#ifdef _DEBUG_
  STD_COUT << "Exiting " << "ZDvidBufferReader::read" << std::endl;
#endif
}

#if 0
void ZDvidBufferReader::readPartial(
    const QString &url, int maxSize, bool outputingUrl)
{
  if (outputingUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

  startReading();

  m_maxSize = maxSize;

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }

  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBufferPartial()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
}
#endif

/*
void ZDvidBufferReader::readQt(const QString &url, bool outputUrl)
{
  if (outputUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

  startReading();

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }

  m_networkReply = m_networkManager->get(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
}
*/

#if 0
bool ZDvidBufferReader::isReadable(const QString &url)
{
  QTimer::singleShot(15000, this, SLOT(handleTimeout()));

  startReading();

  qDebug() << url;

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }

  m_networkReply = getNetworkAccessManager()->get(QNetworkRequest(url));

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

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }

  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));

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

  if (m_networkReply != NULL) {
    m_networkReply->disconnect();
    m_networkReply->deleteLater();
  }

  m_networkReply = getNetworkAccessManager()->head(QNetworkRequest(url));
  connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishReading()));
  connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readBuffer()));
  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(handleError(QNetworkReply::NetworkError)));

  waitForReading();
}
#endif

void ZDvidBufferReader::startReading()
{
  m_isReadingDone = false;
  m_buffer.clear();
  m_status = neutube::EReadStatus::OK;
}

bool ZDvidBufferReader::isReadingDone() const
{
  return m_isReadingDone;
}
#if 0
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
  endReading(neutube::EReadStatus::READ_FAILED);
}

void ZDvidBufferReader::readBuffer()
{
  m_buffer.append(m_networkReply->readAll());
}

void ZDvidBufferReader::readBufferPartial()
{
  m_buffer.append(m_networkReply->readAll());
  if (m_buffer.size() > m_maxSize) {
    endReading(m_status);
  }
}

void ZDvidBufferReader::finishReading()
{
  endReading(m_status);
}

void ZDvidBufferReader::handleTimeout()
{
  endReading(neutube::READ_TIMEOUT);
}

void ZDvidBufferReader::cancelReading()
{
  endReading(neutube::READ_CANCELED);
}
#endif

void ZDvidBufferReader::endReading(neutube::EReadStatus status)
{
  m_status = status;
  m_isReadingDone = true;
#if 0
  if (m_networkReply != NULL) {
    QVariant statusCode = m_networkReply->attribute(
          QNetworkRequest::HttpStatusCodeAttribute);
#ifdef _DEBUG_
    qDebug() << "Status code: " << statusCode;
#endif
    m_statusCode = statusCode.toInt();
    if (m_statusCode != 200) {
      m_status = READ_BAD_RESPONSE;
    }
    m_networkReply->deleteLater();
    m_networkReply = NULL;
  }

  emit readingDone();
#endif
}

neutube::EReadStatus ZDvidBufferReader::getStatus() const
{
  return m_status;
}
