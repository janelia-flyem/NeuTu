#include "zbodysplitcommand.h"

#include <QUrl>

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
#include "dvid/zdvidresultservice.h"
#include "zglobal.h"
#include "dvid/zdvidwriter.h"
#include "zobject3dfactory.h"
#include "zobject3dscanarray.h"

ZBodySplitCommand::ZBodySplitCommand()
{

}

int ZBodySplitCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &/*config*/)
{
  int status = 1;

  ZJsonObject inputJson;
  inputJson.load(input.front());

  std::string dataDir = ZString(input.front()).dirPath();

  std::string signalUrl = ZJsonParser::stringValue(inputJson["signal"]);
  signalUrl = ZString(signalUrl).absolutePath(dataDir);

  ZStack signalStack;
  signalStack.load(signalUrl);

  if (!signalStack.isEmpty()) {
    ZStackWatershedContainer container(&signalStack);

    ZJsonArray seedArrayJson(inputJson.value("seeds"));
    for (size_t i = 0; i < seedArrayJson.size(); ++i) {
      ZJsonObject seedJson(seedArrayJson.value(i));
      if (seedJson.hasKey("type")) {

        std::string seedUrl = ZJsonParser::stringValue(seedJson["url"]);
        seedUrl = ZString(seedUrl).absolutePath(dataDir);

        std::string type = ZJsonParser::stringValue(seedJson["type"]);
        if (type == "ZObject3dScan" && !seedUrl.empty() && seedJson.hasKey("label")) {
          int label = ZJsonParser::integerValue(seedJson["label"]);
          ZObject3dScan obj;
          obj.setLabel(label);
          QUrl url(seedUrl.c_str());
          if (url.scheme() == "http") {
            QByteArray data = ZDvidResultService::ReadData(seedUrl.c_str());
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
      }
    }

#ifdef _DEBUG_2
    container.exportMask(GET_TEST_DATA_DIR + "/test2.tif");
#endif

    ZStack *result = container.run();

//    ZStackWatershed watershed;
//    watershed.setFloodingZero(false);

//    ZStack *result = watershed.run(&signalStack, seedMask);

    if (result != NULL) {
      QUrl outputUrl(output.c_str());

      if (outputUrl.scheme() == "dvid" || outputUrl.scheme() == "http") {
        ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(output);
        ZJsonArray resultArray;

        ZObject3dScanArray objArray;
        ZObject3dFactory::MakeObject3dScanArray(
              *result, NeuTube::Z_AXIS, true, &objArray);
        for (ZObject3dScanArray::const_iterator iter = objArray.begin();
             iter != objArray.end(); ++iter) {
          const ZObject3dScan &obj = *iter;
          std::string endPoint =
              writer->writeServiceResult(obj.toDvidPayload());
          ZJsonObject regionJson;
          regionJson.setEntry("label", (int) obj.getLabel());
          regionJson.setEntry("ref", endPoint);
          resultArray.append(regionJson);
#ifdef _DEBUG_2
          obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif
        }

        if (!resultArray.isEmpty()) {
          ZJsonObject resultJson;
          resultJson.addEntry("type", "split");
          resultJson.addEntry("result", resultArray);
          std::string endPoint = writer->writeServiceResult(
                QByteArray(resultJson.dumpString(0).c_str()));
          std::cout << "Result endpoint: " << endPoint << std::endl;
        }
      } else {
        ZStackWriter writer;
        writer.write(output, result);
      }

      delete result;
    } else {
      std::cout << "WARNING: Failed to produce result." << std::endl;
    }

    status = 0;
  }

  return status;
}
