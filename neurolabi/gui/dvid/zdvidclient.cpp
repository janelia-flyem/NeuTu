#include "zdvidclient.h"
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

#include "zerror.h"

ZDvidClient::ZDvidClient(const QString &server, QObject *parent) :
  QObject(parent), m_serverAddress(server), m_dataPath("api/node/62"),
  m_networkReply(NULL), m_targetDirectory("/tmp"),
  m_tmpDirectory("/tmp"), m_file(NULL),
  m_uploadStream(NULL)
{
  m_networkManager = new QNetworkAccessManager(this);
}

bool ZDvidClient::postRequest(EDvidRequest request, const QVariant &parameter)
{
  QUrl requestUrl;

  switch (request) {
  case DVID_GET_OBJECT:
  case DVID_SAVE_OBJECT:
    requestUrl.setUrl(QString("%1/%2/sp2body/sparsevol/%3").
                      arg(m_serverAddress).arg(m_dataPath).
                      arg(parameter.toInt()));
    break;
  case DVID_GET_SWC:
  case DVID_UPLOAD_SWC:
    requestUrl.setUrl(QString("%1/%2/skeletons/%3.swc").
                      arg(m_serverAddress).arg(m_dataPath).
                      arg(parameter.toInt()));
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
  }

  qDebug() << requestUrl.toString();

  switch (request) {
  case DVID_GET_OBJECT:
  case DVID_SAVE_OBJECT:
  case DVID_GET_SWC:
    m_networkReply = m_networkManager->get(QNetworkRequest(requestUrl));
    break;
  case DVID_UPLOAD_SWC:
#if 1
    QString command = QString(
          "curl -X POST http://emdata1.int.janelia.org/api/node/62/skeletons/"
          "%1.swc --data-binary @%2/%3.swc").arg(parameter.toInt()).
        arg(m_tmpDirectory).arg(parameter.toInt());
    QProcess::execute(command);
#else
    m_uploadStream = new QFile(QString("%1/%2.swc").arg(m_tmpDirectory).
                               arg(parameter.toInt()));
    if (!m_uploadStream->open(QIODevice::ReadOnly)) {
      RECORD_ERROR_UNCOND("Unable to open swc port");
      delete m_uploadStream;
      m_uploadStream = NULL;
      return false;
    }
    QByteArray data;
    QNetworkRequest request(requestUrl);
    QString crlf;
    crlf = 0x0d;
    crlf.append(0x0a);
    data.append(crlf + "Content-Type: application/octet-stream" + crlf + crlf);
    data.append(m_uploadStream->readAll());

    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant("application/octet-stream"));
    m_networkReply = m_networkManager->post(request, data);
    qDebug() << m_networkReply->errorString();
#endif
    break;
  }

  if (m_networkReply == NULL) {
    RECORD_ERROR_UNCOND("No reply");
    return false;
  }

  switch (request) {
  case DVID_SAVE_OBJECT:
  {
    m_file = new QFile(QString("%1/%2.dvid").
                       arg(m_targetDirectory).arg(parameter.toInt()));
    if (!m_file->open(QIODevice::WriteOnly)) {
      RECORD_ERROR_UNCOND("Unable to write object");
      delete m_file;
      m_file = NULL;
      return false;
    }

    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(writeObject()));
  }
    break;
  case DVID_GET_OBJECT:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readObject()));
    break;
  case DVID_GET_SWC:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readSwc()));
    break;
  case DVID_UPLOAD_SWC:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    break;
  }

  connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(finishRequest(QNetworkReply::NetworkError)));

  return true;
}

void ZDvidClient::readObject()
{
  m_objectBuffer.append(m_networkReply->readAll());
}

void ZDvidClient::readSwc()
{
  m_swcBuffer.append(m_networkReply->readAll());
}

bool ZDvidClient::writeObject()
{
  if (m_file != NULL) {
    /*
    ZObject3dScan obj;
    QByteArray buffer = m_networkReply->readAll();
    obj.importDvidObject(buffer.constData(), buffer.size());
    obj.save(m_targetDirectory.toStdString() + "/test.sobj");
    */
    QByteArray buffer = m_networkReply->readAll();
    qDebug() << buffer.size();
    m_file->write(buffer);
    return true;
  }

  return false;
}

void ZDvidClient::finishRequest(QNetworkReply::NetworkError error)
{
  if (m_file != NULL) {
    m_file->flush();
    m_file->close();
  }

  if (error != QNetworkReply::NoError) {
    if (m_file != NULL) {
      m_file->remove();
    }
    RECORD_ERROR_UNCOND(std::string("Unable to finish operation: ") +
                        m_networkReply->errorString().toStdString());
    m_objectBuffer.clear();
    m_swcBuffer.clear();
  } else {
    if (!m_objectBuffer.isEmpty()) {
      m_obj.importDvidObject(m_objectBuffer.constData(), m_objectBuffer.size());
      emit objectRetrieved();
    }

    if (!m_swcBuffer.isEmpty()) {
      m_swcBuffer.append('\n');
      m_swcBuffer.append('\0');
      m_swcTree.loadFromBuffer(m_swcBuffer.constData());
      emit swcRetrieved();
    }
  }

  if (m_uploadStream != NULL) {
    m_uploadStream->close();
    delete m_uploadStream;
    m_uploadStream = NULL;
  }

  m_networkReply->deleteLater();
  m_networkReply = NULL;

  if (m_file != NULL) {
    delete m_file;
    m_file = NULL;
  }

  m_objectBuffer.clear();
  m_swcBuffer.clear();
}

