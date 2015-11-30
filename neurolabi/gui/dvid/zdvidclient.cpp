#include "zdvidclient.h"
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <QImageReader>

#include "zerror.h"
#include "zdvidbuffer.h"
#include "neutubeconfig.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdviddata.h"

ZDvidClient::ZDvidClient(QObject *parent) :
  QObject(parent),
  m_networkReply(NULL), m_targetDirectory("/tmp"),
  m_tmpDirectory("/tmp"), m_file(NULL),
  m_uploadStream(NULL), m_isCanceling(false)
{
  m_networkManager = new QNetworkAccessManager(this);
  m_dvidBuffer = new ZDvidBuffer(this);
  createConnection();
}

void ZDvidClient::setServer(const QString &server)
{
  m_dvidTarget.setServer(server.toStdString());
}

void ZDvidClient::setUuid(const QString &uuid)
{
  m_dvidTarget.setUuid(uuid.toStdString());
}

void ZDvidClient::setPort(int port)
{
  m_dvidTarget.setPort(port);
}

void ZDvidClient::createConnection()
{
  connect(m_dvidBuffer, SIGNAL(dataTransfered()),
          this, SLOT(postNextRequest()));
  connect(this, SIGNAL(requestFailed()), this, SLOT(postNextRequest()));
  connect(this, SIGNAL(objectRetrieved()), m_dvidBuffer, SLOT(importSparseObject()));
  connect(this, SIGNAL(swcRetrieved()), m_dvidBuffer, SLOT(importSwcTree()));
  connect(this, SIGNAL(imageRetrieved()), m_dvidBuffer, SLOT(importImage()));
  connect(this, SIGNAL(infoRetrieved()), m_dvidBuffer, SLOT(importInfo()));
  connect(this, SIGNAL(keyValueRetrieved()), m_dvidBuffer, SLOT(importKeyValue()));
  connect(this, SIGNAL(keysRetrieved()), m_dvidBuffer, SLOT(importKeys()));
}

bool ZDvidClient::postRequest(
    ZDvidRequest::EDvidRequest request, const QVariant &parameter)
{
  QUrl requestUrl;
  QString urlString;
  ZDvidUrl dvidUrl(getDvidTarget());

  switch (request) {
  case ZDvidRequest::DVID_GET_SUPERPIXEL_INFO:
    urlString = dvidUrl.getInfoUrl(
          ZDvidData::GetName(ZDvidData::ROLE_SUPERPIXEL)).c_str();
    /*
    urlString = QString("%1/%2/superpixels/info").
        arg(m_serverAddress).arg(m_dataPath);
        */
    break;
  case ZDvidRequest::DVID_GET_GRAYSCALE_INFO:
    urlString = dvidUrl.getInfoUrl(
          ZDvidData::GetName(ZDvidData::ROLE_GRAY_SCALE)).c_str();
    /*
    urlString = QString("%1/%2/grayscale/info").
        arg(m_serverAddress).arg(m_dataPath);
        */
    break;
  case ZDvidRequest::DVID_GET_SP2BODY_STRING:
    urlString = dvidUrl.getSp2bodyUrl(parameter.toString().toStdString()).c_str();
    /*
    urlString = QString("%1/%2/sp2body/%3").arg(m_serverAddress).
        arg(m_dataPath).arg(parameter.toString());
        */
    break;
  case ZDvidRequest::DVID_GET_OBJECT:
  case ZDvidRequest::DVID_SAVE_OBJECT:
    urlString = dvidUrl.getSparsevolUrl(
          parameter.toInt(), m_dvidTarget.getBodyLabelName()).c_str();
    /*
    QString("%1/%2/sp2body/sparsevol/%3").
        arg(m_serverAddress).arg(m_dataPath).
        arg(parameter.toInt());
        */
    break;
  case ZDvidRequest::DVID_GET_SWC:
  case ZDvidRequest::DVID_UPLOAD_SWC:
    urlString = dvidUrl.getSkeletonUrl(
          parameter.toInt(), m_dvidTarget.getBodyLabelName()).c_str();
    /*
    urlString = QString("%1/%2/skeletons/%3.swc").
        arg(m_serverAddress).arg(m_dataPath).
        arg(parameter.toInt());
        */
    break;
  case ZDvidRequest::DVID_GET_THUMBNAIL:
    urlString = dvidUrl.getThumbnailUrl(
          parameter.toInt(), m_dvidTarget.getBodyLabelName()).c_str();
    /*
    urlString = QString("%1/%2/thumbnails/%3.mraw").arg(m_serverAddress).
        arg(m_dataPath).arg(parameter.toInt());
        */
    break;
  case ZDvidRequest::DVID_GET_GRAY_SCALE:
  {
    QList<QVariant> parameterList = parameter.toList();
    //if (parameterList)
    if (parameterList.size() == 5) {
      urlString = dvidUrl.getGrayscaleUrl(parameterList[3].toInt(),
          parameterList[4].toInt(), parameterList[0].toInt(),
          parameterList[1].toInt(), parameterList[2].toInt()).c_str();
      /*
      urlString = QString("%1/%2/grayscale8/raw/0_1/%3_%4/%5_%6_%7").
          arg(m_serverAddress).
          arg(m_dataPath).
          arg(parameterList[3].toInt()).arg(parameterList[4].toInt()).
          arg(parameterList[0].toInt()).arg(parameterList[1].toInt()).
          arg(parameterList[2].toInt());
          */
    } else {
      urlString = dvidUrl.getGrayscaleUrl(parameterList[3].toInt(),
          parameterList[4].toInt(), parameterList[5].toInt(),
          parameterList[0].toInt(),
          parameterList[1].toInt(), parameterList[2].toInt()).c_str();
      /*
      urlString = QString("%1/%2/grayscale8/raw/0_1_2/%3_%4_%5/%6_%7_%8").
          arg(m_serverAddress).
          arg(m_dataPath).
          arg(parameterList[3].toInt()).arg(parameterList[4].toInt()).
          arg(parameterList[5].toInt()).
          arg(parameterList[0].toInt()).arg(parameterList[1].toInt()).
          arg(parameterList[2].toInt());
          */
    }
  }
    break;
  case ZDvidRequest::DVID_GET_BODY_LABEL:
  {
#if 1
    QList<QVariant> parameterList = parameter.toList();
    //if (parameterList)
    if (parameterList.size() == 5) {
//      urlString = QString("%1/%2/bodies/raw/0_1/%3_%4/%5_%6_%7").
//          arg(m_serverAddress).
//          arg(m_dataPath).
//          arg(parameterList[3].toInt()).arg(parameterList[4].toInt()).
//          arg(parameterList[0].toInt()).arg(parameterList[1].toInt()).
//          arg(parameterList[2].toInt());
    } else if (parameterList.size() == 6) {
      urlString = dvidUrl.getLabels64Url(
            m_dvidTarget.getBodyLabelName(),parameterList[3].toInt(),
          parameterList[4].toInt(), parameterList[5].toInt(),
            parameterList[0].toInt(), parameterList[1].toInt(),
          parameterList[2].toInt()).c_str();
//      urlString = QString("%1/%2/bodies/raw/0_1_2/%3_%4_%5/%6_%7_%8").
//          arg(m_serverAddress).
//          arg(m_dataPath).
//          arg(parameterList[3].toInt()).arg(parameterList[4].toInt()).
//          arg(parameterList[5].toInt()).
//          arg(parameterList[0].toInt()).arg(parameterList[1].toInt()).
//          arg(parameterList[2].toInt());
    } else if (parameterList.size() == 7) {
      urlString = dvidUrl.getLabels64Url(
            parameterList[6].toString().toStdString(), parameterList[3].toInt(),
          parameterList[4].toInt(), parameterList[5].toInt(),
          parameterList[0].toInt(), parameterList[1].toInt(),
          parameterList[2].toInt()).c_str();
    }
#endif
  }
    break;
  case ZDvidRequest::DVID_GET_KEYVALUE:
  {
    QList<QVariant> parameterList = parameter.toList();
    urlString = dvidUrl.getKeyUrl(parameterList[0].toString().toStdString(),
        parameterList[1].toString().toStdString()).c_str();
    /*
    urlString = QString("%1/%2/%3/%4").
        arg(m_serverAddress).arg(m_dataPath).
        arg(parameterList[0].toString()).arg(parameterList[1].toString());
        */
  }
    break;
  case ZDvidRequest::DVID_GET_KEYS:
  {
    QList<QVariant> parameterList = parameter.toList();
    urlString = dvidUrl.getKeyRangeUrl(
          parameterList[0].toString().toStdString(),
        parameterList[1].toString().toStdString(),
        parameterList[2].toString().toStdString()).c_str();
  }
    break;
  default:
    RECORD_ERROR_UNCOND("Invalid request");
    return false;
  }

  if (!urlString.startsWith("http")) {
    urlString = "http://" + urlString;
  }

  requestUrl.setUrl(urlString);

  qDebug() << requestUrl.toString();

  switch (request) {
  case ZDvidRequest::DVID_GET_OBJECT:
  case ZDvidRequest::DVID_SAVE_OBJECT:
  case ZDvidRequest::DVID_GET_SWC:
  case ZDvidRequest::DVID_GET_GRAY_SCALE:
  case ZDvidRequest::DVID_GET_BODY_LABEL:
  case ZDvidRequest::DVID_GET_SUPERPIXEL_INFO:
  case ZDvidRequest::DVID_GET_GRAYSCALE_INFO:
  case ZDvidRequest::DVID_GET_SP2BODY_STRING:
  case ZDvidRequest::DVID_GET_KEYVALUE:
  case ZDvidRequest::DVID_GET_KEYS:
  case ZDvidRequest::DVID_GET_THUMBNAIL:
    m_networkReply = m_networkManager->get(QNetworkRequest(requestUrl));
#ifdef _DEBUG_2
  {
    QVariant statusCode =
        m_networkReply->attribute( QNetworkRequest::HttpStatusCodeAttribute);

    qDebug() << statusCode.toInt();
    qDebug() << m_networkReply->errorString();
    qDebug() << m_networkReply->request().url();
  }
#endif
    break;
  case ZDvidRequest::DVID_UPLOAD_SWC:
  {
#if 0
    QString command = QString(
          "curl -X POST http://%1/api/node/%2/skeletons/%3.swc"
          " --data-binary @%4/%5.swc").arg(m_serverAddress).arg(m_dataPath).
        arg(parameter.toInt()).
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
    } else {
      QByteArray data;
      QNetworkRequest request(requestUrl);
      //QString crlf;
      //crlf = 0x0d;
      //crlf.append(0x0a);
      //data.append(crlf + "Content-Type: application/octet-stream" + crlf + crlf);
      data.append(m_uploadStream->readAll());

      request.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("application/octet-stream"));

      //m_networkReply = m_networkManager->get(request);
      m_networkReply = m_networkManager->post(request, data);
    }
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
  case ZDvidRequest::DVID_GET_BODY_LABEL:
  case ZDvidRequest::DVID_GET_THUMBNAIL:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readImage()));
    break;
  case ZDvidRequest::DVID_UPLOAD_SWC:
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    break;
  case ZDvidRequest::DVID_GET_SUPERPIXEL_INFO:
  case ZDvidRequest::DVID_GET_GRAYSCALE_INFO:
  case ZDvidRequest::DVID_GET_SP2BODY_STRING:
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readInfo()));
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    break;
  case ZDvidRequest::DVID_GET_KEYVALUE:
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readKeyValue()));
    connect(m_networkReply, SIGNAL(finished()), this, SLOT(finishRequest()));
    break;
  case ZDvidRequest::DVID_GET_KEYS:
    connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readKeys()));
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
  if (m_networkReply != NULL) {
    m_swcBuffer.append(m_networkReply->readAll());
  }

  //qDebug() << m_swcBuffer;
}

void ZDvidClient::readImage()
{
  if (m_networkReply != NULL) {
    m_imageBuffer.append(m_networkReply->readAll());
  }
}

void ZDvidClient::readInfo()
{
  if (m_networkReply != NULL) {
    m_infoBuffer.append(m_networkReply->readAll());
  }
}

void ZDvidClient::readKeys()
{
  if (m_networkReply != NULL) {
    m_keysBuffer.append(m_networkReply->readAll());
  }
}

void ZDvidClient::readKeyValue()
{
  if (m_networkReply != NULL) {
    m_keyValueBuffer.append(m_networkReply->readAll());
  }
}

bool ZDvidClient::writeObject()
{
  if (m_file != NULL && m_networkReply != NULL) {
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
  bool infoRetrievalDone = false;
  bool keyValueRetrievalDone = false;
  bool keysRetrievalDone = false;

#ifdef _DEBUG_
  if (m_networkReply != NULL) {
    QVariant statusCode =
        m_networkReply->attribute( QNetworkRequest::HttpStatusCodeAttribute);

    qDebug() << statusCode.toInt();
    qDebug() << m_networkReply->errorString();
    qDebug() << m_networkReply->request().url();
  }
#endif

  if (error != QNetworkReply::NoError) {
    if (m_file != NULL) {
      m_file->remove();
    }
    if (m_networkReply != NULL) {
      RECORD_ERROR_UNCOND(std::string("Unable to finish operation: ") +
                          m_networkReply->errorString().toStdString());
    }
    m_objectBuffer.clear();
    m_swcBuffer.clear();
    m_imageBuffer.clear();
    m_infoBuffer.clear();
    m_keyValueBuffer.clear();
    m_keysBuffer.clear();

    qDebug() << "Request failed";
    emit requestFailed();
  } else {
    if (!m_objectBuffer.isEmpty()) {
#ifdef _DEBUG_2
    ptoc();
#endif

      m_obj.importDvidObjectBuffer(
            m_objectBuffer.constData(), m_objectBuffer.size());
      objectRetrievalDone = true;
    }

    if (!m_swcBuffer.isEmpty()) {
      m_swcBuffer.append('\n');
      m_swcBuffer.append('\0');
      m_swcTree.loadFromBuffer(m_swcBuffer.constData());
      swcRetrievalDone = true;
    }

    if (!m_imageBuffer.isEmpty()) {
      QList<QVariant> parameterList = m_currentRequest.getParameter().toList();
      if (parameterList.size() == 5) {
        QImage image;
        QBuffer buffer(&m_imageBuffer);
        QImageReader imageReader(&buffer);
        imageReader.setFormat("png");
        imageReader.read(&image);

        qDebug() << image.width() << " " << image.width() << " " << image.format();

        if (m_image.width() != image.width() ||
            m_image.height() != image.height()) {
          int width = parameterList.at(3).toInt();
          int height = parameterList.at(4).toInt();
          m_image.setData(C_Stack::make(GREY, width, height, 1, 1));
        }

        for (int y = 0; y < m_image.height(); ++y) {
          C_Stack::setValue(m_image.c_stack(), m_image.kind() * y * m_image.width(),
                            image.scanLine(y), image.width());
        }

        m_image.setOffset(parameterList.at(0).toInt(),
                          parameterList.at(1).toInt(),
                          parameterList.at(2).toInt());
      } else if (m_currentRequest.getParameter().toList().size() >= 6){
        int width = parameterList.at(3).toInt();
        int height = parameterList.at(4).toInt();
        int depth = parameterList.at(5).toInt();
        int voxelNumber = width * height * depth;
        if (voxelNumber == m_imageBuffer.size()) {
          m_image.setData(C_Stack::make(GREY, width, height, depth, 1));
          const char *dataArray = m_imageBuffer.constData();
          m_image.loadValue(dataArray, m_imageBuffer.size());
          m_image.setOffset(parameterList.at(0).toInt(),
                            parameterList.at(1).toInt(),
                            parameterList.at(2).toInt());
        } else if (m_imageBuffer.size() % voxelNumber == 0) {
          int channelNumber = m_imageBuffer.size() / voxelNumber;
          const char *dataArray = m_imageBuffer.constData();
          if (channelNumber == 8) {
            m_image.setData(C_Stack::make(FLOAT64, width, height, depth, 1));
            m_image.loadValue(dataArray, m_imageBuffer.size());
          } else {
            m_image.setData(
                  C_Stack::make(GREY, width, height, depth, channelNumber));

            for (int i = 0; i < channelNumber; ++i) {
              m_image.loadValue(dataArray + voxelNumber * i, voxelNumber, i);
            }
          }
          m_image.setOffset(parameterList.at(0).toInt(),
                            parameterList.at(1).toInt(),
                            parameterList.at(2).toInt());
        } else {
          qWarning() << "Image retrieval failed.";
//          RECORD_WARNING_UNCOND("Image retrieval failed.");
          m_image.clear();
        }
      } else if (m_currentRequest.getType() == ZDvidRequest::DVID_GET_THUMBNAIL) {
        const char *dataArray = m_imageBuffer.constData();
        Mc_Stack *stack = C_Stack::readMrawFromBuffer(dataArray);
        m_image.setData(stack);
      }

      imageRetrievalDone = true;
    }

    if (!m_infoBuffer.isEmpty()) {
      //qDebug() << m_infoBuffer;
      m_dataInfo = QString(m_infoBuffer);
      infoRetrievalDone = true;
    }

    if (!m_keyValueBuffer.isEmpty()) {
      m_keyValue = m_keyValueBuffer;
      //qDebug() << QString(m_keyValue);
      keyValueRetrievalDone = true;
    }

    if (!m_keysBuffer.isEmpty()) {
      m_keys = m_keysBuffer;
      keysRetrievalDone = true;
    }

    if (!objectRetrievalDone && !swcRetrievalDone && !imageRetrievalDone &&
        !infoRetrievalDone && !keyValueRetrievalDone && !keysRetrievalDone) {
      qDebug() << "Request failed: no data retrieved.";
      emit requestFailed();
    }
  }

  if (m_uploadStream != NULL) {
    m_uploadStream->close();
    delete m_uploadStream;
    m_uploadStream = NULL;
  }

  if (m_networkReply != NULL) {
    m_networkReply->deleteLater();
    m_networkReply = NULL;
  }

  if (m_file != NULL) {
    delete m_file;
    m_file = NULL;
  }

  m_objectBuffer.clear();
  m_swcBuffer.clear();
  m_imageBuffer.clear();
  m_infoBuffer.clear();
  m_keyValueBuffer.clear();
  m_keysBuffer.clear();

  if (objectRetrievalDone) {
    emit objectRetrieved();
  }

  if (swcRetrievalDone) {
    emit swcRetrieved();
  }

  if (imageRetrievalDone) {
    emit imageRetrieved();
  }

  if (infoRetrievalDone) {
    emit infoRetrieved();
  }

  if (keyValueRetrievalDone) {
    emit keyValueRetrieved();
  }

  if (keysRetrievalDone) {
    emit keysRetrieved();
  }
}

void ZDvidClient::setServer(const QString &server, int port)
{
  setServer(server);
  setPort(port);
}

void ZDvidClient::postNextRequest()
{
  if (!isCanceling()) {
    if (m_requestQueue.isEmpty()) {
      qDebug() << "Emitting noRequestLeft()";
      emit noRequestLeft();
    } else {
      ZDvidRequest request = m_requestQueue.dequeue();
      m_currentRequest = request;
      qDebug() << "Posting next request: " << request.getParameter();
      if (postRequest(request.getType(), request.getParameter()) == false) {
        qDebug() << "Emitting requestFailed()";
        emit requestFailed();
      }
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
  m_infoBuffer.clear();
  m_keyValueBuffer.clear();
  m_keysBuffer.clear();

  if (m_uploadStream != NULL) {
    m_uploadStream->close();
    delete m_uploadStream;
    m_uploadStream = NULL;
  }

  if (m_file != NULL) {
    delete m_file;
    m_file = NULL;
  }

  emit requestCanceled();
}

void ZDvidClient::setDefaultServer()
{
#if defined(_FLYEM_)
  const ZDvidTarget &dvidTarget =
      NeutubeConfig::getInstance().getFlyEmConfig().getDvidTarget();
  setServer(dvidTarget.getAddress().c_str(), dvidTarget.getPort());
  setUuid(dvidTarget.getUuid().c_str());
#endif
}
