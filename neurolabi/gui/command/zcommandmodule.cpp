#include "zcommandmodule.h"

ZCommandModule::ZCommandModule()
{
  m_forceUpdate = false;
}

int ZCommandModule::run(
    const std::vector<std::string> &/*input*/,
    const std::string &/*output*/,
    const ZJsonObject &/*config*/)
{
  return 1;
}

void ZCommandModule::setForceUpdate(bool on)
{
  m_forceUpdate = on;
}

bool ZCommandModule::forcingUpdate() const
{
  return m_forceUpdate;
}
