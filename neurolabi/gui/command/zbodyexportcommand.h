#ifndef ZBODYEXPORTCOMMAND_H
#define ZBODYEXPORTCOMMAND_H

#include "zcommandmodule.h"

class ZBodyExportCommand : public ZCommandModule
{
public:
  ZBodyExportCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};

#endif // ZBODYEXPORTCOMMAND_H
