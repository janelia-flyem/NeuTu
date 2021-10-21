#ifndef COMMAND_UTILITIES_H
#define COMMAND_UTILITIES_H

#include <vector>
#include <iostream>

namespace neutu {

std::vector<uint64_t> ImportBodies(std::istream &stream);
std::vector<uint64_t> ImportBodies(const std::string &filePath);

}

#endif // UTILITIES_H
