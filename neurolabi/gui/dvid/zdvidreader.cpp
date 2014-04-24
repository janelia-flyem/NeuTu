#include "zdvidreader.h"
#include "zdvidbuffer.h"
#include "zstackfactory.h"
#include "zdvidinfo.h"

ZDvidReader::ZDvidReader(QObject *parent) :
  QObject(parent)
{
  m_eventLoop = new QEventLoop(this);
  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);

  connect(m_dvidClient, SIGNAL(noRequestLeft()), m_eventLoop, SLOT(quit()));
  //connect(m_dvidClient, SIGNAL(requestFailed()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(requestCanceled()), m_eventLoop, SLOT(quit()));

  connect(m_timer, SIGNAL(timeout()), m_dvidClient, SLOT(cancelRequest()));
}

void ZDvidReader::slotTest()
{
  qDebug() << "ZDvidReader::slotTest";
}

void ZDvidReader::open(
    const QString &serverAddress, const QString &uuid, int port)
{
  m_dvidClient->reset();
  m_dvidClient->setServer(serverAddress, port);
  m_dvidClient->setUuid(uuid);
}

ZObject3dScan ZDvidReader::readBody(int bodyId)
{
  ZDvidRequest request;
  request.setGetObjectRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

  const QVector<ZObject3dScan>& bodyArray = dvidBuffer->getBodyArray();

  if (!bodyArray.empty()) {
    return bodyArray[0];
  }

  return ZObject3dScan();
}

ZSwcTree* ZDvidReader::readSwc(int bodyId)
{
  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetSwcRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  const QVector<ZSwcTree*>& treeArray = dvidBuffer->getSwcTreeArray();

  if (!treeArray.empty()) {
    return treeArray[0]->clone();
  }

  return NULL;
}

ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth)
{
  ZDvidRequest request;
  for (int z = 0; z < depth; ++z) {
    request.setGetImageRequest(x0, y0, z0 + z, width, height);
    m_dvidClient->appendRequest(request);
  }
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();

  ZStack *stack = NULL;

  if (!imageArray.isEmpty()) {
    stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
  }

  stack->setOffset(x0, y0, z0);

  return stack;
}

QString ZDvidReader::readInfo(const QString &dataType)
{
  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetInfoRequest(dataType);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  const QStringList& infoArray = dvidBuffer->getInfoArray();

  return infoArray.join(" ");
}

std::vector<int> ZDvidReader::readBodyId(
    int x0, int y0, int z0, int width, int height, int depth)
{
  ZDvidInfo dvidInfo;
  dvidInfo.setFromJsonString(readInfo("superpixels").toStdString());

  std::vector<int> startIndex = dvidInfo.getBlockIndex(x0, y0, z0);
  std::vector<int> endIndex = dvidInfo.getBlockIndex(x0 + width - 1,
                                                     y0 + height - 1,
                                                     z0 + depth - 1);

#ifdef _DEBUG_
  std::cout << "Region: " << x0 << ", " << y0 << ", " << z0 << " -> "
            << x0 + width - 1 << ", " << y0 + height - 1 << ", "
            << z0 + depth - 1 << std::endl;
#endif

  std::vector<int> idArray;

  if (!startIndex.empty() && !endIndex.empty()) {
    ZDvidRequest request;
    request.setGetStringRequest("sp2body");

    request.setParameter(
          QVariant(QString("intersect/%1_%2_%3/%4_%5_%6").
          arg(startIndex[0]).arg(startIndex[1]).arg(startIndex[2]).
        arg(endIndex[0]).arg(endIndex[1]).arg(endIndex[2])));
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();

    m_eventLoop->exec();

    ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

    const QStringList& infoArray = dvidBuffer->getInfoArray();

    if (infoArray.size() > 1) {
      ZJsonArray array;
      qDebug() << infoArray[1];
      array.decode(infoArray[1].toStdString());
      idArray = array.toIntegerArray();
    }
  }

  return idArray;
}
