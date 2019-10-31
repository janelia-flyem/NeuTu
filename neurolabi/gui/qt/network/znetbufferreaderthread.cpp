#include "znetbufferreaderthread.h"

#include "znetbufferreader.h"

ZNetBufferReaderThread::ZNetBufferReaderThread(QObject *parent) :
  QThread(parent)
{
}

ZNetBufferReaderThread::~ZNetBufferReaderThread()
{
  delete m_reader;
  m_reader = nullptr;
}

void ZNetBufferReaderThread::setOperation(znetwork::EOperation op)
{
  m_op = op;
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
  return m_status;
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

neutu::EReadStatus ZNetBufferReaderThread::getStatus() const
{
  if (m_reader) {
    return m_reader->getStatus();
  }

  return neutu::EReadStatus::NONE;
}

void ZNetBufferReaderThread::run()
{
  delete m_reader;
  m_reader = nullptr;
  if (!m_url.isEmpty()) {
    m_reader = new ZNetBufferReader;
    switch (m_op) {
    case znetwork::EOperation::HAS_HEAD:
      m_status = m_reader->hasHead(m_url);
      break;
    case znetwork::EOperation::IS_READABLE:
      m_status = m_reader->isReadable(m_url);
      break;
    case znetwork::EOperation::POST:
      m_reader->post(m_url, m_payload);
      break;
    case znetwork::EOperation::READ:
      m_reader->read(m_url, false);
      break;
    case znetwork::EOperation::READ_HEAD:
      m_reader->readHead(m_url);
      break;
    case znetwork::EOperation::READ_PARTIAL:
      m_reader->readPartial(m_url, m_partialReadSize, false);
      break;
    default:
      break;
    }
  }
}
