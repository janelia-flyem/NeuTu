#ifndef ZNETBUFFERREADER_H
#define ZNETBUFFERREADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkReply>

#include "neutube_def.h"

class QNetworkAccessManager;
class QNetworkReply;
class QEventLoop;

class ZNetBufferReader : public QObject
{
  Q_OBJECT
public:
  explicit ZNetBufferReader(QObject *parent = nullptr);

  void read(const QString &url, bool outputingUrl);
  void readPartial(const QString &url, int maxSize, bool outputingUrl);
  void readHead(const QString &url);
  bool isReadable(const QString &url);
  bool hasHead(const QString &url);

  neutube::EReadStatus getStatus() const;
  int getStatusCode() const {
    return m_statusCode;
  }

  inline const QByteArray& getBuffer() const {
    return m_buffer;
  }

  void clearBuffer();

signals:

public slots:

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
  void readBufferPartial();
  void waitForReading();
  void resetNetworkReply();
  void connectNetworkReply();

private:
  void _init();
  void startReading();
  void endReading(neutube::EReadStatus status);
  bool isReadingDone() const;
  QNetworkAccessManager* getNetworkAccessManager() {
    return m_networkManager;
  }

private:
  QByteArray m_buffer;
  QNetworkAccessManager *m_networkManager = nullptr;
  QNetworkReply *m_networkReply = nullptr;
  QEventLoop *m_eventLoop = nullptr;
  bool m_isReadingDone = false;
  neutube::EReadStatus m_status = neutube::READ_NULL;
  int m_statusCode = 0;
  int m_maxSize = 0;
};

#endif // ZNETBUFFERREADER_H
