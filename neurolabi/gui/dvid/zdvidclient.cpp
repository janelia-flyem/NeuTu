#include "zdvidclient.h"
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <QImageReader>

#include "zerror.h"
#include "zdvidbuffer.h"

ZDvidClient::ZDvidClient(const QString &server, QObject *parent) :
  QObject(parent), m_serverAddress(server), m_dataPath("api/node/624"),
  m_networkReply(NULL), m_targetDirectory("/tmp"),
  m_tmpDirectory("/tmp"), m_file(NULL),
  m_uploadStream(NULL), m_isCanceling(false)
{
  m_networkManager = new QNetworkAccessManager(this);
  m_dvidBuffer = new ZDvidBuffer(this);
  createConnection();
}

void ZDvidClient::createConnection()
{
  connect(m_dvidBuffer, SIGNAL(dataTransfered()),
          this, SLOT(postNextRequest()));
  connect(this, SIGNAL(objectRetrieved()), m_dvidBuffer, SLOT(importSparseObject()));
  connect(this, SIGNAL(swcRetrieved()), m_dvidBuffer, SLOT(importSwcTree()));
  connect(this, SIGNAL(imageRetrieved()), m_dvidBuffer, SLOT(importImage()));
}

bool ZDvidClient::postRequest(
    ZDvidRequest::EDvidRequest request, const QVariant &parameter)
{
  QUrl requestUrl;

  switch (request) {
  case ZDvidRequest::DVID_GET_OBJECT:
  case ZDvidRequest::DVID_SAVE_OBJECT:
    requestUrl.setUrl(QString("%1/%2/sp2body/sparsevol/%3").
                      arg(m_serverAddress).arg(m_dataPath).
                      arg(parameter.toInt()));
    break;
  case ZDvidRequest::DVID_GET_SWC:
  case ZDvidRequest::DVID_UPLOAD_SWC:
    requestUrl.setUrl(QString("%1/%2/skeletons/%3.swc").
                      arg(m_serverAddress).arg(m_dataPath).
                      arg(parameter.toInt()));
    break;
  case ZDvidRequest::DVID_GET_GRAY_SCALE:
  {
    QList<QVariant> parameterList = parameter.toList();
    requestUrl.setUrl(
          QString("%1/%2/grayscale8/xy/%3_%4/%5_%6_%7").arg(m_serverAddress).
          arg(m_dataPath).
          arg(parameterList[3].toInt()).arg(parameterList[4].toInt()).
        arg(parameterList[0].toInt()).arg(parameterList[1].toInt()).
        arg(parameterList[2].toInt()));
  }
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
  }

  qDebug() << requestUrl.toString();

  switch (request) {
  case ZDvidRequest::DVID_GET_OBJECT:
  case ZDvidRequest::DVID_SAVE_OBJECT:
  case ZDvidRequest::DVID_GET_SWC:
  case ZDvidRequest::DVID_GET_GRAY_SCALE:
    m_networkReply = m_networkManager->get(QNetworkRequest(requestUrl));
    break;
  case ZDvidRequest::DVID_UPLOAD_SWC:
  {
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
  }
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
  }

  if (m_networkReply == NULL) {
    RECORD_ERROR_UNCOND("No reply");
    return false;
  }

  switch (request) {
  case ZDvidRequest::DVID_SAVE_OBJECT:
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
  case ZDvidRequest::DVID_GET_OBJECT:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readObject()));
    break;
  case ZDvidRequest::DVID_GET_SWC:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readSwc()));
    break;
  case ZDvidRequest::DVID_GET_GRAY_SCALE:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readImage()));
    break;
  case ZDvidRequest::DVID_UPLOAD_SWC:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
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

void ZDvidClient::readImage()
{
  m_imageBuffer.append(m_networkReply->readAll());
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
  qDebug() << "Request finished.";

  if (m_file != NULL) {
    m_file->flush();
    m_file->close();
  }

  bool objectRetrievalDone = false;
  bool swcRetrievalDone = false;
  bool imageRetrievalDone = false;

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
      objectRetrievalDone = true;
    }

    if (!m_swcBuffer.isEmpty()) {
      m_swcBuffer.append('\n');
      m_swcBuffer.append('\0');
      m_swcTree.loadFromBuffer(m_swcBuffer.constData());
      swcRetrievalDone = true;
    }

    if (!m_imageBuffer.isEmpty()) {
      QImage image;
      QBuffer buffer(&m_imageBuffer);
      QImageReader imageReader(&buffer);
      imageReader.setFormat("png");
      imageReader.read(&image);

      qDebug() << image.width() << " " << image.width() << " " << image.format();

      if (m_image.width() != image.width() || m_image.height() != image.height()) {
        int width = m_currentRequestParameter.toList().at(3).toInt();
        int height = m_currentRequestParameter.toList().at(4).toInt();
        m_image.setData(C_Stack::make(GREY, width, height, 1, 1));
      }

      for (int y = 0; y < m_image.height(); ++y) {
        C_Stack::setValue(m_image.c_stack(), m_image.kind() * y * m_image.width(),
                          image.scanLine(y), image.width());
      }

      m_image.setOffset(m_currentRequestParameter.toList().at(0).toInt(),
                        m_currentRequestParameter.toList().at(1).toInt(),
                        m_currentRequestParameter.toList().at(2).toInt());

      imageRetrievalDone = true;
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
  m_imageBuffer.clear();

  if (objectRetrievalDone) {
    emit objectRetrieved();
  }

  if (swcRetrievalDone) {
    emit swcRetrieved();
  }

  if (imageRetrievalDone) {
    emit imageRetrieved();
  }
}

void ZDvidClient::postNextRequest()
{
  if (!isCanceling()) {
    if (m_requestQueue.isEmpty()) {
      qDebug() << "Emitting noRequestLeft()";
      emit noRequestLeft();
    } else {
      ZDvidRequest request = m_requestQueue.dequeue();
      m_currentRequestParameter = request.getParameter();
      qDebug() << "Posting next request: " << request.getParameter();
      postRequest(request.getType(), request.getParameter());
    }
  }
}

void ZDvidClient::reset()
{
  m_isCanceling = false;
}

void ZDvidClient::appendRequest(ZDvidRequest request)
{
  m_requestQueue.enqueue(request);
}

void ZDvidClient::cancelRequest()
{
  m_isCanceling = true;

  qDebug() << "Canceling request.";

  if (m_networkReply != NULL) {
    m_networkReply->abort();
    m_networkReply->deleteLater();
    m_networkReply = NULL;
  }

  if (m_file != NULL) {
    m_file->flush();
    m_file->close();
    m_file->remove();
  }

  m_objectBuffer.clear();
  m_swcBuffer.clear();
  m_imageBuffer.clear();

  if (m_uploadStream != NULL) {
    m_uploadStream->close();
    delete m_uploadStream;
    m_uploadStream = NULL;
  }

  if (m_file != NULL) {
    delete m_file;
    m_file = NULL;
  }
}
