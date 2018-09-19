#ifndef ZSPARSESTACKCOMMANDMODULE_H
#define ZSPARSESTACKCOMMANDMODULE_H

#include "zcommandmodule.h"

class ZSparseStackCommand : public ZCommandModule
{
public:
  ZSparseStackCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};

#endif // ZSPARSESTACKCOMMANDMODULE_H
