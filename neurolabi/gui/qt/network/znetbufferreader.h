#ifndef ZNETBUFFERREADER_H
#define ZNETBUFFERREADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkReply>
#include <QMap>

#include "common/neutudefs.h"

class QNetworkAccessManager;
class QNetworkReply;
class QEventLoop;
class QTimer;

class ZNetBufferReader : public QObject
{
  Q_OBJECT
public:
  explicit ZNetBufferReader(QObject *parent = nullptr);
  ~ZNetBufferReader();

  void read(const QString &url, bool outputingUrl);
  void readPartial(const QString &url, int maxSize, bool outputingUrl);
  void readHead(const QString &url, int timeout = 0);
  void readOptions(const QString &url, int timeout = 0);
  bool isReadable(const QString &url);
  bool hasHead(const QString &url, int timeout = 0);
  bool hasOptions(const QString &url, int timeout = 0);
  void post(const QString &url, const QByteArray &data);

  neutu::EReadStatus getStatus() const;
  int getStatusCode() const {
    return m_statusCode;
  }

  inline const QByteArray& getBuffer() const {
    return m_buffer;
  }

  inline const QList<QNetworkReply::RawHeaderPair> getResponseHeader() const {
    return m_responseHeader;
  }
  QByteArray getResponseHeader(const QByteArray &headerName) const;

  QMap<QByteArray, QByteArray> getResponseHeaderMap() const;

  void clearBuffer();

  void setRequestHeader(const QString &key, const QString &value);
  bool hasRequestHeader(const QString &key) const;
  void removeRequestHeader(const QString &key);

  void abort();

signals:

public slots:

signals:
  void readingDone();
  void readingCanceled();
//  void checkingStatus();

public slots:

private slots:
  void handleError(QNetworkReply::NetworkError error);
  void finishReading();
  void handleTimeout();
  void readBuffer();
  void readBufferPartial();
  void resetNetworkReply();
  void connectNetworkReply();

private:
  void _init();
  void startReading();
  void endReading(neutu::EReadStatus status);
  void waitForReading();
  void cancelReading();
  bool isReadingDone() const;
  bool isReadingInproress() const;
  QNetworkAccessManager* getNetworkAccessManager();
  QTimer *getTimer();
  void startRequestTimer(int timeout);

private:
  QByteArray m_buffer;
  QNetworkAccessManager *m_networkManager = nullptr;
  QNetworkReply *m_networkReply = nullptr;
  QEventLoop *m_eventLoop = nullptr;
  QTimer *m_timer = nullptr;
//  bool m_isReadingDone = false;
  neutu::EReadStatus m_status = neutu::EReadStatus::NONE;
  int m_statusCode = 0;
  int m_maxSize = 0;
  QMap<QString, QString> m_header;
  QList<QNetworkReply::RawHeaderPair> m_responseHeader;
};

#endif // ZNETBUFFERREADER_H
