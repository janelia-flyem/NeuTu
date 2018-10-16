#include "core/utilities.h"

#include <cstdlib>

#include "neutube_def.h"

bool neutube::HasEnv(const std::string &name, const std::string &value)
{
  bool result = false;

  if (const char* setting = std::getenv(name.c_str())) {
    result = (std::string(setting) == value);
  }

  return result;
}

std::string neutube::GetVersionString()
{
  return std::string(neutube::VERSION) + " (" + neutube::PKG_VERSION + ")";
}
