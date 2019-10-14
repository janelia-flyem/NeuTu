#include "utilities.h"

#include <boost/filesystem.hpp>

bool neutu::FileExists(const std::string &path)
{
  return boost::filesystem::exists(path);
}

uint64_t neutu::FileSize(const std::string &path)
{
  return boost::filesystem::file_size(path);
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

std::string neutu::Join(const std::vector<std::string> &pathList)
{
  boost::filesystem::path result;
  for (const std::string &path : pathList) {
    result = result / path;
  }

  return result.string();
}
