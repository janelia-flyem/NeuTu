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
};

#endif // FLYEMDATAWRITER_H
