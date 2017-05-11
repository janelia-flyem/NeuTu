#include "zbodysplitcommand.h"

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zobject3dscan.h"
#include "zstack.hxx"
#include "imgproc/zstackwatershed.h"
#include "zstackwriter.h"

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

  std::string signalUrl = ZJsonParser::stringValue(inputJson["signal"]);

  ZStack signalStack;
  signalStack.load(signalUrl);

  if (!signalStack.isEmpty()) {
    std::vector<ZStack*> seedMask;

    ZJsonArray seedArrayJson(inputJson.value("seeds"));
    for (size_t i = 0; i < seedArrayJson.size(); ++i) {
      ZJsonObject seedJson(seedArrayJson.value(i));
      if (seedJson.hasKey("label") && seedJson.hasKey("url")) {
        int label = ZJsonParser::integerValue(seedJson["label"]);
        std::string seedUrl = ZJsonParser::stringValue(seedJson["url"]);
        ZObject3dScan obj;
        obj.load(seedUrl);
        ZStack *seedStack = obj.toStackObject(label);
        seedMask.push_back(seedStack);
      }
    }

    ZStackWatershed watershed;
    watershed.setFloodingZero(false);

    ZStack *result = watershed.run(&signalStack, seedMask);
    ZStackWriter writer;
    writer.write(output, result);

    delete result;
    for (std::vector<ZStack*>::iterator iter = seedMask.begin();
         iter != seedMask.end(); ++iter) {
      delete *iter;
    }

    status = 0;
  }

  return status;
}
