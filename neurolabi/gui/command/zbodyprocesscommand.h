#ifndef ZBODYPROCESSCOMMAND_H
#define ZBODYPROCESSCOMMAND_H

#include "zcommandmodule.h"

class ZBodyProcessCommand : public ZCommandModule
{
public:
  ZBodyProcessCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config) override;
};

#endif // ZBODYPROCESSCOMMAND_H
