#include "zcommandmodule.h"

#include <iostream>

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

void ZCommandModule::warn(const std::string &msg) const
{
  std::cout << "WARNING: " << msg << std::endl;
}

void ZCommandModule::error(const std::string &msg) const
{
  std::cerr << "ERROR: " << msg << std::endl;
}
