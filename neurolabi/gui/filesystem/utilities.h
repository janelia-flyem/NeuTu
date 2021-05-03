#ifndef ZFILESYSTEM_H
#define ZFILESYSTEM_H

#include <string>
#include <vector>

namespace neutu
{

bool FileExists(const std::string &path);
uint64_t FileSize(const std::string &path);
std::string FileExtension(const std::string &path);
std::string JoinPath(const std::vector<std::string> &pathList);
std::string JoinPath(const std::string &p1, const std::string &p2);

std::string Absolute(const std::string p, const std::string &base);

template<typename ...Args>
std::string JoinPath(const std::string &p1, const std::string &p2, Args... args) {
  return JoinPath(JoinPath(p1, p2), args...);
}
}

#endif // ZFILESYSTEM_H
