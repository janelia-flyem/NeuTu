//#define _NEUTU_USE_REF_KEY_
#include "zdvidreader.h"

#include <vector>
#include <ctime>
#include <future>

#include <archive.h>
#include <archive_entry.h>
#include <QThread>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>

#include "zqslog.h"

#include "zjsondef.h"
#include "zstack.hxx"
//#include "zdvidbuffer.h"
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
#include "neutubeconfig.h"
#include "flyem/zflyemmisc.h"
#include "zdvidutil.h"
#include "dvid/zdvidroi.h"
#include "zflyemutilities.h"
#include "zobject3dscanarray.h"
#include "zdvidpath.h"
#include "flyem/zserviceconsumer.h"
#include "zmeshio.h"
#include "zmesh.h"
#include "zstackobjectsourcefactory.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "znetbufferreader.h"
#include "geometry/zaffinerect.h"
#include "zarrayfactory.h"
#include "zobject3dfactory.h"
#include "dvid/zdvidstackblockfactory.h"

ZDvidReader::ZDvidReader(/*QObject *parent*/) :
  /*QObject(parent),*/ m_verbose(true)
{
  init();
}

ZDvidReader::~ZDvidReader()
{
}

void ZDvidReader::init()
{
  m_readingTime = 0;
  m_statusCode = 0;
}

int ZDvidReader::getStatusCode() const
{
  return m_statusCode;
}

void ZDvidReader::setStatusCode(int code) const
{
  m_statusCode = code;
}

void ZDvidReader::clear()
{
  m_dvidTarget.clear();
#if defined(_ENABLE_LOWTIS_)
  m_lowtisService.reset();
  m_lowtisServiceGray.reset();
#endif
  m_service.reset();
  m_connection.reset();

  m_errorMsg.clear();
  m_statusCode = 0;
  m_bufferReader.clear();

}

bool ZDvidReader::startService()
{
  if (getDvidTarget().isMock()) {
    return true;
  }

#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    m_service = ZDvid::MakeDvidNodeService(getDvidTarget());
    m_connection = ZDvid::MakeDvidConnection(getDvidTarget().getAddressWithPort());
    m_bufferReader.setService(getDvidTarget());
  } catch (std::exception &e) {
    m_service.reset();
    m_errorMsg = e.what();
    STD_COUT << e.what() << std::endl;
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
  if (serverAddress.isEmpty()) {
    return false;
  }

  if (uuid.isEmpty()) {
    return false;
  }

  ZDvidTarget target;
  target.set(serverAddress.toStdString(), uuid.toStdString(), port);

  return open(target);
}

bool ZDvidReader::openRaw(const ZDvidTarget &target)
{
  if (!target.isValid()) {
    return false;
  }

  m_dvidTarget = target;

  bool succ = startService();

  if (!succ) {
    m_dvidTarget.setNodeStatus(ZDvid::NODE_OFFLINE);
  }

  return succ;
}

bool ZDvidReader::open(const ZDvidTarget &target)
{
  if (!target.isValid()) {
    return false;
  }

  bool succ = false;

  if (target.isInferred() || target.isMock()) {
    succ = openRaw(target);
  } else {
    m_dvidTarget = target;

    std::string masterNode = ReadMasterNode(target);
    if (!masterNode.empty()) {
      m_dvidTarget.setUuid(masterNode);
    }

    succ = startService();

    if (succ && !getDvidTarget().isMock()) { //Read default settings
      updateNodeStatus();

      if (getDvidTarget().usingDefaultDataSetting()) {
        loadDefaultDataSetting();
      }

      updateSegmentationData();
      m_dvidTarget.setInferred(true);
    } else {
      m_dvidTarget.setNodeStatus(ZDvid::NODE_OFFLINE);
    }
  }

  return succ;
}

std::vector<std::string> ZDvidReader::readDataInstances(const std::string &type)
{
  std::vector<std::string> dataList;

  ZJsonObject meta = readInfo();

  //
  ZJsonValue datains = meta.value("DataInstances");

  if(datains.isObject())
  {
    ZJsonObject insList(datains);
    std::vector<std::string> keys = insList.getAllKey();

    for(std::size_t i=0; i<keys.size(); i++)
    {
      std::string name = keys.at(i);
      ZJsonObject roiJson(insList.value(name.c_str()));
      if (roiJson.hasKey("Base")) {
        ZJsonObject baseJson(roiJson.value("Base"));
        std::string typeName =
            ZJsonParser::stringValue(baseJson["TypeName"]);
        if (typeName == type) {
          dataList.push_back(name);
        }
      }
    }
  }

  return dataList;
}

void ZDvidReader::updateSegmentationData()
{
  std::string typeName;
  if (getDvidTarget().hasSegmentation()) {
    typeName = getType(getDvidTarget().getSegmentationName());
  } else {
    typeName = getType(getDvidTarget().getBodyLabelName());
  }

  if (typeName == "labelarray") {
    getDvidTarget().setSegmentationType(ZDvidData::TYPE_LABELARRAY);
  } else if (typeName == "labelmap") {
    getDvidTarget().setSegmentationType(ZDvidData::TYPE_LABELMAP);
  }

  if (getDvidTarget().getBodyLabelName().empty()) {
    syncBodyLabelName();
  }
  if (!getDvidTarget().hasSegmentation()) {
    if (getDvidTarget().segmentationAsBodyLabel()) {
      getDvidTarget().setSegmentationName(getDvidTarget().getBodyLabelName());
    }
  }
}

bool ZDvidReader::open(const QString &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString.toStdString());
  return open(target);
}

void ZDvidReader::updateNodeStatus()
{
  m_dvidTarget.setNodeStatus(getNodeStatus());
}

ZDvid::ENodeStatus ZDvidReader::getNodeStatus() const
{
  ZDvid::ENodeStatus status = ZDvid::NODE_NORMAL;

#ifdef _ENABLE_LIBDVIDCPP_
  ZDvidUrl url(getDvidTarget());
  std::string repoUrl = url.getRepoUrl();
  if (repoUrl.empty()) {
    status = ZDvid::NODE_INVALID;
  } else {
    int statusCode = 0;
    ZDvid::MakeHeadRequest(repoUrl, statusCode);
    if (statusCode != 200) {
      status = ZDvid::NODE_OFFLINE;
    } else {
      ZJsonObject obj = readJsonObject(url.getCommitInfoUrl());
      if (obj.hasKey("Locked")) {
        bool locked = ZJsonParser::booleanValue(obj["Locked"]);
        if (locked) {
          status = ZDvid::NODE_LOCKED;
        }
      }
    }
  }
#endif

  return status;
}

void ZDvidReader::testApiLoad()
{
#if defined(_ENABLE_LIBDVIDCPP_)
  ZDvid::MakeRequest(*m_connection, "/api/load", "GET",
                     libdvid::BinaryDataPtr(), libdvid::DEFAULT,
                     m_statusCode);
#endif
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType,
    int z, neutube::EAxis axis, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    std::string url;
    if (labelType == flyem::EBodyLabelType::BODY) {
      url = dvidUrl.getSparsevolUrl(bodyId, z, z, axis);
    } else {
      url = dvidUrl.getSupervoxelUrl(bodyId, z, z, axis);
    }

    QElapsedTimer timer;
    timer.start();
    reader.read(url.c_str(), isVerbose());
    ZOUT(LTRACE(), 5) << "Reading time:" << url << timer.elapsed() << "ms";

    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size());
    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
//      result->fullySortedCanonize();
    }

    result->setLabel(bodyId);
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, int z, neutube::EAxis axis, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    std::string url = dvidUrl.getSparsevolUrl(bodyId, z, z, axis);
    QElapsedTimer timer;
    timer.start();
    reader.read(url.c_str(), isVerbose());
    ZOUT(LTRACE(), 5) << "Reading time:" << url << timer.elapsed() << "ms";

    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size());
    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
//      result->fullySortedCanonize();
    }

    result->setLabel(bodyId);
  }

  return result;
}

ZObject3dScan* ZDvidReader::readBodyWithPartition(
    uint64_t bodyId, flyem::EBodyLabelType labelType, ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    const ZObject3dScan &coarseBody = readCoarseBody(bodyId, labelType);
    ZDvidInfo dvidInfo = readLabelInfo();
    int minZ = dvidInfo.getCoordZ(coarseBody.getMinZ());
    int maxZ = dvidInfo.getCoordZ(coarseBody.getMaxZ()) +
        dvidInfo.getBlockSize().getZ() - 1;

    int dz = 1024 / dvidInfo.getBlockSize().getZ() *
        dvidInfo.getBlockSize().getZ() - 1;

    int startZ = minZ;
    int endZ = (startZ + dz) / dvidInfo.getBlockSize().getZ() *
        dvidInfo.getBlockSize().getZ() - 1;
    ZObject3dScan part;
    while (startZ <= maxZ) {
      if (endZ > maxZ) {
        endZ = maxZ;
      }
      if (startZ == minZ) {
#ifdef _DEBUG_
        STD_COUT << "Read first part: " << startZ << "--" << endZ << std::endl;
#endif
        readBody(bodyId, labelType, startZ, endZ, true, neutube::EAxis::Z, result);
      } else {
#ifdef _DEBUG_
        STD_COUT << "Read part: " << startZ << "--" << endZ << std::endl;
#endif
        readBody(bodyId, labelType, startZ, endZ, true, neutube::EAxis::Z, &part);
        if (!part.isEmpty()) {
          result->unify(part);
        }
      }
      startZ = endZ + 1;
      endZ = startZ + dz;
    }
  }

  return result;
}

ZObject3dScan* ZDvidReader::readBodyWithPartition(
    uint64_t bodyId, ZObject3dScan *result) const
{
  return readBodyWithPartition(bodyId, flyem::EBodyLabelType::BODY, result);
#if 0
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    const ZObject3dScan &coarseBody = readCoarseBody(bodyId);
    ZDvidInfo dvidInfo = readLabelInfo();
    int minZ = dvidInfo.getCoordZ(coarseBody.getMinZ());
    int maxZ = dvidInfo.getCoordZ(coarseBody.getMaxZ()) +
        dvidInfo.getBlockSize().getZ() - 1;

    int dz = 1000;

    int startZ = minZ;
    int endZ = startZ + dz;
    ZObject3dScan part;
    while (startZ <= maxZ) {
      if (endZ > maxZ) {
        endZ = maxZ;
      }
      if (startZ == minZ) {
#ifdef _DEBUG_
        STD_COUT << "Read first part: " << startZ << "--" << endZ << std::endl;
#endif
        readBody(bodyId, startZ, endZ, true, neutube::Z_AXIS, result);
      } else {
#ifdef _DEBUG_
        STD_COUT << "Read part: " << startZ << "--" << endZ << std::endl;
#endif
        readBody(bodyId, startZ, endZ, true, neutube::Z_AXIS, &part);
        if (!part.isEmpty()) {
          result->unify(part);
        }
      }
      startZ = endZ + 1;
      endZ = startZ + dz;
    }
  }

  return result;
#endif
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, int minZ, int maxZ, bool canonizing, neutube::EAxis axis,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());
    reader.read(dvidUrl.getSparsevolUrl(bodyId, minZ, maxZ, axis).c_str(),
                isVerbose());
    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size());
    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
    }

    result->setLabel(bodyId);
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType,
    int minZ, int maxZ, bool canonizing, neutube::EAxis axis,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    if (labelType == flyem::EBodyLabelType::BODY) {
      reader.read(dvidUrl.getSparsevolUrl(bodyId, minZ, maxZ, axis).c_str(),
                  isVerbose());
    } else if (labelType == flyem::EBodyLabelType::SUPERVOXEL) {
      reader.read(dvidUrl.getSupervoxelUrl(bodyId, minZ, maxZ, axis).c_str(),
                  isVerbose());
    }

    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size());
    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
    }

    result->setLabel(bodyId);
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, const ZIntCuboid &box, bool canonizing,
    ZObject3dScan *result) const
{
  return readBody(bodyId, flyem::EBodyLabelType::BODY, box, canonizing, result);

#if 0
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());
    reader.read(dvidUrl.getSparsevolUrl(bodyId, box).c_str(),
                isVerbose());
    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size());

    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
    }

    result->setLabel(bodyId);
  }

  return result;
#endif
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType,
    const ZIntCuboid &box, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    bool needPartition = false;

    if (box.isEmpty()) {
      size_t bodySize = readBodySize(bodyId, labelType);

      if (bodySize / neutube::ONEGIGA > 4) {
        needPartition = true;
      }
    }

    ZDvidBufferReader &reader = m_bufferReader;

    if (needPartition == false) {
      ZDvidUrl dvidUrl(getDvidTarget());
      switch (labelType) {
      case flyem::EBodyLabelType::BODY:
        reader.read(dvidUrl.getSparsevolUrl(bodyId, box).c_str(),
                    isVerbose());
        break;
      case flyem::EBodyLabelType::SUPERVOXEL:
        reader.read(dvidUrl.getSupervoxelUrl(bodyId, box).c_str(),
                    isVerbose());
        break;
      }

      if (reader.getStatus() != neutube::EReadStatus::FAILED) {
        const QByteArray &buffer = reader.getBuffer();
        result->importDvidObjectBuffer(buffer.data(), buffer.size());
      } else {
        if (box.isEmpty()) {
          needPartition = true;
        }
      }
      reader.clearBuffer();
    }

    if (needPartition) {
      result = readBodyWithPartition(bodyId, labelType, result);
    }

    if (result) {
      if (canonizing) {
        result->canonize();
      }

      result->setLabel(bodyId);
    }
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType, int zoom,
    const ZIntCuboid &box, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    if (getDvidTarget().hasBlockCoding()) {
      ZDvidUrl::SparsevolConfig config;
      config.bodyId = bodyId;
      config.format = "blocks";
      config.range = box;
      config.zoom = zoom;
      config.labelType = labelType;

      ZDvidUrl dvidUrl(getDvidTarget());
      QByteArray buffer = readBuffer(dvidUrl.getSparsevolUrl(config));
      result->importDvidBlockBuffer(buffer.data(), buffer.size(), canonizing);
    } else {
      readBodyRle(bodyId, labelType, zoom, box, canonizing, result);
    }

  }

  return result;
}

ZObject3dScan *ZDvidReader::readBodyRle(
    uint64_t bodyId, flyem::EBodyLabelType labelType, int zoom,
    const ZIntCuboid &box, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    /*
    ZIntCuboid range = box;
    if (!range.isEmpty()) {
      if (zoom > 0) {
        int scale = pow(2, zoom);
        range.scaleDown(scale);
      }
    }
    */

    bool buffered = true;

    //  reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());
    switch (labelType) {
    case flyem::EBodyLabelType::BODY:
      if (zoom == 0 || box.isEmpty()) {
        reader.read(dvidUrl.getSparsevolUrl(bodyId, zoom, box).c_str(),
                    isVerbose());
      } else {
        ZObject3dScan coarseBody;
        readCoarseBody(bodyId, labelType, box, &coarseBody);
        int scale = zgeom::GetZoomScale(zoom);
        coarseBody.downsampleMax(scale - 1, scale -1, scale -1);
        std::vector<ZArray*> blockArray = readLabelBlock(coarseBody, zoom);

        ZIntCuboid range = box;
        range.scaleDown(scale);
        ZObject3dFactory::MakeObject3dScan(blockArray, bodyId, range, result);
        for (ZArray *array : blockArray) {
          delete array;
        }
        buffered = false;
      }
      break;
    case flyem::EBodyLabelType::SUPERVOXEL:
      reader.read(dvidUrl.getSupervoxelUrl(bodyId, zoom, box).c_str(),
                  isVerbose());
      break;
    }

    if (buffered) {
      const QByteArray &buffer = reader.getBuffer();
      result->importDvidObjectBuffer(buffer.data(), buffer.size());
    }

    reader.clearBuffer();

    if (canonizing) {
      result->canonize();
    }

    result->setLabel(bodyId);
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBodyDs(
    uint64_t bodyId, bool canonizing, ZObject3dScan *result)
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    QElapsedTimer timer;
    timer.start();

    reader.read(dvidUrl.getSparsevolUrl(bodyId).c_str(), isVerbose());

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    timer.start();
    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBufferDs(buffer.data(), buffer.size());
    if (canonizing) {
      result->canonize();
    }

    STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;

    reader.clearBuffer();

    result->setLabel(bodyId);
  }

  return result;
}



ZObject3dScan *ZDvidReader::readBodyDs(
    uint64_t bodyId, int xIntv, int yIntv, int zIntv, bool canonizing,
    ZObject3dScan *result)
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    QElapsedTimer timer;
    timer.start();

    reader.read(dvidUrl.getSparsevolUrl(bodyId).c_str(), isVerbose());

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    timer.start();
    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBuffer(buffer.data(), buffer.size(),
                                   xIntv, yIntv, zIntv);
    if (canonizing) {
      result->canonize();
    }

    STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;

    reader.clearBuffer();

    result->setLabel(bodyId);
  }

  return result;
}

QByteArray ZDvidReader::readBuffer(const std::string &url) const
{
  if (isVerbose()) {
    std::cout << "Reading " << url << std::endl;
  }
  m_bufferReader.read(url.c_str());

  return m_bufferReader.getBuffer();
}

QByteArray ZDvidReader::readDataFromEndpoint(
    const std::string &endPoint, bool tryingCompress) const
{
  QByteArray buffer;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    if (m_service.get() != NULL) {
      libdvid::BinaryDataPtr data = m_service->custom_request(
            endPoint, libdvid::BinaryDataPtr(), libdvid::GET, tryingCompress);
      buffer.append(data->get_data().c_str(), data->length());
      m_statusCode = 200;
    } else {
      m_statusCode = 0;
    }
  } catch (libdvid::DVIDException &e) {
    STD_COUT << e.what() << std::endl;
    m_statusCode = e.getStatus();
  }
#endif

  return buffer;
}

ZObject3dScan *ZDvidReader::readSupervoxel(
    uint64_t bodyId, bool canonizing, ZObject3dScan *result) const
{
  return readBody(
        bodyId, flyem::EBodyLabelType::SUPERVOXEL, ZIntCuboid(), canonizing, result);
#if 0
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    size_t bodySize = readBodySize(bodyId, flyem::LABEL_SUPERVOXEL);
    bool needPartition = false;
    if (bodySize / neutube::ONEGIGA > 4) {
      needPartition = true;
    }

    if (!needPartition) {
      ZDvidBufferReader &reader = m_bufferReader;

      reader.tryCompress(true);
      ZDvidUrl dvidUrl(getDvidTarget());

      QElapsedTimer timer;
      timer.start();

      reader.read(dvidUrl.getSupervoxelUrl(bodyId).c_str(), isVerbose());

      reader.tryCompress(false);

      STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

      if (reader.getStatus() != neutube::EReadStatus::READ_FAILED) {
        timer.start();
        const QByteArray &buffer = reader.getBuffer();
        result->importDvidObjectBuffer(buffer.data(), buffer.size());

#ifdef _DEBUG_
        STD_COUT << "Canonized:" << result->isCanonizedActually() << std::endl;
        //    result->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

        STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;
        result->setLabel(bodyId);
        needPartition = false;

        reader.clearBuffer();
      } else {
        needPartition = true;
      }
    }

    if (needPartition) {
      result = readBodyWithPartition(bodyId, flyem::LABEL_SUPERVOXEL, result);
    }

    if (canonizing && result) {
      result->canonize();
    }


  }

  return result;
#endif
}

uint64_t ZDvidReader::readParentBodyId(uint64_t spId) const
{
  uint64_t parentId = spId;

  ZDvidUrl url(getDvidTarget());

  QByteArray payload(("[" + std::to_string(spId) + "]").c_str());
  ZJsonArray arrayJson = readJsonArray(url.getLabelMappingUrl(), payload);
  if (!arrayJson.isEmpty()) {
    parentId = ZJsonParser::integerValue(arrayJson.at(0));
  }

  return parentId;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, bool canonizing, ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    QElapsedTimer timer;
    timer.start();

    reader.read(dvidUrl.getSparsevolUrl(bodyId).c_str(), isVerbose());

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    if (reader.getStatus() != neutube::EReadStatus::FAILED) {
      timer.start();
      const QByteArray &buffer = reader.getBuffer();
      result->importDvidObjectBuffer(buffer.data(), buffer.size());
      if (canonizing) {
        result->canonize();
      }

#ifdef _DEBUG_
      STD_COUT << "Canonized:" << result->isCanonizedActually() << std::endl;
      //    result->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

      STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;
      result->setLabel(bodyId);
    } else {
      readBodyWithPartition(bodyId, result);
    }
    reader.clearBuffer();
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType,
    bool canonizing, ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    QElapsedTimer timer;
    timer.start();

    switch (labelType) {
    case flyem::EBodyLabelType::BODY:
      reader.read(dvidUrl.getSparsevolUrl(bodyId).c_str(), isVerbose());
      break;
    case flyem::EBodyLabelType::SUPERVOXEL:
      reader.read(dvidUrl.getSupervoxelUrl(bodyId).c_str(), isVerbose());
      break;
    }

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    if (reader.getStatus() != neutube::EReadStatus::FAILED) {
      timer.start();
      const QByteArray &buffer = reader.getBuffer();
      result->importDvidObjectBuffer(buffer.data(), buffer.size());
      if (canonizing) {
        result->canonize();
      }

#ifdef _DEBUG_
      STD_COUT << "Canonized:" << result->isCanonizedActually() << std::endl;
      //    result->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

      STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;
      result->setLabel(bodyId);
    } else {
      readBodyWithPartition(bodyId, result);
    }
    reader.clearBuffer();
  }

  return result;
}

ZObject3dScanArray* ZDvidReader::readBody(const std::set<uint64_t> &bodySet) const
{
  ZObject3dScanArray *objArray = NULL;

  if (!bodySet.empty()) {
    objArray = new ZObject3dScanArray[bodySet.size()];

    int index = 0;
    for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter) {
      ZObject3dScan *obj = (*objArray)[index++];
      readBody(*iter, false, obj);
    }
  }

  return objArray;
}

ZMesh* ZDvidReader::readMeshFromUrl(const std::string &url) const
{
  ZDvidTarget target;
  target.setFromUrl(url);
  if (target.getAddressWithPort() != getDvidTarget().getAddressWithPort() ||
      target.getUuid() != getDvidTarget().getUuid()) {
    LWARN() << "Unmatched target";
    return NULL;
  }

  ZMesh *mesh = NULL;

  std::string format = "obj";

  ZJsonObject infoJson = readJsonObject(ZDvidUrl::GetMeshInfoUrl(url));
  if (infoJson.hasKey("format")) {
    format = ZJsonParser::stringValue(infoJson["format"]);
  }

  m_bufferReader.read(url.c_str(), isVerbose());
  if (m_bufferReader.getStatus() != neutube::EReadStatus::FAILED) {
    const QByteArray &buffer = m_bufferReader.getBuffer();
    mesh = ZMeshIO::instance().loadFromMemory(buffer, format);
  }
  m_bufferReader.clearBuffer();

  return mesh;
}

ZMesh* ZDvidReader::readMesh(uint64_t bodyId, int zoom) const
{
  ZDvidUrl dvidUrl(getDvidTarget());
  std::string meshUrl = dvidUrl.getMeshUrl(bodyId, zoom);
  ZMesh *mesh = readMeshFromUrl(meshUrl);

  if (mesh != NULL) {
    mesh->setLabel(bodyId);
  }

  return mesh;
}

ZMesh* ZDvidReader::readMesh(const std::string &data, const std::string &key)
const
{
  ZDvidUrl dvidUrl(getDvidTarget());
  std::string meshUrl = dvidUrl.getKeyUrl(data, key);
  ZMesh *mesh = readMeshFromUrl(meshUrl);

  return mesh;
}

ZMesh* ZDvidReader::readSupervoxelMesh(uint64_t svId) const
{
  ZMesh *mesh = NULL;

  ZDvidUrl dvidUrl(getDvidTarget());
  std::string url = dvidUrl.getSupervoxelMeshUrl(svId);
  if (!url.empty()) {
    m_bufferReader.read(url.c_str(), isVerbose());
    if (m_bufferReader.getStatus() != neutube::EReadStatus::FAILED) {
      const QByteArray &buffer = m_bufferReader.getBuffer();
      mesh = ZMeshIO::instance().loadFromMemory(buffer, "drc");
    }
    m_bufferReader.clearBuffer();
  }

  return mesh;
}

struct archive *ZDvidReader::readMeshArchiveStart(uint64_t bodyId, bool useOldMeshesTars) const
{
  size_t bytesTotal;
  return readMeshArchiveStart(bodyId, bytesTotal, useOldMeshesTars);
}

struct archive *ZDvidReader::readMeshArchiveStart(uint64_t bodyId, size_t &bytesTotal, bool useOldMeshesTars) const
{
  bytesTotal = 0;

  ZDvidUrl dvidUrl(getDvidTarget());

  std::string tarUrl = useOldMeshesTars ? dvidUrl.getMeshesTarsUrl(bodyId) :
                                          dvidUrl.getTarSupervoxelsUrl(bodyId);
  if (tarUrl.empty()) {
    LWARN() << "Empty mesh archive url for" << bodyId;
  } else {
    LINFO() << "Reading mesh archive:" << tarUrl;
  }

  QTime timer;
  timer.start();

  m_bufferReader.read(tarUrl.c_str(), isVerbose());
  if (m_bufferReader.getStatus() == neutube::EReadStatus::FAILED) {
    return nullptr;
  }

  const QByteArray &buffer = m_bufferReader.getBuffer();

  LINFO() << "Reading the mesh archive for ID " << bodyId << " (" << buffer.size() << " bytes) took " << timer.elapsed() << " ms.";

  struct archive *arc = archive_read_new();
  archive_read_support_format_all(arc);

  int result = archive_read_open_memory(arc, buffer.constData(), buffer.size());
    if (result != ARCHIVE_OK) {
    return nullptr;
  }

  bytesTotal = buffer.size();
  return arc;
}

ZMesh *ZDvidReader::readMeshArchiveNext(struct archive *arc) const
{
  size_t bytesJustRead;
  return readMeshArchiveNext(arc, bytesJustRead);
}

ZMesh *ZDvidReader::readMeshArchiveNext(struct archive *arc, size_t &bytesJustRead) const
{
  bytesJustRead = 0;

  struct archive_entry *entry;
  if (archive_read_next_header(arc, &entry) != ARCHIVE_OK) {
    return nullptr;
  }

  std::string pathname = archive_entry_pathname(entry);
  auto i = pathname.find_last_of('.');
  std::string bodyIdStr = (i != std::string::npos) ? pathname.substr(0, i) : pathname;

  const struct stat *s = archive_entry_stat(entry);
  size_t size = s->st_size;

  QByteArray buffer(size, 0);
  archive_read_data(arc, buffer.data(), size);

  std::string format = "drc";
  ZMesh *mesh = ZMeshIO::instance().loadFromMemory(buffer, format);
  if (mesh != NULL) {
    mesh->setLabel(std::stoull(bodyIdStr));
  }

  bytesJustRead = size;
  return mesh;
}

namespace {
struct BodyIdStrAndBuffer
{
  BodyIdStrAndBuffer(const std::string& bid = "", const QByteArray& bu = QByteArray())
    : bodyIdStr(bid), buffer(bu) {}
  std::string bodyIdStr;
  QByteArray buffer;
};
}

void ZDvidReader::readMeshArchiveAsync(archive *arc, std::vector<ZMesh *> &results,
                                       const std::function<void(size_t, size_t)>& progress) const
{
  QTime timer;
  timer.start();

  // Read all the compressed meshes into a vector of buffers.

  QVector<BodyIdStrAndBuffer> buffers;
  struct archive_entry *entry;
  while (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
    std::string pathname = archive_entry_pathname(entry);
    auto i = pathname.find_last_of('.');
    std::string bodyIdStr = (i != std::string::npos) ? pathname.substr(0, i) : pathname;

    const struct stat *s = archive_entry_stat(entry);
    size_t size = s->st_size;

    QByteArray buffer(size, 0);
    archive_read_data(arc, buffer.data(), size);

    buffers.push_back(BodyIdStrAndBuffer(bodyIdStr, buffer));
  }

  // This function will decompress one buffer in the vector, and it will be applied in parallel.

  std::function<ZMesh*(const BodyIdStrAndBuffer&)> buildMesh = [](const BodyIdStrAndBuffer& idBuf) {
    ZMesh *mesh = nullptr;
    try {
      mesh = ZMeshIO::instance().loadFromMemory(idBuf.buffer, "drc");
    } catch (...) {}
    if (mesh) {
      mesh->setLabel(std::stoull(idBuf.bodyIdStr));
    }
    return mesh;
  };

  // Decompress the meshes in parallel.  This Qt function does use a thread pool, so it does not
  // have the problem that std::async() has on Linux (as of 2018), that std::launch::async is necessary
  // to get parallelism but it will allow unlimited thread creation until the limit is reached an
  // exception is generated.

  QFuture<ZMesh*> futures = QtConcurrent::mapped(buffers, buildMesh);

  // Get each decompressed mesh from the QFuture, with progress reporting.  Despite the Qt documentation,
  // it seems to be necssary to use the QFuture::resultAt() function to get a result when it is ready.
  // The QFuture::const_iterator API causes a crash.

  for (int i = 0; i < buffers.size(); ++i) {
    ZMesh *mesh = futures.resultAt(i);
    if (mesh) {
      results.push_back(mesh);
    }

    if (progress) {
      progress(i+1, buffers.size());
    }
  }

  if (buffers.size() == 1) {
    LINFO() << "Decompressing the mesh archive (1 mesh) took " << timer.elapsed() << " ms.";
  } else {
    LINFO() << "Decompressing the mesh archive (" << buffers.size() << " meshes) took " << timer.elapsed() << " ms.";
  }
}

void ZDvidReader::readMeshArchiveEnd(struct archive *arc) const
{
  m_bufferReader.clearBuffer();
  archive_read_free(arc);
}

ZObject3dScan* ZDvidReader::readMultiscaleBody(
    uint64_t bodyId, int zoom, bool canonizing, ZObject3dScan *result) const
{
  result = readBody(
        bodyId, flyem::EBodyLabelType::BODY, zoom, ZIntCuboid(), canonizing, result);
  int scale = zgeom::GetZoomScale(zoom);
  result->setDsIntv(scale - 1);

  return result;

#if 0
  if (result != NULL) {
    result->clear();
  }

  if (isReady() && zoom <= getDvidTarget().getMaxLabelZoom()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    ZDvidBufferReader &reader = m_bufferReader;

    reader.tryCompress(true);
    ZDvidUrl dvidUrl(getDvidTarget());

    QElapsedTimer timer;
    timer.start();

    reader.read(
          dvidUrl.getMultiscaleSparsevolUrl(bodyId, zoom).c_str(), isVerbose());

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    if (reader.getStatus() != neutube::EReadStatus::FAILED) {
      timer.start();
      const QByteArray &buffer = reader.getBuffer();
      result->importDvidObjectBuffer(buffer.data(), buffer.size());
      if (canonizing) {
        result->canonize();
      }

#ifdef _DEBUG_2
      STD_COUT << "Canonized:" << result->isCanonizedActually() << std::endl;
      //    result->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

      STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;
      result->setLabel(bodyId);
      int zoomRatio = pow(2, zoom);
      result->setDsIntv(zoomRatio - 1);
    }

    reader.clearBuffer();
  }

  return result;
#endif
}

/*
ZObject3dScan ZDvidReader::readBody(uint64_t bodyId, bool canonizing)
{
  ZObject3dScan obj;
  readBody(bodyId, canonizing, &obj);

  return obj;
}
*/

ZSwcTree* ZDvidReader::readSwc(uint64_t bodyId) const
{
  if (!isReady()) {
    return NULL;
  }

  ZDvidBufferReader &reader = m_bufferReader;
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

    reader.clearBuffer();
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
  if (!isReady()) {
    return NULL;
  }

  ZDvidUrl url(getDvidTarget());
  qDebug() << url.getThumbnailUrl(bodyId);

  ZDvidBufferReader &reader = m_bufferReader;
  reader.read(url.getThumbnailUrl(bodyId).c_str(), isVerbose());

  Mc_Stack *rawStack =
      C_Stack::readMrawFromBuffer(reader.getBuffer().constData());

  reader.clearBuffer();

  ZStack *stack = NULL;
  if (rawStack != NULL) {
    stack = new ZStack;
    stack->setData(rawStack);
    //  m_image.setData(stack);
  }

  return stack;
}

ZStack* ZDvidReader::readGrayScale(const ZIntCuboid &cuboid) const
{
  return readGrayScale(cuboid.getFirstCorner().getX(),
                       cuboid.getFirstCorner().getY(),
                       cuboid.getFirstCorner().getZ(),
                       cuboid.getWidth(), cuboid.getHeight(),
                       cuboid.getDepth());
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlockOld(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
    int blockNumber) const
{
  std::vector<ZStack*> stackArray(blockNumber, NULL);

  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
#ifdef _DEBUG_2
  tic();
#endif

  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ(),
                                                 blockNumber).c_str(),
                    isVerbose());
  m_statusCode = bufferReader.getStatusCode();
#ifdef _DEBUG_2
  STD_COUT << "reading time:" << std::endl;
  ptoc();
#endif

#ifdef _DEBUG_2
  tic();
#endif

  if (bufferReader.getStatus() == neutube::EReadStatus::OK) {
    const QByteArray &data = bufferReader.getBuffer();
    if (data.length() > 0) {
//      int realBlockNumber = *((int*) data.constData());

      ZIntCuboid currentBox = dvidInfo.getBlockBox(blockIndex);
      for (int i = 0; i < blockNumber; ++i) {
        //stackArray[i] = ZStackFactory::makeZeroStack(GREY, currentBox);
        stackArray[i] = new ZStack(GREY, currentBox, 1);
#ifdef _DEBUG_2
        STD_COUT << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
        stackArray[i]->copyValueFrom(data.constData() + i * currentBox.getVolume(),
                         currentBox.getVolume(), stackArray[i]->array8());
        currentBox.translateX(currentBox.getWidth());
      }
    }
  }

#ifdef _DEBUG_2
  STD_COUT << "parsing time:" << std::endl;
  ptoc();
#endif

  return stackArray;
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
    int blockNumber, int zoom) const
{
  std::vector<ZStack*> stackArray(blockNumber, NULL);

  bool processed = false;
#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service != NULL && getDvidTarget().getMaxLabelZoom() == 0) {
    try {
      std::vector<int> blockCoords(3);
      blockCoords[0] = blockIndex.getX();
      blockCoords[1] = blockIndex.getY();
      blockCoords[2] = blockIndex.getZ();
#ifdef _DEBUG_
        STD_COUT << "starting reading: zoom = " << zoom << std::endl;
        STD_COUT << getDvidTarget().getGrayScaleName() << std::endl;
        STD_COUT << blockCoords[0] << " " << blockCoords[1] << " " << blockCoords[2] << std::endl;

        STD_COUT << blockNumber << std::endl;
#endif
      libdvid::GrayscaleBlocks blocks = m_service->get_grayblocks(
            getDvidTarget().getGrayScaleName(zoom), blockCoords, blockNumber);
#ifdef _DEBUG_
        STD_COUT << "one read done" << std::endl;
#endif

      ZIntCuboid currentBox = dvidInfo.getBlockBox(blockIndex);
      for (int i = 0; i < blockNumber; ++i) {
#ifdef _DEBUG_
        STD_COUT << "block:" << i << "/" << blockNumber << std::endl;
#endif
        ZStack *stack = new ZStack(GREY, currentBox, 1);
        stackArray[i] = stack;
        stack->copyValueFrom(blocks.get_raw() + i * currentBox.getVolume(),
                             currentBox.getVolume(), stack->array8());
        currentBox.translateX(currentBox.getWidth());
#ifdef _DEBUG_2
        stack->save(GET_TEST_DATA_DIR + "/test.tif");
        STD_COUT << "Stack binary: " << stack->isBinary() << std::endl;
#endif
      }
      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      setStatusCode(e.getStatus());
    }

    processed = true;
  }
#endif

  if (!processed) {
    stackArray = readGrayScaleBlockOld(blockIndex, dvidInfo, blockNumber);
  }

  return stackArray;
}

ZStack* ZDvidReader::readGrayScaleBlock(
    const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo) const
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(getDvidTarget());
  bufferReader.read(dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ()).c_str(),
                    isVerbose());
  setStatusCode(bufferReader.getStatusCode());
  ZStack *stack = NULL;
  if (bufferReader.getStatus() == neutube::EReadStatus::OK) {
    const QByteArray &data = bufferReader.getBuffer();
    int realBlockNumber = *((int*) data.constData());

    if (!data.isEmpty() && realBlockNumber == 1) {
      ZIntCuboid box = dvidInfo.getBlockBox(blockIndex);
      stack = ZStackFactory::MakeZeroStack(GREY, box);
#ifdef _DEBUG_2
      STD_COUT << data.length() << " " << stack->getVoxelNumber() << std::endl;
#endif
      stack->copyValueFrom(data.constData() + 4, data.length() - 4, stack->array8());
    }

    bufferReader.clearBuffer();
  }

  return stack;
}

#if 0
ZStack* ZDvidReader::readGrayScaleBlock(int bx, int by, int bz, int zoom) const
{
  std::vector<int> blockcoords;
  blockcoords.push_back(bx);
  blockcoords.push_back(by);
  blockcoords.push_back(bz);

  ZStack *stack = NULL;

  std::vector<libdvid::DVIDCompressedBlock> c_blocks;
  try {
    m_service->get_specificblocks3D(
          getDvidTarget().getGrayScaleName(), blockcoords, false, c_blocks, zoom);

    if (c_blocks.size() == 1) {
      libdvid::DVIDCompressedBlock &block = c_blocks[0];
      libdvid::BinaryDataPtr data = block.get_uncompressed_data();
      std::vector<int> offset = block.get_offset();

      ZDvidInfo info = readLabelInfo();

      ZIntCuboid box;
      box.setFirstCorner(offset[0], offset[1], offset[2]);
      box.setSize(info.getBlockSize());
      stack = ZStackFactory::MakeZeroStack(box, mylib::UINT64_TYPE);
      stack->copyValueFrom(data->get_raw(), data->length());
    }
  } catch(libdvid::DVIDException &e) {
    LERROR() << e.what();
    m_statusCode = e.getStatus();
  } catch (std::exception &e) {
    LERROR() << e.what();
    m_statusCode = 0;
  }

  return stack;
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZObject3dScan &blockObj, int zoom) const
{
  std::vector<ZStack*> result;

  std::vector<libdvid::DVIDCompressedBlock> c_blocks;

  std::vector<int> blockcoords;
  ZObject3dScan::ConstVoxelIterator objIter(&blockObj);
  while (objIter.hasNext()) {
    ZIntPoint pt = objIter.next();
    blockcoords.push_back(pt.getX());
    blockcoords.push_back(pt.getY());
    blockcoords.push_back(pt.getZ());
  }

  try {
    m_service->get_specificblocks3D(
          getDvidTarget().getGrayScaleName(), blockcoords, false, c_blocks, zoom);

    ZDvidInfo info = readLabelInfo();
    for (libdvid::DVIDCompressedBlock &block : c_blocks) {
      libdvid::BinaryDataPtr data = block.get_uncompressed_data();

      std::vector<int> offset = block.get_offset();
      ZIntPoint startCoord(offset[0], offset[1], offset[2]);
      ZStack *stack = ZStackFactory::MakeZeroStack(
            GREY, ZIntCuboid(startCoord, startCoord + info.getBlockSize() - 1));
      stack->copyValueFrom(data->get_raw(), data->length());

      result.push_back(stack);
    }
  } catch(libdvid::DVIDException &e) {
    LERROR() << e.what();
    m_statusCode = e.getStatus();
  } catch (std::exception &e) {
    LERROR() << e.what();
    m_statusCode = 0;
  }

  return result;
}
#endif

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZObject3dScan &blockObj, const ZDvidInfo &info, int zoom) const
{
  DEBUG_OUT << "Reading grayscale blocks: zoom = " << zoom << std::endl;

  std::vector<ZStack*> result;

  ZObject3dScan::ConstSegmentIterator segIter(&blockObj);
  while (segIter.hasNext()) {
    const ZObject3dScan::Segment &seg = segIter.next();
    if (!seg.isEmpty()) {
      ZIntPoint blockIndex(seg.getStart(), seg.getY(), seg.getZ());
      int blockCount = seg.getEnd() - seg.getStart() + 1;
      std::vector<ZStack*> stackArray =
          readGrayScaleBlock(blockIndex, info, blockCount, zoom);
      result.insert(result.end(), stackArray.begin(), stackArray.end());
    }
  }

  return result;
}

std::vector<ZStack*> ZDvidReader::readGrayScaleBlock(
    const ZObject3dScan &blockObj, int zoom) const
{
  return readGrayScaleBlock(blockObj, readGrayScaleInfo(), zoom);
}

ZDvidSparseStack* ZDvidReader::readDvidSparseStack(
    uint64_t bodyId, flyem::EBodyLabelType labelType) const
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setLabelType(labelType);
  spStack->setDvidTarget(getDvidTarget());
  spStack->loadBody(bodyId);
  m_statusCode = spStack->getReadStatusCode();

  return spStack;
}

/*
ZDvidSparseStack* ZDvidReader::readDvidSparseStack(uint64_t bodyId) const
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setDvidTarget(getDvidTarget());
  spStack->loadBody(bodyId);
  m_statusCode = spStack->getReadStatusCode();

  return spStack;
}
*/

ZDvidSparseStack* ZDvidReader::readDvidSparseStack(
    uint64_t bodyId, const ZIntCuboid &range) const
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setDvidTarget(getDvidTarget());

  if (range.isEmpty()) {
    spStack->loadBody(bodyId);
  } else {
    spStack->loadBody(bodyId, range);
  }
  m_statusCode = spStack->getReadStatusCode();

  return spStack;
}

ZDvidSparseStack* ZDvidReader::readDvidSparseStackAsync(
    uint64_t bodyId, flyem::EBodyLabelType labelType) const
{
  ZDvidSparseStack *spStack = new ZDvidSparseStack;
  spStack->setLabelType(labelType);
  spStack->setLabel(bodyId);
  spStack->setDvidTarget(getDvidTarget());
  spStack->loadBodyAsync(bodyId);
  m_statusCode = spStack->getReadStatusCode();

  return spStack;
}

ZSparseStack* ZDvidReader::readSparseStack(uint64_t bodyId, int zoom) const
{
  ZSparseStack *spStack = NULL;

  ZObject3dScan *body = readMultiscaleBody(bodyId, zoom, true, NULL);

  //ZSparseObject *body = new ZSparseObject;
  //body->append(reader.readBody(bodyId));
  //body->canonize();
#ifdef _DEBUG_2
  tic();
#endif

  if (!body->isEmpty()) {
    spStack = new ZSparseStack;
    spStack->setObjectMask(body);

    ZDvidInfo dvidInfo = readDataInfo(getDvidTarget().getGrayScaleName());
    ZObject3dScan blockObj = dvidInfo.getBlockIndex(*body);;
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    spStack->setGreyScale(grid);
//    grid->setMinPoint(dvidInfo.getStartCoordinates());
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
              ZIntPoint(x, y, z);// - dvidInfo.getStartBlockIndex();
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

ZSparseStack* ZDvidReader::readSparseStackOnDemand(
    uint64_t bodyId, flyem::EBodyLabelType type, ZSparseStack *out) const
{
  ZSparseStack *spStack = out;

  ZObject3dScan *body = readBody(bodyId, type, ZIntCuboid(), true, NULL);

#ifdef _DEBUG_2
  tic();
#endif

  if (!body->isEmpty()) {
    if (spStack == nullptr) {
      spStack = new ZSparseStack;
    }
    spStack->setObjectMask(body);

    spStack->setBlockMask(readCoarseBody(bodyId, type, NULL));

    ZDvidStackBlockFactory *blockFactory = new ZDvidStackBlockFactory;
    blockFactory->setDvidTarget(getDvidTarget());

    spStack->setBlockFactory(blockFactory);
  }

  return spStack;
}

ZSparseStack* ZDvidReader::readSparseStack(uint64_t bodyId) const
{
  ZSparseStack *spStack = NULL;

  ZObject3dScan *body = readBody(bodyId, true, NULL);

  //ZSparseObject *body = new ZSparseObject;
  //body->append(reader.readBody(bodyId));
  //body->canonize();
#ifdef _DEBUG_2
  tic();
#endif

  if (!body->isEmpty()) {
    spStack = new ZSparseStack;
    spStack->setObjectMask(body);

    ZDvidInfo dvidInfo = readDataInfo(getDvidTarget().getGrayScaleName());
    ZObject3dScan blockObj = dvidInfo.getBlockIndex(*body);;
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    spStack->setGreyScale(grid);
//    grid->setMinPoint(dvidInfo.getStartCoordinates());
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
              ZIntPoint(x, y, z);// - dvidInfo.getStartBlockIndex();
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

#if 0
ZStack* ZDvidReader::readGrayScaleOld(
    int x0, int y0, int z0, int width, int height, int depth) const
{
  ZStack *stack = NULL;

  ZDvidUrl url(getDvidTarget());

  m_bufferReader.read(url.getGrayscaleUrl(
                        width, height, depth, x0, y0, z0).c_str(), isVerbose());

  setStatusCode(m_bufferReader.getStatusCode());

  const QByteArray &buffer = m_bufferReader.getBuffer();

  if (!buffer.isEmpty()) {
    ZIntCuboid box(x0, y0, z0, x0 + width - 1, y0 + height - 1, z0 + depth - 1);
    stack = new ZStack(GREY, box, 1);

    memcpy(stack->array8(), buffer.constData(), buffer.size());
  }

  return stack;
}
#endif


ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth, int zoom) const
{
  int zoomRatio = pow(2, zoom);

  return readGrayScale(getDvidTarget().getGrayScaleName(zoom),
                     x0 / zoomRatio, y0 / zoomRatio, z0 / zoomRatio,
                      width / zoomRatio, height / zoomRatio, depth / zoomRatio);
}

ZStack* ZDvidReader::readGrayScale(
    const std::string &dataName,
    int x0, int y0, int z0, int width, int height, int depth) const
{
#if 1
  ZStack *stack = NULL;

  QElapsedTimer timer;
  timer.start();

#if defined(_ENABLE_LIBDVIDCPP_)
  libdvid::Dims_t dims(3);
  dims[0] = width;
  dims[1] = height;
  dims[2] = depth;
  std::vector<int> offset(3);
  offset[0] = x0;
  offset[1] = y0;
  offset[2] = z0;

  try {
    libdvid::Grayscale3D data = m_service->get_gray3D(
          dataName, dims, offset, false);
    ZIntCuboid box(x0, y0, z0, x0 + width - 1, y0 + height - 1, z0 + depth - 1);
    stack = new ZStack(GREY, box, 1);
    memcpy(stack->array8(), data.get_binary()->get_raw(),
           width * height * depth);

    setStatusCode(200);
  } catch (libdvid::DVIDException &e) {
    STD_COUT << e.what() << std::endl;
    setStatusCode(e.getStatus());
  } catch (std::exception &e) {
    STD_COUT << e.what() << std::endl;
    setStatusCode(0);
  }

#else
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl url(getDvidTarget());
  /*
  if (depth == 1) {
    bufferReader.read(url.getGrayscaleUrl(width, height, x0, y0, z0).c_str());
  } else {
  */
    bufferReader.read(url.getGrayscaleUrl(
                        width, height, depth, x0, y0, z0).c_str(), isVerbose());
  //}

  setStatusCode(bufferReader.getStatusCode());

  const QByteArray &buffer = bufferReader.getBuffer();

  if (!buffer.isEmpty()) {
    ZIntCuboid box(x0, y0, z0, x0 + width - 1, y0 + height - 1, z0 + depth - 1);
    stack = new ZStack(GREY, box, 1);

    memcpy(stack->array8(), buffer.constData(), buffer.size());
  }
#endif

  ZOUT(LTRACE(), 5) << "Grayscale reading time: " << timer.elapsed();

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


ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth) const
{
  return readGrayScale(getDvidTarget().getGrayScaleName(),
                       x0, y0, z0, width, height, depth);
}

/*
bool ZDvidReader::isReadingDone()
{
  return m_isReadingDone;
}
*/

ZJsonObject ZDvidReader::readInfo() const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonObject(url.getInfoUrl());
}

ZJsonObject ZDvidReader::readInfo(const std::string &dataName) const
{
 std::string url = ZDvidUrl(getDvidTarget()).getInfoUrl(dataName);

 return readJsonObject(url);
}

ZDvidInfo ZDvidReader::readDataInfo(const std::string &dataName) const
{
  ZJsonObject obj = readInfo(dataName);

  ZDvidInfo dvidInfo;

  if (!obj.isEmpty()) {
    dvidInfo.set(obj);
    dvidInfo.setDvidNode(getDvidTarget().getAddress(), getDvidTarget().getPort(),
                         getDvidTarget().getUuid());
  }

  return dvidInfo;
}

#if 0
QString ZDvidReader::readInfo(const QString &dataName) const
{
  ZDvidBufferReader &reader = m_bufferReader;
//  reader.setService(m_service);

  std::string url = ZDvidUrl(getDvidTarget()).getInfoUrl(dataName.toStdString());
  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());
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
#endif

std::string ZDvidReader::readBodyLabelName() const
{
  std::string name;
  if (getDvidTarget().hasSegmentation()) {
    ZJsonObject dataInfo = readInfo(getDvidTarget().getSegmentationName());
    if (dataInfo.hasKey("Base")) {
      ZJsonObject baseObj(dataInfo.value("Base"));
      if (baseObj.hasKey("Syncs")) {
        ZJsonArray syncArray(baseObj.value("Syncs"));
        for (size_t i = 0; i < syncArray.size(); ++i) {
          std::string dataName =
              ZJsonParser::stringValue(syncArray.getData(), i);
          if (ZDvid::GetDataType(getType(dataName)) == ZDvid::TYPE_LABELVOL) {
            name = dataName;
            break;
          }
        }
      }
    }
  }

  return name;
}

void ZDvidReader::syncBodyLabelName()
{
  if (getDvidTarget().hasSegmentation()) {
    if (getDvidTarget().segmentationAsBodyLabel()) {
      getDvidTarget().setBodyLabelName(getDvidTarget().getSegmentationName());
    } else {
      getDvidTarget().setBodyLabelName(readBodyLabelName());
    }
  }
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

  return bodySet;
}

#if 0
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
#endif

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
  ZDvidBufferReader &bufferReader = m_bufferReader;

  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize).c_str(), isVerbose());
  setStatusCode(bufferReader.getStatusCode());

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

  bufferReader.clearBuffer();

  return bodySet;
}

std::set<uint64_t> ZDvidReader::readBodyId(size_t minSize, size_t maxSize)
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getBodyListUrl(minSize, maxSize).c_str(), isVerbose());
  setStatusCode(bufferReader.getStatusCode());

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

  bufferReader.clearBuffer();

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

bool ZDvidReader::hasKey(const QString &dataName, const QString &key) const
{
  return !readKeyValue(dataName, key).isEmpty();
}

QByteArray ZDvidReader::readKeyValue(const QString &dataName, const QString &key) const
{
  ZDvidUrl url(getDvidTarget());

  ZDvidBufferReader &bufferReader = m_bufferReader;

  bufferReader.read(
        url.getKeyUrl(dataName.toStdString(), key.toStdString()).c_str(),
        isVerbose());
  setStatusCode(bufferReader.getStatusCode());

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

QList<QByteArray> ZDvidReader::readKeyValues(const QString &dataName, const QStringList &keyList) const
{

    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader &bufferReader = m_bufferReader;

    // encode keylist into json payload
    QJsonArray keys = QJsonArray::fromStringList(keyList);
    QJsonDocument doc(keys);
    QByteArray payload = doc.toJson();

    // make call with json keylist
    bufferReader.read(QString::fromStdString(url.getKeyValuesUrl(dataName.toStdString())),
        payload,
        "GET",
        isVerbose());
    setStatusCode(bufferReader.getStatusCode());

    // untar response into list of byte arrays
    QList<QByteArray> ans;
    const QByteArray &buffer = m_bufferReader.getBuffer();
    struct archive *archive = archive_read_new();
    archive_read_support_format_all(archive);

    int result = archive_read_open_memory(archive, buffer.constData(), buffer.size());
    if (result != ARCHIVE_OK) {
        LINFO() << "couldn't expand keyvalue archive";
        return ans;
        }

    struct archive_entry *entry;
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        const struct stat *s = archive_entry_stat(entry);
        size_t size = s->st_size;

        QByteArray buffer(size, 0);
        archive_read_data(archive, buffer.data(), size);
        ans.append(buffer);
    }
    result = archive_read_free(archive);
    if (result != ARCHIVE_OK) {
        LINFO() << "couldn't close keyvalue archive";
    }

    return ans;
}

QStringList ZDvidReader::readKeys(const QString &dataName) const
{
  if (dataName.isEmpty()) {
    return QStringList();
  }

  ZDvidBufferReader &reader = m_bufferReader;

  ZDvidUrl dvidUrl(m_dvidTarget);

  reader.read(dvidUrl.getAllKeyUrl(dataName.toStdString()).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  reader.clearBuffer();

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey)
{
  ZDvidBufferReader &reader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  const std::string &maxKey = "\xff";

  reader.read(dvidUrl.getKeyRangeUrl(
                dataName.toStdString(), minKey.toStdString(), maxKey).c_str(),
              isVerbose());
  setStatusCode(reader.getStatusCode());

  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i));
    }
  }

  reader.clearBuffer();

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey, const QString &maxKey) const
{
  ZDvidUrl url(getDvidTarget());

  ZDvidBufferReader &reader = m_bufferReader;
  reader.read(url.getKeyRangeUrl(dataName.toStdString(), minKey.toStdString(),
                                 maxKey.toStdString()).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

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

  reader.clearBuffer();

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

ZJsonObject ZDvidReader::readContrastProtocal() const
{
  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName<QString>(ZDvidData::ROLE_NEUTU_CONFIG), "contrast");

  ZJsonObject config;
  if (!byteArray.isEmpty()) {
    config.decodeString(byteArray.data());
  }

  return config;
}

ZJsonArray ZDvidReader::readBodyStatusList() const
{
  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName<QString>(ZDvidData::ROLE_NEUTU_CONFIG), "body_status");

  ZJsonArray config;
  if (!byteArray.isEmpty()) {
    config.decodeString(byteArray.data());
  }

  return config;
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

ZIntPoint ZDvidReader::readRoiBlockSize(const std::string &dataName) const
{
  ZJsonObject obj = readInfo(dataName);
  ZIntPoint pt;
  if (obj.hasKey("Extended")) {
    ZJsonObject extJson(obj.value("Extended"));
    if (extJson.hasKey("BlockSize")) {
      ZJsonArray blockSizeJson(extJson.value("BlockSize"));
      if (blockSizeJson.size() == 3) {
        pt.set(ZJsonParser::integerValue(blockSizeJson.getData(), 0),
               ZJsonParser::integerValue(blockSizeJson.getData(), 1),
               ZJsonParser::integerValue(blockSizeJson.getData(), 2));
      }
    }
  }

  return pt;
}

ZDvidInfo ZDvidReader::readGrayScaleInfo() const
{
  return readDataInfo(getDvidTarget().getGrayScaleName());
}

ZDvidInfo ZDvidReader::readLabelInfo() const
{
  return readDataInfo(getDvidTarget().getSegmentationName());
}

bool ZDvidReader::hasData(const std::string &dataName) const
{
  if (dataName.empty()) {
    return false;
  }

  ZDvidUrl dvidUrl(m_dvidTarget);
  ZNetBufferReader bufferReader;
  return bufferReader.isReadable(dvidUrl.getInfoUrl(dataName).c_str());
}

std::string ZDvidReader::getType(const std::string &dataName) const
{
  std::string type;

  if (!dataName.empty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZDvidBufferReader &bufferReader = m_bufferReader;

    bufferReader.read(dvidUrl.getInfoUrl(dataName).c_str());

    const QByteArray &buffer = bufferReader.getBuffer();

    QJsonDocument doc = QJsonDocument::fromJson(buffer);
    QJsonObject json = doc.object();
    if (json.contains("Base")) {
      type = json.value("Base").toObject().value("TypeName").toString().toStdString();
    }
#if 0
    ZJsonObject json;
    json.decodeString(buffer.data());

    if (json.hasKey("Base")) {
      ZJsonObject baseJson(json.value("Base"));
      if (baseJson.hasKey("TypeName")) {
        type = ZJsonParser::stringValue(baseJson["TypeName"]);
      }
    }
#endif

    bufferReader.clearBuffer();
  }

  return type;
}

ZIntPoint ZDvidReader::readBodyBottom(uint64_t bodyId) const
{
  ZIntPoint pt;

  setStatusCode(0);

#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    try {
      libdvid::PointXYZ coord = m_service->get_body_extremum(
            getDvidTarget().getBodyLabelName(), bodyId, 2, false);
      pt.set(coord.x, coord.y, coord.z);
      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      setStatusCode(e.getStatus());
    }
  }
#endif

  return pt;
}

ZIntPoint ZDvidReader::readBodyTop(uint64_t bodyId) const
{
  ZIntPoint pt;

  setStatusCode(0);

#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    try {
      libdvid::PointXYZ coord = m_service->get_body_extremum(
            getDvidTarget().getBodyLabelName(), bodyId, 2, true);
      pt.set(coord.x, coord.y, coord.z);
      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      setStatusCode(e.getStatus());
    }
  }
#endif

  return pt;
}

ZJsonObject ZDvidReader::readSkeletonConfig() const
{
  ZJsonObject config;

  std::string skeletonName = ZDvidData::GetName(
        ZDvidData::ROLE_SKELETON, ZDvidData::ROLE_BODY_LABEL,
        getDvidTarget().getBodyLabelName());

  if (!skeletonName.empty()) {
    if (hasKey(skeletonName.c_str(), "config.json")) {
      ZDvidUrl dvidUrl(getDvidTarget());
      config = readJsonObject(
            dvidUrl.getSkeletonConfigUrl(getDvidTarget().getBodyLabelName()));
    }
  }

  return config;
}

ZIntPoint ZDvidReader::readBodyPosition(uint64_t bodyId) const
{
  ZIntPoint pt;

  pt.invalidate();

  ZSwcTree *tree = readSwc(bodyId);
  if (tree != NULL) {
    Swc_Tree_Node *tn = tree->getThickestNode();
    if (tn != NULL) {
      pt = SwcTreeNode::center(tn).toIntPoint();
    }
    /*
    if (!tree->isEmpty()) {
      ZSwcPath path = tree->getLongestPath();

      if (!path.empty()) {
        Swc_Tree_Node *tn = path[path.size() / 2];
        pt = SwcTreeNode::center(tn).toIntPoint();
      }
    }
    */

    delete tree;
  }

  if (pt.isValid()) {
    if (bodyId != readBodyIdAt(pt)) {
      pt.invalidate();
    }
  }

  if (!pt.isValid()) {
    ZObject3dScan body = readCoarseBody(bodyId);
    if (!body.isEmpty()) {
      ZDvidInfo dvidInfo = readLabelInfo();

      ZObject3dScan objSlice = body.getMedianSlice();
      ZVoxel voxel = objSlice.getMarker();
      //        ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
      pt.set(voxel.x(), voxel.y(), voxel.z());
//      pt -= dvidInfo.getStartBlockIndex();
      pt *= dvidInfo.getBlockSize();
//      pt += ZIntPoint(dvidInfo.getBlockSize().getX() / 2,
//                      dvidInfo.getBlockSize().getY() / 2, 0);
//      pt += dvidInfo.getStartCoordinates();

      ZIntCuboid box;
      box.setFirstCorner(pt);
      box.setSize(dvidInfo.getBlockSize().getX(),
                  dvidInfo.getBlockSize().getY(),
                  dvidInfo.getBlockSize().getZ());
      ZObject3dScan *fineBody = readBody(bodyId, box, true, NULL);

      if (fineBody != NULL) {
        ZObject3dScan objSlice = fineBody->getMedianSlice();
        if (!objSlice.isEmpty()) {
          voxel = objSlice.getMarker();
          pt.set(voxel.x(), voxel.y(), voxel.z());
        }
        delete fineBody;
      }
    }
  }

  return pt;
}

ZIntCuboid ZDvidReader::readBodyBoundBox(uint64_t bodyId) const
{
  ZIntCuboid box;

  setStatusCode(0);
#if defined(_ENABLE_LIBDVIDCPP_)
  if (m_service.get() != NULL) {
    try {
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
      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      setStatusCode(e.getStatus());
    }
  }
#endif

  return box;
}

ZIntPoint ZDvidReader::readPosition(uint64_t bodyId, const ZIntPoint &pt) const
{
  return readPosition(bodyId, pt.getX(), pt.getY(), pt.getZ());
}

ZIntPoint ZDvidReader::readPosition(uint64_t bodyId, int x, int y, int z) const
{
  if (bodyId == 0 || bodyId == readBodyIdAt(x, y, z)) {
    return ZIntPoint(x, y, z);
  }

  ZDvidInfo dvidInfo = readLabelInfo();
  ZIntPoint blockIndex = dvidInfo.getBlockIndex(x, y, z);
  ZIntCuboid box = dvidInfo.getBlockBox(blockIndex);
  ZArray *label = readLabels64(box);

  ZStack *stack = new ZStack(GREY, box, 1);
  size_t voxelCount = stack->getVoxelNumber();
  uint64_t *array = label->getDataPointer<uint64_t>();
  uint8_t *stackArray = stack->array8();
  bool found = false;
  for (size_t offset = 0; offset < voxelCount; ++offset) {
#ifdef _DEBUG_2
    STD_COUT << "Label64: " << array[offset] << " <> " << bodyId <<std::endl;
#endif
    if (array[offset] == bodyId) {
      stackArray[offset] = 0;
      found = true;
    } else {
      stackArray[offset] = 1;
    }
  }

  ZIntPoint pt;
  pt.invalidate();

  if (found) {
    pt = flyem::FindClosestBg(stack, x, y, z);
  }

  delete label;
  delete stack;

  return pt;
}

ZArray* ZDvidReader::readLabels64(
    int x0, int y0, int z0, int width, int height, int depth, int zoom) const
{
  int zoomRatio = pow(2, zoom);

  return readLabels64(getDvidTarget().getValidSegmentationName(zoom),
                     x0 / zoomRatio, y0 / zoomRatio, z0 / zoomRatio,
                      width / zoomRatio, height / zoomRatio, depth);
}

ZArray* ZDvidReader::readLabels64Raw(
    int x0, int y0, int z0, int width, int height, int depth, int zoom) const
{
  return readLabels64(getDvidTarget().getValidSegmentationName(zoom),
                     x0, y0, z0, width, height, depth);
}

ZArray* ZDvidReader::readLabels64(const ZIntCuboid &box, int zoom) const
{
  return readLabels64(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                      box.getFirstCorner().getZ(), box.getWidth(),
                      box.getHeight(), box.getDepth(), zoom);
}

ZArray* ZDvidReader::readLabels64(
    const std::string &dataName, int x0, int y0, int z0,
    int width, int height, int depth) const
{
  if (dataName.empty()) {
    return NULL;
  }

  ZArray *array = NULL;

#if defined(_ENABLE_LIBDVIDCPP_)
  qDebug() << "Using libdvidcpp";

  const ZDvidTarget &target = getDvidTarget();
  if (!target.getUuid().empty()) {
    try {
      ZDvidUrl dvidUrl(m_dvidTarget);
      STD_COUT << dvidUrl.getLabels64Url(
                     dataName, width, height, depth, x0, y0, z0).c_str()
                << std::endl;

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
      m_readingTime = timer.elapsed();
      LINFO() << "label reading time: " << m_readingTime;
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
      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      LERROR() << e.what();
      setStatusCode(e.getStatus());
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

//  STD_COUT << "label reading time: " << (finish - start) / CLOCKS_PER_SEC << std::endl;
//  STD_COUT << "label reading time: " << toc() << std::endl;

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


std::vector<ZArray*> ZDvidReader::readLabelBlock(
    const ZObject3dScan &blockObj, int zoom) const
{
  std::vector<ZArray*> result;

  std::vector<libdvid::DVIDCompressedBlock> c_blocks;

  std::vector<int> blockcoords;
  ZObject3dScan::ConstVoxelIterator objIter(&blockObj);
  while (objIter.hasNext()) {
    ZIntPoint pt = objIter.next();
    blockcoords.push_back(pt.getX());
    blockcoords.push_back(pt.getY());
    blockcoords.push_back(pt.getZ());
  }

  try {
    m_service->get_specificblocks3D(
          getDvidTarget().getSegmentationName(), blockcoords, false, c_blocks, zoom);

    ZDvidInfo info = readLabelInfo();
    for (libdvid::DVIDCompressedBlock &block : c_blocks) {
      libdvid::BinaryDataPtr data = block.get_uncompressed_data();

      std::vector<int> offset = block.get_offset();

      mylib::Dimn_Type arrayDims[3];
      arrayDims[0] = info.getBlockSize().getX();
      arrayDims[1] = info.getBlockSize().getY();
      arrayDims[2] = info.getBlockSize().getZ();
      ZArray *array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);
      array->copyDataFrom(data->get_raw());
      array->setStartCoordinate(offset);

      result.push_back(array);
    }
  } catch(libdvid::DVIDException &e) {
    LERROR() << e.what();
    m_statusCode = e.getStatus();
  } catch (std::exception &e) {
    LERROR() << e.what();
    m_statusCode = 0;
  }

  return result;
}

ZArray* ZDvidReader::readLabelBlock(int bx, int by, int bz, int zoom) const
{
  std::vector<int> blockcoords;
  blockcoords.push_back(bx);
  blockcoords.push_back(by);
  blockcoords.push_back(bz);

  std::vector<libdvid::DVIDCompressedBlock> c_blocks;
  m_service->get_specificblocks3D(
        getDvidTarget().getSegmentationName(), blockcoords, false, c_blocks, zoom);

  ZArray *array = NULL;
  if (c_blocks.size() == 1) {
    libdvid::DVIDCompressedBlock &block = c_blocks[0];
    libdvid::BinaryDataPtr data = block.get_uncompressed_data();
    std::vector<int> offset = block.get_offset();

    ZDvidInfo info = readLabelInfo();

    ZIntCuboid box;
    box.setFirstCorner(offset[0], offset[1], offset[2]);
    box.setSize(info.getBlockSize());
    array = ZArrayFactory::MakeArray(box, mylib::UINT64_TYPE);
    array->copyDataFrom(data->get_raw());
  }

  return array;
}

bool ZDvidReader::refreshLabelBuffer() const
{
#if defined(_ENABLE_LOWTIS_)
  if (m_lowtisService.get() != NULL) {
    try {
      m_lowtisService->flush_cache();
    } catch (std::exception &e) {
      ZOUT(LTRACE(), 5) << e.what();
      return false;
    }
  }

#endif
  return true;
}

namespace {
void LogReadingTime(int64_t time, int64_t thre, const std::string &name)
{
  bool logging = (NeutubeConfig::GetVerboseLevel() >= 5) || (time > thre);
  if (logging) {
    LINFO() << name + " reading time: " << time;
  }
}
}

template<typename T>
void ZDvidReader::configureLowtis(T *config, const std::string &dataName) const
{
  config->username = neutube::GetCurrentUserName();
  config->dvid_server = getDvidTarget().getAddressWithPort();
  config->dvid_uuid = getDvidTarget().getUuid();
  config->datatypename = dataName;
  config->enableprefetch = NeutubeConfig::LowtisPrefetching();
}

#if defined(_ENABLE_LOWTIS_)
ZStack* ZDvidReader::readGrayScaleLowtis(int x0, int y0, int z0,
    int width, int height, int zoom, int cx, int cy, bool centerCut) const
{
#if 0
  if (!getDvidTarget().hasGray()) {
    return NULL;
  }
#endif

  int scale = 1;
  if (zoom > 0) {
    scale = pow(2, zoom);
    width /= scale;
    height /= scale;
  }

  ZStack *stack = NULL;

  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;

  if (m_lowtisServiceGray.get() == NULL) {
    try {
      configureLowtis(&m_lowtisConfigGray, getDvidTarget().getGrayScaleName());
//      m_lowtisConfigGray.username = neutube::GetCurrentUserName();
//      m_lowtisConfigGray.dvid_server = getDvidTarget().getAddressWithPort();
//      m_lowtisConfigGray.dvid_uuid = getDvidTarget().getUuid();
//      m_lowtisConfigGray.datatypename = getDvidTarget().getGrayScaleName();
      m_lowtisConfigGray.centercut = std::tuple<int, int>(cx, cy);
//      m_lowtisConfigGray.enableprefetch = NeutubeConfig::LowtisPrefetching();

      m_lowtisServiceGray = ZSharedPointer<lowtis::ImageService>(
            new lowtis::ImageService(m_lowtisConfigGray));

    } catch (libdvid::DVIDException &e) {
      m_lowtisServiceGray.reset();

      LERROR() << e.what();
      setStatusCode(e.getStatus());
    }
  }

  qDebug() << "  Prefetching:" << m_lowtisConfigGray.enableprefetch;

  QElapsedTimer timer;
  timer.start();
  if (m_lowtisServiceGray.get() != NULL) {
//    m_lowtisService->config.bytedepth = 8;

    ZIntCuboid box;
    box.setFirstCorner(x0 / scale, y0 / scale, z0 / scale);
    box.setWidth(width);
    box.setHeight(height);
    box.setDepth(1);


    stack = new ZStack(GREY, box, 1);

    try {
      std::vector<int> offset(3);
      offset[0] = x0;
      offset[1] = y0;
      offset[2] = z0;

      if (zoom == getDvidTarget().getMaxGrayscaleZoom() ||
          width < cx || height < cy) {
        centerCut = false;
      }

      if (zoom >= 1) {
//        zoom -= 1;
      }

      m_lowtisServiceGray->retrieve_image(
            width, height, offset, (char*) stack->array8(), zoom, centerCut);

      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      LERROR() << e.what();
      setStatusCode(e.getStatus());

      delete stack;
      stack = NULL;
    }

    m_readingTime = timer.elapsed();
    if (NeutubeConfig::GetVerboseLevel() < 5) {
      if (m_readingTime > 10) {
        LINFO() << "grayscale reading time: " << m_readingTime;
      }
    } else {
      LINFO() << "grayscale reading time: " << m_readingTime;
    }
  }

  return stack;
}


ZStack* ZDvidReader::readGrayScaleLowtis(int x0, int y0, int z0,
    int width, int height, int zoom) const
{
  return readGrayScaleLowtis(x0, y0, z0, width, height, zoom, 256, 256, true);
}

namespace {

template<typename T>
std::vector<T> MakeVec3(const T& x0, const T& x1, const T& x2)
{
  std::vector<T> vec(3);
  vec[0] = x0;
  vec[1] = x1;
  vec[2] = x2;

  return vec;
}

ZArray* MakeArray64(int x0, int y0, int z0, int width, int height, int depth)
{
  mylib::Dimn_Type arrayDims[3];
  arrayDims[0] = width;
  arrayDims[1] = height;
  arrayDims[2] = depth;
  ZArray *array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);

  array->setStartCoordinate(0, x0);
  array->setStartCoordinate(1, y0);
  array->setStartCoordinate(2, z0);

  return array;
}

ZArray* MakeArray64(const ZIntCuboid &box)
{
  return MakeArray64(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                     box.getFirstCorner().getZ(),
                     box.getWidth(), box.getHeight(), box.getDepth());
}

} //namespace

std::vector<int> ZDvidReader::GetOffset(int x0, int y0, int z0)
{
  std::vector<int> offset(3);
  offset[0] = x0;
  offset[1] = y0;
  offset[2] = z0;

  return offset;
}

std::vector<int> ZDvidReader::GetOffset(
    int cx, int cy, int cz, int width, int height)
{
  std::vector<int> offset(3);
  offset[0] = cx - width / 2;
  offset[1] = cy - height / 2;
  offset[2] = cz;

  return offset;
}

ZIntCuboid ZDvidReader::GetStackBox(
    int x0, int y0, int z0, int width, int height, int zoom)
{
  int scale = 1;
  if (zoom > 0) {
    scale = pow(2, zoom);
    width /= scale;
    height /= scale;
  }

  ZIntCuboid box;
  box.setFirstCorner(x0 / scale, y0 / scale, z0 / scale);
  box.setWidth(width);
  box.setHeight(height);
  box.setDepth(1);

  return box;
}

ZIntCuboid ZDvidReader::GetStackBoxAtCenter(
    int x0, int y0, int z0, int width, int height, int zoom)
{
  x0 -= width / 2;
  y0 -= height / 2;

  int scale = 1;
  if (zoom > 0) {
    scale = pow(2, zoom);
    width /= scale;
    height /= scale;
  }

  ZIntCuboid box;
  box.setFirstCorner(x0 / scale, y0 / scale, z0 / scale);
  box.setWidth(width);
  box.setHeight(height);
  box.setDepth(1);

  return box;
}

void ZDvidReader::prepareLowtisService(
    ZSharedPointer<lowtis::ImageService> &service, const std::string &dataName,
    lowtis::DVIDConfig &config, int cx, int cy) const
{
  if (service.get() == NULL) {
    try {
      configureLowtis(&config, dataName);
//      config.centercut = std::tuple<int, int>(cx, cy);

      service = ZSharedPointer<lowtis::ImageService>(
            new lowtis::ImageService(config));
    } catch (libdvid::DVIDException &e) {
      service.reset();

      LERROR() << e.what();
      setStatusCode(e.getStatus());
    }
  }

  if (service.get() != NULL) {
    service->set_centercut(std::tuple<int,int>(cx, cy));
  }
}

lowtis::ImageService* ZDvidReader::getLowtisServiceLabel(int cx, int cy) const
{
  if (!getDvidTarget().hasSegmentation()) {
    return NULL;
  }

  prepareLowtisService(m_lowtisService, getDvidTarget().getSegmentationName(),
                       m_lowtisConfig, cx, cy);

  return m_lowtisService.get();
}

lowtis::ImageService* ZDvidReader::getLowtisServiceGray(int cx, int cy) const
{
  if (!getDvidTarget().hasGrayScaleData()) {
    return NULL;
  }

  prepareLowtisService(
        m_lowtisServiceGray, getDvidTarget().getGrayScaleName(),
        m_lowtisConfigGray, cx, cy);

  return m_lowtisServiceGray.get();
}

void ZDvidReader::setGrayCenterCut(int cx, int cy)
{
  lowtis::ImageService *service = getLowtisServiceGray(cx, cy);
  if (service != NULL) {
    service->set_centercut(std::tuple<int,int>(cx, cy));
  }
}

void ZDvidReader::setLabelCenterCut(int cx, int cy)
{
  lowtis::ImageService *service = getLowtisServiceLabel(cx, cy);
  if (service != NULL) {
    service->set_centercut(std::tuple<int,int>(cx, cy));
  }
}


ZStack* ZDvidReader::readGrayScaleLowtis(
    int x0, int y0, int z0, double vx1, double vy1, double vz1,
    double vx2, double vy2, double vz2,
    int width, int height, int zoom, int cx, int cy, bool centerCut) const
{
  if (getLowtisServiceGray(cx, cy) == NULL) {
    return NULL;
  }

  ZStack *stack = NULL;

  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;
  qDebug() << "  Prefetching:" << m_lowtisConfigGray.enableprefetch;

  QElapsedTimer timer;
  timer.start();
  if (m_lowtisServiceGray.get() != NULL) {
//    m_lowtisService->config.bytedepth = 8;

    ZIntCuboid box = GetStackBoxAtCenter(x0, y0, z0, width, height, zoom);

    stack = new ZStack(GREY, box, 1);

    try {
      std::vector<int> offset = GetOffset(x0, y0, z0);

      if (zoom == getDvidTarget().getMaxGrayscaleZoom() ||
          box.getWidth() < cx || box.getHeight() < cy) {
        centerCut = false;
      }

      if (zoom >= 1) {
//        zoom -= 1;
      }

      std::vector<double> dim1vec;
      dim1vec.push_back(vx1);
      dim1vec.push_back(vy1);
      dim1vec.push_back(vz1);

      std::vector<double> dim2vec;
      dim2vec.push_back(vx2);
      dim2vec.push_back(vy2);
      dim2vec.push_back(vz2);

#ifdef _DEBUG_
      STD_COUT << "Stack info:";
      stack->printInfo();

      STD_COUT << "Reading size:" << box.getWidth() << "x" << box.getHeight()
                << std::endl;
      qDebug() << QString("Call: retrieve_arbimage(%1, %2, [%3, %4, %5], "
                          "[%6, %7, %8], [%9, %10, %11], array, %12, %13)").
                  arg(box.getWidth()).arg(box.getHeight()).
                  arg(offset[0]).arg(offset[1]).arg(offset[2]).
                  arg(dim1vec[0]).arg(dim1vec[1]).arg(dim1vec[2]).
                  arg(dim2vec[0]).arg(dim2vec[1]).arg(dim2vec[2]).arg(zoom).
                  arg(centerCut);
#endif

      m_lowtisServiceGray->retrieve_arbimage(
            box.getWidth(), box.getHeight(), offset, dim1vec, dim2vec,
            (char*) stack->array8(), zoom, centerCut);

      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      LERROR() << e.what();
      setStatusCode(e.getStatus());

      delete stack;
      stack = NULL;
    }

    m_readingTime = timer.elapsed();
    LogReadingTime(m_readingTime, 10, "grayscale");
  }

  return stack;
}

ZStack *ZDvidReader::readGrayScaleLowtis(
    const ZIntPoint &center, const ZPoint &v1, const ZPoint &v2,
    int width, int height, int zoom, int cx, int cy, bool centerCut) const
{
  return readGrayScaleLowtis(
        center.getX(), center.getY(), center.getZ(),
        v1.getX(), v1.getY(), v1.getZ(), v2.getX(), v2.getY(), v2.getZ(),
        width, height, zoom, cx, cy, centerCut);
}

ZStack* ZDvidReader::readGrayScaleLowtis(
    const ZAffineRect &ar, int zoom, int cx, int cy, bool centerCut) const
{
  return readGrayScaleLowtis(
        ar.getCenter().toIntPoint(), ar.getV1(), ar.getV2(),
        ar.getWidth(), ar.getHeight(), zoom, cx, cy, centerCut);
}

ZArray* ZDvidReader::readLabels64Lowtis(
    const ZIntPoint &center, const ZPoint &v1, const ZPoint &v2,
    int width, int height, int zoom, int cx, int cy, bool centerCut) const
{
  return readLabels64Lowtis(center.getX(), center.getY(), center.getZ(),
                            v1.x(), v1.y(), v1.z(), v2.x(), v2.y(), v2.z(),
                            width, height, zoom, cx, cy, centerCut);
}

ZArray* ZDvidReader::readLabels64Lowtis(
    const ZAffineRect &ar, int zoom, int cx, int cy, bool centerCut) const
{
  return readLabels64Lowtis(
        ar.getCenter().toIntPoint(), ar.getV1(), ar.getV2(),
        ar.getWidth(), ar.getHeight(), zoom, cx, cy, centerCut);
}

ZArray* ZDvidReader::readLabels64Lowtis(
    int x0, int y0, int z0, double vx1, double vy1, double vz1,
    double vx2, double vy2, double vz2, int width, int height, int zoom,
    int cx, int cy, bool centerCut) const
{
  lowtis::ImageService *service = getLowtisServiceLabel(cx, cy);
  if (service == NULL) {
    return NULL;
  }

  ZIntCuboid box = GetStackBoxAtCenter(x0, y0, z0, width, height, zoom);

  ZArray *array = NULL;

  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;


  QElapsedTimer timer;
  timer.start();
  if (service != NULL) {
    array = MakeArray64(box);

    try {
      std::vector<int> offset = GetOffset(x0, y0, z0);
      std::vector<double> dim1vec = MakeVec3(vx1, vy1, vz1);
      std::vector<double> dim2vec = MakeVec3(vx2, vy2, vz2);

#ifdef _DEBUG_
      STD_COUT << "Stack info:";
      array->printInfo();

      STD_COUT << "Reading size:" << box.getWidth() << "x" << box.getHeight()
                << std::endl;
      qDebug() << QString("Call: retrieve_arbimage(%1, %2, [%3, %4, %5], "
                          "[%6, %7, %8], [%9, %10, %11], array, %12)").
                  arg(box.getWidth()).arg(box.getHeight()).
                  arg(offset[0]).arg(offset[1]).arg(offset[2]).
                  arg(dim1vec[0]).arg(dim1vec[1]).arg(dim1vec[2]).
                  arg(dim2vec[0]).arg(dim2vec[1]).arg(dim2vec[2]).arg(zoom);
#endif

      if (zoom == getDvidTarget().getMaxLabelZoom() || width < cx || height < cy) {
        centerCut = false;
      }

      service->retrieve_arbimage(
            box.getWidth(), box.getHeight(), offset, dim1vec, dim2vec,
            array->getDataPointer<char>(), zoom, centerCut);

      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      LERROR() << e.what();
      setStatusCode(e.getStatus());

      delete array;
      array = NULL;
    }

    m_readingTime = timer.elapsed();
    LINFO() << "label reading time: " << m_readingTime;
  }

  return array;
}

ZArray* ZDvidReader::readLabels64Lowtis(
    int x0, int y0, int z0, int width, int height, int zoom, int cx, int cy,
    bool centerCut) const
{
  lowtis::ImageService *service = getLowtisServiceLabel(cx, cy);
  if (service == NULL) {
    return NULL;
  }
  /*
  if (!getDvidTarget().hasSegmentation()) {
    return NULL;
  }*/

  int scale = 1;
  if (zoom > 0) {
    scale = pow(2, zoom);
    width /= scale;
    height /= scale;
  }

  ZArray *array = NULL;

  qDebug() << "Reading" << getDvidTarget().getSegmentationName() << "at"
           << x0 << y0 << z0;
  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;


  QElapsedTimer timer;
  timer.start();
  if (service != NULL) {
//    m_lowtisService->config.bytedepth = 8;

    mylib::Dimn_Type arrayDims[3];
    arrayDims[0] = width;
    arrayDims[1] = height;
    arrayDims[2] = 1;
    array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);

    array->setStartCoordinate(0, x0 / scale);
    array->setStartCoordinate(1, y0 / scale);
    array->setStartCoordinate(2, z0 / scale);

    try {
      std::vector<int> offset(3);
      offset[0] = x0;
      offset[1] = y0;
      offset[2] = z0;

      if (zoom == getDvidTarget().getMaxLabelZoom() ||
          width < cx || height < cy) {
        centerCut = false;
      }
      service->retrieve_image(
            width, height, offset, array->getDataPointer<char>(), zoom, centerCut);

      setStatusCode(200);
    } catch (libdvid::DVIDException &e) {
      LERROR() << e.what();
      setStatusCode(e.getStatus());

      delete array;
      array = NULL;
    }

    m_readingTime = timer.elapsed();
    LINFO() << "label reading time: " << m_readingTime;
  }

  return array;
}

ZArray* ZDvidReader::readLabels64Lowtis(int x0, int y0, int z0,
    int width, int height, int zoom) const
{
  return readLabels64Lowtis(x0, y0, z0, width, height, zoom, 256, 256, true);
}
#endif

bool ZDvidReader::hasSparseVolume() const
{
  return hasData(m_dvidTarget.getBodyLabelName());
  //return true;
  //return hasData(ZDvidData::getName(ZDvidData::ROLE_SP2BODY));
}

bool ZDvidReader::hasGrayscale() const
{
  return hasData(m_dvidTarget.getGrayScaleName());
}

bool ZDvidReader::hasBody(uint64_t bodyId) const
{
#if 1
  if (m_service.get() != NULL) {
    try {
#if 0
      ZString endpoint = getDvidTarget().getSegmentationName() + "/sparsevol/";
      endpoint.appendNumber(bodyId);
      m_service->custom_request(
            endpoint, libdvid::BinaryDataPtr(), libdvid::HEAD);
      return true;
#endif
      return m_service->body_exists(m_dvidTarget.getBodyLabelName(), bodyId);
    } catch (libdvid::DVIDException &e) {
//      m_statusCode = e.getStatus();
#ifdef _DEBUG_
      STD_COUT << e.what() << std::endl;
#endif
      return false;
    }
  }
#else
  return hasCoarseSparseVolume(bodyId);
#endif

  return false;
}

bool ZDvidReader::hasBody(uint64_t bodyId, flyem::EBodyLabelType type) const
{
  if (type == flyem::EBodyLabelType::BODY) {
    return hasBody(bodyId);
  } else if (type == flyem::EBodyLabelType::SUPERVOXEL) {
    if (getDvidTarget().hasSupervoxel()) {
      try {
        ZString endpoint = getDvidTarget().getSegmentationName() + "/sparsevol/";
        endpoint.appendNumber(bodyId);
        endpoint += "?supervoxels=true";
        m_service->custom_request(
              endpoint, libdvid::BinaryDataPtr(), libdvid::HEAD);
        return true;
      } catch (libdvid::DVIDException &e) {
#ifdef _DEBUG_
        STD_COUT << e.what() << std::endl;
#endif
        return false;
      }
    }
  }

  return false;
}

size_t ZDvidReader::readBodySize(uint64_t bodyId) const
{
  size_t s = 0;
  std::string url = ZDvidUrl(getDvidTarget()).getBodySizeUrl(bodyId);
  if (!url.empty()) {
    ZJsonObject jsonObj = readJsonObject(url);
    s = ZJsonParser::integerValue(jsonObj["voxels"]);
  }

  return s;
}

size_t ZDvidReader::readBodySize(
    uint64_t bodyId, flyem::EBodyLabelType type) const
{
  size_t s = 0;
  std::string url;
  if (type == flyem::EBodyLabelType::BODY) {
    url = ZDvidUrl(getDvidTarget()).getBodySizeUrl(bodyId);
  } else  if (type == flyem::EBodyLabelType::SUPERVOXEL) {
    url = ZDvidUrl(getDvidTarget()).getSupervoxelSizeUrl(bodyId);
  }
  if (!url.empty()) {
    ZJsonObject jsonObj = readJsonObject(url);
    s = ZJsonParser::integerValue(jsonObj["voxels"]);
  }

  return s;
}

std::tuple<size_t, size_t, ZIntCuboid> ZDvidReader::readBodySizeInfo(
    uint64_t bodyId, flyem::EBodyLabelType type) const
{
  size_t voxelCount = 0;
  size_t blockCount = 0;
  ZIntCuboid boundBox;

  std::string url;
  if (type == flyem::EBodyLabelType::BODY) {
    url = ZDvidUrl(getDvidTarget()).getSparsevolSizeUrl(bodyId);
  } else if (type == flyem::EBodyLabelType::SUPERVOXEL) {
    url = ZDvidUrl(getDvidTarget()).getSupervoxelSizeUrl(bodyId);
  }

  if (!url.empty()) {
    ZJsonObject jsonObj = readJsonObject(url);
    voxelCount = ZJsonParser::integerValue(jsonObj["voxels"]);
    blockCount = ZJsonParser::integerValue(jsonObj["numblocks"]);
    boundBox.setFirstCorner(
          ZJsonParser::integerValue(jsonObj["minvoxel"], 0),
          ZJsonParser::integerValue(jsonObj["minvoxel"], 1),
          ZJsonParser::integerValue(jsonObj["minvoxel"], 2)
        );
    boundBox.setLastCorner(
          ZJsonParser::integerValue(jsonObj["maxvoxel"], 0),
          ZJsonParser::integerValue(jsonObj["maxvoxel"], 1),
          ZJsonParser::integerValue(jsonObj["maxvoxel"], 2)
        );
  }

  return std::make_tuple(voxelCount, blockCount, boundBox);
}



ZIntPoint ZDvidReader::readBodyLocation(uint64_t bodyId) const
{
  return readBodyPosition(bodyId);
#if 0
  ZIntPoint location;

#if defined(_ENABLE_LIBDVIDCPP_)
  /*if (m_service.get() != NULL) {
    try {
      libdvid::PointXYZ dpt =
          m_service->get_body_location(
            m_dvidTarget.getBodyLabelName(), bodyId);
      location.set(dpt.x, dpt.y, dpt.z);
    } catch (std::exception &e) {
      STD_COUT << e.what() << std::endl;
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
#endif
}

bool ZDvidReader::hasSparseVolume(uint64_t bodyId) const
{
  ZNetBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getSparsevolUrl(bodyId, getDvidTarget().getBodyLabelName()).c_str());
}

bool ZDvidReader::hasCoarseSparseVolume(uint64_t bodyId) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZNetBufferReader reader;
  reader.readPartial(
        dvidUrl.getCoarseSparsevolUrl(
          bodyId, getDvidTarget().getBodyLabelName()).c_str(),
        12, true);
  QByteArray byteArray = reader.getBuffer();
  if (byteArray.size() >= 12) {
    return *((uint32_t*) (byteArray.data() + 8)) > 0;
  }

  reader.clearBuffer();

  return false;

#if 0
  ZDvidBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getCoarseSparsevolUrl(
          bodyId, getDvidTarget().getBodyLabelName()).c_str());
#endif
}

bool ZDvidReader::hasBodyInfo(uint64_t bodyId) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZNetBufferReader bufferReader;

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

int64_t ZDvidReader::readBodyMutationId(uint64_t bodyId) const
{
  int64_t mutId = 0;

  ZDvidUrl dvidUrl(getDvidTarget());
  std::string url = dvidUrl.getSparsevolLastModUrl(bodyId);
  if (!url.empty()) {
    ZJsonObject obj = readJsonObject(url);
    mutId = ZJsonParser::integerValue(obj["mutation id"]);
  }

  return mutId;
}

void ZDvidReader::updateMaxGrayscaleZoom(
    const ZJsonObject &infoJson, const ZDvidVersionDag &dag)
{
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (ZDvid::IsDataValid(
            m_dvidTarget.getGrayScaleName(level), m_dvidTarget, infoJson, dag)) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    m_dvidTarget.setMaxGrayscaleZoom(maxLabelLevel);
  }
}

void ZDvidReader::updateMaxGrayscaleZoom()
{
  if (m_dvidTarget.isValid()) {
    if (hasGrayscale()) {
      ZJsonObject infoJson = readInfo();
      ZDvidVersionDag dag = readVersionDag();
      updateMaxGrayscaleZoom(infoJson, dag);
    }
  }

#if 0
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (hasData(m_dvidTarget.getGrayScaleName(level))) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    m_dvidTarget.setMaxGrayscaleZoom(maxLabelLevel);
  }
#endif
}

void ZDvidReader::updateMaxLabelZoom(
    const ZJsonObject &infoJson, const ZDvidVersionDag &dag)
{
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (ZDvid::IsDataValid(
            m_dvidTarget.getSegmentationName(level), m_dvidTarget, infoJson, dag)) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    m_dvidTarget.setMaxLabelZoom(maxLabelLevel);
  }
}

void ZDvidReader::updateMaxLabelZoom()
{
  if (m_dvidTarget.isValid()) {
    if (getDvidTarget().hasMultiscaleSegmentation()) {
      ZJsonObject infoJson = readInfo(getDvidTarget().getSegmentationName());
      ZJsonValue v = infoJson.value({"Extended", "MaxDownresLevel"});
      if (!v.isEmpty()) {
        m_dvidTarget.setMaxLabelZoom(v.toInteger());
      }
#if 0
      else { //temporary hack!!!
        if (getDvidTarget().getUuid() == "c140") {
          m_dvidTarget.setMaxLabelZoom(7);
        }
      }
#endif

//      if (infoJson.hasKey("Extended")) {
//        ZJsonObject extJson(infoJson.value("Extended"));

//      }
    } else {
      if (getDvidTarget().hasSegmentation()) {
        ZJsonObject infoJson = readInfo();
        ZDvidVersionDag dag = readVersionDag();
        updateMaxLabelZoom(infoJson, dag);
      }
    }
  }
#if 0
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (hasData(m_dvidTarget.getLabelBlockName(level))) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    m_dvidTarget.setMaxLabelZoom(maxLabelLevel);
  }
#endif
}

#define MAX_BODY_ID_START 50000000
uint64_t ZDvidReader::readMaxBodyId()
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName<QString>(ZDvidData::ROLE_MAX_BODY_ID),
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

  if (!dataName.empty()) {
    ZDvidUrl dvidUrl(getDvidTarget());

    ZDvidBufferReader &bufferReader = m_bufferReader;
    bufferReader.read(dvidUrl.getInfoUrl(dataName).c_str(), isVerbose());
    setStatusCode(bufferReader.getStatusCode());

    ZJsonObject infoJson;
    infoJson.decodeString(bufferReader.getBuffer().data());
    tileInfo.load(infoJson);

    bufferReader.clearBuffer();
  }

  return tileInfo;
}

void ZDvidReader::clearBuffer() const
{
  m_bufferReader.clearBuffer();
}

std::string ZDvidReader::GetMasterNodeFromBuffer(
    const ZDvidBufferReader &bufferReader)
{
  std::string master;

  ZJsonArray branchJson;
  branchJson.decodeString(bufferReader.getBuffer().data());
  if (branchJson.size() > 0) {
    master = ZJsonParser::stringValue(branchJson.at(0));
  }

  return master;
}

std::string ZDvidReader::GetMirrorAddressFromBuffer(
    const ZDvidBufferReader &bufferReader)
{
  std::string mirror;

  if (!bufferReader.getBuffer().isEmpty()) {
    ZJsonObject jsonObj;
    jsonObj.decodeString(bufferReader.getBuffer().data());
    mirror = ZJsonParser::stringValue(jsonObj["address"]);
  }

  return mirror;
}

std::vector<std::string> ZDvidReader::GetMasterListFromBuffer(
    const ZDvidBufferReader &bufferReader)
{
  std::vector<std::string> masterList;

  ZJsonArray branchJson;
  branchJson.decodeString(bufferReader.getBuffer().data());
  for (size_t i = 0; i < branchJson.size(); ++i) {
    masterList.push_back(ZJsonParser::stringValue(branchJson.at(i)));
  }

  return masterList;
}

ZDvidVersionDag ZDvidReader::readVersionDag() const
{
  ZJsonObject jsonInfo = readInfo();

  std::string uuid = getDvidTarget().getUuid();

  if (jsonInfo.hasKey("DAG")) {
    ZJsonObject dagJson(jsonInfo.value("DAG"));
    if (dagJson.hasKey("Root")) {
      uuid = ZJsonParser::stringValue(dagJson["Root"]);
    }
  }

  return readVersionDag(uuid);
}

ZDvidVersionDag ZDvidReader::readVersionDag(const std::string &uuid) const
{
  ZDvidVersionDag dag;

  ZDvidUrl dvidUrl(getDvidTarget());

  ZDvidBufferReader &bufferReader = m_bufferReader;
  bufferReader.read(dvidUrl.getRepoInfoUrl().c_str(), isVerbose());
  setStatusCode(bufferReader.getStatusCode());

  QString str(bufferReader.getBuffer().data());
  str.replace(QRegExp("\"MaxLabel\":\\s*\\{[^{}]*\\}"), "\"MaxLabel\":{}");

//  qDebug() << str;

  ZJsonObject infoJson;
  infoJson.decodeString(str.toStdString().c_str());

  dag.load(infoJson, uuid);

  clearBuffer();

  return dag;
}

int ZDvidReader::readBodyBlockCount(uint64_t bodyId) const
{
  int count = 0;
  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonObject jsonObj = readJsonObject(dvidUrl.getSparsevolSizeUrl(bodyId));
  if (jsonObj.hasKey("numblocks")) {
    count = ZJsonParser::integerValue(jsonObj["numblocks"]);
  } else {
    //Todo: add block count read for labelblk data
    count = readCoarseBodySize(bodyId);
  }

  return count;
}

ZObject3dScan* ZDvidReader::readCoarseBody(uint64_t bodyId, ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(dvidUrl.getCoarseSparsevolUrl(
                bodyId, m_dvidTarget.getBodyLabelName()).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutube::EReadStatus::OK) {
    if (obj == NULL) {
      obj = new ZObject3dScan;
    }

    obj->importDvidObjectBuffer(
          reader.getBuffer().data(), reader.getBuffer().size());
    obj->setLabel(bodyId);
  }

  clearBuffer();

  return obj;
}

ZObject3dScan* ZDvidReader::readCoarseBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType, ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case flyem::EBodyLabelType::BODY:
    url = dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  case flyem::EBodyLabelType::SUPERVOXEL:
    url = dvidUrl.getCoarseSupervoxelUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  }

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutube::EReadStatus::OK) {
    if (obj == NULL) {
      obj = new ZObject3dScan;
    }

    obj->importDvidObjectBuffer(
          reader.getBuffer().data(), reader.getBuffer().size());
    obj->setLabel(bodyId);
  }

  clearBuffer();

  return obj;
}

ZObject3dScan* ZDvidReader::readCoarseBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType, const ZIntCuboid &box,
    ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case flyem::EBodyLabelType::BODY:
    url = dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  case flyem::EBodyLabelType::SUPERVOXEL:
    url = dvidUrl.getCoarseSupervoxelUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  }

  url = ZDvidUrl::AppendRangeQuery(url, box);

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutube::EReadStatus::OK) {
    if (obj == NULL) {
      obj = new ZObject3dScan;
    }

    obj->importDvidObjectBuffer(
          reader.getBuffer().data(), reader.getBuffer().size());
    obj->setLabel(bodyId);
  }

  clearBuffer();

  return obj;
}

ZObject3dScan ZDvidReader::readCoarseBody(uint64_t bodyId) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(dvidUrl.getCoarseSparsevolUrl(
                bodyId, m_dvidTarget.getBodyLabelName()).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  ZObject3dScan obj;
  obj.importDvidObjectBuffer(
        reader.getBuffer().data(), reader.getBuffer().size());
  obj.setLabel(bodyId);

  clearBuffer();

  return obj;
}

ZObject3dScan ZDvidReader::readCoarseBody(
    uint64_t bodyId, flyem::EBodyLabelType labelType) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case flyem::EBodyLabelType::BODY:
    url = dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  case flyem::EBodyLabelType::SUPERVOXEL:
    url = dvidUrl.getCoarseSupervoxelUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  }

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  ZObject3dScan obj;
  obj.importDvidObjectBuffer(
        reader.getBuffer().data(), reader.getBuffer().size());
  obj.setLabel(bodyId);

  clearBuffer();

  return obj;
}

int ZDvidReader::readCoarseBodySize(uint64_t bodyId) const
{
  int count = 0;

  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(dvidUrl.getCoarseSparsevolUrl(
                bodyId, m_dvidTarget.getBodyLabelName()).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutube::EReadStatus::OK) {
    count = ZObject3dScan::CountVoxelNumber(
          reader.getBuffer().data(), reader.getBuffer().size());
  }

  clearBuffer();

  return count;
}

uint64_t ZDvidReader::readBodyIdAt(const ZIntPoint &pt) const
{
  if (pt.isValid()) {
    return readBodyIdAt(pt.getX(), pt.getY(), pt.getZ());
  }

  return 0;
}

uint64_t ZDvidReader::readBodyIdAt(int x, int y, int z) const
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  bufferReader.read(dvidUrl.getLocalBodyIdUrl(x, y, z).c_str(), isVerbose());
  setStatusCode(bufferReader.getStatusCode());

  ZJsonObject infoJson;
  infoJson.decodeString(bufferReader.getBuffer().data());

  uint64_t bodyId = 0;
  if (infoJson.hasKey("Label")) {
    bodyId = (uint64_t) ZJsonParser::integerValue(infoJson["Label"]);
  }

  clearBuffer();

  return bodyId;
}

uint64_t ZDvidReader::readSupervoxelIdAt(const ZIntPoint &pt) const
{
  return readSupervoxelIdAt(pt.getX(), pt.getY(), pt.getZ());
}

uint64_t ZDvidReader::readSupervoxelIdAt(int x, int y, int z) const
{
  uint64_t bodyId = 0;

  if (getDvidTarget().hasSupervoxel()) {
    ZDvidBufferReader &bufferReader = m_bufferReader;
    ZDvidUrl dvidUrl(m_dvidTarget);
    bufferReader.read(dvidUrl.getLocalSupervoxelIdUrl(x, y, z).c_str(), isVerbose());
    setStatusCode(bufferReader.getStatusCode());

    ZJsonObject infoJson;
    infoJson.decodeString(bufferReader.getBuffer().data());

    if (infoJson.hasKey("Label")) {
      bodyId = (uint64_t) ZJsonParser::integerValue(infoJson["Label"]);
    }

    clearBuffer();
  }

  return bodyId;
}

std::vector<uint64_t> ZDvidReader::readSupervoxelSet(uint64_t bodyId) const
{
  std::vector<uint64_t> result;

  ZDvidUrl url(getDvidTarget());
  std::string urlStr = url.getSupervoxelMapUrl(bodyId);
  if (!urlStr.empty()) {
    ZJsonArray jsonArray = readJsonArray(urlStr);
    for (size_t i = 0; i < jsonArray.size(); ++i) {
      result.push_back(ZJsonParser::integerValue(jsonArray.at(i)));
    }
  }

  return result;
}

std::vector<std::vector<uint64_t> > ZDvidReader::readBodyIdAt(
    const std::vector<std::vector<ZIntPoint> > &ptArray) const
{
  //Flatten the point array
  std::vector<ZIntPoint> flatPtArray;
  for (std::vector<std::vector<ZIntPoint> >::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const std::vector<ZIntPoint>& intPtArray = *iter;
    flatPtArray.insert(flatPtArray.end(), intPtArray.begin(), intPtArray.end());
  }

  std::vector<uint64_t> bodyIdArray = readBodyIdAt(flatPtArray);

  //Fold back
  std::vector<std::vector<uint64_t> > result;
  size_t index = 0;
  if (flatPtArray.size() == bodyIdArray.size()) {
    for (std::vector<std::vector<ZIntPoint> >::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const std::vector<ZIntPoint>& intPtArray = *iter;
      std::vector<uint64_t> intIdArray;
      for (size_t i = 0; i < intPtArray.size(); ++i) {
        intIdArray.push_back(bodyIdArray[index++]);
      }
      result.push_back(intIdArray);
    }
  }

  return result;
}

std::vector<uint64_t> ZDvidReader::readBodyIdAt(
    const std::vector<ZIntPoint> &ptArray) const
{
  std::vector<uint64_t> bodyArray;

  if (!ptArray.empty()) {
    ZDvidBufferReader &bufferReader = m_bufferReader;
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
    STD_COUT << "Payload: " << queryForm.toStdString() << std::endl;
#endif

    QByteArray payload;
    payload.append(queryForm);

    bufferReader.read(
          dvidUrl.getLocalBodyIdArrayUrl().data(), payload, "GET", true);
    setStatusCode(bufferReader.getStatusCode());

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

ZJsonObject ZDvidReader::readAnnotationJson(
    const std::string &dataName, const ZIntPoint &pt) const
{
  return readAnnotationJson(dataName, pt.getX(), pt.getY(), pt.getZ());
}

ZJsonObject ZDvidReader::readAnnotationJson(
    const std::string &dataName, int x, int y, int z) const
{
  ZJsonObject annotationJson;

  if (!dataName.empty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);

    ZIntCuboid box(x, y, z, x, y, z);
    ZJsonArray obj = readJsonArray(dvidUrl.getAnnotationUrl(dataName, box));
    if (obj.size() > 0) {
      annotationJson.set(obj.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
    }
  }

  return annotationJson;
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

ZJsonArray ZDvidReader::readRoiJson(const std::string &dataName)
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  bufferReader.read(dvidUrl.getRoiUrl(dataName).c_str());
  const QByteArray &buffer = bufferReader.getBuffer();

  ZJsonArray array;
  array.decodeString(buffer.constData());

  return array;
}

ZObject3dScan* ZDvidReader::readRoi(
    const std::string &dataName, ZObject3dScan *result, bool appending) const
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  bufferReader.read(dvidUrl.getRoiUrl(dataName).c_str());
  const QByteArray &buffer = bufferReader.getBuffer();

  ZJsonArray array;
  array.decodeString(buffer.constData());

  if (result == NULL) {
    result = new ZObject3dScan;
  }
  result->importDvidRoi(array, appending);

  ZIntPoint blockSize = readRoiBlockSize(dataName);
  result->setDsIntv(blockSize.getX() - 1, blockSize.getY() - 1,
                    blockSize.getZ() - 1);

  return result;
}

ZObject3dScan ZDvidReader::readRoi(const std::string &dataName) const
{
  ZObject3dScan obj;
  readRoi(dataName, &obj);
  return obj;
}

ZDvidRoi* ZDvidReader::readRoi(const std::string &dataName, ZDvidRoi *roi)
{
  if (roi == NULL) {
    roi = new ZDvidRoi();
  } else {
    roi->clear();
  }

  readRoi(dataName, roi->getRoiRef());
  roi->setName(dataName);
  roi->setBlockSize(readRoiBlockSize(dataName));

  return roi;
}

ZFlyEmBodyAnnotation ZDvidReader::readBodyAnnotation(uint64_t bodyId) const
{
  ZFlyEmBodyAnnotation annotation;

  if (getDvidTarget().hasBodyLabel()) {
    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader &bufferReader = m_bufferReader;
    bufferReader.read(url.getBodyAnnotationUrl(bodyId).c_str(), isVerbose());

    annotation.loadJsonString(bufferReader.getBuffer().constData());
    annotation.setBodyId(bodyId);
  }

  return annotation;
}

bool ZDvidReader::hasBodyAnnotation() const
{
  return hasData(getDvidTarget().getBodyAnnotationName());
}

ZJsonObject ZDvidReader::readBodyAnnotationJson(uint64_t bodyId) const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonObject(url.getBodyAnnotationUrl(bodyId).c_str());
}

ZJsonObject ZDvidReader::readJsonObject(const std::string &url) const
{
  ZJsonObject obj;

  if (!url.empty()) {
    ZDvidBufferReader &bufferReader = m_bufferReader;
    if (ZString(url).startsWith("http:")) {
      bufferReader.read(url.c_str(), isVerbose());
    } else {
      bufferReader.readFromPath(url.c_str(), isVerbose());
    }
    setStatusCode(bufferReader.getStatusCode());
    const QByteArray &buffer = bufferReader.getBuffer();
    if (!buffer.isEmpty()) {
      obj.decodeString(buffer.constData());
    }
  }

  return obj;
}

ZJsonObject ZDvidReader::readJsonObjectFromKey(
    const QString &dataName, const QString &key) const
{
  ZJsonObject obj;
  const QByteArray &buffer = readKeyValue(dataName, key);

  if (!buffer.isEmpty()) {
    obj.decodeString(buffer.constData());
  }

  return obj;
}

QList<ZJsonObject> ZDvidReader::readJsonObjectsFromKeys(const QString &dataName,
    const QStringList &keyList) const
{
    QList<ZJsonObject> objects;
    const QList<QByteArray> &buffers = readKeyValues(dataName, keyList);

    for (int i=0; i<buffers.size(); i++) {
        ZJsonObject obj;
        if (!buffers[i].isEmpty()) {
            obj.decodeString(buffers[i].constData());
            objects.append(obj);
        }
    }
    return objects;
}

ZJsonArray ZDvidReader::readJsonArray(const std::string &url) const
{
  ZJsonArray obj;

  ZDvidBufferReader &bufferReader = m_bufferReader;
  bufferReader.read(url.c_str(), isVerbose());
  const QByteArray &buffer = bufferReader.getBuffer();
  if (!buffer.isEmpty()) {
    obj.decodeString(buffer.constData());
  }

  return obj;
}

ZJsonArray ZDvidReader::readJsonArray(
    const std::string &url, const QByteArray &payload) const
{
  ZJsonArray obj;

  ZDvidBufferReader &bufferReader = m_bufferReader;
  bufferReader.read(url.c_str(), payload, "GET", isVerbose());
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
    const ZIntCuboid &box, flyem::EDvidAnnotationLoadMode mode) const
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

ZJsonArray ZDvidReader::readSynapseLabelsz(int n, ZDvid::ELabelIndexType index) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(dvidUrl.getSynapseLabelszUrl(n, index));

  return obj;
}

int ZDvidReader::readSynapseLabelszBody(
    uint64_t bodyId, ZDvid::ELabelIndexType index) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject obj = readJsonObject(dvidUrl.getSynapseLabelszBodyUrl(bodyId, index));

  int count = 0;
  std::string key = ZDvidUrl::GetLabelszIndexTypeStr(index);
  if (obj.hasKey(key.c_str())) {
    count = ZJsonParser::integerValue(obj[key.c_str()]);
  }

  return count;
}

ZJsonArray ZDvidReader::readSynapseLabelszThreshold(int threshold, ZDvid::ELabelIndexType index) const {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = readJsonArray(dvidUrl.getSynapseLabelszThresholdUrl(threshold, index));
    return obj;
}

ZJsonArray ZDvidReader::readSynapseLabelszThreshold(int threshold, ZDvid::ELabelIndexType index,
    int offset, int number) const {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = readJsonArray(dvidUrl.getSynapseLabelszThresholdUrl(threshold, index, offset, number));
    return obj;
}

std::vector<ZDvidSynapse> ZDvidReader::readSynapse(
    uint64_t label, flyem::EDvidAnnotationLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonArray obj = readJsonArray(
        dvidUrl.getSynapseUrl(label, mode != flyem::EDvidAnnotationLoadMode::NO_PARTNER));

  std::vector<ZDvidSynapse> synapseArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    synapseArray[i].loadJsonObject(synapseJson, mode);
    synapseArray[i].setBodyId(label);
  }

  return synapseArray;
}

std::vector<ZDvidSynapse> ZDvidReader::readSynapse(
    uint64_t label, const ZDvidRoi &roi,
    flyem::EDvidAnnotationLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonArray obj = readJsonArray(
        dvidUrl.getSynapseUrl(label, mode != flyem::EDvidAnnotationLoadMode::NO_PARTNER));

  std::vector<ZDvidSynapse> synapseArray;

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZIntPoint position = ZDvidAnnotation::GetPosition(synapseJson);
    if (roi.contains(position)) {
      synapseArray.resize(synapseArray.size() + 1);
      synapseArray.back().loadJsonObject(synapseJson, mode);
      synapseArray.back().setBodyId(label);
    }
  }

  return synapseArray;
}

ZDvidSynapse ZDvidReader::readSynapse(
    int x, int y, int z, flyem::EDvidAnnotationLoadMode mode) const
{
  std::vector<ZDvidSynapse> synapseArray =
      readSynapse(ZIntCuboid(x, y, z, x, y, z), mode);
  if (!synapseArray.empty()) {
    if (synapseArray.size() > 1) {
      LWARN() << "Duplicated synapses at" << "(" << x << "" << y << "" << z << ")";
      synapseArray[0].setStatus(ZDvidAnnotation::STATUS_DUPLICATED);
    }
    return synapseArray[0];
  }

  return ZDvidSynapse();
}

ZDvidSynapse ZDvidReader::readSynapse(
    const ZIntPoint &pt, flyem::EDvidAnnotationLoadMode mode) const
{
  return readSynapse(pt.getX(), pt.getY(), pt.getZ(), mode);
}

std::string ZDvidReader::readMirror() const
{
  std::string mirror;

  if (isReady()) {
    ZDvidUrl dvidUrl(getDvidTarget());
    std::string url = dvidUrl.getMirrorInfoUrl();
    m_bufferReader.read(url.c_str());

    mirror = GetMirrorAddressFromBuffer(m_bufferReader);
  }

  return mirror;
}

std::string ZDvidReader::readMasterNode() const
{
  std::string master;

  if (good()) {
    ZDvidUrl dvidUrl(getDvidTarget());
    std::string url = dvidUrl.getMasterUrl();
    LINFO() << "Master url: " << url;
    m_bufferReader.read(url.c_str());
    master = GetMasterNodeFromBuffer(m_bufferReader);
  }

  return master;
}

std::vector<std::string> ZDvidReader::readMasterList() const
{
  std::vector<std::string> masterList;

  if (good()) {
    ZDvidUrl dvidUrl(getDvidTarget());
    std::string url = dvidUrl.getMasterUrl();
    LINFO() << "Master url: " << url;
    m_bufferReader.read(url.c_str());
    masterList = GetMasterListFromBuffer(m_bufferReader);
  }

  return masterList;
}


void ZDvidReader::loadDvidDataSetting(const ZJsonObject obj)
{
  m_dvidTarget.loadDvidDataSetting(obj);
}

void ZDvidReader::loadDefaultDataSetting()
{
  ZJsonObject obj = readDefaultDataSetting(READ_CURRENT);
  loadDvidDataSetting(obj);
}

ZJsonObject ZDvidReader::readDefaultDataSettingCurrent() const
{
  ZJsonObject obj;

  ZDvidUrl url(getDvidTarget());

  obj = readJsonObject(url.getDefaultDataInstancesUrl());

  return obj;
}

ZJsonObject ZDvidReader::readDefaultDataSettingTraceBack() const
{
  ZJsonObject obj;

  std::vector<std::string> uuidList = readMasterList();
  int index = -1;
  for (size_t i = 0; i < uuidList.size(); ++i) {
    const std::string &uuid = uuidList[i];
    if (ZDvid::IsUuidMatched(uuid, getDvidTarget().getUuid())) {
      index = i;
      break;
    }
  }

  obj = readDefaultDataSettingCurrent();

  for (int i = index + 1; i < (int) uuidList.size(); ++i) {
    ZDvidReader nodeReader;
    ZDvidTarget target = getDvidTarget();
    target.setUuid(uuidList[i]);
    if (nodeReader.open(target)) {
      ZJsonObject prevObj = nodeReader.readDefaultDataSettingCurrent();
      obj.addEntryFrom(prevObj);
    }
  }

  return obj;
}

ZJsonObject ZDvidReader::readDefaultDataSetting(EReadOption option) const
{
  ZJsonObject obj;
  switch (option) {
  case READ_CURRENT:
    obj = readDefaultDataSettingCurrent();
    break;
  case READ_TRACE_BACK:
    obj = readDefaultDataSettingTraceBack();
    break;
  }

  return obj;
}

ZJsonObject ZDvidReader::readDataMap() const
{
  ZJsonObject obj;

  ZDvidUrl url(getDvidTarget());

  obj = readJsonObject(url.getDataMapUrl());

  return obj;
}

/*
std::vector<std::string> ZDvidReader::readMasterList() const
{
  return ReadMasterList(getDvidTarget());
}
*/
std::string ZDvidReader::ReadMasterNode(const ZDvidTarget &target)
{
#if defined(_FLYEM_)
  std::string master;
  std::string rootNode =
      GET_FLYEM_CONFIG.getDvidRootNode(target.getUuid());
  if (!rootNode.empty()) {
    ZDvidBufferReader reader;
    ZDvidUrl dvidUrl(target, rootNode);
    std::string url = dvidUrl.getMasterUrl();
    LINFO() << "Master url: " << url;
    reader.read(url.c_str());
    master = GetMasterNodeFromBuffer(reader);
  }

  return master;
#else
  return "";
#endif
}

std::vector<std::string> ZDvidReader::ReadMasterList(const ZDvidTarget &target)
{
#if defined(_FLYEM_)
  std::vector<std::string> masterList;
  std::string rootNode =
      GET_FLYEM_CONFIG.getDvidRootNode(target.getUuid());
  if (!rootNode.empty()) {
    ZDvidBufferReader reader;
    ZDvidUrl dvidUrl(target, rootNode);
    std::string url = dvidUrl.getMasterUrl();
    LINFO() << "Master url: " << url;
    reader.read(url.c_str());
    masterList = GetMasterListFromBuffer(reader);
  }

  return masterList;
#else
  return std::vector<std::string>();
#endif
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
    item.loadJsonObject(itemJson, flyem::EDvidAnnotationLoadMode::PARTNER_RELJSON);
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

ZJsonObject ZDvidReader::readToDoItemJson(int x, int y, int z)
{
  return readAnnotationJson(getDvidTarget().getTodoListName(), x, y, z);
}

ZJsonObject ZDvidReader::readToDoItemJson(const ZIntPoint &pt)
{
  return readToDoItemJson(pt.getX(), pt.getY(), pt.getZ());
}

bool ZDvidReader::reportMissingData(const std::string dataName) const
{
  if (!dataName.empty()) {
    bool missing = !hasData(dataName);

    if (missing) {
      STD_COUT << "WARNING: " << dataName << " is missing" << std::endl;
    }

    return missing;
  }

  return false;
}

QByteArray ZDvidReader::readServiceResult(
    const std::string &group, const std::string &key) const
{
//  ZDvidUrl url(getDvidTarget());

  return readDataFromEndpoint(
        ZDvidPath::GetResultKeyPath(
          QString(group.c_str()), QString(key.c_str())).toStdString());

//  return readBuffer(url.getKeyUrl(ZDvidEndPoint, key));
}

ZJsonObject ZDvidReader::readServiceTask(
    const std::string &group, const std::string &key) const
{
  QByteArray data = readDataFromEndpoint(
        ZDvidPath::GetTaskKeyPath(
          QString(group.c_str()), QString(key.c_str())).toStdString());
  ZJsonObject json;
  json.load(data.constData());
  return json;
}

std::map<std::string, ZJsonObject> ZDvidReader::readSplitTaskMap() const
{
  std::map<std::string, ZJsonObject> taskMap;
  std::string dataName = ZDvidData::GetName(ZDvidData::ROLE_SPLIT_TASK_KEY);
  QStringList keyList = readKeys(dataName.c_str(), "task__0", "task__z");
  foreach (const QString &key, keyList) {
    ZJsonObject obj = readJsonObjectFromKey(dataName.c_str(), key);
    if (obj.hasKey(neutube::json::REF_KEY)) {
      obj = readJsonObject(ZJsonParser::stringValue(obj[neutube::json::REF_KEY]));
    }
    if (!obj.isEmpty()) {
      taskMap[key.toStdString()] = obj;
    }
  }

  return taskMap;
}

QList<ZStackObject*> ZDvidReader::readSeedFromSplitTask(
    const std::string &taskKey, uint64_t bodyId)
{
  ZJsonObject taskJson = readJsonObjectFromKey(
        ZDvidData::GetTaskName("split").c_str(), taskKey.c_str());
  if (taskJson.hasKey(neutube::json::REF_KEY)) {
    taskJson = readJsonObject(
          ZJsonParser::stringValue(taskJson[neutube::json::REF_KEY]));
  }
  ZJsonArray seedArrayJson(taskJson.value("seeds"));
  QList<ZStackObject*> seedList;
  for (size_t i = 0; i < seedArrayJson.size(); ++i) {
    ZJsonObject seedJson(seedArrayJson.value(i));
    if (seedJson.hasKey("type")) {
      std::string type = ZJsonParser::stringValue(seedJson["type"]);
      if (type == "ZStroke2d") {
        ZStroke2d *stroke = new ZStroke2d;
        stroke->loadJsonObject(ZJsonObject(seedJson.value("data")));
        stroke->setPenetrating(false);

        if (!stroke->isEmpty()) {
          seedList.append(stroke);
        } else {
          delete stroke;
        }
      } else if (type == "ZObject3d") {
        ZObject3d *obj = new ZObject3d;
        obj->loadJsonObject(ZJsonObject(seedJson.value("data")));
        if (!obj->isEmpty()) {
          seedList.append(obj);
        } else {
          delete obj;
        }
      }
    }
  }
  foreach (ZStackObject *seed, seedList) {
    seed->addRole(ZStackObjectRole::ROLE_SEED |
                  ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
    seed->setSource(ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId));
    ZLabelColorTable colorTable;
    seed->setColor(colorTable.getColor(seed->getLabel()));
  }

  return seedList;
}

QList<ZStackObject*> ZDvidReader::readSeedFromSplitTask(
    const ZDvidTarget &target, uint64_t bodyId)
{
  ZDvidUrl dvidUrl(target);
  std::string taskKey =dvidUrl.getSplitTaskKey(bodyId);
  return readSeedFromSplitTask(taskKey, bodyId);
}

/*
ZJsonObject ZDvidReader::readTestTask() const
{
  ZDvidUrl url(getDvidTarget());
  std::string taskUrl = url.getTestTaskUrl();
  ZJsonObject taskJson = readJsonObject(taskUrl);

  return taskJson;
}
*/

ZJsonObject ZDvidReader::readTestTask(const std::string &key) const
{
  ZDvidUrl url(getDvidTarget());
  std::string taskUrl = url.getTestTaskUrl(key);
  ZJsonObject taskJson = readJsonObject(taskUrl);

  return taskJson;
}


bool ZDvidReader::hasSplitTask(const QString &key) const
{
  return hasKey(ZDvidData::GetName(ZDvidData::ROLE_SPLIT_TASK_KEY).c_str(), key);
}

int ZDvidReader::checkProofreadingData() const
{
  int missing = 0;

  STD_COUT << "Checking proofreading status" << std::endl;
  missing += reportMissingData(getDvidTarget().getTodoListName());
  missing += reportMissingData(getDvidTarget().getSegmentationName());
  missing += reportMissingData(getDvidTarget().getBodyLabelName());
  missing += reportMissingData(getDvidTarget().getBodyAnnotationName());
  missing += reportMissingData(getDvidTarget().getSkeletonName());
//  missing += reportMissingData(getDvidTarget().getGrayScaleName());
//  missing += reportMissingData(getDvidTarget().getMultiscale2dName());
  missing += reportMissingData(getDvidTarget().getSplitLabelName());

  return missing;
}
