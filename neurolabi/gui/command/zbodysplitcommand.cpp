#include "zbodysplitcommand.h"

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "imgproc/zstackwatershed.h"
#include "zstackwriter.h"
#include "zstring.h"
#include "flyem/zstackwatershedcontainer.h"
#include "zstroke2d.h"

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
          obj.load(seedUrl);
          container.addSeed(obj);
        } else if (type == "ZStroke2d") {
          ZStroke2d stroke;
          stroke.loadJsonObject(ZJsonObject(seedJson.value("data")));
        }
//        ZStack *seedStack = obj.toStackObject(label);
//        seedMask.push_back(seedStack);
      }
    }

    ZStack *result = container.run();

//    ZStackWatershed watershed;
//    watershed.setFloodingZero(false);

//    ZStack *result = watershed.run(&signalStack, seedMask);
    ZStackWriter writer;
    writer.write(output, result);

    delete result;

    status = 0;
  }

  return status;
}
