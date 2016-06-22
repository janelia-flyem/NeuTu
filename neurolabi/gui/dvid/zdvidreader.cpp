#include "zdvidreader.h"

#include <vector>
#include <ctime>

#include <QThread>
#include <QElapsedTimer>

#include "zdvidbuffer.h"
#include "zstackfactory.h"
#include "zswctree.h"
#include "zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidfilter.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"
#include "zarray.h"
#include "zstring.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "dvid/zdvidtile.h"
#include "zdvidtileinfo.h"
#include "zobject3dscan.h"
#include "zsparsestack.h"
#include "zdvidversiondag.h"
#include "dvid/zdvidsparsestack.h"
#include "zflyembodyannotation.h"
#include "dvid/libdvidheader.h"
#include "flyem/zflyemtodoitem.h"

ZDvidReader::ZDvidReader(QObject *parent) :
  QObject(parent), m_verbose(true)
{
  init();
}

ZDvidReader::~ZDvidReader()
{
#if defined(_ENABLE_LIBDVIDCPP_)
//  delete m_service;
//  m_service = NULL;
#endif
}

void ZDvidReader::init()
{
  m_eventLoop = new QEventLoop(this);
//  m_dvidClient = new ZDvidClient(this);
  m_timer = new QTimer(this);
#if defined(_ENABLE_LIBDVIDCPP_)
//  m_service = NULL;
#endif
  //m_timer->setInterval(1000);

  m_isReadingDone = false;

  //connect(m_dvidClient, SIGNAL(noRequestLeft()), m_eventLoop, SLOT(quit()));
//  connect(m_dvidClient, SIGNAL(noRequestLeft()), this, SLOT(endReading()));
  connect(this, SIGNAL(readingDone()), m_eventLoop, SLOT(quit()));
  //connect(m_dvidClient, SIGNAL(requestFailed()), m_eventLoop, SLOT(quit()));
//  connect(m_dvidClient, SIGNAL(requestCanceled()), this, SLOT(endReading()));

//  connect(m_timer, SIGNAL(timeout()), m_dvidClient, SLOT(cancelRequest()));

  m_statusCode = 0;
}

int ZDvidReader::getStatusCode() const
{
  return m_statusCode;
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

bool ZDvidReader::startService()
{
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
//    delete m_service;

    m_service = ZSharedPointer<libdvid::DVIDNodeService>(
          new libdvid::DVIDNodeService(
          m_dvidTarget.getAddressWithPort(), m_dvidTarget.getUuid()));
  } catch (std::exception &e) {
    m_service.reset();
    std::cout << e.what() << std::endl;
    return false;
  }
#endif

  return true;
}

bool ZDvidReader::good() const
{
#if defined(_ENABLE_LIBDVIDCPP_)
  return m_service.get() != NULL;
#else
  return m_dvidTarget.isValid();
#endif
}

bool ZDvidReader::isReady() const
{
  return good();
}

bool ZDvidReader::open(
    const QString &serverAddress, const QString &uuid, int port)
{
//  m_dvidClient->reset();

  if (serverAddress.isEmpty()) {
    return false;
  }

  if (uuid.isEmpty()) {
    return false;
  }

//  m_dvidClient->setServer(serverAddress, port);
//  m_dvidClient->setUuid(uuid);

  /*
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(serverAddress.toStdString(), uuid.toStdString(), port);
  if (!bufferReader.isReadable(dvidUrl.getHelpUrl().c_str())) {
    return false;
  }
  */


  m_dvidTarget.set(serverAddress.toStdString(), uuid.toStdString(), port);

  return startService();
}

bool ZDvidReader::open(const ZDvidTarget &target)
{
//  m_dvidClient->reset();

  if (!target.isValid()) {
    return false;
  }

  /*
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(target);
  if (!bufferReader.isReadable(dvidUrl.getHelpUrl().c_str())) {
    return false;
  }
  */

//  m_dvidClient->setDvidTarget(target);

  m_dvidTarget = target;

  return startService();
}

bool ZDvidReader::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString.toStdString());
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

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, int z, NeuTube::EAxis axis, ZObject3dScan *result)
{
  if (result == NULL) {
    result = new ZObject3dScan;
  } else {
    result->clear();
  }

  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
//  reader.setService(m_service);
#endif

//  reader.tryCompress(true);
  ZDvidUrl dvidUrl(getDvidTarget());
  reader.read(dvidUrl.getSparsevolUrl(bodyId, z, axis).c_str(), isVerbose());
  const QByteArray &buffer = reader.getBuffer();
  result->importDvidObjectBuffer(buffer.data(), buffer.size());

  result->setLabel(bodyId);

  return result;
}

ZObject3dScan *ZDvidReader::readBody(uint64_t bodyId, ZObject3dScan *result)
{
  if (result == NULL) {
    result = new ZObject3dScan;
  } else {
    result->clear();
  }

  ZDvidBufferReader reader;

#if defined(_ENABLE_LIBDVIDCPP_)
//  reader.setService(m_service);
#endif

//  reader.tryCompress(true);
  ZDvidUrl dvidUrl(getDvidTarget());

  QElapsedTimer timer;
  timer.start();

  reader.read(dvidUrl.getSparsevolUrl(bodyId).c_str(), isVerbose());

  std::cout << "Body reading time: " << timer.elapsed() << std::endl;

  timer.start();
  const QByteArray &buffer = reader.getBuffer();
  result->importDvidObjectBuffer(buffer.data(), buffer.size());
  std::cout << "Body parsing time: " << timer.elapsed() << std::endl;

  result->setLabel(bodyId);

#if 0
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearBodyArray();

  ZDvidRequest request;
  request.setGetObjectRequest(bodyId);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<ZObject3dScan>& bodyArray = dvidBuffer->getBodyArray();

  if (!bodyArray.empty()) {
    *result = bodyArray[0];
  }
#endif

  return result;
}

ZObject3dScan ZDvidReader::readBody(uint64_t bodyId)
{
  ZObject3dScan obj;
  readBody(bodyId, &obj);
  return obj;
}

ZSwcTree* ZDvidReader::readSwc(uint64_t bodyId)
{
  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
  reader.setService(m_service);
#endif
  ZDvidUrl url(getDvidTarget());
//  qDebug() << url.getSkeletonUrl(bodyId);

  reader.read(url.getSkeletonUrl(bodyId).c_str(), isVerbose());

  const QByteArray &buffer = reader.getBuffer();

  ZSwcTree *tree = NULL;

  if (!buffer.isEmpty()) {
    tree = new ZSwcTree;
    tree->loadFromBuffer(buffer.constData());
    if (tree->isEmpty()) {
      delete tree;
      tree = NULL;
    }
  }
  /*
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
  */

  return tree;
}

ZStack* ZDvidReader::readThumbnail(uint64_t bodyId)
{
  ZDvidUrl url(getDvidTarget());
  qDebug() << url.getThumbnailUrl(bodyId);

  ZDvidBufferReader reader;
  reader.read(url.getThumbnailUrl(bodyId).c_str(), isVerbose());

  Mc_Stack *rawStack =
      C_Stack::readMrawFromBuffer(reader.getBuffer().constData());

  ZStack *stack = NULL;
  if (rawStack != NULL) {
    stack = new ZStack;
    stack->setData(rawStack);
    //  m_image.setData(stack);
  }

  return stack;
}

ZStack* ZDvidReader::readGrayScale(const ZIntCuboid &cuboid)
{
  return readGrayScale(cuboid.getFirstCorner().getX(),
                       cuboid.getFirstCorner().getY(),
                       cuboid.getFirstCorner().getZ(),
                       cuboid.getWidth(), cuboid.getHeight(),
                       cuboid.getDepth());
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
    int blockNumber)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
#ifdef _DEBUG_2
  tic();
#endif

  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ(),
                                                 blockNumber).c_str(),
                    isVerbose());
#ifdef _DEBUG_2
  std::cout << "reading time:" << std::endl;
  ptoc();
#endif

#ifdef _DEBUG_2
  tic();
#endif

  std::vector<ZStack*> stackArray(blockNumber, NULL);

  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    const QByteArray &data = bufferReader.getBuffer();
    if (data.length() > 0) {
//      int realBlockNumber = *((int*) data.constData());

      ZIntCuboid currentBox = dvidInfo.getBlockBox(blockIndex);
      for (int i = 0; i < blockNumber; ++i) {
        //stackArray[i] = ZStackFactory::makeZeroStack(GREY, currentBox);
        stackArray[i] = new ZStack(GREY, currentBox, 1);
#ifdef _DEBUG_2
        std::cout << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
        stackArray[i]->loadValue(data.constData() + i * currentBox.getVolume(),
                         currentBox.getVolume(), stackArray[i]->array8());
        currentBox.translateX(currentBox.getWidth());
      }
    }
  }

#ifdef _DEBUG_2
  std::cout << "parsing time:" << std::endl;
  ptoc();
#endif

  return stackArray;
}

ZStack* ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ()).c_str(),
                    isVerbose());
  ZStack *stack = NULL;
  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    const QByteArray &data = bufferReader.getBuffer();
    int realBlockNumber = *((int*) data.constData());

    if (!data.isEmpty() && realBlockNumber == 1) {
      ZIntCuboid box = dvidInfo.getBlockBox(blockIndex);
      stack = ZStackFactory::makeZeroStack(GREY, box);
#ifdef _DEBUG_
      std::cout << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
      stack->loadValue(data.constData() + 4, data.length() - 4, stack->array8());
    }
  }

  return stack;
}

ZDvidSparseStack* ZDvidReader::readDvidSparseStack(uint64_t bodyId)
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setDvidTarget(getDvidTarget());
  spStack->loadBody(bodyId);

  return spStack;
}

ZDvidSparseStack* ZDvidReader::readDvidSparseStackAsync(uint64_t bodyId)
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setLabel(bodyId);
  spStack->setDvidTarget(getDvidTarget());
  spStack->loadBodyAsync(bodyId);

  return spStack;
}

ZSparseStack* ZDvidReader::readSparseStack(uint64_t bodyId)
{
  ZSparseStack *spStack = NULL;

  ZObject3dScan *body = readBody(bodyId, NULL);

  //ZSparseObject *body = new ZSparseObject;
  //body->append(reader.readBody(bodyId));
  //body->canonize();
#ifdef _DEBUG_2
  tic();
#endif

  if (!body->isEmpty()) {
    spStack = new ZSparseStack;
    spStack->setObjectMask(body);

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(
          readInfo(getDvidTarget().getGrayScaleName().c_str()).toStdString());
    ZObject3dScan blockObj = dvidInfo.getBlockIndex(*body);;
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    spStack->setGreyScale(grid);
    grid->setMinPoint(dvidInfo.getStartCoordinates());
    grid->setBlockSize(dvidInfo.getBlockSize());
    grid->setGridSize(dvidInfo.getGridSize());

    /*
    for (ZIntPointArray::const_iterator iter = blockArray.begin();
         iter != blockArray.end(); ++iter) {
         */
    size_t stripeNumber = blockObj.getStripeNumber();
    for (size_t s = 0; s < stripeNumber; ++s) {
      const ZObject3dStripe &stripe = blockObj.getStripe(s);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int i = 0; i < segmentNumber; ++i) {
        int x0 = stripe.getSegmentStart(i);
        int x1 = stripe.getSegmentEnd(i);
        //tic();
#if 0
        const ZIntPoint blockIndex =
            ZIntPoint(x0, y, z) - dvidInfo.getStartBlockIndex();
        std::vector<ZStack*> stackArray =
            readGrayScaleBlock(blockIndex, dvidInfo, x1 - x0 + 1);
        grid->consumeStack(blockIndex, stackArray);
#else

        for (int x = x0; x <= x1; ++x) {
          const ZIntPoint blockIndex =
              ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();
          //ZStack *stack = readGrayScaleBlock(blockIndex, dvidInfo);
          //const ZIntPoint blockIndex = *iter - dvidInfo.getStartBlockIndex();
          ZIntCuboid box = grid->getBlockBox(blockIndex);
          ZStack *stack = readGrayScale(box);
          grid->consumeStack(blockIndex, stack);
        }
#endif
        //ptoc();
      }
    }
    //}
  } else {
    delete body;
  }

#ifdef _DEBUG_2
  ptoc();
#endif

  return spStack;
}

ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth)
{
#if 1
  ZStack *stack = NULL;

  ZDvidBufferReader bufferReader;
  ZDvidUrl url(getDvidTarget());
  /*
  if (depth == 1) {
    bufferReader.read(url.getGrayscaleUrl(width, height, x0, y0, z0).c_str());
  } else {
  */
    bufferReader.read(url.getGrayscaleUrl(
                        width, height, depth, x0, y0, z0).c_str(), isVerbose());
  //}

  const QByteArray &buffer = bufferReader.getBuffer();

  if (!buffer.isEmpty()) {
    ZIntCuboid box(x0, y0, z0, x0 + width - 1, y0 + height - 1, z0 + depth - 1);
    stack = new ZStack(GREY, box, 1);

    memcpy(stack->array8(), buffer.constData(), buffer.size());
  }

  return stack;
#else
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;

  if (depth == 1) {
    request.setGetImageRequest(x0, y0, z0, width, height);
    m_dvidClient->appendRequest(request);
    m_dvidClient->postNextRequest();
  } else {
    std::vector<std::pair<int, int> > partition =
        partitionStack(x0, y0, z0, width, height, depth);
    for (std::vector<std::pair<int, int> >::const_iterator
         iter = partition.begin(); iter != partition.end(); ++iter) {
      request.setGetImageRequest(x0, y0, iter->first, width, height, iter->second);
      m_dvidClient->appendRequest(request);
      m_dvidClient->postNextRequest();
    }
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

  dvidBuffer->clearImageArray();

  return stack;
#endif

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

ZJsonObject ZDvidReader::readInfo() const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonObject(url.getInfoUrl());
}

QString ZDvidReader::readInfo(const QString &dataName) const
{
  ZDvidBufferReader reader;
  reader.read(ZDvidUrl(getDvidTarget()).getInfoUrl(dataName.toStdString()).c_str(),
             isVerbose());
  const QByteArray &buffer = reader.getBuffer();

  return QString(buffer);
  /*
  ZDvidUrl dvidUrl(getDvidTarget());
  dvidUrl.getInfoUrl(dataName);




  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetInfoRequest(dataName);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QStringList& infoArray = dvidBuffer->getInfoArray();

  QString info = infoArray.join(" ");
  dvidBuffer->clearInfoArray();

  return info;
  */
}

std::set<uint64_t> ZDvidReader::readBodyId(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner, bool ignoringZero)
{
  return readBodyId(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
                    lastCorner.getX() - firstCorner.getX() + 1,
                    lastCorner.getY() - firstCorner.getY() + 1,
                    lastCorner.getZ() - firstCorner.getZ() + 1,
                    ignoringZero);
}

std::set<uint64_t> ZDvidReader::readBodyId(
    int x0, int y0, int z0, int width, int height, int depth, bool ignoringZero)
{
  ZArray *array = readLabels64(x0, y0, z0, width, height, depth);

  uint64_t *dataArray = array->getDataPointer<uint64_t>();
  std::set<uint64_t> bodySet;
  for (size_t i = 0; i < array->getElementNumber(); ++i) {
    if (!ignoringZero || dataArray[i] > 0) {
      bodySet.insert(dataArray[i]);
    }
  }
#if 0
  ZStack *stack = readBodyLabel(x0, y0, z0, width, height, depth);

  std::set<int> bodySet;

  size_t voxelNumber = stack->getVoxelNumber();

  FlyEm::TBodyLabel *labelArray =
      (FlyEm::TBodyLabel*) (stack->array8());
  for (size_t i = 0; i < voxelNumber; ++i) {
    bodySet.insert((int) labelArray[i]);
  }

  delete stack;
#endif

  return bodySet;
}

std::set<uint64_t> ZDvidReader::readBodyId(const QString /*sizeRange*/)
{
  std::set<uint64_t> bodySet;
#if 0
  if (!sizeRange.isEmpty()) {
    std::vector<int> idArray;
    startReading();

    ZDvidRequest request;
    request.setGetStringRequest("sp2body");

    request.setParameter(QVariant("sizerange/" + sizeRange));
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
#endif
  return bodySet;
}

std::set<uint64_t> ZDvidReader::readBodyId(const ZDvidFilter &filter)
{
  std::set<uint64_t> bodyIdSet;

  if (filter.hasUpperBodySize()) {
    bodyIdSet = readBodyId(filter.getMinBodySize(), filter.getMaxBodySize());
  } else {
    bodyIdSet = readBodyId(filter.getMinBodySize());
  }

  if (filter.hasExclusion()) {
    std::set<uint64_t> newBodySet;
    for (std::set<uint64_t>::const_iterator iter = bodyIdSet.begin();
         iter != bodyIdSet.end(); ++iter) {
      int bodyId = *iter;
      if (!filter.isExcluded(bodyId)) {
        newBodySet.insert(bodyId);
      }
    }
    return newBodySet;
  }

  return bodyIdSet;
}

std::set<uint64_t> ZDvidReader::readBodyId(size_t minSize)
{
  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize).c_str(), isVerbose());

  std::set<uint64_t> bodySet;

  QString idStr = bufferReader.getBuffer().data();

  if (!idStr.isEmpty()) {
    std::vector<int> idArray;
    ZJsonArray array;
      //qDebug() << infoArray[0];
    array.decode(idStr.toStdString());
    idArray = array.toIntegerArray();
    bodySet.insert(idArray.begin(), idArray.end());
  }

  return bodySet;
}

std::set<uint64_t> ZDvidReader::readBodyId(size_t minSize, size_t maxSize)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize, maxSize).c_str(), isVerbose());

  std::set<uint64_t> bodySet;

  QString idStr = bufferReader.getBuffer().data();

  if (!idStr.isEmpty()) {
    std::vector<int> idArray;
    ZJsonArray array;
      //qDebug() << infoArray[0];
    array.decode(idStr.toStdString());
    idArray = array.toIntegerArray();
    bodySet.insert(idArray.begin(), idArray.end());
  }

  return bodySet;
}

std::set<uint64_t> ZDvidReader::readAnnnotatedBodySet()
{
  QStringList annotationList = readKeys(
        ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                           ZDvidData::ROLE_BODY_LABEL,
                           getDvidTarget().getBodyLabelName()).c_str());

  std::set<uint64_t> bodySet;
  foreach (const QString &idStr, annotationList) {
    uint64_t bodyId = ZString(idStr.toStdString()).firstUint64();
    bodySet.insert(bodyId);
  }

  return bodySet;
}

bool ZDvidReader::hasKey(const QString &dataName, const QString &key)
{
  return !readKeyValue(dataName, key).isEmpty();
}

QByteArray ZDvidReader::readKeyValue(const QString &dataName, const QString &key)
{
  ZDvidUrl url(getDvidTarget());

  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(
        url.getKeyUrl(dataName.toStdString(), key.toStdString()).c_str(),
        isVerbose());

  return bufferReader.getBuffer();
#if 0
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
#endif
}

QStringList ZDvidReader::readKeys(const QString &dataName)
{
  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
  reader.setService(m_service);
#endif

  ZDvidUrl dvidUrl(m_dvidTarget);

  reader.read(dvidUrl.getAllKeyUrl(dataName.toStdString()).c_str(), isVerbose());
  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey)
{
  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
  reader.setService(m_service);
#endif
  ZDvidUrl dvidUrl(m_dvidTarget);
  const std::string &maxKey = "\xff";

  reader.read(dvidUrl.getKeyRangeUrl(
                dataName.toStdString(), minKey.toStdString(), maxKey).c_str(),
              isVerbose());
  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey, const QString &maxKey)
{
  ZDvidUrl url(getDvidTarget());

  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
  reader.setService(m_service);
#endif
  reader.read(url.getKeyRangeUrl(dataName.toStdString(), minKey.toStdString(),
                                 maxKey.toStdString()).c_str(), isVerbose());
  const QByteArray &keyBuffer = reader.getBuffer();

  QStringList keys;

#if 0
  startReading();

  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clear();

  ZDvidRequest request;
  request.setGetKeysRequest(dataName, minKey, maxKey);
  m_dvidClient->appendRequest(request);
  m_dvidClient->postNextRequest();

  waitForReading();

  const QVector<QByteArray> &array = dvidBuffer->getKeysArray();
  QByteArray keyBuffer;
  if (!array.isEmpty()) {
    keyBuffer = array[0];
  }

  dvidBuffer->clearKeysArray();
#endif

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  return keys;
}

#if 0
ZStack* ZDvidReader::readBodyLabel(
    int x0, int y0, int z0, int width, int height, int depth)
{
#if 1
  startReading();
  ZDvidBuffer *dvidBuffer = m_dvidClient->getDvidBuffer();
  dvidBuffer->clearImageArray();

  ZDvidRequest request;
  std::vector<std::pair<int, int> > partition =
      partitionStack(x0, y0, z0, width, height, depth);
  for (std::vector<std::pair<int, int> >::const_iterator
       iter = partition.begin(); iter != partition.end(); ++iter) {
    request.setGetBodyLabelRequest(
          m_dvidTarget.getLabelBlockName().c_str(),
          x0, y0, iter->first, width, height, iter->second);
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
#endif

  return stack;
}
#endif

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

ZClosedCurve* ZDvidReader::readRoiCurve(
    const std::string &key, ZClosedCurve *result)
{
  if (result != NULL) {
    result->clear();
  }

  QByteArray byteArray = readKeyValue("roi_curve", key.c_str());
  if (!byteArray.isEmpty()) {
    ZJsonObject obj;
    obj.decode(byteArray.constData());

    if (!obj.isEmpty()) {
      if (result == NULL) {
        result = new ZClosedCurve;
      }
      result->loadJsonObject(obj);
    }
  }

  return result;
}

ZIntCuboid ZDvidReader::readBoundBox(int z)
{
  QByteArray byteArray = readKeyValue("bound_box", QString("%1").arg(z));
  ZIntCuboid cuboid;

  if (!byteArray.isEmpty()) {
    ZJsonArray obj;
    obj.decode(byteArray.constData());
    if (obj.size() == 6) {
      cuboid.set(ZJsonParser::integerValue(obj.at(0)),
                 ZJsonParser::integerValue(obj.at(1)),
                 ZJsonParser::integerValue(obj.at(2)),
                 ZJsonParser::integerValue(obj.at(3)),
                 ZJsonParser::integerValue(obj.at(4)),
                 ZJsonParser::integerValue(obj.at(5)));
    }
  }

  return cuboid;
}

ZDvidInfo ZDvidReader::readGrayScaleInfo() const
{
  QString infoString = readInfo(getDvidTarget().getGrayScaleName().c_str());
  ZDvidInfo dvidInfo;
  if (!infoString.isEmpty()) {
    dvidInfo.setFromJsonString(infoString.toStdString());
  }

  return dvidInfo;
}

bool ZDvidReader::hasData(const std::string &dataName) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZDvidBufferReader bufferReader;
  return bufferReader.isReadable(dvidUrl.getInfoUrl(dataName).c_str());
}

ZArray* ZDvidReader::readLabels64(
    int x0, int y0, int z0, int width, int height, int depth) const
{
  return readLabels64(getDvidTarget().getLabelBlockName(),
                     x0, y0, z0, width, height, depth);
}

ZArray* ZDvidReader::readLabels64(const ZIntCuboid &box)
{
  return readLabels64(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                      box.getFirstCorner().getZ(), box.getWidth(),
                      box.getHeight(), box.getDepth());
}

ZIntPoint ZDvidReader::readBodyBottom(uint64_t bodyId) const
{
  ZIntPoint pt;

#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    libdvid::PointXYZ coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 2, false);
    pt.set(coord.x, coord.y, coord.z);
  }
#endif

  return pt;
}

ZIntPoint ZDvidReader::readBodyTop(uint64_t bodyId) const
{
  ZIntPoint pt;

#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    libdvid::PointXYZ coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 2, true);
    pt.set(coord.x, coord.y, coord.z);
  }
#endif

  return pt;
}

ZIntCuboid ZDvidReader::readBodyBoundBox(uint64_t bodyId) const
{
  ZIntCuboid box;

#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    libdvid::PointXYZ coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 0, true);
    box.setFirstX(coord.x);

    coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 0, false);
    box.setLastX(coord.x);

    coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 1, true);
    box.setFirstY(coord.y);

    coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 1, false);
    box.setLastY(coord.y);

    coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 2, true);
    box.setFirstZ(coord.z);

    coord = m_service->get_body_extremum(
          getDvidTarget().getBodyLabelName(), bodyId, 2, false);
    box.setLastZ(coord.z);
  }
#endif

  return box;
}

ZArray* ZDvidReader::readLabels64(
    const std::string &dataName, int x0, int y0, int z0,
    int width, int height, int depth) const
{

  ZArray *array = NULL;

#if defined(_ENABLE_LIBDVIDCPP_)
  qDebug() << "Using libdvidcpp";

  const ZDvidTarget &target = getDvidTarget();
  if (!target.getUuid().empty()) {
    try {
      ZDvidUrl dvidUrl(m_dvidTarget);
      std::cout << dvidUrl.getLabels64Url(
                     dataName, width, height, depth, x0, y0, z0).c_str() << std::endl;

      /*
      libdvid::DVIDNodeService service(
            target.getAddressWithPort(), target.getUuid());
*/
      libdvid::Dims_t dims(3);
      dims[0] = width;
      dims[1] = height;
      dims[2] = depth;

      std::vector<int> offset(3);
      offset[0] = x0;
      offset[1] = y0;
      offset[2] = z0;

      std::vector<unsigned int> channels(3);
      channels[0] = 0;
      channels[1] = 1;
      channels[2] = 2;

      QElapsedTimer timer;
      timer.start();
      libdvid::Labels3D labels = m_service->get_labels3D(
            dataName, dims, offset, channels, false, true);
      std::cout << "label reading time: " << timer.elapsed() << std::endl;
//      return array;

      mylib::Dimn_Type arrayDims[3];
      arrayDims[0] = width;
      arrayDims[1] = height;
      arrayDims[2] = depth;
      array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);
      array->copyDataFrom(labels.get_raw());
      array->setStartCoordinate(0, x0);
      array->setStartCoordinate(1, y0);
      array->setStartCoordinate(2, z0);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
    }
  }
#else
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZDvidBufferReader bufferReader;
//  tic();
//  clock_t start = clock();

  bufferReader.read(dvidUrl.getLabels64Url(
                      dataName, width, height, depth, x0, y0, z0).c_str());

//  qDebug() << timer.elapsed();
//  clock_t finish = clock();

//  std::cout << "label reading time: " << (finish - start) / CLOCKS_PER_SEC << std::endl;
//  std::cout << "label reading time: " << toc() << std::endl;

  if (bufferReader.getStatus() == ZDvidBufferReader::READ_OK) {
    //bufferReader.getBuffer();
    int dims[3];
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;
    array = new ZArray(mylib::UINT64_TYPE, 3, dims);

    array->setStartCoordinate(0, x0);
    array->setStartCoordinate(1, y0);
    array->setStartCoordinate(2, z0);

    array->copyDataFrom(bufferReader.getBuffer().constData());
  }
#endif



  return array;
}

bool ZDvidReader::hasSparseVolume() const
{
  return hasData(m_dvidTarget.getBodyLabelName());
  //return true;
  //return hasData(ZDvidData::getName(ZDvidData::ROLE_SP2BODY));
}

bool ZDvidReader::hasBody(uint64_t bodyId) const
{
#if 0
  if (m_service.get() != NULL) {
    try {
      return m_service->body_exists(m_dvidTarget.getBodyLabelName(), bodyId);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      return false;
    }
  }
#else
  return hasSparseVolume(bodyId);
#endif

  return false;
}

ZIntPoint ZDvidReader::readBodyLocation(uint64_t bodyId) const
{
  ZIntPoint location;

#if defined(_ENABLE_LIBDVIDCPP_)
  /*if (m_service.get() != NULL) {
    try {
      libdvid::PointXYZ dpt =
          m_service->get_body_location(
            m_dvidTarget.getBodyLabelName(), bodyId);
      location.set(dpt.x, dpt.y, dpt.z);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
    }
  } else {*/
    ZObject3dScan body = readCoarseBody(bodyId);
    if (!body.isEmpty()) {
      ZDvidInfo dvidInfo = readGrayScaleInfo();

      ZObject3dScan objSlice = body.getMedianSlice();
      ZVoxel voxel = objSlice.getMarker();
  //        ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
      ZIntPoint pt(voxel.x(), voxel.y(), voxel.z());
      pt -= dvidInfo.getStartBlockIndex();
      pt *= dvidInfo.getBlockSize();
      pt += ZIntPoint(dvidInfo.getBlockSize().getX() / 2,
                      dvidInfo.getBlockSize().getY() / 2, 0);
      pt += dvidInfo.getStartCoordinates();
      location = pt;
    }
//  }
#else
  ZObject3dScan body = readCoarseBody(bodyId);
  if (!body.isEmpty()) {
    ZDvidInfo dvidInfo = readGrayScaleInfo();

    ZObject3dScan objSlice = body.getMedianSlice();
    ZVoxel voxel = objSlice.getMarker();
//        ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
    ZIntPoint pt(voxel.x(), voxel.y(), voxel.z());
    pt -= dvidInfo.getStartBlockIndex();
    pt *= dvidInfo.getBlockSize();
    pt += ZIntPoint(dvidInfo.getBlockSize().getX() / 2,
                    dvidInfo.getBlockSize().getY() / 2, 0);
    pt += dvidInfo.getStartCoordinates();
  }
#endif

  return location;
}

bool ZDvidReader::hasSparseVolume(uint64_t bodyId) const
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getSparsevolUrl(bodyId, getDvidTarget().getBodyLabelName()).c_str());
}

bool ZDvidReader::hasCoarseSparseVolume(uint64_t bodyId) const
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getCoarseSparsevolUrl(
          bodyId, getDvidTarget().getBodyLabelName()).c_str());
}

bool ZDvidReader::hasBodyInfo(uint64_t bodyId) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZDvidBufferReader bufferReader;

  return  bufferReader.isReadable(
        dvidUrl.getBodyInfoUrl(bodyId, m_dvidTarget.getBodyLabelName()).c_str());
}

ZFlyEmNeuronBodyInfo ZDvidReader::readBodyInfo(uint64_t bodyId)
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName(ZDvidData::ROLE_BODY_INFO,
                           ZDvidData::ROLE_BODY_LABEL,
                           m_dvidTarget.getBodyLabelName()).c_str(),
        ZString::num2str(bodyId).c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  ZFlyEmNeuronBodyInfo bodyInfo;
  bodyInfo.loadJsonObject(obj);

  return bodyInfo;
}

#define MAX_BODY_ID_START 50000000
uint64_t ZDvidReader::readMaxBodyId()
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName(ZDvidData::ROLE_MAX_BODY_ID),
        m_dvidTarget.getBodyLabelName().c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  uint64_t id = MAX_BODY_ID_START;
  if (obj.hasKey("max_body_id")) {
    id = ZJsonParser::integerValue(obj["max_body_id"]);
  }

  return id;
}

ZDvidTile* ZDvidReader::readTile(int resLevel, int xi0, int yi0, int z0) const
{
  ZDvidTile *tile = new ZDvidTile;
  tile->setResolutionLevel(resLevel);
  tile->setDvidTarget(getDvidTarget(), ZDvidTileInfo());
  tile->setTileIndex(xi0, yi0);
  tile->update(z0);

  return tile;
}

#if 0
ZDvidTile* ZDvidReader::readTile(
    const std::string &dataName, int resLevel, int xi0, int yi0, int z0) const
{
  ZDvidTile *tile = NULL;

//  ZDvidUrl dvidUrl(getDvidTarget());
//  ZDvidBufferReader bufferReader;
//  bufferReader.read(dvidUrl.getTileUrl(dataName, resLevel, xi0, yi0, z0).c_str());
//  QByteArray buffer = bufferReader.getBuffer();

//  ZDvidTileInfo tileInfo = readTileInfo(dataName);

  if (!buffer.isEmpty()) {
    tile = new ZDvidTile;
    tile->setResolutionLevel(resLevel);
    ZDvidTarget target = dataName;
    tile->setDvidTarget(getDvidTarget());
    tile->update(z0);
    /*
    tile->loadDvidPng(buffer);
    tile->setResolutionLevel(resLevel);
    tile->setTileOffset(
          xi0 * tileInfo.getWidth(), yi0 * tileInfo.getHeight(), z0);
          */
  }

  return tile;
}
#endif


ZDvidTileInfo ZDvidReader::readTileInfo(const std::string &dataName) const
{
  ZDvidTileInfo tileInfo;
  ZDvidUrl dvidUrl(getDvidTarget());

  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(dvidUrl.getInfoUrl(dataName).c_str(), isVerbose());

  ZJsonObject infoJson;
  infoJson.decodeString(bufferReader.getBuffer().data());
  tileInfo.load(infoJson);

  return tileInfo;
}

ZDvidVersionDag ZDvidReader::readVersionDag() const
{
  return readVersionDag(getDvidTarget().getUuid());
}

ZDvidVersionDag ZDvidReader::readVersionDag(const std::string &uuid) const
{
  ZDvidVersionDag dag;

  ZDvidUrl dvidUrl(getDvidTarget());

  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(dvidUrl.getRepoInfoUrl().c_str(), isVerbose());

  QString str(bufferReader.getBuffer().data());
  str.replace(QRegExp("\"MaxLabel\":\\s*\\{[^{}]*\\}"), "\"MaxLabel\":{}");

//  qDebug() << str;

  ZJsonObject infoJson;
  infoJson.decodeString(str.toStdString().c_str());

  dag.load(infoJson, uuid);

  return dag;
}

ZObject3dScan ZDvidReader::readCoarseBody(uint64_t bodyId) const
{
  ZDvidBufferReader reader;
#if defined(_ENABLE_LIBDVIDCPP_)
  reader.setService(m_service);
#endif
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(dvidUrl.getCoarseSparsevolUrl(
                bodyId, m_dvidTarget.getBodyLabelName()).c_str(), isVerbose());

  ZObject3dScan obj;
  obj.importDvidObjectBuffer(
        reader.getBuffer().data(), reader.getBuffer().size());
  obj.setLabel(bodyId);

  return obj;
}

uint64_t ZDvidReader::readBodyIdAt(const ZIntPoint &pt)
{
  return readBodyIdAt(pt.getX(), pt.getY(), pt.getZ());
}

uint64_t ZDvidReader::readBodyIdAt(int x, int y, int z)
{
  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getLocalBodyIdUrl(x, y, z).c_str(), isVerbose());

  ZJsonObject infoJson;
  infoJson.decodeString(bufferReader.getBuffer().data());

  uint64_t bodyId = 0;
  if (infoJson.hasKey("Label")) {
    bodyId = (uint64_t) ZJsonParser::integerValue(infoJson["Label"]);
  }

  return bodyId;
}

std::vector<uint64_t> ZDvidReader::readBodyIdAt(
    const std::vector<ZIntPoint> &ptArray)
{
  std::vector<uint64_t> bodyArray;

  if (!ptArray.empty()) {
    ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
    bufferReader.setService(m_service);
#endif
    ZDvidUrl dvidUrl(m_dvidTarget);

    ZJsonArray queryObj;

    for (std::vector<ZIntPoint>::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      ZJsonArray coordObj;
      coordObj.append(pt.getX());
      coordObj.append(pt.getY());
      coordObj.append(pt.getZ());

      queryObj.append(coordObj);
    }

    QString queryForm = queryObj.dumpString(0).c_str();

#ifdef _DEBUG_
    std::cout << "Payload: " << queryForm.toStdString() << std::endl;
#endif

    QByteArray payload;
    payload.append(queryForm);

    bufferReader.read(
          dvidUrl.getLocalBodyIdArrayUrl().data(), payload, "GET", true);

    ZJsonArray infoJson;
    infoJson.decodeString(bufferReader.getBuffer().data());

    if (infoJson.size() == ptArray.size()) {
      for (size_t i = 0; i < infoJson.size(); ++i) {
        uint64_t bodyId = (uint64_t) ZJsonParser::integerValue(infoJson.at(i));
        bodyArray.push_back(bodyId);
      }
    }
  }

  return bodyArray;
}

ZJsonArray ZDvidReader::readAnnotation(
    const std::string &dataName, const std::string &tag) const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonArray(url.getAnnotationUrl(dataName, tag));
}

ZJsonArray ZDvidReader::readAnnotation(
    const std::string &dataName, uint64_t label) const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonArray(url.getAnnotationUrl(dataName, label));
}

ZJsonArray ZDvidReader::readTaggedBookmark(const std::string &tag) const
{
  return readAnnotation(getDvidTarget().getBookmarkName(), tag);
}

ZJsonObject ZDvidReader::readBookmarkJson(int x, int y, int z) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject bookmarkJson;
  ZIntCuboid box(x, y, z, x, y, z);
  ZJsonArray obj = readJsonArray(dvidUrl.getBookmarkUrl(box));
  if (obj.size() > 0) {
    bookmarkJson.set(obj.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
  }

  return bookmarkJson;
}

ZJsonObject ZDvidReader::readBookmarkJson(const ZIntPoint &pt) const
{
  return readBookmarkJson(pt.getX(), pt.getY(), pt.getZ());
}

bool ZDvidReader::isBookmarkChecked(int x, int y, int z) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonObject obj = readJsonObject(dvidUrl.getBookmarkKeyUrl(x, y, z));

  if (obj.hasKey("checked")) {
    return ZJsonParser::booleanValue(obj["checked"]);
  }

  return false;
}

bool ZDvidReader::isBookmarkChecked(const ZIntPoint &pt) const
{
  return isBookmarkChecked(pt.getX(), pt.getY(), pt.getZ());
}

ZObject3dScan ZDvidReader::readRoi(const std::string &dataName)
{
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  bufferReader.readQt(dvidUrl.getRoiUrl(dataName).c_str());
  const QByteArray &buffer = bufferReader.getBuffer();

#ifdef _DEBUG_
  std::cout << buffer.constData() << std::endl;
#endif

  ZJsonArray array;
  array.decodeString(buffer.constData());

  ZObject3dScan obj;
  obj.importDvidRoi(array);

  return obj;
}

ZFlyEmBodyAnnotation ZDvidReader::readBodyAnnotation(uint64_t bodyId) const
{
  ZDvidUrl url(getDvidTarget());
  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(url.getBodyAnnotationUrl(bodyId).c_str(), isVerbose());

  ZFlyEmBodyAnnotation annotation;
  annotation.loadJsonString(bufferReader.getBuffer().constData());
  annotation.setBodyId(bodyId);

  return annotation;
}

ZJsonObject ZDvidReader::readJsonObject(const std::string &url) const
{
  ZJsonObject obj;

  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(url.c_str(), isVerbose());
  const QByteArray &buffer = bufferReader.getBuffer();
  if (!buffer.isEmpty()) {
    obj.decodeString(buffer.constData());
  }

  return obj;
}

ZJsonArray ZDvidReader::readJsonArray(const std::string &url) const
{
  ZJsonArray obj;

  ZDvidBufferReader bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  bufferReader.setService(m_service);
#endif
  bufferReader.read(url.c_str(), isVerbose());
  const QByteArray &buffer = bufferReader.getBuffer();
  if (!buffer.isEmpty()) {
    obj.decodeString(buffer.constData());
  }

  return obj;
}

std::vector<ZIntPoint> ZDvidReader::readSynapsePosition(
    const ZIntCuboid &box) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(dvidUrl.getSynapseUrl(box));

  std::vector<ZIntPoint> posArray;

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (synapseJson.hasKey("Pos")) {
      ZJsonArray posJson(synapseJson.value("Pos"));
      int x = ZJsonParser::integerValue(posJson.at(0));
      int y = ZJsonParser::integerValue(posJson.at(1));
      int z = ZJsonParser::integerValue(posJson.at(2));
      posArray.push_back(ZIntPoint(x, y, z));
    }
  }

  return posArray;
}

ZJsonObject ZDvidReader::readSynapseJson(const ZIntPoint &pt) const
{
  return readSynapseJson(pt.getX(), pt.getY(), pt.getZ());
}

ZJsonObject ZDvidReader::readSynapseJson(int x, int y, int z) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject synapseJson;
  ZIntCuboid box(x, y, z, x, y, z);
  ZJsonArray obj = readJsonArray(dvidUrl.getSynapseUrl(box));
  if (obj.size() > 0) {
    synapseJson.set(obj.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
  }

  return synapseJson;
}

std::vector<ZDvidSynapse> ZDvidReader::readSynapse(
    const ZIntCuboid &box, NeuTube::FlyEM::ESynapseLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(dvidUrl.getSynapseUrl(box));

  std::vector<ZDvidSynapse> synapseArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    synapseArray[i].loadJsonObject(synapseJson, mode);
  }

  return synapseArray;
}

std::vector<ZDvidSynapse> ZDvidReader::readSynapse(
    uint64_t label, NeuTube::FlyEM::ESynapseLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(dvidUrl.getSynapseUrl(label));

  std::vector<ZDvidSynapse> synapseArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    synapseArray[i].loadJsonObject(synapseJson, mode);
  }

  return synapseArray;
}

ZDvidSynapse ZDvidReader::readSynapse(
    int x, int y, int z, NeuTube::FlyEM::ESynapseLoadMode mode) const
{
  std::vector<ZDvidSynapse> synapseArray =
      readSynapse(ZIntCuboid(x, y, z, x, y, z), mode);
  if (!synapseArray.empty()) {
    return synapseArray[0];
  }

  return ZDvidSynapse();
}

std::vector<ZFlyEmToDoItem> ZDvidReader::readToDoItem(
    const ZIntCuboid &box) const
{
  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonArray obj = readJsonArray(dvidUrl.getTodoListUrl(box));

  std::vector<ZFlyEmToDoItem> itemArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject itemJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZFlyEmToDoItem &item = itemArray[i];
    item.loadJsonObject(itemJson);
  }

  return itemArray;
}

ZFlyEmToDoItem ZDvidReader::readToDoItem(int x, int y, int z) const
{
  std::vector<ZFlyEmToDoItem> itemArray =
      readToDoItem(ZIntCuboid(x, y, z, x, y, z));
  if (!itemArray.empty()) {
    return itemArray[0];
  }

  return ZFlyEmToDoItem();
}
