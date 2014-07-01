#include "zdvidreader.h"

#include <QThread>

#include "zdvidbuffer.h"
#include "zstackfactory.h"
#include "zdvidinfo.h"
#include "dvid/zdvidtarget.h"

ZDvidReader::ZDvidReader(QObject *parent) :
  QObject(parent)
{
  m_eventLoop = new QEventLoop(this);
  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);
  //m_timer->setInterval(1000);

  m_isReadingDone = false;

  //connect(m_dvidClient, SIGNAL(noRequestLeft()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(noRequestLeft()), this, SLOT(endReading()));
  connect(this, SIGNAL(readingDone()), m_eventLoop, SLOT(quit()));
  //connect(m_dvidClient, SIGNAL(requestFailed()), m_eventLoop, SLOT(quit()));
  connect(m_dvidClient, SIGNAL(requestCanceled()), this, SLOT(endReading()));

  connect(m_timer, SIGNAL(timeout()), m_dvidClient, SLOT(cancelRequest()));
}

void ZDvidReader::slotTest()
{
  qDebug() << "ZDvidReader::slotTest";
  qDebug() << QThread::currentThread();

  m_eventLoop->quit();
}

void ZDvidReader::startReading()
{
  m_isReadingDone = false;
}

void ZDvidReader::endReading()
{
  m_isReadingDone = true;

  emit readingDone();
}

bool ZDvidReader::open(
    const QString &serverAddress, const QString &uuid, int port)
{
  m_dvidClient->reset();

  if (serverAddress.isEmpty()) {
    return false;
  }

  if (uuid.isEmpty()) {
    return false;
  }

  m_dvidClient->setServer(serverAddress, port);
  m_dvidClient->setUuid(uuid);

  return true;
}

bool ZDvidReader::open(const ZDvidTarget &target)
{
  return open(("http://" + target.getAddress()).c_str(),
              target.getUuid().c_str(), target.getPort());
}

bool ZDvidReader::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.set(sourceString.toStdString());
  return open(target);
}

void ZDvidReader::waitForReading()
{
#ifdef _DEBUG_
  std::cout << "Start waiting ..." << std::endl;
  qDebug() << QThread::currentThread();
#endif
  if (!isReadingDone()) {
    m_eventLoop->exec();
  }
}

ZObject3dScan ZDvidReader::readBody(int bodyId)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearBodyArray();

  ZDvidRequest request;
  request.setGetObjectRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZObject3dScan>& bodyArray = dvidBuffer->getBodyArray();

  ZObject3dScan obj;

  if (!bodyArray.empty()) {
    obj = bodyArray[0];
  }

  return obj;
}

ZSwcTree* ZDvidReader::readSwc(int bodyId)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearTreeArray();

  ZDvidRequest request;
  request.setGetSwcRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZSwcTree*>& treeArray = dvidBuffer->getSwcTreeArray();

  if (!treeArray.empty()) {
    return treeArray[0]->clone();
  }

  return NULL;
}

ZStack* ZDvidReader::readGrayScale(const ZIntCuboid &cuboid)
{
  return readGrayScale(cuboid.getFirstCorner().getX(),
                       cuboid.getFirstCorner().getY(),
                       cuboid.getFirstCorner().getZ(),
                       cuboid.getWidth(), cuboid.getHeight(),
                       cuboid.getDepth());
}

ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;

  std::vector<std::pair<int, int> > partition =
      partitionStack(x0, y0, z0, width, height, depth);
  for (std::vector<std::pair<int, int> >::const_iterator
       iter = partition.begin(); iter != partition.end(); ++iter) {
    request.setGetImageRequest(x0, y0, iter->first, width, height, iter->second);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  }

  waitForReading();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();

  ZStack *stack = NULL;
  if (!imageArray.isEmpty()) {
    //stack = imageArray[0]->clone();
    if (!imageArray.isEmpty()) {
      stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
    }
  }

  return stack;

#if 0 //old version
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
  dvidBuffer->clearImageArray();

  return stack;
#endif
}

bool ZDvidReader::isReadingDone()
{
  return m_isReadingDone;
}

QString ZDvidReader::readInfo(const QString &dataType)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetInfoRequest(dataType);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QStringList& infoArray = dvidBuffer->getInfoArray();

  QString info = infoArray.join(" ");
  dvidBuffer->clearInfoArray();

  return info;
}

std::set<int> ZDvidReader::readBodyId(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  return readBodyId(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
                    lastCorner.getX() - firstCorner.getX() + 1,
                    lastCorner.getY() - firstCorner.getY() + 1,
                    lastCorner.getZ() - firstCorner.getZ() + 1);
}

std::set<int> ZDvidReader::readBodyId(
    int x0, int y0, int z0, int width, int height, int depth)
{
  ZStack *stack = readBodyLabel(x0, y0, z0, width, height, depth);

  std::set<int> bodySet;

  size_t voxelNumber = stack->getVoxelNumber();

  FlyEm::TBodyLabel *labelArray =
      (FlyEm::TBodyLabel*) (stack->array8());
  for (size_t i = 0; i < voxelNumber; ++i) {
    bodySet.insert((int) labelArray[i]);
  }

  delete stack;

  return bodySet;

#if 0
  ZDvidInfo dvidInfo;
  dvidInfo.setFromJsonString(readInfo("superpixels").toStdString());

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearInfoArray();

  ZIntPoint startIndex = dvidInfo.getBlockIndex(x0, y0, z0);
  ZIntPoint endIndex = dvidInfo.getBlockIndex(x0 + width - 1,
                                              y0 + height - 1,
                                              z0 + depth - 1);

#ifdef _DEBUG_
  std::cout << "Region: " << x0 << ", " << y0 << ", " << z0 << " -> "
            << x0 + width - 1 << ", " << y0 + height - 1 << ", "
            << z0 + depth - 1 << std::endl;
#endif

  std::vector<int> idArray;

  if (dvidInfo.isValidBlockIndex(startIndex) &&
      dvidInfo.isValidBlockIndex(endIndex)) {
    ZDvidRequest request;
    request.setGetStringRequest("sp2body");

    request.setParameter(
          QVariant(QString("intersect/%1_%2_%3/%4_%5_%6").
          arg(startIndex[0]).arg(startIndex[1]).arg(startIndex[2]).
        arg(endIndex[0]).arg(endIndex[1]).arg(endIndex[2])));
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();

    m_eventLoop->exec();



    const QStringList& infoArray = dvidBuffer->getInfoArray();

    if (infoArray.size() > 0) {
      ZJsonArray array;
      qDebug() << infoArray[0];
      array.decode(infoArray[0].toStdString());
      idArray = array.toIntegerArray();
    }
  }

  return idArray;
#endif
}

std::set<int> ZDvidReader::readBodyId(size_t minSize, size_t maxSize)
{
  std::set<int> bodySet;

  std::vector<int> idArray;

  if (minSize <= maxSize) {
    startReading();

    ZDvidRequest request;
    request.setGetStringRequest("sp2body");

    request.setParameter(QVariant(QString("sizerange/%1/%2").
                                  arg(minSize).arg(maxSize)));
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();

    waitForReading();

    ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();

    const QStringList& infoArray = dvidBuffer->getInfoArray();

    if (infoArray.size() > 0) {
      ZJsonArray array;
      //qDebug() << infoArray[0];
      array.decode(infoArray[0].toStdString());
      idArray = array.toIntegerArray();
      bodySet.insert(idArray.begin(), idArray.end());
    }

    dvidBuffer->clearInfoArray();
  }

  return bodySet;
}

QByteArray ZDvidReader::readKeyValue(const QString &dataName, const QString &key)
{
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetKeyValueRequest(dataName, key);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<QByteArray> &array = dvidBuffer->getKeyValueArray();

  QByteArray keyValue;
  if (!array.isEmpty()) {
    keyValue = array[0];
  }

  dvidBuffer->clearKeyValueArray();

  return keyValue;
}

ZStack* ZDvidReader::readBodyLabel(
    int x0, int y0, int z0, int width, int height, int depth)
{
  startReading();
  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;
  std::vector<std::pair<int, int> > partition =
      partitionStack(x0, y0, z0, width, height, depth);
  for (std::vector<std::pair<int, int> >::const_iterator
       iter = partition.begin(); iter != partition.end(); ++iter) {
    request.setGetBodyLabelRequest(x0, y0, iter->first, width, height, iter->second);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  }
#if 0
  size_t voxelNumber = (size_t) width * height * depth;
  size_t dvidSizeLimit = MAX_INT32 / 2;
  //if (voxelNumber > dvidSizeLimit) {
    int nseg = voxelNumber / dvidSizeLimit + 1;
    int z = 0;
    int subdepth = depth / nseg;
    while (z < depth) {
      int leftDepth = depth - z;
      if (leftDepth < subdepth) {
        subdepth = leftDepth;
      }
      request.setGetBodyLabelRequest(x0, y0, z + z0, width, height, subdepth);
      m_dvidClient->appendRequest(request);
      m_dvidClient->postNextRequest();
      z += subdepth;
    }
  /*} else {
    request.setGetBodyLabelRequest(x0, y0, z0, width, height, depth);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  }*/
#endif

  waitForReading();

  const QVector<ZStack*>& imageArray = dvidBuffer->getImageArray();
  ZStack *stack = NULL;
  if (!imageArray.isEmpty()) {
    //stack = imageArray[0]->clone();
    if (!imageArray.isEmpty()) {
      if (imageArray.size() == 1) {
        stack = imageArray[0]->clone();
      } else {
        stack = ZStackFactory::composite(imageArray.begin(), imageArray.end());
      }
    }
  }


  return stack;
}

std::vector<std::pair<int, int> > ZDvidReader::partitionStack(
    int x0, int y0, int z0, int width, int height, int depth)
{
  UNUSED_PARAMETER(x0);
  UNUSED_PARAMETER(y0);
  std::vector<std::pair<int, int> > partition;
  size_t voxelNumber = (size_t) width * height * depth;
  size_t dvidSizeLimit = MAX_INT32 / 2;
  int nseg = voxelNumber / dvidSizeLimit + 1;
  int z = 0;
  int subdepth = depth / nseg;
  while (z < depth) {
    int leftDepth = depth - z;
    if (leftDepth < subdepth) {
      subdepth = leftDepth;
    }
    partition.push_back(std::pair<int, int>(z + z0, subdepth));
    z += subdepth;
  }

  return partition;
}
