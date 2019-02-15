#ifndef ZMULTISCALEWATERSHEDCOMMAND_H
#define ZMULTISCALEWATERSHEDCOMMAND_H
#include "zcommandmodule.h"
#include "mvc/zstackdoc.h"

class ZMultiscaleWatershedCommand : public ZCommandModule
{
public:
  ZMultiscaleWatershedCommand();
  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};

#endif // ZMULTISCALEWATERSHEDCOMMAND_H
