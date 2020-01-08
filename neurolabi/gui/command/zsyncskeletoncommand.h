#ifndef ZSYNCSKELETONCOMMAND_H
#define ZSYNCSKELETONCOMMAND_H

#include "zcommandmodule.h"

class ZSyncSkeletonCommand : public ZCommandModule
{
public:
  ZSyncSkeletonCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
          const ZJsonObject &config) override;
};

#endif // ZSYNCSKELETONCOMMAND_H
