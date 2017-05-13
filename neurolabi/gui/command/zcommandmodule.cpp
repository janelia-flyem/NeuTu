#include "zcommandmodule.h"

ZCommandModule::ZCommandModule()
{

}

int ZCommandModule::run(const std::vector<std::string> &/*input*/,
    const std::string &/*output*/,
    const ZJsonObject &/*config*/)
{
  return 1;
}
