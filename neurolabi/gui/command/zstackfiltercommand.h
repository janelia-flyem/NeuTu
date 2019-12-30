#ifndef ZSTACKFILTERCOMMAND_H
#define ZSTACKFILTERCOMMAND_H

#include "zcommandmodule.h"

class ZStackFilterCommand : public ZCommandModule
{
public:
  ZStackFilterCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
           const ZJsonObject &config);
};

#endif // ZSTACKFILTERCOMMAND_H
