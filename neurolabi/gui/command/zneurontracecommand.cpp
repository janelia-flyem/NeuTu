#include "zneurontracecommand.h"

#include <iostream>

#include "neutubeconfig.h"
#include "zstack.hxx"
#include "zneurontracer.h"
#include "zneurontracerconfig.h"
#include "zjsonobjectparser.h"
#include "zswctree.h"
#include "zstackreader.h"
#include "zjsonobjectparser.h"
#include "zfiletype.h"
#include "filesystem/utilities.h"

ZNeuronTraceCommand::ZNeuronTraceCommand()
{

}

int ZNeuronTraceCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  ZJsonObject inputJson(config.value("_input"));

  ZJsonObjectParser parser(inputJson);
  std::string additionalInput = parser.getValue("signal", "");

  std::vector<std::string> updatedInput = input;
  if (!additionalInput.empty() && updatedInput.empty()) {
    updatedInput.push_back(additionalInput);
  }

  if(updatedInput.empty() || output.empty()) {
    return 1;
  }

  loadTraceConfig(config);

  ZSwcTree *tree = traceFile(updatedInput[0], inputJson);

  if (tree) {
    std::cout << "Saving " + output + "..." << std::endl;
    tree->save(output);
  } else {
    std::cout << "WARNING: No result generated." << std::endl;
  }

  return 0;
}


void ZNeuronTraceCommand::loadTraceConfig(const ZJsonObject &config)
{
  if (config.hasKey("path")) {
    ZJsonObject actualConfig;
    ZJsonObjectParser parser;
    std::string path = parser.getValue(config, "path", "");
    if (path.empty() || path == "default") {
      path = NeutubeConfig::getInstance().getPath(
            NeutubeConfig::EConfigItem::CONFIG_DIR) + "/json/trace_config.json";
    }
    actualConfig.load(path);

    ZNeuronTracerConfig::getInstance().loadJsonObject(actualConfig);
  } else {
    ZNeuronTracerConfig::getInstance().loadJsonObject(config);
  }
//  ZNeuronTracerConfig::getInstance().setCrossoverTest(false);
}

ZSwcTree* ZNeuronTraceCommand::traceFile(
    const std::string &filePath, const ZJsonObject &inputConfig)
{
//  ZStack signal;
//  signal.load(filePath);

  ZStack *signal = ZStackReader::Read(filePath);

  ZSwcTree *tree = nullptr;

  if (signal) {
    ZNeuronTracer tracer;
    tracer.setIntensityField(signal);
    tracer.setTraceLevel(m_level);

    ZJsonObjectParser parser(inputConfig);
    std::string maskPath = parser.getValue("mask", "");
    ZStack *mask = nullptr;
    if (!maskPath.empty()) {
      std::cout << "Using a predefined mask: " << maskPath << std::endl;
      if (!neutu::FileExists(maskPath)) {
        error("Cannot find the mask file " + maskPath + ".");
        return nullptr;
      }
      if (ZFileType::FileType(maskPath) != ZFileType::EFileType::TIFF) {
        error("Failed to recognize the mask file " + maskPath + " as a TIFF");
        return nullptr;
      }

      mask = ZStackReader::Read(maskPath);
      if (mask == nullptr) {
        warn("Failed to read mask file " + maskPath + ".");
      } else {
        int threshold = parser.getValue("maskThreshold", 0);
        if (threshold < 0) {
          //        Stack *newMask = tracer.makeMask(mask);
          tracer._makeMask = [&](Stack */*stack*/) {
            return tracer.makeMask(mask->c_stack());
          };
        } else {
          mask->binarize(threshold);
          tracer._makeMask = [=](Stack */*stack*/) {
            return C_Stack::clone(mask->c_stack());
          };
        }
      }
    }

    tree = tracer.trace(signal);

    delete mask;
    delete signal;

    return tree;
  }

  return tree;
}
