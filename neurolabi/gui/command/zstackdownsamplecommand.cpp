#include "zstackdownsamplecommand.h"

#include "zfiletype.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstack.hxx"
#include "zstackwriter.h"

ZStackDownsampleCommand::ZStackDownsampleCommand()
{

}

int ZStackDownsampleCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  int ret = 1;

  if (!input.empty() && !output.empty()) {
    if (config.hasKey("intv")) {
      std::vector<int64_t> intv = ZJsonParser::integerArray(config["intv"]);
      if (intv.size() == 3) {
        if (intv[0] >= 0 && intv[1] >= 0 && intv[2] >= 0) {
          ZStack stack;
          stack.load(input.front());
          if (!stack.isEmpty()) {
            std::string option = ZJsonParser::stringValue(config["option"]);

            if (option == "min") {
              stack.downsampleMin(intv[0], intv[1], intv[2]);
            } else if (option == "mean") {
              stack.downsampleMean(intv[0], intv[1], intv[2]);
            } else {
              stack.downsampleMax(intv[0], intv[1], intv[2]);
            }

            ZStackWriter writer;
            writer.write(output, &stack);

            ret = 0;
          }
        }
      }
    }
  }

  return ret;
}
