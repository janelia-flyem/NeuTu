#include "zstackdownsamplecommand.h"

#include "zfiletype.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstack.hxx"
#include "zstackwriter.h"
#include "zfiletype.h"
#include "zobject3dscan.h"

ZStackDownsampleCommand::ZStackDownsampleCommand()
{

}

/*!
 * Configuration
 * {
 *   "command": "downsample_stack",
 *   "intv": [<int>, <int>, <int>],
 *   "option": "max"|"min"|"mean"
 * }
 */
int ZStackDownsampleCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  int ret = 1;

  std::string option = ZJsonParser::stringValue(config["option"]);

  if (!input.empty() && !output.empty()) {
    if (config.hasKey("intv")) {
      std::vector<int64_t> intv = ZJsonParser::integerArray(config["intv"]);
      if (intv.size() == 3) {
        if (intv[0] >= 0 && intv[1] >= 0 && intv[2] >= 0) {
          if (ZFileType::IsImageFile(input.front())) {
            ZStack stack;
            stack.load(input.front());
            if (!stack.isEmpty()) {


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
          } else if (ZFileType::FileType(
                       input.front()) == ZFileType::EFileType::OBJECT_SCAN) {
            ZObject3dScan obj;
            obj.load(input.front());
            if (option == "min") {
              obj.downsampleMin(intv[0], intv[1], intv[2]);
            } else if (option == "mean") {
              obj.downsample(intv[0], intv[1], intv[2]);
            } else {
              obj.downsampleMax(intv[0], intv[1], intv[2]);
            }
            obj.save(output);
          }
        }
      }
    }
  }

  return ret;
}
