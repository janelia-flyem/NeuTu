#include "zbodysplitcommand.h"

#include <QUrl>
#include <QDateTime>

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
#include "dvid/zdvidendpoint.h"

ZBodySplitCommand::ZBodySplitCommand()
{
}

int ZBodySplitCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &/*config*/)
{
  int status = 1;

  ZJsonObject inputJson;

  const std::string &inputPath = input.front();

  QUrl inputUrl(inputPath.c_str());

  std::string splitTaskKey;
  std::string splitResultKey;
  bool isFile = true;
  ZDvidReader *reader= NULL;
  if (inputUrl.scheme() == "dvid" || inputUrl.scheme() == "http") {
    reader = ZGlobal::GetInstance().getDvidReaderFromUrl(input.front());
    inputJson = reader->readJsonObject(input.front());
    if (inputJson.hasKey(NeuTube::Json::REF_KEY)) {
      inputJson =
          reader->readJsonObject(
            ZJsonParser::stringValue(inputJson[NeuTube::Json::REF_KEY]));
    }
    isFile = false;
    splitTaskKey = ZDvidUrl::ExtractSplitTaskKey(inputPath);
    splitResultKey = ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey);
  } else {
    inputJson.load(input.front());
  }

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

  ZSparseStack *spStack = NULL;
  ZStack *signalStack = NULL;

  std::string signalPath = ZJsonParser::stringValue(inputJson["signal"]);

  std::string dataDir;
  if (isFile) {
    dataDir = ZString(input.front()).dirPath();
  }

  QUrl signalUrl(signalPath.c_str());
  if (signalUrl.scheme() == "http") { //Sparse stack
    ZDvidReader *reader = ZGlobal::GetInstance().getDvidReaderFromUrl(signalPath);
    if (reader != NULL) {
      spStack = reader->readSparseStack(ZDvidUrl::GetBodyId(signalPath));
    }
  } else {
    if (isFile) {
      signalPath = ZString(signalPath).absolutePath(dataDir);
    }

    if (ZFileType::FileType(signalPath) == ZFileType::FILE_SPARSE_STACK) {
      spStack = new ZSparseStack;
      spStack->load(signalPath);
    } else {
      signalStack = new ZStack;
      signalStack->load(signalPath);
    }
  }



//  ZStack signalStack;
//  signalStack.load(signalUrl);

  if (signalStack != NULL || spStack != NULL) {
    ZStackWatershedContainer container(signalStack, spStack);

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
        }
//        ZStack *seedStack = obj.toStackObject(label);
//        seedMask.push_back(seedStack);
      } else if (seedJson.hasKey("stroke")) {
        ZStroke2d stroke;
        stroke.loadJsonObject(seedJson);
        container.addSeed(stroke);
      }
    }

#ifdef _DEBUG_2
    container.exportMask(GET_TEST_DATA_DIR + "/test2.tif");
    container.exportSource(GET_TEST_DATA_DIR + "/test3.tif");
#endif

    container.run();
    ZStack *resultStack = container.getResultStack();

#ifdef _DEBUG_2
    resultStack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

//    ZStackWatershed watershed;
//    watershed.setFloodingZero(false);

//    ZStack *result = watershed.run(&signalStack, seedMask);

    if (resultStack != NULL) {
      QUrl outputUrl(output.c_str());
      ZObject3dScanArray *result = container.makeSplitResult();

      if (outputUrl.scheme() == "dvid" || outputUrl.scheme() == "http") {
        ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(output);
        ZJsonArray resultArray;

//        ZObject3dScanArray objArray;
//        ZObject3dFactory::MakeObject3dScanArray(
//              *result, NeuTube::Z_AXIS, true, &objArray);
        for (ZObject3dScanArray::const_iterator iter = result->begin();
             iter != result->end(); ++iter) {
          const ZObject3dScan &obj = *iter;
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

        if (!resultArray.isEmpty()) {
          ZJsonObject resultJson;
          resultJson.addEntry("type", "split");
          resultJson.addEntry("result", resultArray);
          std::string endPoint =
              writer->writeServiceResult("split", resultJson);
          std::cout << "Result endpoint: " << endPoint << std::endl;
          if (!splitTaskKey.empty()) {
            QString refEndPoint = ZDvidEndPoint::GetResultKeyEndPoint(
                  "split",
                  ZDvidUrl::GetResultKeyFromTaskKey(splitTaskKey).c_str());
            ZJsonObject refJson;
            refJson.setEntry(NeuTube::Json::REF_KEY, endPoint);
            refJson.setEntry(
                  "timestamp", QDateTime::currentMSecsSinceEpoch() / 1000);
            writer->writeJson(refEndPoint.toStdString(), refJson);
          }
        }

        if (!splitTaskKey.empty()) {
          writer->deleteKey(ZDvidData::GetName(ZDvidData::ROLE_SPLIT_TASK_KEY),
                            splitTaskKey);
        }
      } else {
        ZStackWriter writer;
        writer.write(output, resultStack);
      }

      delete result;
    } else {
      std::cout << "WARNING: Failed to produce result." << std::endl;
    }

    status = 0;
  }

  return status;
}
