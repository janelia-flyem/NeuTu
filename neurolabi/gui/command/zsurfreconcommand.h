#ifndef ZSURFRECONCOMMAND_H
#define ZSURFRECONCOMMAND_H
#include "zcommandmodule.h"

class ZSurfreconCommand : public ZCommandModule
{
public:
  ZSurfreconCommand();
  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};

#endif // ZSURFRECONCOMMAND_H
