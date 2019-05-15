#include "zbodysplitcommand.h"

#include <QUrl>
#include <QDateTime>

#include "zjsondef.h"
#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "imgproc/zstackwatershed.h"
#include "zstackwriter.h"
#include "zstring.h"
#include "flyem/zstackwatershedcontainer.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "flyem/zserviceconsumer.h"
#include "zglobal.h"
#include "dvid/zdvidwriter.h"
#include "zobject3dfactory.h"
#include "zobject3dscanarray.h"
#include "zfiletype.h"
//#include "dvid/zdvidendpoint.h"
#include "dvid/zdvidpath.h"
#include "zstackgarbagecollector.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidurl.h"
#include "zswctree.h"
#include "logging/utilities.h"

ZBodySplitCommand::ZBodySplitCommand()
{
}

ZDvidReader *ZBodySplitCommand::ParseInputPath(
    const std::string &inputPath, ZJsonObject &inputJson, std::string &splitTaskKey,
    std::string &splitResultKey, std::string &dataDir, bool &isFile)
{
  QUrl inputUrl(inputPath.c_str());

  ZDvidReader *reader= NULL;
  if (inputUrl.scheme() == "dvid" || inputUrl.scheme() == "http") {
    reader = ZGlobal::GetInstance().getDvidReaderFromUrl(inputPath);
    inputJson = reader->readJsonObject(inputPath);
    if (inputJson.hasKey(neutu::json::REF_KEY)) {
      inputJson =
          reader->readJsonObject(
            ZJsonParser::stringValue(inputJson[neutu::json::REF_KEY]));
    }
    isFile = false;
    splitTaskKey = ZDvidUrl::ExtractSplitTaskKey(inputPath);
    splitResultKey = ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey);
  } else {
    if (ZFileType::FileType(inputPath) == ZFileType::EFileType::JSON) {
      inputJson.load(inputPath);
    }
  }

  if (isFile) {
    dataDir = ZString(inputPath).dirPath();
  }

  return reader;
}

std::pair<ZStack*, ZSparseStack*>
ZBodySplitCommand::parseSignalPath(
    std::string &signalPath, const ZJsonObject &signalInfo,
    const std::string &dataDir, bool isFile,
    const ZIntCuboid &/*range*/, ZStackGarbageCollector &gc)
{
  ZSparseStack *spStack = NULL;
  ZStack *signalStack = NULL;

  QUrl signalUrl(signalPath.c_str());
  if (signalUrl.scheme() == "http") { //Sparse stack
    m_bodyId = ZDvidUrl::GetBodyId(signalPath);
    if (m_bodyId > 0) {
      ZDvidReader reader;
      ZDvidTarget target;

      target.setFromUrl(signalPath);
      if (!signalInfo.isEmpty()) {
        if (signalInfo.hasKey("address")) {
          target.setServer(ZJsonParser::stringValue(signalInfo["address"]));
        }
        if (signalInfo.hasKey("port")) {
          target.setPort(ZJsonParser::integerValue(signalInfo["port"]));
        }

        target.updateData(signalInfo);
      }

      reader.open(target);
      if (reader.isReady()) {
        size_t blockCount = reader.readCoarseBodySize(m_bodyId);
        if (blockCount < 50000000) {
          std::cout << "Block count: " << blockCount << std::endl;

          /*
          ZDvidSparseStack *dvidStack =
              reader.readDvidSparseStack(m_bodyId, m_labelType);
          spStack = dvidStack->getSparseStack(range);
          gc.registerObject(dvidStack);
          */
          spStack = reader.readSparseStackOnDemand(m_bodyId, m_labelType, NULL);
        } else {
          LINFO() << m_bodyId << "ignored.";
        }
      }
    }
  } else {
    if (isFile) {
      signalPath = ZString(signalPath).absolutePath(dataDir);
    }

    if (ZFileType::FileType(signalPath) == ZFileType::EFileType::SPARSE_STACK) {
      spStack = new ZSparseStack;
      spStack->load(signalPath);
      gc.registerObject(spStack);
    } else {
      signalStack = new ZStack;
      signalStack->load(signalPath);
      gc.registerObject(signalStack);
    }
  }

  return std::pair<ZStack*, ZSparseStack*>(signalStack, spStack);
}

int ZBodySplitCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  int status = 1;

  if (output.empty()) {
    std::cout << "No output is spedified. Abort." << std::endl;
    return status;
  }

  const std::string &inputPath = input.front();

  std::string splitTaskKey;
  std::string splitResultKey;
  ZJsonObject inputJson;
  bool isFile = true;
  std::string dataDir;

  int seedIntv = 0;

  if (config.hasKey("seed_scale")) {
    seedIntv = ZJsonParser::integerValue(config["seed_scale"]) - 1;
    if (seedIntv < 0) {
      seedIntv = 0;
    }
  }

  bool preservingGap = true;
  if (config.hasKey("preserving_gap")) {
    preservingGap = ZJsonParser::booleanValue(config["preserving_gap"]);
  }

  ZDvidReader *reader = ParseInputPath(
       inputPath, inputJson, splitTaskKey, splitResultKey, dataDir, isFile);

  if (!splitTaskKey.empty()) {
    if (!splitResultKey.empty()) {
      if (!forcingUpdate()) {
        if (ZServiceConsumer::HasNonemptySplitResult(reader, splitTaskKey.c_str())) {
          std::cout << "The task has already been processed. Please find the result @"
                    << splitResultKey << "."
                    << std::endl;
          return 0;
        }
      }
    } else {
      std::cout << "Invalid task key: " << splitTaskKey << "."
                << std::endl;
      return 1;
    }
  }

  bool commiting = false;
  bool testing = false;

  if (config.hasKey("commit")) {
    commiting = ZJsonParser::booleanValue(config["commit"]);
  }

  std::string signalPath = ZJsonParser::stringValue(inputJson["signal"]);
  std::cout << "Signal: " << signalPath << std::endl;

  std::string commitPath = signalPath;
  if (config.hasKey("commit_path")) {
    commitPath = ZJsonParser::stringValue(config["commit_path"]);
  }

  if (config.hasKey("supervoxel")) {
    if (ZJsonParser::booleanValue(config["supervoxel"]) == true) {
      m_labelType = neutu::EBodyLabelType::SUPERVOXEL;
    }
  }

  ZJsonObject signalInfo;
  const char *signalInfoKey = "signal info";
  if (config.hasKey(signalInfoKey)) {
    signalInfo.set(config.value(signalInfoKey));
  } else {
    if (inputJson.hasKey(signalInfoKey)) {
      signalInfo.set(inputJson.value(signalInfoKey));
    }
  }

  ZIntCuboid range;

  if (inputJson.hasKey("range")) {
    ZJsonArray rangeJson(inputJson.value("range"));

    range.loadJson(rangeJson);
  }

  ZStackGarbageCollector gc;
  std::pair<ZStack*, ZSparseStack*> data =
      parseSignalPath(signalPath, signalInfo, dataDir, isFile, range, gc);
#ifdef _DEBUG_2
  ZSparseStack *spStack = data.second;
  if (spStack != NULL) {
    std::cout << "Saving sparse stack ..." << std::endl;
    spStack->save(GET_TEST_DATA_DIR + "/test.zss");
//    spStack->getObjectMask()->save(GET_TEST_DATA_DIR + "/test.sobj");
  }
#endif

//  ZStack *signalStack = data.first;

  if (data.first == NULL && data.second == NULL) {
    return 1;
  }


  ZStackWatershedContainer container(data);
  container.setProfileLogger(neutu::LogProfileInfo);

  container.setRefiningBorder(true);
  container.setCcaPost(true);
  container.setPreservingGap(preservingGap);

  if (!container.isEmpty()) {
    if (!range.isEmpty()) {
      container.setRange(range);
    }

    LoadSeeds(inputJson, container, dataDir, isFile);

    if (seedIntv > 0) {
      container.downsampleSeed(seedIntv, seedIntv, seedIntv);
    }
#ifdef _DEBUG_2
    container.exportMask(GET_TEST_DATA_DIR + "/test2.tif");
    container.exportSource(GET_TEST_DATA_DIR + "/test3.tif");
#endif

    container.run();

    if (testing) {
      ZObject3dScanArray result;
      container.makeSplitResult(1, &result, NULL);
      ZStack *labelStack = result.toColorField();
      labelStack->save(output);
      delete labelStack;
    } else {
      processResult(
            container, output, splitTaskKey, signalPath, commiting, commitPath);
    }

#ifdef _DEBUG_2
    resultStack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

//    ZStackWatershed watershed;
//    watershed.setFloodingZero(false);

//    ZStack *result = watershed.run(&signalStack, seedMask);


    status = 0;
  }

  return status;
}

void ZBodySplitCommand::LoadSeeds(
    const ZJsonObject &inputJson, ZStackWatershedContainer &container,
    const std::string &dataDir, bool isFile)
{
  ZJsonArray seedArrayJson(inputJson.value("seeds"));
  for (size_t i = 0; i < seedArrayJson.size(); ++i) {
    ZJsonObject seedJson(seedArrayJson.value(i));
    if (seedJson.hasKey("type")) {
      std::string seedUrl = ZJsonParser::stringValue(seedJson["url"]);
      if (isFile) {
        seedUrl = ZString(seedUrl).absolutePath(dataDir);
      }

      std::string type = ZJsonParser::stringValue(seedJson["type"]);
      if (type == "ZObject3dScan" && !seedUrl.empty() && seedJson.hasKey("label")) {
        int label = ZJsonParser::integerValue(seedJson["label"]);
        ZObject3dScan obj;
        obj.setLabel(label);
        QUrl url(seedUrl.c_str());
        if (url.scheme() == "http") {
          QByteArray data = ZServiceConsumer::ReadData(seedUrl.c_str());
          obj.importDvidObjectBuffer(data.data(), data.length());
        } else {
          obj.load(seedUrl);
        }
        container.addSeed(obj);
      } else if (type == "ZStroke2d") {
        ZStroke2d stroke;
        stroke.loadJsonObject(ZJsonObject(seedJson.value("data")));
        container.addSeed(stroke);
      } else if (type == "ZObject3d") {
        ZObject3d obj;
        obj.loadJsonObject(ZJsonObject(seedJson.value("data")));
        container.addSeed(obj);
      } else if (type == "swc" || type == "SWC") {
        ZSwcTree tree;
        if (isFile) {
          tree.load(seedUrl);
        } else {
          QByteArray data = ZServiceConsumer::ReadData(seedUrl.c_str());
          if (!data.isEmpty()) {
            tree.loadFromBuffer(data.constData());
          }
        }
        container.addSeed(tree);
      }
    } else if (seedJson.hasKey("stroke")) {
      ZStroke2d stroke;
      stroke.loadJsonObject(seedJson);
      container.addSeed(stroke);
    } else if (seedJson.hasKey("obj3d")) {
      ZObject3d obj;
      obj.loadJsonObject(seedJson);
      container.addSeed(obj);
    }
  }
}

std::vector<uint64_t> ZBodySplitCommand::commitResult(
    ZObject3dScanArray *objArray, ZDvidWriter &writer)
{
  std::vector<uint64_t> newBodyIdArray;
  uint64_t currentBodyId = m_bodyId;
  if (currentBodyId > 0) {
    for (ZObject3dScan *obj : *objArray) {
      if (m_labelType == neutu::EBodyLabelType::BODY) {
        uint64_t newBodyId = writer.writeSplit(*obj, currentBodyId, 0);
        newBodyIdArray.push_back(newBodyId);
      } else {
        std::cout << "Splitting supervoxel: " << currentBodyId << std::endl;
        std::pair<uint64_t, uint64_t> idPair = writer.writeSupervoxelSplit(
              *obj, currentBodyId);
        if (currentBodyId != idPair.first) { //The current id is gone
          bool splitRecorded = false;
          if (!newBodyIdArray.empty()) { //Replace last id if it's invalid
            if (currentBodyId == newBodyIdArray.back()) {
              std::cout << "Overwrite remainder: " << currentBodyId << std::endl;
              newBodyIdArray.back() = idPair.second;
              newBodyIdArray.push_back(idPair.first);
              splitRecorded = true;
            }
          }
          if (!splitRecorded) {
            newBodyIdArray.push_back(idPair.second);
            newBodyIdArray.push_back(idPair.first);
          }
          currentBodyId = idPair.first;
        }
        std::cout << "New IDs: ";
        for (uint64_t id : newBodyIdArray) {
          std::cout << id << " ";
        }
        std::cout << std::endl;
      }
    }
  }
  return newBodyIdArray;
}

void ZBodySplitCommand::processResult(
    ZStackWatershedContainer &container, const std::string &output,
    const std::string &splitTaskKey, const std::string &/*signalPath*/,
    bool committing, const std::string &commitPath)
{
//  ZStack *resultStack = container.getResultStack();
  std::cout << "Processing results ..." << std::endl;
  if (container.hasResult()) {
    QUrl outputUrl(output.c_str());
    ZObject3dScanArray *result = container.makeSplitResult(2, NULL, NULL);

    if (outputUrl.scheme() == "dvid" || outputUrl.scheme() == "http") {
      ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(output);
      ZJsonArray resultArray;

      if (committing) {
        std::cout << "Commit path: " << commitPath << std::endl;
        ZDvidWriter *bodyWriter =
            ZGlobal::GetInstance().getDvidWriterFromUrl(commitPath);
        std::vector<uint64_t> bodyIdArray = commitResult(result, *bodyWriter);
        ZJsonArray resultArray;
        for (uint64_t bodyId : bodyIdArray) {
          resultArray.append(bodyId);
        }
        ZJsonObject resultJson;
        resultJson.setEntry("committed", resultArray);
        resultJson.setEntry(
              "timestamp", (int64_t)(QDateTime::currentMSecsSinceEpoch() / 1000));
        if (!splitTaskKey.empty()) {
          QString refPath = ZDvidPath::GetResultKeyPath(
                ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_GROUP),
                ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey).c_str());
          std::cout << "Writing result summary to " << refPath.toStdString()
                    << std::endl;
          writer->writeJson(refPath.toStdString(), resultJson);
        } else {
          LINFO() << resultJson.dumpString(0);
        }
      } else {
        for (ZObject3dScanArray::const_iterator iter = result->begin();
             iter != result->end(); ++iter) {
          const ZObject3dScan &obj = **iter;
          std::string endPoint =
              writer->writeServiceResult("split", obj.toDvidPayload(), false);
          ZJsonObject regionJson;
          regionJson.setEntry("label", (int) obj.getLabel());
          regionJson.setEntry(neutu::json::REF_KEY, endPoint);
          resultArray.append(regionJson);
#ifdef _DEBUG_2
          obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif
        }

        ZJsonObject resultJson;
        QString refPath;
        ZJsonObject refJson;

        if (!splitTaskKey.empty()) {
          refPath = ZDvidPath::GetResultKeyPath(
                ZDvidData::GetName<QString>(ZDvidData::ERole::SPLIT_GROUP),
                ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey).c_str());
          refJson.setEntry(
                "timestamp", (int64_t)(QDateTime::currentMSecsSinceEpoch() / 1000));
        }

        if (!resultArray.isEmpty()) {
          resultJson.addEntry("type", "split");
          resultJson.addEntry("result", resultArray);
          std::string endPoint =
              writer->writeServiceResult("split", resultJson);
          std::cout << "Result endpoint: " << endPoint << std::endl;

          if (!splitTaskKey.empty()) {
            refJson.setEntry(neutu::json::REF_KEY, endPoint);
          }
        } else {
          if (!splitTaskKey.empty()) {
            refJson.setEntry("message", "Split failed.");
          }
        }

        if (!refJson.isEmpty() && !refPath.isEmpty()) {
          std::cout << "Writing results to " << refPath.toStdString() << std::endl;
          writer->writeJson(refPath.toStdString(), refJson);
        }
      }

//      if (!splitTaskKey.empty()) {
//          writer->deleteKey(ZDvidData::GetName(ZDvidData::ERole::ROLE_SPLIT_TASK_KEY),
//                            splitTaskKey);
//      }
    } else {
      ZStackWriter writer;
      ZObject3dScanArray result;
      container.makeSplitResult(1, &result, NULL);

      if (ZFileType::isImageFile(output)) {
        ZStack *labelStack = result.toColorField();

        writer.write(output, labelStack);
        delete labelStack;
      } else {
        for (ZObject3dScan *obj : result) {
          obj->setColor(ZStroke2d::GetLabelColor(obj->getLabel()));
        }
        result.save(output);
      }
      std::cout << "Result saved as " << output<< std::endl;
    }

//      delete result;
  } else {
    std::cout << "WARNING: Failed to produce result." << std::endl;
  }
}
