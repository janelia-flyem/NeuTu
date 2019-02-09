#include "utilities.h"

#include <cstdlib>
#include <chrono>
#include <sstream>

#include "common/neutube_def.h"

bool neutu::HasEnv(const std::string &name, const std::string &value)
{
  bool result = false;

  if (const char* setting = std::getenv(name.c_str())) {
    result = (std::string(setting) == value);
  }

  return result;
}

std::string neutu::GetVersionString()
{
  return std::string(neutu::VERSION) + " (" + neutu::PKG_VERSION + ")";
}

uint64_t neutu::GetTimestamp()
{
  return std::chrono::duration_cast<std::chrono::seconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string neutu::ToString(void *p)
{
  std::ostringstream stream;
  stream << p;
  return stream.str();
}
