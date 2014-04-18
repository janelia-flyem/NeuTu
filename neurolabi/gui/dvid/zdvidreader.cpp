#include "zdvidreader.h"
#include "zdvidbuffer.h"
#include "zstackfactory.h"

ZDvidReader::ZDvidReader(QObject *parent) :
  QObject(parent)
{
  m_eventLoop = new QEventLoop(this);
  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);

  connect(m_dvidClient, SIGNAL(noRequestLeft()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(requestFailed()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(requestCanceled()), m_eventLoop, SLOT(quit()));

  connect(m_timer, SIGNAL(timeout()), m_dvidClient, SLOT(cancelRequest()));
}


void ZDvidReader::open(const QString &serverAddress, const QString &uuid)
{
  m_dvidClient->reset();
  m_dvidClient->setServer(serverAddress);
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
  ZDvidRequest request;
  request.setGetSwcRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  m_eventLoop->exec();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

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
