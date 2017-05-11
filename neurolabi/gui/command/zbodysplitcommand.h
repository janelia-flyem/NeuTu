#ifndef ZBODYSPLITCOMMAND_H
#define ZBODYSPLITCOMMAND_H

#include "zcommandmodule.h"

class ZJsonObject;

class ZBodySplitCommand : public ZCommandModule
{
public:
  ZBodySplitCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);

};

#endif // ZBODYSPLITCOMMAND_H
