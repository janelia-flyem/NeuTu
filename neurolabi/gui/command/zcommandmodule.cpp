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

std::string ZCommandModule::composeMessage(
    const std::string &title, const std::string &description) const
{
  std::string msg = description;
  if (!title.empty()) {
     msg = title + ": " + msg;
  }

  return msg;
}

void ZCommandModule::info(
    const std::string &title, const std::string &description) const
{
  std::cout << composeMessage(title, description) << std::endl;
}

void ZCommandModule::warn(
    const std::string &title, const std::string &description) const
{
  std::cout << "[WARNING] " << composeMessage(title, description) << std::endl;
}

void ZCommandModule::error(
    const std::string &title, const std::string &description) const
{
  std::cerr << "[ERROR] "<< composeMessage(title, description) << std::endl;
}

#if defined(_DEBUG_)
void ZCommandModule::debug(
    const std::string &title, const std::string &description) const
{
  info(title, description);
}
#else
void ZCommandModule::debug(
    const std::string &/*title*/, const std::string &/*description*/) const
{
}
#endif
