#include "core/utilities.h"

#include <cstdlib>

bool neutube::HasEnv(const std::string &name, const std::string &value)
{
  bool result = false;

  if (const char* setting = std::getenv(name.c_str())) {
    result = (std::string(setting) == value);
  }

  return result;
}
