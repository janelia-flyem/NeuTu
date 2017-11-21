#ifndef ZSTACKDIFFCOMMAND_H
#define ZSTACKDIFFCOMMAND_H

#include "zcommandmodule.h"

class ZStackDiffCommand : public ZCommandModule
{
public:
  ZStackDiffCommand();
  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};


#endif // ZSTACKDIFFCOMMAND_H
