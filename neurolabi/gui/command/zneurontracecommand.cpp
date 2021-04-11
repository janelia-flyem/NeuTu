#include "zneurontracecommand.h"

#include <iostream>

#include "neutubeconfig.h"
#include "zstack.hxx"
#include "zneurontracer.h"
#include "zneurontracerconfig.h"
#include "zjsonobjectparser.h"
#include "zswctree.h"
#include "zstackreader.h"

ZNeuronTraceCommand::ZNeuronTraceCommand()
{

}

int ZNeuronTraceCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  if(input.empty() || output.empty()) {
    return 1;
  }

  loadTraceConfig(config);

  ZSwcTree *tree = traceFile(input[0]);

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
    std::string path = parser.GetValue(config, "path", "");
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

ZSwcTree* ZNeuronTraceCommand::traceFile(const std::string &filePath)
{
//  ZStack signal;
//  signal.load(filePath);

  ZStack *signal = ZStackReader::Read(filePath);

  ZSwcTree *tree = nullptr;

  if (signal) {
    ZNeuronTracer tracer;
    tracer.setIntensityField(signal);
    tracer.setTraceLevel(m_level);

    tree = tracer.trace(signal);

    delete signal;

    return tree;
  }

  return tree;
}
