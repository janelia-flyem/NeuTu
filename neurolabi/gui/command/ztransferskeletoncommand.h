#ifndef ZTRANSFERSKELETONCOMMAND_H
#define ZTRANSFERSKELETONCOMMAND_H

#include "zcommandmodule.h"

class ZTransferSkeletonCommand : public ZCommandModule
{
public:
  ZTransferSkeletonCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
          const ZJsonObject &config) override;
};

#endif // ZTRANSFERSKELETONCOMMAND_H
