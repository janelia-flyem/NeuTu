#include "zbodysplitcommand.h"

#include <QUrl>
#include <QDateTime>

#define _NEUTU_USE_REF_KEY_

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

ZBodySplitCommand::ZBodySplitCommand()
{
}

ZDvidReader *ZBodySplitCommand::ParseInputPath(
    const std::string inputPath, ZJsonObject &inputJson, std::string &splitTaskKey,
    std::string &splitResultKey, std::string &dataDir, bool &isFile)
{
  QUrl inputUrl(inputPath.c_str());

  ZDvidReader *reader= NULL;
  if (inputUrl.scheme() == "dvid" || inputUrl.scheme() == "http") {
    reader = ZGlobal::GetInstance().getDvidReaderFromUrl(inputPath);
    inputJson = reader->readJsonObject(inputPath);
    if (inputJson.hasKey(NeuTube::Json::REF_KEY)) {
      inputJson =
          reader->readJsonObject(
            ZJsonParser::stringValue(inputJson[NeuTube::Json::REF_KEY]));
    }
    isFile = false;
    splitTaskKey = ZDvidUrl::ExtractSplitTaskKey(inputPath);
    splitResultKey = ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey);
  } else {
    inputJson.load(inputPath);
  }

  if (isFile) {
    dataDir = ZString(inputPath).dirPath();
  }

  return reader;
}

std::pair<ZStack*, ZSparseStack*>
ZBodySplitCommand::parseSignalPath(
    std::string &signalPath, const std::string &dataDir, bool isFile,
    const ZIntCuboid &range, ZStackGarbageCollector &gc)
{
  ZSparseStack *spStack = NULL;
  ZStack *signalStack = NULL;

  QUrl signalUrl(signalPath.c_str());
  if (signalUrl.scheme() == "http") { //Sparse stack
    ZDvidReader *reader =
        ZGlobal::GetInstance().getDvidReaderFromUrl(signalPath);
    if (reader != NULL) {
      m_bodyId = ZDvidUrl::GetBodyId(signalPath);
      ZDvidSparseStack *dvidStack =
          dvidStack = reader->readDvidSparseStack(m_bodyId);
      spStack = dvidStack->getSparseStack(range);
      gc.registerObject(dvidStack);
//      spStack = reader->readSparseStack(ZDvidUrl::GetBodyId(signalPath));
//      gc.registerObject(spStack);
    }
  } else {
    if (isFile) {
      signalPath = ZString(signalPath).absolutePath(dataDir);
    }

    if (ZFileType::FileType(signalPath) == ZFileType::FILE_SPARSE_STACK) {
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
    const ZJsonObject &/*config*/)
{
  int status = 1;


  const std::string &inputPath = input.front();

  std::string splitTaskKey;
  std::string splitResultKey;
  ZJsonObject inputJson;
  bool isFile = true;
  std::string dataDir;

  ZDvidReader *reader = ParseInputPath(
       inputPath, inputJson, splitTaskKey, splitResultKey, dataDir, isFile);

  if (!splitTaskKey.empty()) {
    if (!splitResultKey.empty()) {
      if (!forcingUpdate()) {
        if (ZServiceConsumer::HasSplitResult(reader, splitTaskKey.c_str())) {
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

  std::string signalPath = ZJsonParser::stringValue(inputJson["signal"]);
  std::cout << "Signal: " << signalPath << std::endl;

  ZIntCuboid range;

  if (inputJson.hasKey("range")) {
    ZJsonArray rangeJson(inputJson.value("range"));

    range.loadJson(rangeJson);
  }

  ZStackGarbageCollector gc;
  std::pair<ZStack*, ZSparseStack*> data =
      parseSignalPath(signalPath, dataDir, isFile, range, gc);
//  ZSparseStack *spStack = data.second;
//  ZStack *signalStack = data.first;


  ZStackWatershedContainer container(data);

  if (!container.isEmpty()) {
    if (!range.isEmpty()) {
      container.setRange(range);
    }

    LoadSeeds(inputJson, container, dataDir, isFile);
#ifdef _DEBUG_2
    container.exportMask(GET_TEST_DATA_DIR + "/test2.tif");
    container.exportSource(GET_TEST_DATA_DIR + "/test3.tif");
#endif

    container.run();
    ProcessResult(container, output, splitTaskKey);

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
//        ZStack *seedStack = obj.toStackObject(label);
//        seedMask.push_back(seedStack);
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
  if (m_bodyId > 0) {
    for (ZObject3dScan *obj : *objArray) {
      uint64_t newBodyId = writer.writeSplit(*obj, m_bodyId, 0);
      newBodyIdArray.push_back(newBodyId);
    }
  }
  return newBodyIdArray;
}

void ZBodySplitCommand::ProcessResult(
    ZStackWatershedContainer &container, const std::string &output,
    const std::string &splitTaskKey)
{
  ZStack *resultStack = container.getResultStack();
  if (resultStack != NULL) {
    QUrl outputUrl(output.c_str());
    ZObject3dScanArray *result = container.makeSplitResult(2, NULL);

    if (outputUrl.scheme() == "dvid" || outputUrl.scheme() == "http") {
      ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(output);
      ZJsonArray resultArray;

//        ZObject3dScanArray objArray;
//        ZObject3dFactory::MakeObject3dScanArray(
//              *result, NeuTube::Z_AXIS, true, &objArray);
      for (ZObject3dScanArray::const_iterator iter = result->begin();
           iter != result->end(); ++iter) {
        const ZObject3dScan &obj = **iter;
        std::string endPoint =
            writer->writeServiceResult("split", obj.toDvidPayload(), false);
        ZJsonObject regionJson;
        regionJson.setEntry("label", (int) obj.getLabel());
        regionJson.setEntry(NeuTube::Json::REF_KEY, endPoint);
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
              ZDvidData::GetName<QString>(ZDvidData::ROLE_SPLIT_GROUP),
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
          refJson.setEntry(NeuTube::Json::REF_KEY, endPoint);
        }
      } else {
        if (!splitTaskKey.empty()) {
          refJson.setEntry("message", "Split failed.");
        }
      }

      if (!refJson.isEmpty() && !refPath.isEmpty()) {
        writer->writeJson(refPath.toStdString(), refJson);
      }

//      if (!splitTaskKey.empty()) {
//          writer->deleteKey(ZDvidData::GetName(ZDvidData::ROLE_SPLIT_TASK_KEY),
//                            splitTaskKey);
//      }
    } else {
      ZStackWriter writer;
      writer.write(output, resultStack);
    }

//      delete result;
  } else {
    std::cout << "WARNING: Failed to produce result." << std::endl;
  }
}
