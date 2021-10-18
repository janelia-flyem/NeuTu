#include "utilities.h"

#include <cstdlib>
#include <chrono>
#include <sstream>
#include <fstream>
#include <regex>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <locale>
#include <iomanip>

#include "common/neutudefs.h"

bool neutu::HasEnv(const std::string &name, const std::string &value)
{
  bool result = false;

  if (const char* setting = std::getenv(name.c_str())) {
    result = (std::string(setting) == value);
  }

  return result;
}

std::string neutu::GetEnv(const std::string &name)
{
  std::string result;
  if (const char* setting = std::getenv(name.c_str())) {
    result = std::string(setting);
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

void neutu::RangePartitionProcess(
    int x0, int x1, int n, std::function<void(int, int)> f)
{
  if (f) {
    if (x1 >= x0) {
      if (n > 0) {
        int currentMin = x0;
        int length = x1 - x0 + 1;
        int dx = length / n;
        int remain = length % n;
        if (dx == 0) {
          n = remain;
        }
        int currentMax = x0 + dx - 1;
        for (int i = 0; i < n; i++) {
          if (i < remain) {
            currentMax += 1;
          }
          f(currentMin, currentMax);
          currentMin = currentMax + 1;
          currentMax += dx;
        }
      }
    }
  }
}

void neutu::ReportError(const std::string &msg)
{
  std::cerr << "ERROR: " << msg << std::endl;
}

void neutu::ReportWarning(const std::string &msg)
{
  std::cout << "WARNING: " << msg << std::endl;
}

std::string neutu::GetRootUrl(const std::string &url)
{
  std::regex rgx("([^/]+://[^/]+).*");
  std::smatch matches;
  std::regex_search(url, matches, rgx);
  if (matches.size() > 1) {
    return matches[1];
  }

  return "";
}

uint64_t neutu::ToUint64(const std::string &s)
{
  char *se;
  return std::strtoull(s.c_str(), &se, 10);
}

int64_t neutu::ToInt64(const std::string &s)
{
  char *se;
  return std::strtoll(s.c_str(), &se, 10);
}

int64_t neutu::GetTimeStamp()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string neutu::GetUtcTimeString()
{
  time_t now = time(0);
  std::ostringstream stream;
  tm *ltm = gmtime(&now);
  stream << 1900 + ltm->tm_year << '-'
         << std::setfill('0') << std::setw(2) << 1 + ltm->tm_mon << '-'
         << std::setfill('0') << std::setw(2) << ltm->tm_mday << ' '
         << std::setfill('0') << std::setw(2) << ltm->tm_hour << ':'
         << std::setfill('0') << std::setw(2) << ltm->tm_min << ':'
         << std::setfill('0') << std::setw(2) << ltm->tm_sec
         << "+00:00";
  return stream.str();
}
