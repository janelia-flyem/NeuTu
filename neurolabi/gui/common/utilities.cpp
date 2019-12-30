#include "utilities.h"

#include <cstdlib>
#include <chrono>
#include <sstream>
#include <fstream>
#include <regex>
#include <cstdio>

#include "common/neutudefs.h"

/*
bool neutu::FileExists(const std::string &path)
{
  if (path.empty()) {
    return false;
  }

  FILE* fp = fopen(path.c_str(), "r");
  if (fp != NULL) {
    fclose(fp);
    return true;
  }

  return false;
}
*/

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

std::string neutu::ToString(const void *p)
{
  std::ostringstream stream;
  stream << p;
  return stream.str();
}

bool neutu::UsingLocalHost(const std::string &url)
{
  std::regex reg("^([^\\s\\/]+:\\/\\/)?(127\\.0\\.0\\.1|localhost)([:\\/].*|\\s*)$");

  return std::regex_match(url, reg);
}
