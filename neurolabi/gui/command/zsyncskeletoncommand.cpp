#include "zsyncskeletoncommand.h"

#include <memory>
#include <fstream>
#include <algorithm>
#include <random>

#include <QDebug>
#include <QUrl>

#include "neulib/core/stringbuilder.h"

#include "zstring.h"
//#include "common/zstringbuilder.h"

#include "zjsonobjectparser.h"
#include "zswctree.h"
#include "filesystem/utilities.h"
#include "command/utilities.h"
#include "filesystem/utilities.h"

#include "zdvidutil.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"

#include "neutuse/taskwriter.h"
#include "neutuse/taskfactory.h"
#include "neutuse/task.h"

#include "flyem/flyemdatareader.h"
#include "flyem/zflyembodyannotation.h"
#include "zflyemutilities.h"
#include "service/neuprintreader.h"
#include "zjsonobjectparser.h"

ZSyncSkeletonCommand::ZSyncSkeletonCommand()
{

}

namespace {

bool passed(
    const ZFlyEmBodyAnnotation &annot, const std::set<std::string>& statusSet)
{
  const std::string status = ZString(annot.getStatus()).lower();

  if (statusSet.empty()) {
    return true;
  } else {
    return statusSet.count(status) > 0;
  }
}

NeuPrintReader* get_neuprint_reader(const ZJsonObject &obj, const QString &uuid)
{
  NeuPrintReader *reader = nullptr;

  ZJsonObjectParser parser;
  QString url = parser.GetValue(obj, "url", "").c_str();
  QString token = parser.GetValue(obj, "token", "").c_str();

  if (!url.isEmpty() && !uuid.isEmpty()) {
    reader = new NeuPrintReader(url);
    reader->authorize(token);
    reader->updateCurrentDatasetFromUuid(uuid);
    if (!reader->isReady()) {
      delete reader;
      reader = nullptr;
    }
  }

  return reader;
}

void report_progress(size_t index, size_t count)
{
  if (index % 1000 == 0) {
    std::cout << "Checking " << index + 1
              << "/" << count << "..." << std::endl;
  }
}

void process_body(uint64_t bodyId, int index, int totalCount,
                  const ZDvidReader &reader,
                  std::function<void(uint64_t)> processBody)
{
  ZNetBufferReader bufferReader;
  ZDvidUrl dvidUrl(reader.getDvidTarget());
  bufferReader.hasHead(dvidUrl.getSparsevolUrl(bodyId).c_str());
  bool hasBody = false;
  if (bufferReader.getStatusCode() == 200) {
    hasBody = true;
  } else if (bufferReader.getStatusCode() != 204) {
    hasBody = reader.hasBody(bodyId);
  }

  if (hasBody) {
    int64_t bodyMod = reader.readBodyMutationId(bodyId);
    std::unique_ptr<ZSwcTree> tree =
        std::unique_ptr<ZSwcTree>(reader.readSwc(bodyId));
    bool syncing = false;
    std::string reason;
    if (!tree) {
      reason = "no skeleton";
      syncing = true;
    } else if (bodyMod >= 0) { //mutation ID must be available
      int64_t swcMod = flyem::GetMutationId(tree.get());
      if (bodyMod != swcMod) {
        reason = neulib::StringBuilder("").
            append(swcMod).append("->").append(bodyMod);
        syncing = true;
      }
    }

    if (syncing) {
      std::cout << "To sync [" << index << "/" << totalCount
                << "]: " << bodyId
                << " (" << reason << ")" << std::endl;
      processBody(bodyId);
    }
  } else {
    std::cout << bodyId << " does not exist." << std::endl;
  }
}

std::set<std::string> get_status_set(const ZJsonObject &config)
{
  std::set<std::string> statusSet;
  ZJsonArray statusJson(config.value("bodyStatus"));
  for (size_t i = 0; i < statusJson.size(); ++i) {
    std::string status =
        ZString(ZJsonParser::stringValue(statusJson.at(i))).lower();
    if (!status.empty()) {
      statusSet.insert(status);
    }
  }

  return statusSet;
}

QStringList get_status_list(const ZJsonObject &config)
{
  std::set<std::string> statusSet = get_status_set(config);
  QStringList statusList;
  for (const std::string &status : statusSet) {
    statusList.append(status.c_str());
  }

  return statusList;
}

std::set<uint64_t> get_body_set_from_annotation(
    const ZJsonObject &config, const ZDvidReader &reader)
{
  std::set<uint64_t> bodySet;
  QStringList annotList =
      reader.readKeys(reader.getDvidTarget().getBodyAnnotationName().c_str());
  auto statusSet = get_status_set(config);
  foreach (const QString &bodyStr, annotList) {
    uint64_t bodyId = ZString(bodyStr.toStdString()).firstUint64();
    if (bodyId > 0) {
      ZFlyEmBodyAnnotation annot =
          FlyEmDataReader::ReadBodyAnnotation(reader, bodyId);
      if (passed(annot, statusSet)) {
        bodySet.insert(bodyId);
//        process_body(bodyId, index, annotList.size(), reader, processBody);
      }
    }
  }

  return bodySet;
}


std::set<uint64_t> get_body_set_from_neuprint(
    const ZJsonObject &config, const ZDvidReader &reader)
{
  std::set<uint64_t> bodySet;
  if (config.hasKey("neuprint")) {
    NeuPrintReader *neuprintReader;
    ZJsonObject neuprintObj(config.value("neuprint"));
    std::string uuid = reader.getDvidTarget().getUuid();
    if (neuprintObj.hasKey("uuid")) {
      uuid = ZJsonParser::stringValue(neuprintObj["uuid"]);
    }
    neuprintReader = get_neuprint_reader(neuprintObj, uuid.c_str());
    if (neuprintReader) {
      ZJsonArray predefinedBodyList =
          neuprintReader->queryNeuronByStatus(get_status_list(config));
      for (size_t i = 0; i < predefinedBodyList.size(); ++i) {
        ZJsonObject bodyJson(predefinedBodyList.value(i));
        uint64_t bodyId = ZJsonParser::integerValue(bodyJson["body ID"]);
        bodySet.insert(bodyId);
      }
    } else {
      qWarning() << "Failed to load dataset from neuprint:"
                 << neuprintObj.dumpString(0).c_str();
    }
  }

  return bodySet;
}


std::set<uint64_t> get_body_set(
    const ZJsonObject &config, const ZDvidReader &reader)
{
  std::set<uint64_t> bodySet;
  if (config.hasKey("bodyStatus")) { //Get body set by status filter
    if (config.hasKey("neuprint")) {
      bodySet = get_body_set_from_neuprint(config, reader);
    } else {
      bodySet = get_body_set_from_annotation(config, reader);
    }
  } else if (config.hasKey("bodyList")) {
    if (ZJsonParser::IsArray(config["bodyList"])) {
      auto bodyList = ZJsonParser::integerArray(config["bodyList"]);
      bodySet.insert(bodyList.begin(), bodyList.end());
    } else {
      std::string bodyFile = ZJsonObjectParser::GetValue(config, "bodyList", "");
      if (!bodyFile.empty()) {
        std::string sourcePath = ZJsonObjectParser::GetValue(config, "_source", "");
        if (!sourcePath.empty()) {
          bodyFile = neutu::Absolute(bodyFile, neutu::ParentPath(sourcePath));
        }
        std::cout << "Getting body list from " << bodyFile << "..." << std::endl;
        auto bodyList = neutu::ImportBodies(bodyFile);
        bodySet.insert(bodyList.begin(), bodyList.end());
      }
    }
  } else { //Get body set by lowres labels
    int zoom = reader.getMaxLabelZoom();
    ZDvidInfo info = reader.readDataInfo(
          reader.getDvidTarget().getSegmentationName());
    ZIntPoint startPoint = info.getStartCoordinates();
    ZIntPoint endPoint = info.getEndCoordinates();
    if (config.hasKey("minLowresVoxelCount")) {
      bodySet = reader.readBodyId(
            ZIntCuboid(startPoint, endPoint), zoom,
            ZJsonParser::integerValue(config["minLowresVoxelCount"]), true);
    } else {
      bodySet = reader.readBodyId(
            ZIntCuboid(startPoint, endPoint), zoom);
    }
  }

  if (config.hasKey("minBodySize")) {
    size_t minBodySize = ZJsonParser::integerValue(config["minBodySize"]);
    std::set<uint64_t> oldBodySet = bodySet;;
    bodySet.clear();
    for (uint64_t body : oldBodySet) {
      size_t voxelCount;
      size_t blockCount;
      ZIntCuboid range;
      std::tie(voxelCount, blockCount, range) = reader.readBodySizeInfo(
            body, neutu::EBodyLabelType::BODY);
//      std::cout << "Size: " << voxelCount << " " << blockCount << std::endl;
      if (voxelCount >= minBodySize) {
        bodySet.insert(body);
      }
    }
  }

  return bodySet;
}

}

int ZSyncSkeletonCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  if(input.empty() || output.empty()) {
    return 1;
  }

  neutuse::TaskWriter taskWriter;
  std::ofstream outStream;

  ZJsonObjectParser parser;

  QUrl outputUrl(output.c_str());

  ZDvidTarget target = dvid::MakeTargetFromUrlSpec(input[0]);
//  target.setFromSourceString(input[0]);

  std::function<void(uint64_t)> processBody;
  neutuse::TaskFactory taskFactory;
  taskFactory.setForceUpdate(true);
  taskFactory.setPriority(parser.GetValue(config, "priority", 5));

  if (outputUrl.scheme() == "http") {
    std::string neutuseServer = output;
    if (neutuseServer.empty()) {
      qWarning() << "Neutuse server is not specified. Abort!";
      return 1;
    } else {
      taskWriter.open(neutuseServer);
    }

    if (!taskWriter.ready()) {
      qWarning() << "Cannot connect to neutuse: " << neutuseServer.c_str()
                 << ". Abort!";
      return 1;
    }

    processBody = [&](uint64_t bodyId) {
      taskWriter.uploadTask(
            taskFactory.makeDvidSkeletonizeTask(target, bodyId));
    };

  } else if (outputUrl.scheme() == "file") {
    outStream.open(outputUrl.path().toStdString());
    if (!outStream.good()) {
      qWarning() << "Cannot open output file: " << outputUrl.path() << ". Abort!";
      return 1;
    }

    processBody = [&](uint64_t bodyId) {
      outStream << bodyId << std::endl;
    };
  } else {
    qWarning() << "Invalid output: " << output.c_str();
    qWarning() << "It must be a valid URL with http or file scheme.";
    qWarning() << "Abort!";
    return 1;
  }


  ZDvidReader reader;
  if (reader.open(target)) {
    reader.updateMaxLabelZoom();
    target = reader.getDvidTarget();
    reader.setVerbose(false);
    if (reader.hasData(reader.getDvidTarget().getSkeletonName())) {
      std::set<uint64_t> bodySet = get_body_set(config, reader);
      std::cout << "Checking " << bodySet.size() << " bodies..." << std::endl;
      size_t i = 1;
      for (uint64_t bodyId : bodySet) {
        report_progress(i, bodySet.size());
        process_body(
              bodyId, i++, bodySet.size(), reader, processBody);
      }

#if 0
      std::set<std::string> statusSet;
      if (config.hasKey("bodyStatus")) {
        ZJsonArray statusJson(config.value("bodyStatus"));
        for (size_t i = 0; i < statusJson.size(); ++i) {
          std::string status =
              ZString(ZJsonParser::stringValue(statusJson.at(i))).lower();
          if (!status.empty()) {
            statusSet.insert(status);
          }
        }
      }

      QStringList statusList;
      for (const std::string &status : statusSet) {
        statusList.append(status.c_str());
      }

      ZJsonArray predefinedBodyList;

      if (config.hasKey("neuprint")) {
        NeuPrintReader *neuprintReader;
        ZJsonObject neuprintObj(config.value("neuprint"));
        std::string uuid = reader.getDvidTarget().getUuid();
        if (neuprintObj.hasKey("uuid")) {
          uuid = ZJsonParser::stringValue(neuprintObj["uuid"]);
        }
        neuprintReader = get_neuprint_reader(neuprintObj, uuid.c_str());
        if (neuprintReader) {
          predefinedBodyList = neuprintReader->queryNeuronByStatus(statusList);
        } else {
          qWarning() << "Failed to load dataset from neuprint:"
                     << neuprintObj.dumpString(0).c_str();
        }
      }

      if (config.hasKey("bodyList")) {
        if (ZJsonValue(config.value("bodyList")).isArray()) {
          ZJsonArray bodyArray(config.value("bodyList"));
          for (size_t i = 0; i < bodyArray.size(); ++i) {
            uint64_t bodyId = ZJsonParser::integerValue(bodyArray.at(i));
            ZJsonObject bodyJson;
            bodyJson.setEntry("body ID", bodyId);
            predefinedBodyList.append(bodyJson);
          }
        } else {
          std::string sourceFile = ZJsonParser::stringValue(config["_source"]);
          std::string bodyFilePath =
              neutu::Absolute(
                ZJsonParser::stringValue(config["bodyList"]),
                neutu::ParentPath(sourceFile));

          std::ifstream stream(bodyFilePath);
          if (stream.is_open()) {
            ZString line;
            while (getline(stream, line)) {
              auto bodyIdArray = line.toUint64Array();
              for (uint64_t bodyId : bodyIdArray) {
                ZJsonObject bodyJson;
                bodyJson.setEntry("body ID", bodyId);
                predefinedBodyList.append(bodyJson);
              }
            }
          }
        }
      }

      bool shuffling = ZJsonObjectParser::GetValue(config, "shuffle", false);
      if (!predefinedBodyList.isEmpty()) {
        std::vector<size_t> m_indexArray(predefinedBodyList.size());
        for (size_t i = 0; i < predefinedBodyList.size(); ++i) {
          m_indexArray[i] = i;
        }
        if (shuffling) {
          std::random_device rd;
          std::mt19937 g(rd());
          std::shuffle(m_indexArray.begin(), m_indexArray.end(), g);
        }
        for (size_t i = 0; i < predefinedBodyList.size(); ++i) {
          size_t index = m_indexArray[i];
          ZJsonObject bodyJson(predefinedBodyList.value(index));
          uint64_t bodyId = ZJsonParser::integerValue(bodyJson["body ID"]);
          report_progress(i, predefinedBodyList.size());
          process_body(
                bodyId, i + 1, predefinedBodyList.size(), reader, processBody);
        }
      } else {
        QStringList annotList =
            reader.readKeys(reader.getDvidTarget().getBodyAnnotationName().c_str());
        int index = 1;
        if (shuffling) {
          std::random_device rd;
          std::mt19937 g(rd());
          std::shuffle(annotList.begin(), annotList.end(), g);
        }
        foreach (const QString &bodyStr, annotList) {
          report_progress(index - 1, annotList.size());
          uint64_t bodyId = ZString(bodyStr.toStdString()).firstUint64();
          if (bodyId > 0) {
            ZFlyEmBodyAnnotation annot =
                FlyEmDataReader::ReadBodyAnnotation(reader, bodyId);
            if (passed(annot, statusSet)) {
              process_body(bodyId, index, annotList.size(), reader, processBody);
            }
          }
          index++;
        }
      }
#endif
    } else {
      qWarning() << "Skeleton data store does not exist. Abort!";
      return 1;
    }
  }

  return 0;
}
