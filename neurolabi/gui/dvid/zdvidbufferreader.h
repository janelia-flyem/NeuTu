#ifndef ZDVIDBUFFERREADER_H
#define ZDVIDBUFFERREADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QEventLoop>
#include <QString>
#include <QNetworkReply>

/*!
 * \brief The class of reading dvid data into buffer
 */
class ZDvidBufferReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidBufferReader(QObject *parent = 0);

  enum EStatus {
    READ_NULL, READ_OK, READ_FAILED, READ_TIMEOUT, READ_CANCELED
  };

  void read(const QString &url);
  void readHead(const QString &url);
  bool isReadable(const QString &url);

  void startReading();
  void endReading(EStatus status);
  void waitForReading();

  bool isReadingDone() const;

  EStatus getStatus() const;

  inline const QByteArray& getBuffer() const {
    return m_buffer;
  }

signals:
  void readingDone();
  void readingCanceled();

public slots:

private slots:
  void handleError(QNetworkReply::NetworkError error);
  void finishReading();
  void handleTimeout();
  void cancelReading();
  void readBuffer();

private:
  QByteArray m_buffer;
  QNetworkAccessManager *m_networkManager;
  QNetworkReply *m_networkReply;
  QEventLoop *m_eventLoop;
  bool m_isReadingDone;
  EStatus m_status;
};

#endif // ZDVIDBUFFERREADER_H
