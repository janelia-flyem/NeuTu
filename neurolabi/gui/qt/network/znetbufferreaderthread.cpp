#include "znetbufferreaderthread.h"

#include <QMutexLocker>
#include <QCoreApplication>

#include "znetbufferreader.h"

ZNetBufferReaderThread::ZNetBufferReaderThread(QObject *parent) :
  QThread(parent)
{
}

ZNetBufferReaderThread::~ZNetBufferReaderThread()
{
  QMutexLocker locker(&m_readerMutex);
  if (m_reader) {
    m_reader->abort();
    delete m_reader;
    m_reader = nullptr;
  }
}

void ZNetBufferReaderThread::setOperation(znetwork::EOperation op, int timeout)
{
  m_op = op;
  m_operationTimeout = timeout;
}

void ZNetBufferReaderThread::setUrl(const QString &url)
{
  m_url = url;
}

void ZNetBufferReaderThread::setPayload(const QByteArray &payload)
{
  m_payload = payload;
}

bool ZNetBufferReaderThread::getResultStatus() const
{
  return m_successful;
}

QByteArray ZNetBufferReaderThread::getData() const
{
  if (m_reader) {
    return m_reader->getBuffer();
  }

  return QByteArray();
}

int ZNetBufferReaderThread::getStatusCode() const
{
  if (m_reader) {
    return m_reader->getStatusCode();
  }

  return 0;
}

QByteArray ZNetBufferReaderThread::getResponseHeader(
    const QByteArray &headerName) const
{
  if (m_reader) {
    return m_reader->getResponseHeader(headerName);
  }

  return QByteArray();
}

neutu::EReadStatus ZNetBufferReaderThread::getStatus() const
{
  if (m_reader) {
    return m_reader->getStatus();
  }

  return neutu::EReadStatus::NONE;
}

void ZNetBufferReaderThread::run()
{
  QMutexLocker locker(&m_readerMutex);
  delete m_reader;
  m_reader = nullptr;
  if (!m_url.isEmpty()) {
    m_reader = new ZNetBufferReader;
    switch (m_op) {
    case znetwork::EOperation::HAS_HEAD:
      m_successful = m_reader->hasHead(m_url, m_operationTimeout);
      break;
    case znetwork::EOperation::IS_READABLE:
      m_successful = m_reader->isReadable(m_url);
      break;
    case znetwork::EOperation::HAS_OPTIONS:
      m_successful = m_reader->hasOptions(m_url, m_operationTimeout);
      break;
    case znetwork::EOperation::READ_OPTIONS:
      m_reader->readOptions(m_url, m_operationTimeout);
      m_successful = m_reader->getStatus() == neutu::EReadStatus::OK;
      break;
    case znetwork::EOperation::POST:
      m_reader->post(m_url, m_payload);
      m_successful = m_reader->getStatus() == neutu::EReadStatus::OK;
      break;
    case znetwork::EOperation::READ:
      m_reader->read(m_url, false);
      m_successful = m_reader->getStatus() == neutu::EReadStatus::OK;
      break;
    case znetwork::EOperation::READ_HEAD:
      m_reader->readHead(m_url, m_operationTimeout);
      m_successful = m_reader->getStatus() == neutu::EReadStatus::OK;
      break;
    case znetwork::EOperation::READ_PARTIAL:
      m_reader->readPartial(m_url, m_partialReadSize, false);
      m_successful = m_reader->getStatus() == neutu::EReadStatus::OK;
      break;
    default:
      break;
    }
  }
}
