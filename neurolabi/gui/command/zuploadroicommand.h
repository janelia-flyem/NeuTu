#ifndef ZUPLOADROICOMMAND_H
#define ZUPLOADROICOMMAND_H

#include "zcommandmodule.h"

class ZUploadRoiCommand : public ZCommandModule
{
public:
  ZUploadRoiCommand();

  int run(const std::vector<std::string> &input, const std::string &output,
          const ZJsonObject &config) override;
};

#endif // ZUPLOADROICOMMAND_H
