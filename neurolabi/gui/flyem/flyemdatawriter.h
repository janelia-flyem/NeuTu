#ifndef FLYEMDATAWRITER_H
#define FLYEMDATAWRITER_H

#include <string>
#include <functional>

class ZDvidWriter;
class ZDvidReader;
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

  /*!
   * \brief Transfer ROI from one node to another
   *
   * \a targetRoiName will be the same as \a sourceRoiname if it is empty.
   *
   * \param reader source reader
   * \param sourceRoiName source ROI name for both roi and mesh data.
   * \param writer target writer
   * \param targetRoiName target ROI name for both roi and mesh data.
   */
  static void TransferRoi(
      const ZDvidReader &reader, const std::string &sourceRoiName,
      ZDvidWriter &writer, const std::string &targetRoiName,
      std::function<void(std::string)> errorMsgHandler = nullptr);

  static void TransferRoiData(
      const ZDvidReader &reader, const std::string &sourceRoiName,
      ZDvidWriter &writer, const std::string &targetRoiName,
      std::function<void(std::string)> errorMsgHandler = nullptr);

  static void TransferRoiRef(
      const ZDvidReader &reader, const std::string &sourceRoiName,
      ZDvidWriter &writer, const std::string &targetRoiName,
      std::function<void(std::string)> errorMsgHandler = nullptr);

  static void TransferKeyValue(
      const ZDvidReader &reader,
      const std::string &sourceDataName, const std::string &sourcekey,
      ZDvidWriter &writer,
      const std::string &targetDataName, const std::string &targetKey,
      std::function<void(std::string)> errorMsgHandler = nullptr);
};

#endif // FLYEMDATAWRITER_H
