#ifndef ZSTACKDOWNSAMPLECOMMAND_H
#define ZSTACKDOWNSAMPLECOMMAND_H

#include "zcommandmodule.h"

class ZStackDownsampleCommand : public ZCommandModule
{
public:
  ZStackDownsampleCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
           const ZJsonObject &config);
};

#endif // ZSTACKDOWNSAMPLECOMMAND_H
