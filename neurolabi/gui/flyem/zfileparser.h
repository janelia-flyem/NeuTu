#ifndef ZFILEPARSER_H
#define ZFILEPARSER_H

#include <map>
#include <string>
#include <vector>

#include "zvaa3dmarker.h"
#include "zvaa3dapo.h"

namespace flyem {

class ZFileParser
{
public:
  ZFileParser();

public:
  static std::map<uint64_t, std::string> loadBodyList(std::string filePath,
                                                 std::string workDir = "");
  static bool writeVaa3dMakerFile(std::string filePath,
                                  const std::vector<ZVaa3dMarker> &markerArray);
  static bool writeVaa3dApoFile(std::string filePath,
                                const std::vector<ZVaa3dApo> &markerArray);

  static std::vector<ZVaa3dMarker> readVaa3dMarkerFile(
      const std::string &filePath);

  static std::string bodyNameToFileName(const std::string bodyName);
};

}

#endif // ZFILEPARSER_H
