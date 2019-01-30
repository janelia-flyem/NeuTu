#ifndef ZDVIDBUFFERREADER_H
#define ZDVIDBUFFERREADER_H

//#include <QObject>
#include <QByteArray>
//#include <QNetworkAccessManager>
#include <QUrl>
//#include <QEventLoop>
#include <QString>
//#include <QNetworkReply>

#include "common/neutube_def.h"
#include "common/zsharedpointer.h"
namespace libdvid{
class DVIDNodeService;
}

//class QTimer;
class ZDvidTarget;

/*!
 * \brief The class of reading dvid data into buffer
 */
class ZDvidBufferReader
{
public:
  explicit ZDvidBufferReader();

  /*
  enum EStatus {
    READ_NULL, READ_OK, READ_FAILED, READ_TIMEOUT, READ_CANCELED,
    READ_BAD_RESPONSE
  };
  */

  void readFromPath(const QString &path, bool outputingUrl = true);

  void read(const QString &url, bool outputingUrl = true);
//  void readPartial(const QString &url, int maxSize, bool outputingUrl);

  void read(const QString &url, const QByteArray &payload,
            const std::string &method,
            bool outputingUrl = true);
//  void readHead(const QString &url);
//  bool isReadable(const QString &url);
//  bool hasHead(const QString &url);

  neutube::EReadStatus getStatus() const;
  int getStatusCode() const {
    return m_statusCode;
  }

  inline const QByteArray& getBuffer() const {
    return m_buffer;
  }

  void clear();

  void clearBuffer();

//  void readQt(const QString &url, bool outputUrl = true);

  void tryCompress(bool compress) {
    m_tryingCompress = compress;
  }

  bool tryingCompress() const {
    return m_tryingCompress;
  }

#if defined(_ENABLE_LIBDVIDCPP_)
  void setService(const ZSharedPointer<libdvid::DVIDNodeService> &service);
  void setService(const ZDvidTarget &target);
#endif

#if 0
signals:
  void readingDone();
  void readingCanceled();
  void checkingStatus();

public slots:

private slots:
  void handleError(QNetworkReply::NetworkError error);
  void finishReading();
  void handleTimeout();
  void cancelReading();
  void readBuffer();
//  void readBufferPartial();
//  void waitForReading();
#endif

private:
  void _init();

  void startReading();
  void endReading(neutube::EReadStatus status);


  bool isReadingDone() const;
//  QNetworkAccessManager* getNetworkAccessManager();

private:
  QByteArray m_buffer;
//  QNetworkAccessManager *m_networkManager = nullptr;
//  QNetworkReply *m_networkReply;
//  QEventLoop *m_eventLoop;
  bool m_isReadingDone;
  neutube::EReadStatus m_status;
  int m_statusCode;
  bool m_tryingCompress;
  int m_maxSize;
#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> m_service;
#endif
//  QTimer *m_timer;
};

#endif // ZDVIDBUFFERREADER_H
