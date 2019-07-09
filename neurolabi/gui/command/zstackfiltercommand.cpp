#include "zstackfiltercommand.h"

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstack.hxx"
#include "zstackwriter.h"
#include "imgproc/zstackprocessor.h"

ZStackFilterCommand::ZStackFilterCommand()
{

}

/*!
 * Configuration
 * {
 *   "command": "filter_stack",
 *   "method": "mexhat", "gaussian"
 *   "sigma": double | [double, double, double]
 * }
 */
int ZStackFilterCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  int ret = 1;

  if (!input.empty() && !output.empty()) {
    if (config.hasKey("method")) {
      ZStack stack;
      stack.load(input.front());
      if (!stack.isEmpty()) {
        double sigmas[3] = {1.0, 1.0, 1.0};

        if (config.hasKey("sigma")) {
          if (ZJsonParser::IsArray(config["sigma"])) {
            for (size_t i = 0; i < 3; ++i) {
              sigmas[i] = ZJsonParser::numberValue(config["sigma"], i);
            }
          } else {
            double sigma = ZJsonParser::numberValue(config["sigma"]);
            for (size_t i = 0; i < 3; ++i) {
              sigmas[i] = sigma;
            }
          }
        }

        std::string method = ZJsonParser::stringValue(config["method"]);

        std::cout << "Running " + method + " filter: "
                  << sigmas[0] << "x" << sigmas[1] << "x" << sigmas[2]
                  << std::endl;

        if (method == "mexhat") {
          ZStackProcessor::MexihatFilter(&stack, sigmas[0]);
        } else if (method == "gaussian") {
          ZStackProcessor::GaussianSmooth(&stack, sigmas[1], sigmas[1], sigmas[2]);
        } else if (method == "gradient") {
          Stack *out = C_Stack::computeGradient(stack.c_stack());
          stack.load(out);
          ZStackProcessor::GaussianSmooth(
                &stack, sigmas[1], sigmas[1], sigmas[2]);
        }

        ZStackWriter writer;
        writer.write(output, &stack);
        ret = 0;
      }
    }
  }

  return ret;
}
