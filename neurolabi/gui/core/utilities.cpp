#include "core/utilities.h"

#include <cstdlib>
#include <chrono>

#include "core/neutube_def.h"

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

uint64_t neutube::GetTimestamp()
{
  return std::chrono::duration_cast<std::chrono::seconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();
}
