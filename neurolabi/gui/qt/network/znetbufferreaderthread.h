#ifndef ZNETBUFFERREADERTHREAD_H
#define ZNETBUFFERREADERTHREAD_H

#include <QString>
#include <QThread>

#include "common/neutube_def.h"

class ZNetBufferReader;

class ZNetBufferReaderThread : public QThread
{
  Q_OBJECT
public:
  enum class EOperation {
    NONE, READ, READ_PARTIAL, READ_HEAD, IS_READABLE, HAS_HEAD, POST
  };

  explicit ZNetBufferReaderThread(QObject *parent = Q_NULLPTR);
  ~ZNetBufferReaderThread();

  void setOperation(EOperation op);
  void setUrl(const QString &url);
  void setPayload(const QByteArray &payload);

  bool getResultStatus() const;
  QByteArray getData() const;

  void run() override;

  neutu::EReadStatus getStatus() const;
  int getStatusCode() const;
  /*
  ZNetBufferReader* getReader() const {
    return m_reader;
  }
  */

private:
  ZNetBufferReader *m_reader = nullptr;
  EOperation m_op = EOperation::NONE;
  QString m_url;
  QByteArray m_payload;
  int m_partialReadSize = 12;
  bool m_status = false;
};

#endif // ZNETBUFFERREADERTHREAD_H
