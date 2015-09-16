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

ZDvidBufferReader::ZDvidBufferReader(QObject *parent) :
  QObject(parent), m_networkReply(NULL), m_isReadingDone(false),
  m_status(ZDvidBufferReader::READ_NULL)
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

void ZDvidBufferReader::read(const QString &url, bool outputUrl)
{
  if (outputUrl) {
    qDebug() << url;
  }

  m_buffer.clear();

#if defined(_ENABLE_LIBDVIDCPP_)
//  qDebug() << "Using libdvidcpp";

  ZDvidTarget target;
  target.setFromUrl(url.toStdString());

  if (!target.getUuid().empty()) {
    try {
      libdvid::DVIDNodeService service(
            target.getAddressWithPort(), target.getUuid());
      std::string endPoint = ZDvidUrl::GetEndPoint(url.toStdString());
      libdvid::BinaryDataPtr data = service.custom_request(
            endPoint, libdvid::BinaryDataPtr(), libdvid::GET);

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
