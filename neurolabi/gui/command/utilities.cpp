#include "utilities.h"

#include <fstream>

#include "zstring.h"

std::vector<uint64_t> neutu::ImportBodies(std::istream &stream)
{
  std::vector<uint64_t> bodyIdArray;
  if (stream.good()) {
    ZString line;
    while (getline(stream, line)) {
      auto bodies = line.toUint64Array();
      bodyIdArray.insert(
            bodyIdArray.end(), bodies.begin(), bodies.end());
    }
  }

  return bodyIdArray;
}

std::vector<uint64_t> neutu::ImportBodies(const std::string &filePath)
{
  std::ifstream stream(filePath);
  std::vector<uint64_t> bodyIdArray;
  if (stream.is_open()) {
    return ImportBodies(stream);
  }

  return bodyIdArray;
}
