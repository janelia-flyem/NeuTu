#include "utilities.h"

#include <boost/filesystem.hpp>

bool neutu::FileExists(const std::string &path)
{
  return boost::filesystem::exists(path);
}

uint64_t neutu::FileSize(const std::string &path)
{
  try {
    return boost::filesystem::file_size(path);
  }
  catch (...) {
    return 0;
  }
}

std::string neutu::FileExtension(const std::string &path)
{
  std::string ext = boost::filesystem::extension(path);
  if (!ext.empty()) {
    if (ext[0] == '.') {
      ext = ext.substr(1);
    }
  }

  return ext;
}

std::string neutu::JoinPath(const std::vector<std::string> &pathList)
{
  boost::filesystem::path result;
  for (const std::string &path : pathList) {
    result = result / path;
  }

  return result.string();
}

std::string neutu::JoinPath(const std::string &p1, const std::string &p2)
{
  return JoinPath({p1, p2});
}
