#ifndef FLYEMDATAWRITER_H
#define FLYEMDATAWRITER_H

#include <string>

class ZDvidWriter;
class ZIntPoint;
class FlyEmDataConfig;

class FlyEmDataWriter
{
public:
  FlyEmDataWriter();

  static void UpdateBodyStatus(
      ZDvidWriter &writer, const ZIntPoint &pos, const std::string &newStatus);
  static void RewriteBody(ZDvidWriter &writer, uint64_t bodyId);
  static void UploadUserDataConfig(
      ZDvidWriter &writer, const FlyEmDataConfig &config);
  static void UploadRoi(
      ZDvidWriter &writer, const std::string &name, const std::string &roiFile,
      const std::string &meshFile);
};

#endif // FLYEMDATAWRITER_H
