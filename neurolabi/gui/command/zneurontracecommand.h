#ifndef ZNEURONTRACECOMMAND_H
#define ZNEURONTRACECOMMAND_H

#include "zcommandmodule.h"

class ZSwcTree;

class ZNeuronTraceCommand : public ZCommandModule
{
public:
  ZNeuronTraceCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
          const ZJsonObject &config) override;

private:
  void loadTraceConfig(const ZJsonObject &config);
  ZSwcTree* traceFile(const std::string &filePath, const ZJsonObject &inputConfig);

private:
  int m_level = 0;
};

#endif // ZNEURONTRACECOMMAND_H
