#ifndef ZFILESYSTEM_H
#define ZFILESYSTEM_H

#include <string>

namespace neutu
{

bool FileExists(const std::string &path);
uint64_t FileSize(const std::string &path);
std::string FileExtension(const std::string &path);
std::string Join(const std::vector<std::string> &pathList);

}

#endif // ZFILESYSTEM_H
