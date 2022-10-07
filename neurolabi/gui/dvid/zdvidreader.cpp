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

#include "logging/zqslog.h"
#include "logging/zlog.h"
#include "qt/network/znetworkutils.h"

//#include "zglobal.h"
#include "zjsondef.h"
#include "zstack.hxx"
//#include "zdvidbuffer.h"
#include "zstackfactory.h"
#include "zswctree.h"
#include "zclosedcurve.h"
#include "zstackutil.h"

#include "zdvidinfo.h"

#include "neutubeconfig.h"

#include "common/utilities.h"
#include "geometry/zaffinerect.h"
#include "zarray.h"
#include "zstring.h"
#include "zobject3dscan.h"
#include "zsparsestack.h"
#include "zobject3dscanarray.h"
#include "zmeshio.h"
#include "zmesh.h"
#include "zstroke2d.h"
#include "zobject3d.h"

#include "zstackobjectsourcefactory.h"
#include "zarrayfactory.h"
#include "zobject3dfactory.h"

//#include "qt/network/znetbufferreader.h"
#include "qt/network/znetbufferreaderthread.h"

#include "zdvidversiondag.h"
#include "zdvidfilter.h"
#include "zdvidurl.h"
#include "zdvidtile.h"
#include "zdvidtileinfo.h"
#include "zdvidsparsestack.h"
#include "libdvidheader.h"
#include "zdvidutil.h"
#include "zdvidroi.h"
#include "zdvidstackblockfactory.h"
#include "zdvidsynapse.h"
#include "zdvidpath.h"
#include "zjsonobjectparser.h"
#include "zdvidutil.h"
//#include "flyem/zflyemtodoitem.h"
//#include "zflyemutilities.h"


//#include "flyem/zserviceconsumer.h"



ZDvidReader::ZDvidReader(/*QObject *parent*/) :
  /*QObject(parent),*/ m_verbose(true)
{
}

ZDvidReader::~ZDvidReader()
{
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
  m_dvidTarget = ZDvidTarget();
//  m_dvidTarget.clear();
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

  if (!dvid::IsServerReachable(getDvidTarget())) {
    return false;
  }

#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    m_service = dvid::MakeDvidNodeService(getDvidTarget());
    m_connection = dvid::MakeDvidConnection(getDvidTarget().getRootUrl());
    m_bufferReader.setService(m_service);
//    m_bufferReader.setService(getDvidTarget());
  } catch (std::exception &e) {
    m_service.reset();
    m_errorMsg = e.what();
    LERROR() <<  e.what();
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
    m_dvidTarget.setNodeStatus(dvid::ENodeStatus::OFFLINE);
  }

  return succ;
}

bool ZDvidReader::open(const ZDvidTarget &target)
{
  if (!target.isValid()) {
    return false;
  }

  if (target.isMock()) {
    m_dvidTarget = target;
    return getNodeStatus() == dvid::ENodeStatus::NORMAL;
  }

  bool succ = false;

  if (target.isInferred()) {
    succ = openRaw(target);
  } else {
    m_dvidTarget = target;

    if (dvid::IsServerReachable(target)) {
      std::string mappedUuid = InferUuid(target);
      if (!mappedUuid.empty()) {
        m_dvidTarget.setMappedUuid(target.getUuid(), mappedUuid);
      }
      /*
      std::string masterNode = ReadMasterNode(target);
      if (!masterNode.empty()) {
        m_dvidTarget.setMappedUuid(target.getUuid(), masterNode);
      } else {
        std::string userNode = ReadUserNode(target);
        if (!userNode.empty()) {
          m_dvidTarget.setMappedUuid(target.getUuid(), userNode);
        }
      }
      */

      succ = startService();

      if (succ) { //Read default settings
        updateNodeStatus();

        if (getDvidTarget().usingDefaultDataSetting()) {
          loadDefaultDataSetting();
        }

        updateSegmentationData();
        m_dvidTarget.setInferred(true);
      }
    }

    if (!succ) {
      m_dvidTarget.setNodeStatus(dvid::ENodeStatus::OFFLINE);
    }
  }

  return succ;
}

void ZDvidReader::updateDataStatus()
{
  ZJsonObject obj = readJsonObjectFromKey("neutu_config", "data_status");
  if (!obj.isEmpty()) {
    if (obj.hasKey(getDvidTarget().getSynapseName().c_str())) {
      ZJsonObject synapseObj(obj.value(getDvidTarget().getSynapseName().c_str()));
      if (synapseObj.hasKey("role")) {
        if (std::string(ZJsonParser::stringValue(synapseObj["role"]))
            == "synapse") {
          if (synapseObj.hasKey("readonly")) {
            getDvidTarget().setSynapseReadonly(
                  ZJsonParser::booleanValue(synapseObj["readonly"]));
          }
        }
      }
    }

    if (obj.hasKey("@default")) {
      ZJsonObject defaultDataJson(obj.value("@default"));
      if (getDvidTarget().getSynapseLabelszName().empty()) {
        if (defaultDataJson.hasKey("labelsz")) {
          std::string labelsz = ZJsonParser::stringValue(defaultDataJson["labelsz"]);
          if (hasData(labelsz)) {
            getDvidTarget().setSynapseLabelszName(labelsz);
            getDvidTarget().enableSynapseLabelsz(true);
          }
        }
      }
    }
  }
}

std::vector<std::string> ZDvidReader::readDataInstancesOfType(
    const std::string &type)
{
  std::vector<std::string> dataList;

  ZJsonObject meta = readInfo();
  ZJsonObject datains(meta.value("DataInstances"));

  datains.forEachValue([&](const std::string &key, ZJsonValue v) {
    ZJsonObject dataObj(v);
    std::string dataType = ZJsonObjectParser::GetValue(
          ZJsonObject(dataObj.value("Base")), "TypeName", "");
    if (type == dataType) {
      dataList.push_back(key);
    }
  });

  /*
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
  */

  return dataList;
}

void ZDvidReader::updateSegmentationData()
{
  std::string dataName;
  if (getDvidTarget().hasSegmentation()) {
    dataName = getDvidTarget().getSegmentationName();
  } else {
    dataName = getDvidTarget().getBodyLabelName();
  }

  if (!dataName.empty()) {
    std::string typeName = getType(dataName);

    if (typeName == "labelarray") {
      getDvidTarget().setSegmentationType(ZDvidData::EType::LABELARRAY);
    } else if (typeName == "labelmap") {
      getDvidTarget().setSegmentationType(ZDvidData::EType::LABELMAP);
    } else if (typeName == "labelvol") { //sync to labelblk and labelvol
      getDvidTarget().setSegmentationType(ZDvidData::EType::LABELBLK);
      std::string labelName = readLabelblkName(dataName);
      getDvidTarget().setSegmentationName(labelName);
      getDvidTarget().setBodyLabelName(dataName);
    } else if (typeName == "labelblk") {
      getDvidTarget().setSegmentationType(ZDvidData::EType::LABELBLK);
      if (getDvidTarget().getBodyLabelName().empty()) {
        syncBodyLabelName();
      }
    }

    if (!getDvidTarget().hasSegmentation()) {
      if (getDvidTarget().segmentationAsBodyLabel()) {
        getDvidTarget().setSegmentationName(dataName);
      }
    }
  }

  if (!getDvidTarget().getSynapseLabelszName().empty()) {
    getDvidTarget().enableSynapseLabelsz(
          hasData(getDvidTarget().getSynapseLabelszName()));
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

dvid::ENodeStatus ZDvidReader::getNodeStatus() const
{
  dvid::ENodeStatus status = dvid::ENodeStatus::NORMAL;

#ifdef _ENABLE_LIBDVIDCPP_
  if (!getDvidTarget().isMock()) {
    ZDvidUrl url(getDvidTarget());
    std::string repoUrl = url.getRepoUrl() + "/info";
    if (repoUrl.empty()) {
      status = dvid::ENodeStatus::INVALID;
    } else {
      if (ZNetworkUtils::IsAvailable(repoUrl.c_str(), znetwork::EOperation::HAS_HEAD)) {
        ZJsonObject obj = readJsonObject(
              ZDvidUrl::AppendSourceQuery(url.getCommitInfoUrl()));
        //      ZJsonObject obj =
        //          ZGlobal::GetInstance().readJsonObjectFromUrl(url.getCommitInfoUrl());
        if (obj.hasKey("Locked")) {
          bool locked = ZJsonParser::booleanValue(obj["Locked"]);
          if (locked) {
            status = dvid::ENodeStatus::LOCKED;
          }
        }
      } else {
        status = dvid::ENodeStatus::OFFLINE;
      }
    }
  }
#endif

  return status;
}

namespace {

std::string with_source_query(const std::string &url)
{
  return ZDvidUrl::AppendSourceQuery(url);
}

void read_with_source(
    ZDvidBufferReader &reader, const std::string &url, bool verbose)
{
  reader.read(with_source_query(url).c_str(), verbose);
}

}

/*
void ZDvidReader::testApiLoad()
{
#if defined(_ENABLE_LIBDVIDCPP_)
  dvid::MakeRequest(*m_connection, "/api/load", "GET",
                     libdvid::BinaryDataPtr(), libdvid::DEFAULT,
                     m_statusCode);
#endif
}
*/

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, neutu::EBodyLabelType labelType,
    int z, neutu::EAxis axis, bool canonizing,
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
    if (labelType == neutu::EBodyLabelType::BODY) {
      url = dvidUrl.getSparsevolUrl(bodyId, z, z, axis);
    } else {
      url = dvidUrl.getSupervoxelUrl(bodyId, z, z, axis);
    }

    url = ZDvidUrl::AppendSourceQuery(url);

    QElapsedTimer timer;
    timer.start();
    reader.read(url.c_str(), isVerbose());
    ZOUT(KLog(neutu::TOPIC_NULL), 5) << ZLog::Profile()
                    << ZLog::Diagnostic("Body reading time")
                    << ZLog::Duration(timer.elapsed());
//    ZOUT(LTRACE(), 5) << "Reading time:" << url << timer.elapsed() << "ms";

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
    uint64_t bodyId, int z, neutu::EAxis axis, bool canonizing,
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
    url = ZDvidUrl::AppendSourceQuery(url);
    QElapsedTimer timer;
    timer.start();
    reader.read(url.c_str(), isVerbose());
    ZOUT(KLog(neutu::TOPIC_NULL), 5) << ZLog::Profile()
                    << ZLog::Diagnostic("Body reading time")
                    << ZLog::Duration(timer.elapsed());
//    ZOUT(LTRACE(), 5) << "Reading time:" << url << timer.elapsed() << "ms";

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
    uint64_t bodyId, neutu::EBodyLabelType labelType, ZObject3dScan *result) const
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
        readBody(bodyId, labelType, startZ, endZ, true, neutu::EAxis::Z, result);
      } else {
#ifdef _DEBUG_
        STD_COUT << "Read part: " << startZ << "--" << endZ << std::endl;
#endif
        readBody(bodyId, labelType, startZ, endZ, true, neutu::EAxis::Z, &part);
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
  return readBodyWithPartition(bodyId, neutu::EBodyLabelType::BODY, result);
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
    uint64_t bodyId, int minZ, int maxZ, bool canonizing, neutu::EAxis axis,
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
    std::string url = dvidUrl.getSparsevolUrl(bodyId, minZ, maxZ, axis);
    url = ZDvidUrl::AppendSourceQuery(url);
    reader.read(url.c_str(), isVerbose());
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
    uint64_t bodyId, neutu::EBodyLabelType labelType,
    int minZ, int maxZ, bool canonizing, neutu::EAxis axis,
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
    if (labelType == neutu::EBodyLabelType::BODY) {
      url = dvidUrl.getSparsevolUrl(bodyId, minZ, maxZ, axis);
    } else if (labelType == neutu::EBodyLabelType::SUPERVOXEL) {
      url = dvidUrl.getSupervoxelUrl(bodyId, minZ, maxZ, axis);
    }

    url = ZDvidUrl::AppendSourceQuery(url);
    reader.read(url.c_str(), isVerbose());

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
  return readBody(bodyId, neutu::EBodyLabelType::BODY, box, canonizing, result);

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
    uint64_t bodyId, neutu::EBodyLabelType labelType,
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

      if (bodySize / neutu::ONEGIGA > 4) {
        needPartition = true;
      }
    }

    ZDvidBufferReader &reader = m_bufferReader;

    if (needPartition == false) {
      ZDvidUrl dvidUrl(getDvidTarget());
      std::string url;
      switch (labelType) {
      case neutu::EBodyLabelType::BODY:
        url = dvidUrl.getSparsevolUrl(bodyId, box);
//        reader.read(dvidUrl.getSparsevolUrl(bodyId, box).c_str(),
//                    isVerbose());
        break;
      case neutu::EBodyLabelType::SUPERVOXEL:
        url = dvidUrl.getSupervoxelUrl(bodyId, box);
//        reader.read(dvidUrl.getSupervoxelUrl(bodyId, box).c_str(),
//                    isVerbose());
        break;
      }
      url = ZDvidUrl::AppendSourceQuery(url);
      reader.read(url.c_str(), isVerbose());

      if (reader.getStatus() != neutu::EReadStatus::FAILED) {
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

ZObject3dScan* ZDvidReader::readBodyWithPartition(
    const dvid::SparsevolConfig &config, bool canonizing, int npart,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    if (npart <= 0) {
      npart = 1;
    }

    ZIntCuboid box = config.range;
    if (box.isEmpty()) {
      size_t nvoxels = 0;
      size_t nblocks = 0;
      std::tie(nvoxels, nblocks, box) =
          readBodySizeInfo(config.bodyId, config.labelType);
    }

    ZDvidUrl dvidUrl(getDvidTarget());
    neutu::RangePartitionProcess(
          box.getMinZ(), box.getMaxZ(), npart, [&, this](int z0, int z1) {
      dvid::SparsevolConfig subconfig = config;
      ZObject3dScan part;
      subconfig.range.setMinZ(z0);
      subconfig.range.setMaxZ(z1);
      if (this->getDvidTarget().hasBlockCoding()) {
        subconfig.format = "blocks";
        QByteArray buffer = this->readBuffer(
              with_source_query(dvidUrl.getSparsevolUrl(subconfig)));
        part.importDvidBlockBuffer(buffer.data(), buffer.size(), canonizing);
      } else {
        this->readBody(
              subconfig.bodyId, subconfig.labelType, subconfig.zoom,
              subconfig.range, canonizing, &part);
      }
#ifdef _DEBUG_
      std::cout << "Part size: #voxels: " << part.getVoxelNumber() << "; "
                << part.getIntBoundBox().toString() << std::endl;
#endif
      result->concat(part);
    });
    if (canonizing) {
      result->canonize();
    }
  }

  return result;
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, neutu::EBodyLabelType labelType, int zoom,
    const ZIntCuboid &box, bool canonizing,
    ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (isReady() && zoom <= getMaxLabelZoom()) {
    if (result == NULL) {
      result = new ZObject3dScan;
    }

    if (getDvidTarget().hasBlockCoding()) {
      dvid::SparsevolConfig config;
      config.bodyId = bodyId;
      config.format = "blocks";
      config.range = box;
      config.zoom = zoom;
      config.labelType = labelType;

      ZDvidUrl dvidUrl(getDvidTarget());
      QElapsedTimer timer;
      timer.start();
      QByteArray buffer = readBuffer(
            with_source_query(dvidUrl.getSparsevolUrl(config)));

      if (buffer.isEmpty()) {
        LKINFO(neutu::TOPIC_NULL) << "Failed to read body data.";
        size_t nvoxels = 0;
        size_t nblocks = 0;
        ZIntCuboid newBox;
        std::tie(nvoxels, nblocks, newBox) = readBodySizeInfo(bodyId, labelType);

        if (nvoxels > 0) {
          LKINFO(neutu::TOPIC_NULL) << QString("Try to read %1 with partitions").arg(bodyId);

          int zoomScale = zgeom::GetZoomScale(zoom);
          int npart = nblocks / 500 / zoomScale / zoomScale / zoomScale;
          config.range = newBox;
          readBodyWithPartition(config, canonizing, npart, result);
        }
      } else {
        std::cout << "Reading body data with " << buffer.size() << " bytes: "
                  <<  timer.elapsed() << " ms" << std::endl;

        timer.restart();
        result->importDvidBlockBuffer(buffer.data(), buffer.size(), canonizing);
        std::cout << "Parsing body: " << timer.elapsed() << " ms" << std::endl;
      }
    } else {
      readBodyRle(bodyId, labelType, zoom, box, canonizing, result);
    }

  }

  return result;
}

ZObject3dScan *ZDvidReader::readBodyRle(
    uint64_t bodyId, neutu::EBodyLabelType labelType, int zoom,
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
    case neutu::EBodyLabelType::BODY:
      if (zoom == 0 || box.isEmpty()) {
        reader.read(
              ZDvidUrl::AppendSourceQuery(
                dvidUrl.getSparsevolUrl(bodyId, zoom, box)).c_str(),
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
    case neutu::EBodyLabelType::SUPERVOXEL:
      reader.read(
            ZDvidUrl::AppendSourceQuery(
              dvidUrl.getSupervoxelUrl(bodyId, zoom, box)).c_str(),
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

    reader.read(
          with_source_query(dvidUrl.getSparsevolUrl(bodyId)).c_str(), isVerbose());

    reader.tryCompress(false);

    auto readingTime = timer.elapsed();

//    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    timer.start();
    const QByteArray &buffer = reader.getBuffer();
    result->importDvidObjectBufferDs(buffer.data(), buffer.size());
    if (canonizing) {
      result->canonize();
    }

    auto parsingTime = timer.elapsed();

    KLog(neutu::TOPIC_NULL) << ZLog::Profile()
           << ZLog::Duration(readingTime + parsingTime)
           << ZLog::Description(
                QString("Read %1: reading time = %2 ms; parsing time = %3 ms").
                arg(readingTime).arg(parsingTime).toStdString())
           << ZLog::Time();


//    STD_COUT << "Body parsing time: " << timer.elapsed() << std::endl;

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

    read_with_source(reader, dvidUrl.getSparsevolUrl(bodyId), isVerbose());

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
  m_bufferReader.read(url.c_str(), isVerbose());

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
    LERROR() <<  e.what();
    m_statusCode = e.getStatus();
  }
#endif

  return buffer;
}

ZObject3dScan *ZDvidReader::readSupervoxel(
    uint64_t bodyId, bool canonizing, ZObject3dScan *result) const
{
  return readBody(
        bodyId, neutu::EBodyLabelType::SUPERVOXEL, ZIntCuboid(), canonizing, result);
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
#if 1
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

    reader.read(with_source_query(dvidUrl.getSparsevolUrl(bodyId)).c_str(),
                isVerbose());

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    if (reader.getStatus() != neutu::EReadStatus::FAILED) {
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
#endif
}

ZObject3dScan *ZDvidReader::readBody(
    uint64_t bodyId, neutu::EBodyLabelType labelType,
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
    case neutu::EBodyLabelType::BODY:
      reader.read(
            with_source_query(dvidUrl.getSparsevolUrl(bodyId)).c_str(),
            isVerbose());
      break;
    case neutu::EBodyLabelType::SUPERVOXEL:
      reader.read(
            with_source_query(dvidUrl.getSupervoxelUrl(bodyId)).c_str(),
            isVerbose());
      break;
    }

    reader.tryCompress(false);

    STD_COUT << "Body reading time: " << timer.elapsed() << std::endl;

    if (reader.getStatus() != neutu::EReadStatus::FAILED) {
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

std::tuple<QByteArray, std::string> ZDvidReader::readMeshBufferFromUrl(
    const std::string &url) const
{
  std::tuple<QByteArray, std::string> result;

  ZDvidTarget target;
  target.setFromUrl_deprecated(url);
  if (target.getRootUrl() != getDvidTarget().getRootUrl() ||
      target.getUuid() != getDvidTarget().getUuid()) {
    LWARN() << "Unmatched target";
    return result;
  }

  std::string format = "";
  ZString trimmedUrl = neutu::WithoutQueryString(url);
  if (trimmedUrl.endsWith(".ngmesh", ZString::CASE_INSENSITIVE)) {
    format = "ngmesh";
  } else if (trimmedUrl.endsWith(".drc", ZString::CASE_INSENSITIVE)) {
    format = "drc";
  } else if (trimmedUrl.endsWith(".obj", ZString::CASE_INSENSITIVE)) {
    format = "obj";
  } else {
    ZJsonObject infoJson = readJsonObject(ZDvidUrl::GetMeshInfoUrl(trimmedUrl));
    if (infoJson.hasKey("format")) {
      format = ZJsonParser::stringValue(infoJson["format"]);
    }
  }

//  QByteArray buffer;
  m_bufferReader.read(url.c_str(), isVerbose());
  if (m_bufferReader.getStatus() != neutu::EReadStatus::FAILED) {
    result = std::make_tuple(m_bufferReader.getBuffer(), format);
  }
  m_bufferReader.clearBuffer();

  return result;
}

ZMesh* ZDvidReader::readMeshFromUrl(
    const std::string &url, const ZMeshIO &mio) const
{
  ZMesh *mesh = nullptr;

  QByteArray buffer;
  std::string format;
  std::tie(buffer, format) = ZDvidReader::readMeshBufferFromUrl(url);
  if (!buffer.isEmpty()) {
    mesh = mio.loadFromMemory(buffer, format);
    if (format == "ngmesh") {
      ZDvidInfo info = readLabelInfo();
      ZResolution res = info.getVoxelResolution();
      double sx = 1.0 / res.getVoxelSize(neutu::EAxis::X, res.getUnit());
      double sy = 1.0 / res.getVoxelSize(neutu::EAxis::Y, res.getUnit());
      double sz = 1.0 / res.getVoxelSize(neutu::EAxis::Z, res.getUnit());
      mesh->scale(sx, sy, sz);
    }
  }

  return mesh;
}

ZMesh* ZDvidReader::readMeshFromUrl(const std::string &url) const
{
  return readMeshFromUrl(url, ZMeshIO::instance());
  /*
  ZMesh *mesh = nullptr;

  QByteArray buffer;
  std::string format;
  std::tie(buffer, format) = ZDvidReader::readMeshBufferFromUrl(url);
  if (!buffer.isEmpty()) {
    mesh = ZMeshIO::instance().loadFromMemory(buffer, format);
    if (format == "ngmesh") {
      ZDvidInfo info = readLabelInfo();
      ZResolution res = info.getVoxelResolution();
      double sx = 1.0 / res.getVoxelSize(neutu::EAxis::X, res.getUnit());
      double sy = 1.0 / res.getVoxelSize(neutu::EAxis::Y, res.getUnit());
      double sz = 1.0 / res.getVoxelSize(neutu::EAxis::Z, res.getUnit());
      mesh->scale(sx, sy, sz);
    }
  }


  return mesh;
  */
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

ZMesh* ZDvidReader::readMesh(const std::string &key) const
{
  return readMesh(getDvidTarget().getMeshName(), key);
}

std::vector<uint64_t> ZDvidReader::readMergedMeshKeys(uint64_t bodyId) const
{
  std::vector<uint64_t> bodyArray;

  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonArray mergeArray = readJsonArray(dvidUrl.getMergedMeshUrl(bodyId));
  for (size_t i = 0; i < mergeArray.size(); ++i) {
    int64_t v = ZJsonParser::integerValue(mergeArray.getData(), i);
    if (v >= 0) {
      uint64_t subId = uint64_t(v);
      if (bodyId != subId) {
        std::vector<uint64_t> subArray = readMergedMeshKeys(subId);
        if (subArray.empty()) {
          bodyArray.push_back(subId);
        } else {
          bodyArray.insert(bodyArray.end(), subArray.begin(), subArray.end());
        }
      } else {
        bodyArray.push_back(bodyId);
      }
    }
  }

  return bodyArray;
}

std::vector<uint64_t> ZDvidReader::readMergedMeshKeys(const std::string &key) const
{
  std::vector<uint64_t> bodyArray;

  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonArray mergeArray = readJsonArray(
        dvidUrl.getKeyUrl(getDvidTarget().getMeshName(), key));
  for (size_t i = 0; i < mergeArray.size(); ++i) {
    int64_t v = ZJsonParser::integerValue(mergeArray.getData(), i);
    if (v >= 0) {
      uint64_t subId = uint64_t(v);
      if (key != ZDvidUrl::GetMeshKey(subId, ZDvidUrl::EMeshType::MERGED)) {
        std::vector<uint64_t> subArray = readMergedMeshKeys(subId);
        if (subArray.empty()) {
          bodyArray.push_back(subId);
        } else {
          bodyArray.insert(bodyArray.end(), subArray.begin(), subArray.end());
        }
      } else {
        bodyArray.push_back(subId);
      }
    }
  }

  return bodyArray;
}


bool ZDvidReader::hasConsistentMergedMesh(uint64_t bodyId) const
{
  bool succ = false;

  std::vector<uint64_t> bodyArray = ZDvidReader::readMergedMeshKeys(bodyId);
  if (!bodyArray.empty()) {
    succ = true;

    for (uint64_t bodyId : bodyArray) {
      if (!hasKey(
            getDvidTarget().getMeshName().c_str(),
            ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::NG).c_str())) {
        succ = false;
        break;
      }
    }
  }

  return succ;
}

std::vector<uint64_t>
ZDvidReader::readConsistentMergedMeshKeys(uint64_t bodyId) const
{
  std::vector<uint64_t> bodyArray = ZDvidReader::readMergedMeshKeys(bodyId);
  for (uint64_t subId : bodyArray) {
    if (!hasKey(
          getDvidTarget().getMeshName().c_str(),
          ZDvidUrl::GetMeshKey(subId, ZDvidUrl::EMeshType::NG).c_str())) {
      bodyArray.clear();
      break;
    }
  }

  return bodyArray;
}

std::vector<uint64_t>
ZDvidReader::readConsistentMergedMeshKeys(const std::string &key) const
{
  std::vector<uint64_t> bodyArray = ZDvidReader::readMergedMeshKeys(key);
  for (uint64_t bodyId : bodyArray) {
    if (!hasKey(
          getDvidTarget().getMeshName().c_str(),
          ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::NG).c_str())) {
      bodyArray.clear();
      break;
    }
  }

  return bodyArray;
}

ZMesh* ZDvidReader::readMesh(
    const std::string &data, const std::string &key, const ZMeshIO &mio) const
{
  ZDvidUrl dvidUrl(getDvidTarget());

  ZMesh *mesh = nullptr;

  if (hasKey(data.c_str(), key.c_str())) {
    if (ZString(key).endsWith(".merge")) {
      std::vector<uint64_t> bodyArray = readConsistentMergedMeshKeys(key);
      if (!bodyArray.empty()) {
        mesh = new ZMesh;
        for (uint64_t bodyId : bodyArray) {
          std::unique_ptr<ZMesh> submesh(
                readMesh(data, ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::NG), mio));
          if (submesh) {
            mesh->append(*submesh);
          }
        }
      }
    } else {
      std::string meshUrl = dvidUrl.getKeyUrl(data, key);
      mesh = readMeshFromUrl(with_source_query(meshUrl), mio);
    }
  }

  return mesh;
}

ZMesh* ZDvidReader::readMesh(const std::string &data, const std::string &key) const
{
  return readMesh(data, key, ZMeshIO::instance());
  /*
  ZDvidUrl dvidUrl(getDvidTarget());

  ZMesh *mesh = nullptr;

  if (hasKey(data.c_str(), key.c_str())) {
    if (ZString(key).endsWith(".merge")) {
      std::vector<uint64_t> bodyArray = readConsistentMergedMeshKeys(key);
      if (!bodyArray.empty()) {
        mesh = new ZMesh;
        for (uint64_t bodyId : bodyArray) {
          std::unique_ptr<ZMesh> submesh(
                readMesh(data, ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::NG)));
          if (submesh) {
            mesh->append(*submesh);
          }
        }
      }
    } else {
      std::string meshUrl = dvidUrl.getKeyUrl(data, key);
      mesh = readMeshFromUrl(with_source_query(meshUrl));
    }
  }

  return mesh;
  */
}

ZMesh* ZDvidReader::readSupervoxelMesh(uint64_t svId) const
{
  ZMesh *mesh = NULL;

  ZDvidUrl dvidUrl(getDvidTarget());
  std::string url = dvidUrl.getSupervoxelMeshUrl(svId);
  if (!url.empty()) {
    m_bufferReader.read(with_source_query(url).c_str(), isVerbose());
    if (m_bufferReader.getStatus() != neutu::EReadStatus::FAILED) {
      const QByteArray &buffer = m_bufferReader.getBuffer();
      mesh = ZMeshIO::instance().loadFromMemory(buffer, "drc");
    }
    m_bufferReader.clearBuffer();
  }

  return mesh;
}

struct archive *ZDvidReader::readMeshArchiveStart(
    uint64_t bodyId, bool useOldMeshesTars) const
{
  size_t bytesTotal;
  return readMeshArchiveStart(bodyId, bytesTotal, useOldMeshesTars);
}

struct archive *ZDvidReader::readMeshArchiveStart(
    uint64_t bodyId, size_t &bytesTotal, bool useOldMeshesTars) const
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
  if (m_bufferReader.getStatus() == neutu::EReadStatus::FAILED) {
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
        bodyId, neutu::EBodyLabelType::BODY, zoom, ZIntCuboid(), canonizing, result);
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

  reader.read(
        ZDvidUrl::AppendSourceQuery(url.getSkeletonUrl(bodyId)).c_str(),
        isVerbose());

  ZSwcTree *tree = NULL;
  if (reader.getStatusCode() == 200) {
    const QByteArray &buffer = reader.getBuffer();

    if (!buffer.isEmpty()) {
      tree = new ZSwcTree;
      tree->loadFromBuffer(buffer.constData());
      if (tree->isEmpty()) {
        delete tree;
        tree = NULL;
      }
    }
  }
  reader.clearBuffer();
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

  ZDvidUrl dvidUrl(getDvidTarget());
  std::string url = dvidUrl.getThumbnailUrl(bodyId);

  ZDvidBufferReader &reader = m_bufferReader;
  reader.read(ZDvidUrl::AppendSourceQuery(url).c_str(), isVerbose());

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
  return readGrayScale(cuboid.getMinCorner().getX(),
                       cuboid.getMinCorner().getY(),
                       cuboid.getMinCorner().getZ(),
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

  std::string url = dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                                 blockIndex.getY(),
                                                 blockIndex.getZ(),
                                                 blockNumber);

  bufferReader.read(ZDvidUrl::AppendSourceQuery(url).c_str(), isVerbose());
  m_statusCode = bufferReader.getStatusCode();
#ifdef _DEBUG_2
  STD_COUT << "reading time:" << std::endl;
  ptoc();
#endif

#ifdef _DEBUG_2
  tic();
#endif

  if (bufferReader.getStatus() == neutu::EReadStatus::OK) {
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
#endif

#if 0
        STD_COUT << getDvidTarget().getGrayScaleName() << std::endl;
        STD_COUT << blockCoords[0] << " " << blockCoords[1] << " " << blockCoords[2] << std::endl;

        STD_COUT << blockNumber << std::endl;
#endif
      libdvid::GrayscaleBlocks blocks = m_service->get_grayblocks(
            getDvidTarget().getGrayScaleName(zoom), blockCoords, blockNumber);
#ifdef _DEBUG_2
        STD_COUT << "one read done" << std::endl;
#endif

      ZIntCuboid currentBox = dvidInfo.getBlockBox(blockIndex);
      for (int i = 0; i < blockNumber; ++i) {
#ifdef _DEBUG_2
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
  read_with_source(
        bufferReader,
        dvidUrl.getGrayScaleBlockUrl(blockIndex.getX(),
                                     blockIndex.getY(),
                                     blockIndex.getZ()),
        isVerbose());
  setStatusCode(bufferReader.getStatusCode());
  ZStack *stack = NULL;
  if (bufferReader.getStatus() == neutu::EReadStatus::OK) {
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

void ZDvidReader::readGrayScaleBlock(
    std::vector<int> &blockcoords, int zoom, ZStack *dest) const
{
  if (!blockcoords.empty() && dest) {
    try {
      std::vector<libdvid::DVIDCompressedBlock> c_blocks;
      ZDvidInfo info = readGrayScaleInfo();
      m_service->get_specificblocks3D(
            getDvidTarget().getGrayScaleName(zoom),
            blockcoords, /*gray=*/true, c_blocks, 0, info.isLz4Compression());

      if (!c_blocks.empty()) {
        auto substack = std::unique_ptr<ZStack>(
              ZStackFactory::MakeZeroStack(
                info.getBlockSize().getX(), info.getBlockSize().getY(),
                info.getBlockSize().getZ()));
        for (const auto &block : c_blocks) {
//          libdvid::DVIDCompressedBlock &block = c_blocks[0];
          libdvid::BinaryDataPtr data = block.get_uncompressed_data();
          std::vector<int> offset = block.get_offset();
          substack->setOffset(offset[0], offset[1], offset[2]);
          substack->copyValueFrom(data->get_raw(), data->length());
          substack->paste(dest);
        }
      }
    } catch(libdvid::DVIDException &e) {
      LERROR() << e.what();
      m_statusCode = e.getStatus();
    } catch (std::exception &e) {
      LERROR() << e.what();
      m_statusCode = 0;
    }
  }
}

ZStack* ZDvidReader::readGrayScaleBlock(int bx, int by, int bz, int zoom) const
{
  ZDvidInfo info = readGrayScaleInfo();
  return readGrayScaleBlock(bx, by, bz, zoom, info);
}

ZStack* ZDvidReader::readGrayScaleBlock(
    int bx, int by, int bz, int zoom, const ZDvidInfo &info) const
{
  std::vector<int> blockcoords;
  blockcoords.push_back(bx);
  blockcoords.push_back(by);
  blockcoords.push_back(bz);

  ZStack *stack = NULL;

  std::vector<libdvid::DVIDCompressedBlock> c_blocks;
  try {
    m_service->get_specificblocks3D(
          getDvidTarget().getGrayScaleName(zoom),
          blockcoords, /*gray=*/true, c_blocks, 0, info.isLz4Compression());

    if (c_blocks.size() == 1) {
      libdvid::DVIDCompressedBlock &block = c_blocks[0];
      libdvid::BinaryDataPtr data = block.get_uncompressed_data();
      std::vector<int> offset = block.get_offset();

      ZIntCuboid box;
      box.setMinCorner(offset[0], offset[1], offset[2]);
      box.setSize(info.getBlockSize());
      stack = ZStackFactory::MakeZeroStack(box);
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

#if 0
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
    uint64_t bodyId, neutu::EBodyLabelType labelType) const
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
    uint64_t bodyId, neutu::EBodyLabelType labelType) const
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
    uint64_t bodyId, neutu::EBodyLabelType type, ZSparseStack *out) const
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

//    spStack->setBlockMask(readCoarseBody(bodyId, type, NULL));

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
  if (width <= 0 || height <= 0 || depth <= 0) {
    return nullptr;
  }

  int zoomRatio = pow(2, zoom);

  return readGrayScaleWithBlock(
        /*getDvidTarget().getGrayScaleName(zoom),*/
        x0 / zoomRatio, y0 / zoomRatio, z0 / zoomRatio,
        width / zoomRatio, height / zoomRatio, std::max(1, depth / zoomRatio),
        zoom);
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
    LERROR() << e.what();
    setStatusCode(e.getStatus());
  } catch (std::exception &e) {
    LERROR() << e.what();
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

  ZOUT(KLOG(neutu::TOPIC_NULL), 5) << ZLog::Profile() << ZLog::Description("grayscale reading time")
         << ZLog::Duration(m_readingTime);

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

ZStack* ZDvidReader::readGrayScaleWithBlock(const ZIntCuboid &box, int zoom) const
{
  return readGrayScaleWithBlock(
        box.getMinX(), box.getMinY(), box.getMinZ(),
        box.getWidth(), box.getHeight(), box.getDepth(), zoom);
}

ZStack* ZDvidReader::readGrayScaleWithBlock(
    int x0, int y0, int z0, int width, int height, int depth, int zoom) const
{
  ZStack *stack = nullptr;

  ZDvidInfo info = readGrayScaleInfo();
  ZIntCuboid box;
  box.setMinCorner(x0, y0, z0);
  box.setSize(width, height, depth);
  ZObject3dScan blockObj = info.getBlockIndex(box);

  if (!blockObj.isEmpty()) {
    stack = ZStackFactory::MakeZeroStack(box);
    ZObject3dScan::ConstVoxelIterator iter(&blockObj);
    std::vector<int> blockCoords;
    while (iter.hasNext()) {
      ZIntPoint blockIndex = iter.next();
      if (info.isValidBlockIndex(blockIndex)) {
        blockCoords.push_back(blockIndex.getX());
        blockCoords.push_back(blockIndex.getY());
        blockCoords.push_back(blockIndex.getZ());
      }

//      auto substack = std::unique_ptr<ZStack>(
//            readGrayScaleBlock(
//              blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(), zoom, info));
//      if (substack) {
//        substack->paste(stack);
//      }
    }
    readGrayScaleBlock(blockCoords, zoom, stack);
  }

  return stack;
  /*
  return readGrayScale(getDvidTarget().getGrayScaleName(),
                       x0, y0, z0, width, height, depth);
                       */
}


ZStack* ZDvidReader::readGrayScale(
    int x0, int y0, int z0, int width, int height, int depth) const
{
  return readGrayScale(x0, y0, z0, width, height, depth, 0);
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

//  return ZGlobal::GetInstance().readJsonObjectFromUrl(url.getInfoUrl());
  return readJsonObject(with_source_query(url.getInfoUrl()));
}

ZJsonObject ZDvidReader::readInfo(const std::string &dataName) const
{
 std::string url = ZDvidUrl(getDvidTarget()).getInfoUrl(dataName);

// return ZGlobal::GetInstance().readJsonObjectFromUrl(url);
 return readJsonObject(with_source_query(url));
}

ZDvidInfo ZDvidReader::readDataInfo(const std::string &dataName) const
{
  ZJsonObject obj = readInfo(dataName);

  ZDvidInfo dvidInfo;

  if (!obj.isEmpty()) {
    dvidInfo.set(obj);
    dvidInfo.setDvidNode(
          getDvidTarget().getRootUrl(), getDvidTarget().getPort(),
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
          if (dvid::GetDataType(getType(dataName)) == dvid::EDataType::LABELVOL) {
            name = dataName;
            break;
          }
        }
      }
    }
  }

  return name;
}

std::string ZDvidReader::readLabelblkName(const std::string &labelvolName) const
{
  std::string name;

  ZJsonObject dataInfo = readInfo(labelvolName);
  if (dataInfo.hasKey("Base")) {
    ZJsonObject baseObj(dataInfo.value("Base"));
    if (baseObj.hasKey("Syncs")) {
      ZJsonArray syncArray(baseObj.value("Syncs"));
      for (size_t i = 0; i < syncArray.size(); ++i) {
        std::string dataName =
            ZJsonParser::stringValue(syncArray.getData(), i);
        if (dvid::GetDataType(getType(dataName)) == dvid::EDataType::LABELBLK) {
          name = dataName;
          break;
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
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner, bool ignoringZero) const
{
  return readBodyId(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
                    lastCorner.getX() - firstCorner.getX() + 1,
                    lastCorner.getY() - firstCorner.getY() + 1,
                    lastCorner.getZ() - firstCorner.getZ() + 1,
                    ignoringZero);
}

namespace {

std::set<uint64_t> extract_body_id(const ZArray *array, bool ignoringZero)
{
  std::set<uint64_t> bodySet;

  if (array) {
    uint64_t *dataArray = array->getDataPointer<uint64_t>();
    for (size_t i = 0; i < array->getElementNumber(); ++i) {
      if (!ignoringZero || dataArray[i] > 0) {
        bodySet.insert(dataArray[i]);
      }
    }
  }

  return bodySet;
}

std::set<uint64_t> extract_body_id(
    const ZArray *array, size_t minBodySize, bool ignoringZero)
{
  std::set<uint64_t> bodySet;
  std::unordered_map<uint64_t, size_t> bodyHist;

  if (array) {
    uint64_t *dataArray = array->getDataPointer<uint64_t>();
    for (size_t i = 0; i < array->getElementNumber(); ++i) {
      if (!ignoringZero || dataArray[i] > 0) {
        uint64_t bodyId = dataArray[i];
        if (bodyHist.count(bodyId) == 0) {
          bodyHist[bodyId] = 1;
        } else {
          bodyHist[bodyId]++;
        }
      }
    }
    for (auto h : bodyHist) {
      if (h.second >= minBodySize) {
        bodySet.insert(h.first);
      }
    }
  }

  return bodySet;
}

}

std::set<uint64_t> ZDvidReader::readBodyId(
    int x0, int y0, int z0, int width, int height, int depth, bool ignoringZero) const
{
  std::unique_ptr<ZArray> array(readLabels64(x0, y0, z0, width, height, depth));

  return extract_body_id(array.get(), ignoringZero);
}

std::set<uint64_t> ZDvidReader::readBodyId(
    const ZAffineRect &rect, bool ignoringZero) const
{
  std::unique_ptr<ZArray> array(readLabels64Lowtis(rect, 0, 0, 0, false));

  return extract_body_id(array.get(), ignoringZero);
}

std::set<uint64_t> ZDvidReader::readBodyId(
    const ZIntCuboid &range, int zoom, size_t minBodySize, bool ignoringZero) const
{
  ZArray *array = nullptr;

  if (getDvidTarget().hasMultiscaleSegmentation()) {
    array = readLabels64Lowtis(range, zoom);
  } else {
    array = readLabels64(range, zoom);
  }

  return extract_body_id(array, minBodySize, ignoringZero);
}

std::set<uint64_t> ZDvidReader::readBodyId(
    const ZIntCuboid &range, int zoom, bool ignoringZero) const
{
  ZArray *array = nullptr;

  if (getDvidTarget().hasMultiscaleSegmentation()) {
    array = readLabels64Lowtis(range, zoom);
  } else {
    array = readLabels64(range, zoom);
  }

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

std::set<uint64_t> ZDvidReader::readBodyId(const ZDvidFilter &filter) const
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
      auto bodyId = *iter;
      if (!filter.isExcluded(bodyId)) {
        newBodySet.insert(bodyId);
      }
    }
    return newBodySet;
  }

  return bodyIdSet;
}

std::set<uint64_t> ZDvidReader::readBodyId(size_t minSize) const
{
  ZDvidBufferReader &bufferReader = m_bufferReader;

  ZDvidUrl dvidUrl(m_dvidTarget);
  read_with_source(bufferReader, dvidUrl.getBodyListUrl(minSize), isVerbose());
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

std::set<uint64_t> ZDvidReader::readBodyId(size_t minSize, size_t maxSize) const
{
  ZDvidBufferReader &bufferReader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  read_with_source(
        bufferReader, dvidUrl.getBodyListUrl(minSize, maxSize), isVerbose());
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
        ZDvidData::GetName(ZDvidData::ERole::BODY_ANNOTATION,
                           ZDvidData::ERole::SPARSEVOL,
                           getDvidTarget().getBodyLabelName()).c_str());

  std::set<uint64_t> bodySet;
  foreach (const QString &idStr, annotationList) {
    uint64_t bodyId = ZString(idStr.toStdString()).firstUint64();
    bodySet.insert(bodyId);
  }

  return bodySet;
}

namespace {
bool is_readable(const std::string &url)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::IS_READABLE);
  thread.setUrl(url.c_str());
  thread.start();
  thread.wait();

  return thread.getResultStatus();
}

bool has_key(const ZDvidTarget &target, const QString &dataName, const QString &key)
{
  return is_readable(
        with_source_query(ZDvidUrl(target).getKeyUrl(
                       dataName.toStdString(), key.toStdString())));
  /*
  ZNetBufferReaderThread thread;
  thread.setOperation(ZNetBufferReaderThread::EOperation::IS_READABLE);
  thread.setUrl(ZDvidUrl(target).getKeyUrl(
                  dataName.toStdString(), key.toStdString()).c_str());
  thread.start();
  thread.wait();
  */

//  return thread.getStatus();
//  ZNetBufferReader netBufferReader;
//  return netBufferReader.isReadable(
//        ZDvidUrl(target).getKeyUrl(
//          dataName.toStdString(), key.toStdString()).c_str());
}

}

bool ZDvidReader::hasKey(const QString &dataName, const QString &key) const
{
  QFuture<bool> future = QtConcurrent::run(
        &has_key, getDvidTarget(), dataName, key);
  return future.result();

//  return m_netBufferReader.isReadable(
//        ZDvidUrl(getDvidTarget()).getKeyUrl(dataName.toStdString(), key.toStdString()).c_str());
//  return !readKeyValue(dataName, key).isEmpty();
}

QByteArray ZDvidReader::readKeyValue(const QString &dataName, const QString &key) const
{
  ZDvidUrl url(getDvidTarget());

  ZDvidBufferReader &bufferReader = m_bufferReader;

  read_with_source(
        bufferReader,
        url.getKeyUrl(dataName.toStdString(), key.toStdString()),
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

namespace {

QList<QByteArray> parse_tar_response(const QByteArray &buffer)
{
  QList<QByteArray> ans;
  struct archive *archive = archive_read_new();
  archive_read_support_format_all(archive);

  int result = archive_read_open_memory(
        archive, buffer.constData(), size_t(buffer.size()));
  if (result != ARCHIVE_OK) {
      LINFO() << "couldn't expand keyvalue archive";
      return ans;
      }

  struct archive_entry *entry;
  while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
      const struct stat *s = archive_entry_stat(entry);
      size_t size = size_t(s->st_size);

      QByteArray buffer(int(size), 0);
      archive_read_data(archive, buffer.data(), size);
      ans.append(buffer);
  }
  result = archive_read_free(archive);
  if (result != ARCHIVE_OK) {
      LWARN() << "couldn't close keyvalue archive";
  }

  return ans;
}

}

QList<QByteArray> ZDvidReader::readKeyValues(
    const QString &dataName, const QString &startKey, const QString &endKey) const
{
  ZDvidUrl url(getDvidTarget());
  ZDvidBufferReader &bufferReader = m_bufferReader;

  // encode keylist into json payload
//  QJsonArray keys = QJsonArray::fromStringList(keyList);
//  QJsonDocument doc(keys);
//  QByteArray payload = doc.toJson();

  // make call with json keylist
  read_with_source(
        bufferReader,
        url.getKeyValuesUrl(
          dataName.toStdString(), startKey.toStdString(), endKey.toStdString()),
        isVerbose());
  setStatusCode(bufferReader.getStatusCode());

  // untar response into list of byte arrays
  const QByteArray &buffer = m_bufferReader.getBuffer();

  return parse_tar_response(buffer);
}

QList<QByteArray> ZDvidReader::readKeyValues(
    const QString &dataName, const QStringList &keyList) const
{
    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader &bufferReader = m_bufferReader;

    // encode keylist into json payload
    QJsonArray keys = QJsonArray::fromStringList(keyList);
    QJsonDocument doc(keys);
    QByteArray payload = doc.toJson();

    // make call with json keylist
    bufferReader.read(
          QString::fromStdString(
            with_source_query(url.getKeyValuesUrl(dataName.toStdString()))),
          payload, "GET", isVerbose());
    setStatusCode(bufferReader.getStatusCode());

    // untar response into list of byte arrays
    const QByteArray &buffer = m_bufferReader.getBuffer();

    return parse_tar_response(buffer);
    /*
    QList<QByteArray> ans;
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
    */
}

QStringList ZDvidReader::readKeys(const QString &dataName) const
{
  if (dataName.isEmpty()) {
    return QStringList();
  }

  ZDvidBufferReader &reader = m_bufferReader;

  ZDvidUrl dvidUrl(m_dvidTarget);

  read_with_source(
        reader, dvidUrl.getAllKeyUrl(dataName.toStdString()), isVerbose());
  setStatusCode(reader.getStatusCode());

  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i)).c_str();
    }
  }

  reader.clearBuffer();

  return keys;
}

QStringList ZDvidReader::readKeys(
    const QString &dataName, const QString &minKey) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  const std::string &maxKey = "\xff";

  read_with_source(
        reader, dvidUrl.getKeyRangeUrl(
          dataName.toStdString(), minKey.toStdString(), maxKey),
        isVerbose());
  setStatusCode(reader.getStatusCode());

  QByteArray keyBuffer = reader.getBuffer();

  QStringList keys;

  if (!keyBuffer.isEmpty()) {
    ZJsonArray obj;
    obj.decode(keyBuffer.data());
    for (size_t i = 0; i < obj.size(); ++i) {
      keys << ZJsonParser::stringValue(obj.at(i)).c_str();
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
  read_with_source(
        reader, url.getKeyRangeUrl(dataName.toStdString(), minKey.toStdString(),
                                   maxKey.toStdString()), isVerbose());
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
      keys << ZJsonParser::stringValue(obj.at(i)).c_str();
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
    int /*x0*/, int /*y0*/, int z0, int width, int height, int depth)
{
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
    const std::string &key, ZClosedCurve *result) const
{
  if (result != NULL) {
    result->clear();
  }

  QByteArray byteArray = readKeyValue("roi_curve", key.c_str());
  if (!byteArray.isEmpty()) {
    ZJsonObject obj;
    obj.decode(byteArray.constData(), false);

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
        ZDvidData::GetName<QString>(ZDvidData::ERole::NEUTU_CONFIG), "contrast");

  ZJsonObject config;
  if (!byteArray.isEmpty()) {
    config.decode(byteArray.data(), false);
  }

  return config;
}

ZJsonObject ZDvidReader::readBodyStatusV2() const
{
  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName<QString>(ZDvidData::ERole::NEUTU_CONFIG),
        "body_status_v2");

  ZJsonObject config;
  if (!byteArray.isEmpty()) {
    config.decode(byteArray.toStdString(), false);
  }

  return config;
}

/*
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
*/

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

#if 0
namespace {

class HasDataThread : public QThread
{
public:
  HasDataThread(QObject *parent = nullptr) : QThread(parent) {}
  bool getResult() const {
    return m_result;
  }
  void setParams(const ZDvidUrl &url, const std::string &dataName) {
    m_url = url;
    m_dataName = dataName;
  }


  void run() override {
    ZNetBufferReader netBufferReader;
    m_result = netBufferReader.isReadable(m_url.getInfoUrl(m_dataName).c_str());
  }

private:
  bool m_result = false;
  ZDvidUrl m_url;
  std::string m_dataName;
};

bool has_data(const ZDvidUrl &url, const std::string &dataName)
{
  ZNetBufferReaderThread thread;
  thread.setOperation(ZNetBufferReaderThread::EOperation::IS_READABLE);
  thread.setUrl(url.getInfoUrl(dataName).c_str());
  thread.start();
  thread.wait();

  return thread.getStatus();

//  HasDataThread thread;
//  thread.setParams(url, dataName);
//  thread.start();
//  thread.wait();
//  return thread.getResult();
}

}
#endif

bool ZDvidReader::hasData(const std::string &dataName) const
{
  if (dataName.empty()) {
    return false;
  }

  ZDvidUrl dvidUrl(m_dvidTarget);
  return is_readable(with_source_query(dvidUrl.getInfoUrl(dataName)));

//  return has_data(dvidUrl, dataName);
//  QFuture<bool> future = QtConcurrent::run(&has_data, dvidUrl, dataName);
//  return future.result();

//  ZNetBufferReader bufferReader;

//  return bufferReader.isReadable(dvidUrl.getInfoUrl(dataName).c_str());
}

std::string ZDvidReader::getType(const std::string &dataName) const
{
  std::string type;

  if (!dataName.empty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZDvidBufferReader &bufferReader = m_bufferReader;

    bufferReader.read(with_source_query(dvidUrl.getInfoUrl(dataName)).c_str());

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
        ZDvidData::ERole::SKELETON, ZDvidData::ERole::SPARSEVOL,
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
      pt = SwcTreeNode::center(tn).roundToIntPoint();
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
      box.setMinCorner(pt);
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

ZIntCuboid ZDvidReader::readBodyBoundBox(
    uint64_t bodyId, neutu::EBodyLabelType type) const
{
  ZIntCuboid box = ZIntCuboid::Empty();

  if (getDvidTarget().getSegmentationType() == ZDvidData::EType::LABELMAP) {
    size_t voxelCount;
    size_t blockCount;
    std::tie(voxelCount, blockCount, box) = readBodySizeInfo(bodyId, type);
  } else if (type == neutu::EBodyLabelType::BODY) {
    box = readBodyBoundBox(bodyId);
  }

  return box;
}

ZIntCuboid ZDvidReader::readBodyBoundBox(uint64_t bodyId) const
{
  ZIntCuboid box;

  if (getDvidTarget().getSegmentationType() == ZDvidData::EType::LABELMAP) {
    size_t voxelCount;
    size_t blockCount;
    std::tie(voxelCount, blockCount, box) =
        readBodySizeInfo(bodyId, neutu::EBodyLabelType::BODY);
  } else {
    setStatusCode(0);
#if defined(_ENABLE_LIBDVIDCPP_)
    if (m_service.get() != NULL) {
      try {
        libdvid::PointXYZ coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 0, true);
        box.setMinX(coord.x);

        coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 0, false);
        box.setMaxX(coord.x);

        coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 1, true);
        box.setMinY(coord.y);

        coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 1, false);
        box.setMaxY(coord.y);

        coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 2, true);
        box.setMinZ(coord.z);

        coord = m_service->get_body_extremum(
              getDvidTarget().getBodyLabelName(), bodyId, 2, false);
        box.setMaxZ(coord.z);
        setStatusCode(200);
      } catch (libdvid::DVIDException &e) {
        setStatusCode(e.getStatus());
      }
    }
#endif
  }

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
    pt = zstack::FindClosestBg(stack, x, y, z);
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
  return readLabels64(box.getMinCorner().getX(), box.getMinCorner().getY(),
                      box.getMinCorner().getZ(), box.getWidth(),
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
//      STD_COUT << dvidUrl.getLabels64Url(
//                     dataName, width, height, depth, x0, y0, z0).c_str()
//                << std::endl;

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
      KLOG(neutu::TOPIC_NULL) << ZLog::Profile() <<
              ZLog::Description("label reading time: " + dvidUrl.getLabels64Url(
                                  dataName, width, height, depth, x0, y0, z0))
           << ZLog::Duration(m_readingTime);
//      LINFO() << "label reading time: " << m_readingTime;
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
      KLOG(neutu::TOPIC_NULL) << ZLog::Error() << ZLog::Description(e.what());
//      LERROR() << e.what();
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
    box.setMinCorner(offset[0], offset[1], offset[2]);
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
    KLOG(neutu::TOPIC_NULL) << ZLog::Info() << ZLog::Description(name) << ZLog::Duration(time);
//    LINFO() << name + " reading time: " << time;
  }
}
}

template<typename T>
void ZDvidReader::configureLowtis(T *config, const std::string &dataName) const
{
  config->username = neutu::GetCurrentUserName();
  config->dvid_server = getDvidTarget().getRootUrl();
  config->dvid_uuid = getDvidTarget().getUuid();
  config->datatypename = dataName;
  config->enableprefetch = NeutubeConfig::LowtisPrefetching();
  config->supervoxelview = getDvidTarget().isSupervoxelView();
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

  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height
           << " @" << x0 << "," << y0 << "," << z0 << "c" << centerCut;

  if (m_lowtisServiceGray.get() == NULL) {
    try {
      configureLowtis(&m_lowtisConfigGray, getDvidTarget().getGrayScaleName());
//      m_lowtisConfigGray.username = neutube::GetCurrentUserName();
//      m_lowtisConfigGray.dvid_server = getDvidTarget().getAddressWithPort();
//      m_lowtisConfigGray.dvid_uuid = getDvidTarget().getUuid();
//      m_lowtisConfigGray.datatypename = getDvidTarget().getGrayScaleName();
      m_lowtisConfigGray.centercut = std::tuple<int, int>(cx, cy);
//      m_lowtisConfigGray.enableprefetch = NeutubeConfig::LowtisPrefetching();

      m_lowtisServiceGray = std::shared_ptr<lowtis::ImageService>(
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
    box.setMinCorner(x0 / scale, y0 / scale, z0 / scale);
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

#ifdef _DEBUG_
      std::cout << "Grayscale: " << width << "x" << height << "@" << offset[0]
                << ", " << offset[1] << ", " << offset[2] << "; zoom=" << zoom
                << std::endl;
#endif
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
        KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description("grayscale reading time")
               << ZLog::Duration(m_readingTime);
      }
    } else {
      KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description("grayscale reading time")
             << ZLog::Duration(m_readingTime);
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
  return MakeArray64(box.getMinCorner().getX(), box.getMinCorner().getY(),
                     box.getMinCorner().getZ(),
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
  box.setMinCorner(x0 / scale, y0 / scale, z0 / scale);
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
  box.setMinCorner(x0 / scale, y0 / scale, z0 / scale);
  box.setWidth(width);
  box.setHeight(height);
  box.setDepth(1);

  return box;
}

std::vector<uint64_t> ZDvidReader::readBodyIdAt(const ZJsonArray &queryObj) const
{
  std::vector<uint64_t> bodyArray;

  QString queryForm = queryObj.dumpString(0).c_str();

#ifdef _DEBUG_0
  std::cout << "Payload: " << queryForm.toStdString() << std::endl;
#endif

  QByteArray payload;
  payload.append(queryForm);

  ZDvidUrl dvidUrl(m_dvidTarget);
  m_bufferReader.read(
        with_source_query(dvidUrl.getLocalBodyIdArrayUrl()).c_str(),
        payload, "GET", true);
  setStatusCode(m_bufferReader.getStatusCode());

  ZJsonArray infoJson;
  infoJson.decodeString(m_bufferReader.getBuffer().data());

  for (size_t i = 0; i < infoJson.size(); ++i) {
    uint64_t bodyId = (uint64_t) ZJsonParser::integerValue(infoJson.at(i));
    bodyArray.push_back(bodyId);
  }

  return bodyArray;
}

void ZDvidReader::prepareLowtisService(
    std::shared_ptr<lowtis::ImageService> &service, const std::string &dataName,
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
  if (getLowtisServiceGray(cx, cy) == NULL || width == 0 || height == 0) {
    return NULL;
  }

  ZStack *stack = NULL;

  if (vx1 == 1.0 && vy1 == 0.0 && vz1 == 0.0 &&
      vx2 == 0.0 && vy2 == 1.0 && vz2 == 0.0) {
    // Use xy plane reading for slightly faster slice fetching
    stack = readGrayScaleLowtis(
          x0 - width / 2, y0 - height / 2, z0, width, height, zoom, cx, cy, centerCut);
    return stack;
  }


  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;
  qDebug() << "  Prefetching:" << m_lowtisConfigGray.enableprefetch;

  QElapsedTimer timer;
  timer.start();
  if (m_lowtisServiceGray.get() != NULL) {
//    m_lowtisService->config.bytedepth = 8;

    ZIntCuboid box = GetStackBoxAtCenter(x0, y0, z0, width, height, zoom);

#ifdef _DEBUG_
    std::cout << "Stack size: " << box.toString() << std::endl;
#endif
    stack = new ZStack(GREY, box, 1);

    try {
      std::vector<int> offset({x0, y0, z0});

      if (zoom == getDvidTarget().getMaxGrayscaleZoom() ||
          box.getWidth() < cx || box.getHeight() < cy) {
        centerCut = false;
      }

      if (zoom >= 1) {
//        zoom -= 1;
      }

      std::vector<double> dim1vec({vx1, vy1, vz1});
      std::vector<double> dim2vec({vx2, vy2, vz2});

#ifdef _DEBUG_2
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
        ar.getCenter().roundToIntPoint(), ar.getV1(), ar.getV2(),
        neutu::iround(ar.getWidth()), neutu::iround(ar.getHeight()),
        zoom, cx, cy, centerCut);
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
        ar.getCenter().roundToIntPoint(), ar.getV1(), ar.getV2(),
        neutu::iround(ar.getWidth()), neutu::iround(ar.getHeight()),
        zoom, cx, cy, centerCut);
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

  ZArray *array = NULL;

  if (vx1 == 1.0 && vy1 == 0.0 && vz1 == 0.0 &&
      vx2 == 0.0 && vy2 == 1.0 && vz2 == 0.0) {
    // Use xy plane reading for slightly faster slice fetching
    array = readLabels64Lowtis(
          x0 - width / 2, y0 - height / 2, z0, width, height, zoom, cx, cy, centerCut);
    return array;
  }

  ZIntCuboid box = GetStackBoxAtCenter(x0, y0, z0, width, height, zoom);


  qDebug() << "Using lowtis: (" << zoom << ")" << width << "x" << height;


  QElapsedTimer timer;
  timer.start();
  if (service != NULL) {
    array = MakeArray64(box);

    try {
      std::vector<int> offset = GetOffset(x0, y0, z0);
      std::vector<double> dim1vec = MakeVec3(vx1, vy1, vz1);
      std::vector<double> dim2vec = MakeVec3(vx2, vy2, vz2);

#ifdef _DEBUG_2
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
    } catch (std::exception &e) {
      LERROR() << e.what();
      setStatusCode(0);

      delete array;
      array = NULL;
    }

    m_readingTime = timer.elapsed();
    KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description("label reading time")
         << ZLog::Duration(m_readingTime);
//    LINFO() << "label reading time: " << m_readingTime;
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
    } catch (std::exception &e) {
      LERROR() << e.what();
      setStatusCode(0);
//      setStatusCode(e.getStatus());

      delete array;
      array = NULL;
    }

    m_readingTime = timer.elapsed();
    KLOG(neutu::TOPIC_NULL) << ZLog::Profile() << ZLog::Description("label reading time")
         << ZLog::Duration(m_readingTime);
//    LINFO() << "label reading time: " << m_readingTime;
  }

  return array;
}

ZArray* ZDvidReader::readLabels64Lowtis(int x0, int y0, int z0,
    int width, int height, int zoom) const
{
  return readLabels64Lowtis(x0, y0, z0, width, height, zoom, 256, 256, true);
}

ZArray* ZDvidReader::readLabels64Lowtis(const ZIntCuboid &range, int zoom) const
{
  if (range.isEmpty()) {
    return nullptr;
  }

  return readLabels64Lowtis(
        range.getMinX(), range.getMinY(), range.getMinZ(),
        range.getWidth(), range.getHeight(), range.getDepth(), zoom);
}

ZArray* ZDvidReader::readLabels64Lowtis(int x0, int y0, int z0,
                           int width, int height, int depth, int zoom) const
{
  mylib::Dimn_Type arrayDims[3];
  int scale = int(pow(2, zoom));
  arrayDims[0] = std::max(1, width / scale);
  arrayDims[1] = std::max(1, height / scale);
  arrayDims[2] = std::max(1, depth / scale);

  size_t area = size_t(arrayDims[0] * arrayDims[1]);

  ZArray *array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);
  array->setStartCoordinate(0, x0 / scale);
  array->setStartCoordinate(1, y0 / scale);
  array->setStartCoordinate(2, z0 / scale);

  size_t offset = 0;
  for (int dz = 0; dz < depth; dz += scale) {
    int z = z0 + dz;
    ZArray *subArray = readLabels64Lowtis(x0, y0, z, width, height, zoom);
    if (subArray) {
      array->copyDataFrom(
            subArray->getDataPointer<void>(), offset, area);
      offset += area;
    } else {
      break;
    }
  }

  return array;
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

bool ZDvidReader::hasBody(uint64_t bodyId, neutu::EBodyLabelType type) const
{
  if (type == neutu::EBodyLabelType::BODY) {
    return hasBody(bodyId);
  } else if (type == neutu::EBodyLabelType::SUPERVOXEL) {
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
    ZJsonObject jsonObj = readJsonObject(with_source_query(url));
    s = ZJsonParser::integerValue(jsonObj["voxels"]);
  }

  return s;
}

size_t ZDvidReader::readBodySize(
    uint64_t bodyId, neutu::EBodyLabelType type) const
{
  size_t s = 0;
  std::string url;
  if (type == neutu::EBodyLabelType::BODY) {
    url = ZDvidUrl(getDvidTarget()).getBodySizeUrl(bodyId);
  } else  if (type == neutu::EBodyLabelType::SUPERVOXEL) {
    url = ZDvidUrl(getDvidTarget()).getSupervoxelSizeUrl(bodyId);
  }
  if (!url.empty()) {
    ZJsonObject jsonObj = readJsonObject(with_source_query(url));
    s = ZJsonParser::integerValue(jsonObj["voxels"]);
  }

  return s;
}

std::vector<size_t> ZDvidReader::readBodySize(
    const std::vector<uint64_t> &bodyArray, neutu::EBodyLabelType type) const
{
  std::vector<size_t> result;
  if (!bodyArray.empty()) {
    QString queryForm="[";
    for (size_t i = 0; i< bodyArray.size(); ++i) {
      if (i == 0) {
        queryForm += std::to_string(bodyArray[i]).c_str();
      } else {
        queryForm += ("," + std::to_string(bodyArray[i])).c_str();
      }
    }
    queryForm += "]";

#ifdef _DEBUG_0
    std::cout << "Payload: " << queryForm.toStdString() << std::endl;
#endif

    QByteArray payload;
    payload.append(queryForm);

    ZDvidUrl dvidUrl(m_dvidTarget);
    m_bufferReader.read(
          with_source_query(dvidUrl.getBodySizeUrl(type)).c_str(),
          payload, "GET", true);
    setStatusCode(m_bufferReader.getStatusCode());

    ZJsonArray infoJson;
    infoJson.decodeString(m_bufferReader.getBuffer().data());

    for (size_t i = 0; i < infoJson.size(); ++i) {
      size_t bodySize = size_t(ZJsonParser::integerValue(infoJson.at(i)));
      result.push_back(bodySize);
    }
  }

  return result;
}

std::tuple<size_t, size_t, ZIntCuboid> ZDvidReader::readBodySizeInfo(
    uint64_t bodyId, neutu::EBodyLabelType type) const
{
  size_t voxelCount = 0;
  size_t blockCount = 0;
  ZIntCuboid boundBox;

  std::string url;
  url = ZDvidUrl(getDvidTarget()).getSparsevolSizeUrl(bodyId, type);
//  if (type == neutu::EBodyLabelType::BODY) {
//    url = ZDvidUrl(getDvidTarget()).getSparsevolSizeUrl(bodyId);
//  } else if (type == neutu::EBodyLabelType::SUPERVOXEL) {
//    url = ZDvidUrl(getDvidTarget()).getSupervoxelSizeUrl(bodyId);
//  }

  if (!url.empty()) {
    ZJsonObject jsonObj = readJsonObject(with_source_query(url));
    voxelCount = ZJsonParser::integerValue(jsonObj["voxels"]);
    blockCount = ZJsonParser::integerValue(jsonObj["numblocks"]);
    boundBox.setMinCorner(
          ZJsonParser::integerValue(jsonObj["minvoxel"], 0),
          ZJsonParser::integerValue(jsonObj["minvoxel"], 1),
          ZJsonParser::integerValue(jsonObj["minvoxel"], 2)
        );
    boundBox.setMaxCorner(
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
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::IS_READABLE);
  ZDvidUrl dvidUrl(m_dvidTarget);
  thread.setUrl(with_source_query(dvidUrl.getSparsevolUrl(
                  bodyId, getDvidTarget().getBodyLabelName())).c_str());
  thread.run();

  return thread.getResultStatus();

  /*
  ZNetBufferReader bufferReader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  return  bufferReader.isReadable(
        dvidUrl.getSparsevolUrl(bodyId, getDvidTarget().getBodyLabelName()).c_str());
        */
}

bool ZDvidReader::hasCoarseSparseVolume(uint64_t bodyId) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZNetBufferReaderThread thread;
  thread.setOperation(znetwork::EOperation::READ_PARTIAL);
  thread.setUrl(with_source_query(dvidUrl.getCoarseSparsevolUrl(
                  bodyId, getDvidTarget().getBodyLabelName())).c_str());
  thread.start();
  thread.wait();

  QByteArray byteArray = thread.getData();
  if (byteArray.size() >= 12) {
    return *((uint32_t*) (byteArray.data() + 8)) > 0;
  }
  return false;

  /*
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
  */

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

  return is_readable(
        with_source_query(
          dvidUrl.getBodyInfoUrl(bodyId, m_dvidTarget.getBodyLabelName())));

//  ZNetBufferReader bufferReader;

//  return  bufferReader.isReadable(
//        dvidUrl.getBodyInfoUrl(bodyId, m_dvidTarget.getBodyLabelName()).c_str());
}
/*
ZFlyEmNeuronBodyInfo ZDvidReader::readBodyInfo(uint64_t bodyId)
{
  ZJsonObject obj;

  QByteArray byteArray = readKeyValue(
        ZDvidData::GetName(ZDvidData::ERole::BODY_INFO,
                           ZDvidData::ERole::BODY_LABEL,
                           m_dvidTarget.getBodyLabelName()).c_str(),
        ZString::num2str(bodyId).c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData());
  }

  ZFlyEmNeuronBodyInfo bodyInfo;
  bodyInfo.loadJsonObject(obj);

  return bodyInfo;
}
*/

std::tuple<int64_t, std::string, std::string, std::string> ZDvidReader::readBodyMutationInfo(uint64_t bodyId) const
{
    int64_t mutId = -1;
    std::string modUser = "unknown user";
    std::string modApp = "unknown app";
    std::string modTime = "unknown time";

    ZDvidUrl dvidUrl(getDvidTarget());
    std::string url = dvidUrl.getSparsevolLastModUrl(bodyId);
    if (!url.empty()) {
      ZJsonObject obj = readJsonObject(with_source_query(url));
      ZJsonObjectParser parser;
      mutId = parser.GetValue(obj, "mutation id", int64_t(-1));
      modUser = parser.GetValue(obj, "last mod user", "unknown user");
      modApp = parser.GetValue(obj, "last mod app", "unknown app");
      modTime = parser.GetValue(obj, "last mod time", "unknown time");
      }
    return std::make_tuple(mutId, modUser, modApp, modTime);
}

int64_t ZDvidReader::readBodyMutationId(uint64_t bodyId) const
{
  int64_t mutId = 0;
  std::string modUser, modApp, modTime;
  std::tie(mutId, modUser, modApp, modTime) = readBodyMutationInfo(bodyId);
  return mutId;
}

void ZDvidReader::updateMaxGrayscaleZoom(int zoom)
{
   m_dvidTarget.setMaxGrayscaleZoom(zoom);
}

void ZDvidReader::updateMaxGrayscaleZoom(
    const ZJsonObject &infoJson, const ZDvidVersionDag &dag)
{
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (dvid::IsDataValid(
            m_dvidTarget.getGrayScaleName(level), m_dvidTarget, infoJson, dag)) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    updateMaxGrayscaleZoom(maxLabelLevel);
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

int ZDvidReader::getMaxLabelZoom() const
{
  if (m_maxLabelZoomUpdated == false) {
    const_cast<ZDvidReader&>(*this).updateMaxLabelZoom();
  }

  return getDvidTarget().getMaxLabelZoom();
}

void ZDvidReader::updateMaxLabelZoom(
    const ZJsonObject &infoJson, const ZDvidVersionDag &dag)
{
  if (m_dvidTarget.isValid()) {
    int maxLabelLevel = 0;
    int level = 1;
    while (level < 50) {
      if (dvid::IsDataValid(
            m_dvidTarget.getSegmentationName(level), m_dvidTarget, infoJson, dag)) {
        maxLabelLevel = level;
      } else {
        break;
      }
      ++level;
    }
    updateMaxLabelZoom(maxLabelLevel);
//    m_dvidTarget.setMaxLabelZoom(maxLabelLevel);
//    m_maxLabelZoomUpdated = true;
  }
}

void ZDvidReader::updateMaxLabelZoom(int zoom)
{
  m_dvidTarget.setMaxLabelZoom(zoom);
  m_maxLabelZoomUpdated = true;
}

void ZDvidReader::updateMaxLabelZoom()
{
  if (m_dvidTarget.isValid()) {
    if (getDvidTarget().hasMultiscaleSegmentation()) {
      ZJsonObject infoJson = readInfo(getDvidTarget().getSegmentationName());
      ZJsonValue v = infoJson.value({"Extended", "MaxDownresLevel"});
//      if (v.isInteger()) {
        updateMaxLabelZoom(v.isInteger() ? v.toInteger() : 0);
//      }
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
        ZDvidData::GetName<QString>(ZDvidData::ERole::MAX_BODY_ID),
        m_dvidTarget.getBodyLabelName().c_str());
  if (!byteArray.isEmpty()) {
    obj.decode(byteArray.constData(), false);
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
    bufferReader.read(with_source_query(dvidUrl.getInfoUrl(dataName)).c_str(),
                      isVerbose());
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

std::string ZDvidReader::GetUserNodeFromBuffer(
    const ZDvidBufferReader &bufferReader)
{
  std::string uuid;

  const char *data = bufferReader.getBuffer().data();
  if (data) {
    ZJsonObject obj;
    obj.decode(data, false);
    if (obj.isEmpty()) {
      uuid = ZString(data, bufferReader.getBuffer().length()).trimmed();
    } else {
      uuid = ZJsonObjectParser::GetValue(obj, "uuid", "");
    }
  }

  return uuid;
}

std::string ZDvidReader::GetMasterNodeFromBuffer(
    const ZDvidBufferReader &bufferReader)
{
  std::string master;

  ZJsonArray branchJson;
  branchJson.decodeString(bufferReader.getBuffer().data());
#ifdef _DEBUG_
  branchJson.print();
#endif

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
  bufferReader.read(
        with_source_query(dvidUrl.getReposInfoUrl()).c_str(), isVerbose());
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

int ZDvidReader::readBodyBlockCount(
    uint64_t bodyId, neutu::EBodyLabelType labelType) const
{
  int count = 0;
  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonObject jsonObj = readJsonObject(
        with_source_query(dvidUrl.getSparsevolSizeUrl(bodyId, labelType)));
  if (jsonObj.hasKey("numblocks")) {
    count = ZJsonParser::integerValue(jsonObj["numblocks"]);
  } else {
    //Todo: add block count read for labelblk data
    count = readCoarseBodySize(bodyId, labelType);
  }

  return count;
}

ZObject3dScan* ZDvidReader::readCoarseBody(uint64_t bodyId, ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(
        with_source_query(
          dvidUrl.getCoarseSparsevolUrl(
            bodyId, m_dvidTarget.getBodyLabelName())).c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutu::EReadStatus::OK) {
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
    uint64_t bodyId, neutu::EBodyLabelType labelType, ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case neutu::EBodyLabelType::BODY:
    url = with_source_query(dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName()));
    break;
  case neutu::EBodyLabelType::SUPERVOXEL:
    url = with_source_query(dvidUrl.getCoarseSupervoxelUrl(
          bodyId, m_dvidTarget.getBodyLabelName()));
    break;
  }

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutu::EReadStatus::OK) {
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
    uint64_t bodyId, neutu::EBodyLabelType labelType, const ZIntCuboid &box,
    ZObject3dScan *obj) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case neutu::EBodyLabelType::BODY:
    url = dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  case neutu::EBodyLabelType::SUPERVOXEL:
    url = dvidUrl.getCoarseSupervoxelUrl(
          bodyId, m_dvidTarget.getBodyLabelName());
    break;
  }

  url = with_source_query(ZDvidUrl::AppendRangeQuery(url, box));

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutu::EReadStatus::OK) {
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
  read_with_source(
        reader, dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName()), isVerbose());
  setStatusCode(reader.getStatusCode());

  ZObject3dScan obj;
  obj.importDvidObjectBuffer(
        reader.getBuffer().data(), reader.getBuffer().size());
  obj.setLabel(bodyId);

  clearBuffer();

  return obj;
}

ZObject3dScan ZDvidReader::readCoarseBody(
    uint64_t bodyId, neutu::EBodyLabelType labelType) const
{
  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);

  std::string url;
  switch (labelType) {
  case neutu::EBodyLabelType::BODY:
    url = with_source_query(dvidUrl.getCoarseSparsevolUrl(
                                bodyId, m_dvidTarget.getBodyLabelName()));
    break;
  case neutu::EBodyLabelType::SUPERVOXEL:
    url = with_source_query(dvidUrl.getCoarseSupervoxelUrl(
                                bodyId, m_dvidTarget.getBodyLabelName()));
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
  read_with_source(
        reader, dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName()), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutu::EReadStatus::OK) {
    count = ZObject3dScan::CountVoxelNumber(
          reader.getBuffer().data(), reader.getBuffer().size());
  }

  clearBuffer();

  return count;
}


int ZDvidReader::readCoarseBodySize(
    uint64_t bodyId, neutu::EBodyLabelType labelType) const
{
  int count = 0;

  ZDvidBufferReader &reader = m_bufferReader;
  reader.tryCompress(false);
  ZDvidUrl dvidUrl(m_dvidTarget);
  std::string url = with_source_query(
        dvidUrl.getCoarseSparsevolUrl(
          bodyId, m_dvidTarget.getBodyLabelName(), labelType));
//  if (labelType == neutu::EBodyLabelType::SUPERVOXEL) {
//    url = AppendQuery(url, std::make_pair(SUPERVOXEL_FLAG, true));
//  }

  reader.read(url.c_str(), isVerbose());
  setStatusCode(reader.getStatusCode());

  if (reader.getStatus() == neutu::EReadStatus::OK) {
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
  read_with_source(bufferReader, dvidUrl.getLocalBodyIdUrl(x, y, z), isVerbose());
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
    read_with_source(
          bufferReader, dvidUrl.getLocalSupervoxelIdUrl(x, y, z), isVerbose());
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
    ZJsonArray jsonArray = readJsonArray(with_source_query(urlStr));
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

#ifdef _DEBUG_0
    STD_COUT << "Payload: " << queryForm.toStdString() << std::endl;
#endif

    QByteArray payload;
    payload.append(queryForm);

    bufferReader.read(
          with_source_query(dvidUrl.getLocalBodyIdArrayUrl()).c_str(),
          payload, "GET", true);
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

  return readJsonArray(with_source_query(url.getAnnotationUrl(dataName, tag)));
}

ZJsonArray ZDvidReader::readAnnotation(
    const std::string &dataName, uint64_t label) const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonArray(with_source_query(url.getAnnotationUrl(dataName, label)));
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
  ZJsonArray obj = readJsonArray(with_source_query(dvidUrl.getBookmarkUrl(box)));
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
    ZJsonArray obj = readJsonArray(
          with_source_query(dvidUrl.getAnnotationUrl(dataName, box)));
    if (obj.size() > 0) {
      annotationJson.set(obj.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
    }
  }

  return annotationJson;
}

bool ZDvidReader::isBookmarkChecked(int x, int y, int z) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonObject obj = readJsonObject(
        with_source_query(dvidUrl.getBookmarkKeyUrl(x, y, z)));

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

  read_with_source(bufferReader, dvidUrl.getRoiUrl(dataName), isVerbose());
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

  read_with_source(bufferReader, dvidUrl.getRoiUrl(dataName), isVerbose());
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

/*
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
*/
bool ZDvidReader::hasBodyAnnotation() const
{
  return hasData(getDvidTarget().getBodyAnnotationName());
}

bool ZDvidReader::hasBodyAnnotation(uint64_t bodyId) const
{
  return hasKey(getDvidTarget().getBodyAnnotationName().c_str(),
                QString("%1").arg(bodyId));
}

ZJsonObject ZDvidReader::readBodyAnnotationJson(uint64_t bodyId) const
{
  ZDvidUrl url(getDvidTarget());

  return readJsonObject(
        with_source_query(url.getBodyAnnotationUrl(bodyId)).c_str());
}

std::vector<ZJsonObject> ZDvidReader::readBodyAnnotationJsons(
    const std::vector<uint64_t> &bodyIds) const
{
  std::vector<ZJsonObject> result;

  if (!bodyIds.empty()) {
    QStringList keyList;
    for (uint64_t bodyId : bodyIds) {
      keyList.append(QString::number(bodyId));
    }
    QList<QByteArray> data = readKeyValues(
          getDvidTarget().getBodyAnnotationName().c_str(), keyList);
    foreach (auto entry, data) {
      ZJsonObject obj;
      obj.decode(entry.data(), true);
      result.push_back(obj);
    }
  }

  return result;
}

ZJsonObject ZDvidReader::readBodyAnnotationSchema() const
{
  ZDvidUrl url(getDvidTarget());

  ZJsonObject schema = readJsonObject(
        with_source_query(url.getBodyAnnotationSchemaUrlOld()).c_str());
  if (schema.isEmpty()) {
    schema = readJsonObject(
          with_source_query(url.getBodyAnnotationSchemaUrl()).c_str());
  }

  return schema;
}

ZJsonObject ZDvidReader::readBodyAnnotationBatchSchema() const
{
  ZDvidUrl url(getDvidTarget());

  ZJsonObject schema =  readJsonObject(
        with_source_query(url.getBodyAnnotationBatchSchemaUrlOld()).c_str());
  if (schema.isEmpty()) {
    schema = readJsonObject(
          with_source_query(url.getBodyAnnotationBatchSchemaUrl()).c_str());
  }

  return schema;
}

namespace {

bool is_valid_buffer(const QByteArray &buffer)
{
  return (!buffer.isEmpty() &&
       std::strncmp(buffer.constData(), "null", buffer.size()));
}

ZJsonObject decode_json_object(const QByteArray &buffer) {
  ZJsonObject obj;
  if (is_valid_buffer(buffer)) {
    obj.decode(buffer.constData(), true);
  }

  return obj;
}

ZJsonArray decode_json_array(const QByteArray &buffer)
{
  ZJsonArray obj;
  if (is_valid_buffer(buffer)) {
    obj.decode(buffer.constData());
  }

  return obj;
}

}

ZJsonObject ZDvidReader::readJsonObject(const std::string &url) const
{
  ZJsonObject obj;

  if (!url.empty()) {
    ZDvidBufferReader &bufferReader = m_bufferReader;

    bufferReader.read(url.c_str(), isVerbose());
//    if (ZString(url).startsWith("http:") || ZString(url).startsWith("https:")) {

//    } else {

//      bufferReader.readFromPath(url.c_str(), isVerbose());
//    }
    setStatusCode(bufferReader.getStatusCode());
//    const QByteArray &buffer = bufferReader.getBuffer();
    if (bufferReader.getStatus() == neutu::EReadStatus::OK) {
      obj = decode_json_object(bufferReader.getBuffer());
      if (obj.shellOnly()) {
        obj.denull();
      }
    }
    /*
    if (!buffer.isEmpty()) {
      obj.decodeString(buffer.constData());
    }
    */
  }

  return obj;
}

ZJsonObject ZDvidReader::readJsonObjectFromKey(
    const QString &dataName, const QString &key) const
{
  ZJsonObject obj;
  const QByteArray &buffer = readKeyValue(dataName, key);

  obj = decode_json_object(buffer);

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
        }
        obj.denull();
        // note: object must be appened even if empty!  returned list elements
        //  must correspond to input key list
        objects.append(obj);
    }
    return objects;
}

ZJsonArray ZDvidReader::readJsonArray(const std::string &url) const
{
  ZJsonArray obj;

  ZDvidBufferReader &bufferReader = m_bufferReader;
  bufferReader.read(url.c_str(), isVerbose());
  obj = decode_json_array(bufferReader.getBuffer());
  /*
  const QByteArray &buffer = bufferReader.getBuffer();
  if (!buffer.isEmpty()) {
    obj.decode(buffer.constData());
  }
  */

  return obj;
}

ZJsonArray ZDvidReader::readJsonArray(
    const std::string &url, const QByteArray &payload) const
{
  ZJsonArray obj;

  ZDvidBufferReader &bufferReader = m_bufferReader;
  bufferReader.read(url.c_str(), payload, "GET", isVerbose());
  obj = decode_json_array(bufferReader.getBuffer());
  /*
  const QByteArray &buffer = bufferReader.getBuffer();
  if (!buffer.isEmpty()) {
    obj.decode(buffer.constData());
  }
  */

  return obj;
}


std::vector<ZIntPoint> ZDvidReader::readSynapsePosition(
    const ZIntCuboid &box) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(with_source_query(dvidUrl.getSynapseUrl(box)));

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
  ZJsonArray obj = readJsonArray(with_source_query(dvidUrl.getSynapseUrl(box)));
  if (obj.size() > 0) {
    synapseJson.set(obj.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
  }

  return synapseJson;
}

ZJsonArray ZDvidReader::readSynapseLabelsz(int n, dvid::ELabelIndexType index) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = readJsonArray(
        with_source_query(dvidUrl.getSynapseLabelszUrl(n, index)));

  return obj;
}

int ZDvidReader::readSynapseLabelszBody(
    uint64_t bodyId, dvid::ELabelIndexType index) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonObject obj = readJsonObject(
        with_source_query(dvidUrl.getSynapseLabelszBodyUrl(bodyId, index)));

  int count = 0;
  std::string key = ZDvidUrl::GetLabelszIndexTypeStr(index);
  if (obj.hasKey(key.c_str())) {
    count = ZJsonParser::integerValue(obj[key.c_str()]);
  }

  return count;
}

QList<int> ZDvidReader::readSynapseLabelszBodies(QList<uint64_t> bodyIDs, dvid::ELabelIndexType indexType)
{
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZDvidBufferReader &bufferReader = m_bufferReader;

    // the payload = bodies to get counts for
    QJsonArray bodies;
    foreach (uint64_t bodyID, bodyIDs) {
        bodies.append(QJsonValue((qint64) bodyID));
    }
    QJsonDocument doc(bodies);
    QByteArray payload = doc.toJson();
    bufferReader.read(QString::fromStdString(dvidUrl.getSynapseLabelszBodiesUrl(indexType)),
        payload,
        "GET",
        isVerbose());
    setStatusCode(bufferReader.getStatusCode());

    QList<int> counts;

    const QByteArray &buffer = m_bufferReader.getBuffer();
    if (!buffer.isEmpty()) {

        QJsonDocument doc = QJsonDocument::fromJson(buffer);
        QJsonArray array = doc.array();

        // return is a list of {"Label": label, "PreSyn": count}; not necessarily ordered!
        //  build a map, then build the output list in same order as input
        QMap<uint64_t, int> countMap;
        for (int i=0; i<array.size(); ++i) {
            QJsonObject obj = array.at(i).toObject();
            QVariant temp = obj["Label"].toVariant();
            bool ok = false;
            uint64_t bodyID = temp.toLongLong(&ok);
            if (!ok) {
                // error handling; I'm going to be a little sloppy here; there's no reason in
                //  the world that DVID would return me something that wouldn't parse,
                //  so just log it and skip it
                LWARN() << "error parsing bodyID " << temp.toString().toStdString();
                continue;
            }

            QString indexString = QString::fromStdString(ZDvidUrl::GetLabelszIndexTypeStr(indexType));
            countMap[bodyID] = obj[indexString].toInt();
            }
        foreach (uint64_t bodyID, bodyIDs) {
            counts << countMap[bodyID];
        }
      }
    m_bufferReader.clearBuffer();
    return counts;
}

ZJsonArray ZDvidReader::readSynapseLabelszThreshold(int threshold, dvid::ELabelIndexType index) const {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = readJsonArray(dvidUrl.getSynapseLabelszThresholdUrl(threshold, index));
    return obj;
}

ZJsonArray ZDvidReader::readSynapseLabelszThreshold(int threshold, dvid::ELabelIndexType index,
    int offset, int number) const {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = readJsonArray(dvidUrl.getSynapseLabelszThresholdUrl(threshold, index, offset, number));
    return obj;
}

#if 1
std::vector<ZDvidSynapse> ZDvidReader::readSynapse(
    const ZIntCuboid &box, dvid::EAnnotationLoadMode mode) const
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
    uint64_t label, dvid::EAnnotationLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  bool readingRelation = (mode != dvid::EAnnotationLoadMode::NO_PARTNER);
  ZJsonArray obj = readJsonArray(
        dvidUrl.getSynapseUrl(label, readingRelation));

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
    dvid::EAnnotationLoadMode mode) const
{
  ZDvidUrl dvidUrl(m_dvidTarget);

  ZJsonArray obj = readJsonArray(
        dvidUrl.getSynapseUrl(label, mode != dvid::EAnnotationLoadMode::NO_PARTNER));

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
    int x, int y, int z, dvid::EAnnotationLoadMode mode) const
{
  std::vector<ZDvidSynapse> synapseArray =
      readSynapse(ZIntCuboid(x, y, z, x, y, z), mode);
  if (!synapseArray.empty()) {
    if (synapseArray.size() > 1) {
      LWARN() << "Duplicated synapses at" << "(" << x << "" << y << "" << z << ")";
      synapseArray[0].setStatus(ZDvidAnnotation::EStatus::DUPLICATED);
    }
    return synapseArray[0];
  }

  return ZDvidSynapse();
}

ZDvidSynapse ZDvidReader::readSynapse(
    const ZIntPoint &pt, dvid::EAnnotationLoadMode mode) const
{
  return readSynapse(pt.getX(), pt.getY(), pt.getZ(), mode);
}
#endif

std::string ZDvidReader::readMirror() const
{
  std::string mirror;

  if (isReady()) {
    ZDvidUrl dvidUrl(getDvidTarget());
    std::string url = dvidUrl.getMirrorInfoUrl();
    m_bufferReader.read(with_source_query(url).c_str());

    mirror = GetMirrorAddressFromBuffer(m_bufferReader);
  }

  return mirror;
}

std::string ZDvidReader::GetMasterUrl(const ZDvidUrl &dvidUrl)
{
  std::string url = dvidUrl.getMasterUrl();
  if (!dvid::HasHead(url)) {
    url = dvidUrl.getOldMasterUrl();
  }
  LKINFO(neutu::TOPIC_NULL) << "Master url: " + url;

  return url;
}

/*
std::string ZDvidReader::readMasterNode() const
{
  std::string master;

  if (ReadMasterListBuffer(m_bufferReader, getDvidTarget())) {
    master = GetMasterNodeFromBuffer(m_bufferReader);
  }

  return master;
}
*/

std::vector<std::string> ZDvidReader::readMasterList() const
{
  std::vector<std::string> masterList;

  if (ReadMasterListBuffer(m_bufferReader, getDvidTarget())) {
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
  ZJsonObject obj = readDefaultDataSetting(EReadOption::CURRENT);
  loadDvidDataSetting(obj);
}

ZJsonObject ZDvidReader::readDefaultDataSettingCurrent() const
{
  ZJsonObject obj;

  ZDvidUrl url(getDvidTarget());

  obj = readJsonObject(with_source_query(url.getDefaultDataInstancesUrl()));

  return obj;
}

ZJsonObject ZDvidReader::readDefaultDataSettingTraceBack() const
{
  ZJsonObject obj;

  std::vector<std::string> uuidList = readMasterList();
  int index = -1;
  for (size_t i = 0; i < uuidList.size(); ++i) {
    const std::string &uuid = uuidList[i];
    if (dvid::IsUuidMatched(uuid, getDvidTarget().getUuid())) {
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
  case EReadOption::CURRENT:
    obj = readDefaultDataSettingCurrent();
    break;
  case EReadOption::TRACE_BACK:
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

bool ZDvidReader::ReadUserNodeBuffer(
    ZDvidBufferReader &reader, const ZDvidTarget &target)
{
  std::string uuid = target.getOriginalUuid();
  std::string alias;
  if (ZString(uuid).startsWith("@")) {
    alias = uuid.substr(1);
  }
  std::string rootNode = GET_FLYEM_CONFIG.getDvidRootNode(uuid);
  if (ZString(rootNode).startsWith("$")) {
    rootNode = rootNode.substr(1);
    ZDvidUrl dvidUrl(target, rootNode, true);
    std::string url = with_source_query(
          dvidUrl.getAliasBranchUrl(alias, neutu::GetCurrentUserName()));
    reader.read(url.c_str());
    if (reader.getStatus() == neutu::EReadStatus::OK) {
      return true;
    }
  }

  return false;
}

bool ZDvidReader::ReadMasterListBuffer(
    ZDvidBufferReader &reader, const ZDvidTarget &target)
{
  ZString uuid = target.getOriginalUuid();

  std::string rootNode = GET_FLYEM_CONFIG.getDvidRootNode(uuid);
  if (!rootNode.empty()) {
    bool usingOldUrl = false;
    if (uuid.startsWith("@@")) {
      usingOldUrl = true;
    }

    if (ZString(rootNode).startsWith("@")) {
      rootNode = rootNode.substr(1);
      usingOldUrl = true;
    }

    ZDvidUrl dvidUrl(target, rootNode, true);
    std::string url;

    if (!usingOldUrl) {
      url = with_source_query(dvidUrl.getMasterUrl());
      if (dvid::HasHead(url)) {
        reader.read(url.c_str());
        if (reader.getStatus() == neutu::EReadStatus::BAD_RESPONSE) {
          usingOldUrl = true;
        }
      } else {
        usingOldUrl = true;
      }
    }

    if (usingOldUrl) {
      url = dvidUrl.getOldMasterUrl();
      reader.read(with_source_query(url).c_str());
    }

    return reader.getStatus() == neutu::EReadStatus::OK;
  }

  return false;
}


std::string ZDvidReader::ReadMasterNode(const ZDvidTarget &target)
{
  std::string master;

  ZDvidBufferReader reader;

  if (ReadMasterListBuffer(reader, target)) {
    master = GetMasterNodeFromBuffer(reader);
  }

  return master;
}

std::string ZDvidReader::InferUuid(const ZDvidTarget &target)
{
  ZDvidBufferReader reader;
  std::string uuid;
  if (ReadUserNodeBuffer(reader, target)) {
    uuid = GetUserNodeFromBuffer(reader);
  } else if (ReadMasterListBuffer(reader, target)) {
    uuid = GetMasterNodeFromBuffer(reader);
  }

  return uuid;
}

std::string ZDvidReader::ReadUserNode(const ZDvidTarget &target)
{
  std::string uuid;

  ZDvidBufferReader reader;

  if (ReadUserNodeBuffer(reader, target)) {
    uuid = GetUserNodeFromBuffer(reader);
  }

  return uuid;
}

std::vector<std::string> ZDvidReader::ReadMasterList(const ZDvidTarget &target)
{
  std::vector<std::string> masterList;
  ZDvidBufferReader reader;

  if (ReadMasterListBuffer(reader, target)) {
    masterList = GetMasterListFromBuffer(reader);
  }

  return masterList;


  /*
#if defined(_FLYEM_)
  std::vector<std::string> masterList;
  std::string rootNode =
      GET_FLYEM_CONFIG.getDvidRootNode(target.getUuid());
  if (!rootNode.empty()) {
    ZDvidBufferReader reader;
    ZDvidUrl dvidUrl(target, rootNode);
    std::string url = GetMasterUrl(dvidUrl);
    if (ZString(target.getUuid()).startsWith("@@")) {
      url = dvidUrl.getOldMasterUrl();
    }
    reader.read(url.c_str());
    masterList = GetMasterListFromBuffer(reader);
  }

  return masterList;
#else
  return std::vector<std::string>();
#endif
*/
}

/*
std::vector<ZFlyEmToDoItem> ZDvidReader::readToDoItem(
    const ZIntCuboid &box) const
{
  ZDvidUrl dvidUrl(getDvidTarget());
  ZJsonArray obj = readJsonArray(dvidUrl.getTodoListUrl(box));

  std::vector<ZFlyEmToDoItem> itemArray(obj.size());

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject itemJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZFlyEmToDoItem &item = itemArray[i];
    item.loadJsonObject(itemJson, dvid::EAnnotationLoadMode::PARTNER_RELJSON);
  }

  return itemArray;
}
*/

/*
ZFlyEmToDoItem ZDvidReader::readToDoItem(int x, int y, int z) const
{
  std::vector<ZFlyEmToDoItem> itemArray =
      readToDoItem(ZIntCuboid(x, y, z, x, y, z));
  if (!itemArray.empty()) {
    return itemArray[0];
  }

  return ZFlyEmToDoItem();
}
*/

ZJsonObject ZDvidReader::readToDoItemJson(int x, int y, int z) const
{
  return readAnnotationJson(getDvidTarget().getTodoListName(), x, y, z);
}

ZJsonObject ZDvidReader::readToDoItemJson(const ZIntPoint &pt) const
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
  std::string dataName = ZDvidData::GetName(ZDvidData::ERole::SPLIT_TASK_KEY);
  QStringList keyList = readKeys(dataName.c_str(), "task__0", "task__z");
  foreach (const QString &key, keyList) {
    ZJsonObject obj = readJsonObjectFromKey(dataName.c_str(), key);
    if (obj.hasKey(neutu::json::REF_KEY)) {
      obj = readJsonObject(ZJsonParser::stringValue(obj[neutu::json::REF_KEY]));
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
  if (taskJson.hasKey(neutu::json::REF_KEY)) {
    taskJson = readJsonObject(
          ZJsonParser::stringValue(taskJson[neutu::json::REF_KEY]));
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
  std::string taskUrl = with_source_query(url.getTestTaskUrl(key));
  ZJsonObject taskJson = readJsonObject(taskUrl);

  return taskJson;
}


bool ZDvidReader::hasSplitTask(const QString &key) const
{
  return hasKey(ZDvidData::GetName(ZDvidData::ERole::SPLIT_TASK_KEY).c_str(), key);
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
