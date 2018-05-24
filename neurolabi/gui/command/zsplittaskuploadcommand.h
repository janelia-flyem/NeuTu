#ifndef ZSPLITTASKUPLOADCOMMAND_H
#define ZSPLITTASKUPLOADCOMMAND_H

#include "zcommandmodule.h"

class ZSplitTaskUploadCommand : public ZCommandModule
{
public:
  ZSplitTaskUploadCommand();

  int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);
};

#endif // ZSPLITTASKUPLOADCOMMAND_H
