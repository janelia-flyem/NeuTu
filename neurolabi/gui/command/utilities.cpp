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

std::vector<uint64_t> neutu::ImportBodiesFromCsv(
    std::istream &stream, size_t column, bool hasHead)
{
  std::vector<uint64_t> bodyIdArray;
  if (stream.good()) {
    ZString line;
    if (hasHead) {
      if (!getline(stream, line)) {
        return bodyIdArray;
      }
    }
    while (getline(stream, line)) {
      auto cols = ZString::ToWordArray(line);
      if (cols.size() > column) {
        uint64_t bodyId = ZString(cols[column]).firstUint64();
        if (bodyId > 0) {
          bodyIdArray.push_back(bodyId);
        }
      }
    }
  }

  return bodyIdArray;
}

std::vector<uint64_t> neutu::ImportBodiesFromCsv(
    const std::string &filePath, size_t column, bool hasHead)
{
  std::ifstream stream(filePath);
  std::vector<uint64_t> bodyIdArray;
  if (stream.is_open()) {
    return ImportBodiesFromCsv(stream, column, hasHead);
  }

  return bodyIdArray;
}
