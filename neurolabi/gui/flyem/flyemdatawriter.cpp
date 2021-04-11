#include "flyemdatawriter.h"

#include "geometry/zintpoint.h"
#include "zobject3dscan.h"
#include "zjsonobject.h"
#include "zjsondef.h"
#include "zfiletype.h"

#include "neutubeconfig.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

#include "zflyembodyannotation.h"
#include "flyemdatareader.h"
#include "flyemdataconfig.h"

FlyEmDataWriter::FlyEmDataWriter()
{

}

void FlyEmDataWriter::UpdateBodyStatus(
    ZDvidWriter &writer, const ZIntPoint &pos, const std::string &newStatus)
{
  uint64_t bodyId = writer.getDvidReader().readBodyIdAt(pos);
  ZFlyEmBodyAnnotation annot =
      FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);
#ifdef _DEBUG_
  std::cout << "Old annotation:" << std::endl;
  annot.print();
#endif
  annot.setStatus(newStatus);
  writer.writeBodyAnnotation(annot);

#ifdef _DEBUG_
  ZFlyEmBodyAnnotation newAnnot =
      FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);
  std::cout << "New annotation:" << std::endl;
  newAnnot.print();
#endif
}

void FlyEmDataWriter::RewriteBody(ZDvidWriter &writer, uint64_t bodyId)
{
  uint64_t newBodyId = 0;

  if (writer.good()) {
    ZObject3dScan obj;
    writer.getDvidReader().readBody(bodyId, false, &obj);

    if (!obj.isEmpty()) {
      newBodyId = writer.writeSplit(obj, bodyId, 0);
      //      std::cout << newBodyId << std::endl;

      if (newBodyId > 0) {
        ZFlyEmBodyAnnotation annotation =
            FlyEmDataReader::ReadBodyAnnotation(writer.getDvidReader(), bodyId);

        if (!annotation.isEmpty()) {
          writer.deleteBodyAnnotation(bodyId);
          annotation.setBodyId(newBodyId);
          writer.writeBodyAnnotation(annotation);
        }
      }
    }
  }
}

void FlyEmDataWriter::UploadUserDataConfig(
    ZDvidWriter &writer, const FlyEmDataConfig &config)
{
  ZJsonObject obj;
  ZJsonObject contrastProtocol = config.getContrastProtocol().toJsonObject();
  obj.setEntry(FlyEmDataConfig::KEY_CONTRAST, contrastProtocol);
  std::string userName = NeutubeConfig::GetUserName();
  writer.writeJson(ZDvidData::GetName(ZDvidData::ERole::NEUTU_CONFIG),
                   "user_" + userName, obj);
}

void FlyEmDataWriter::WriteRoiData(
    ZDvidWriter &writer, const std::string &name, const ZObject3dScan &roi)
{
  if (!writer.getDvidReader().hasData(name)) {
    writer.createData("roi", name);
  } else {
    writer.deleteData("roi", name);
  }
  std::cout << "Writing " << name << std::endl;
  writer.writeRoi(roi, name);
}

void FlyEmDataWriter::UploadRoi(
    ZDvidWriter &writer, const std::string &name, const std::string &roiFile,
    const std::string &meshFile)
{
  if (!name.empty()) {
    if (!roiFile.empty()) {
      if (ZFileType::FileType(roiFile) == ZFileType::EFileType::OBJECT_SCAN) {
        ZObject3dScan roi;
        roi.load(roiFile);
        if (!writer.getDvidReader().hasData(name)) {
          writer.createData("roi", name);
        } else {
          writer.deleteData("roi", name);
        }
        std::cout << "Writing " << name << std::endl;
        writer.writeRoi(roi, name);
      } else {
        std::cout << "WARNING: Unexpected file roi file: " << roiFile << std::endl;
      }
    }

    if (!meshFile.empty()) {
      if (ZFileType::FileType(meshFile) == ZFileType::EFileType::MESH) {
        std::cout << "Writing mesh for " << name << std::endl;
        writer.uploadRoiMesh(meshFile, name);
      } else {
        std::cout << "WARNING: Unexpected file mesh file: " << meshFile << std::endl;
      }
    }
  }
}

namespace  {

void process_error_message(
    const std::string &msg, std::function<void(std::string)> errorMsgHandler)
{
  if (!errorMsgHandler) {
    errorMsgHandler = [](const std::string &msg) {
      std::cout << msg << std::endl;
    };
  }
  errorMsgHandler(msg);
}

}

void FlyEmDataWriter::TransferKeyValue(const ZDvidReader &reader, const std::string &sourceDataName,
    const std::string &sourcekey, ZDvidWriter &writer,
    const std::string &targetDataName, const std::string &targetKey,
    std::function<void(std::string)> errorMsgHandler)
{
  if (reader.good() && writer.good() && !sourceDataName.empty() &&
      !sourcekey.empty()) {
    if (reader.hasKey(sourceDataName.c_str(), sourcekey.c_str())) {
      QByteArray data = reader.readKeyValue(
            sourceDataName.c_str(), sourcekey.c_str());
      writer.writeDataToKeyValue(
            targetDataName.empty() ? sourceDataName : targetDataName,
            targetKey.empty() ? sourcekey : targetKey, data);
      std::cout << sourceDataName << " / " << sourcekey << " transfered."
                << std::endl;
    } else {
      process_error_message(
            "WARNING: " + sourceDataName + " / " + sourcekey + " not found in " +
            reader.getDvidTarget().getSourceString(), errorMsgHandler);
    }
  }
}

void FlyEmDataWriter::TransferRoiData(
    const ZDvidReader &reader, const std::string &sourceRoiName,
    ZDvidWriter &writer, const std::string &targetRoiName,
    std::function<void(std::string)> errorMsgHandler)
{
  if (reader.good() && writer.good() && !sourceRoiName.empty()) {
    std::string newTargetRoiName = targetRoiName;
    if (newTargetRoiName.empty()) {
      newTargetRoiName = sourceRoiName;
    }

    ZObject3dScan roi = reader.readRoi(sourceRoiName);
    if (!roi.isEmpty()) {
      if (!writer.getDvidReader().hasData(newTargetRoiName)) {
        writer.createData("roi", newTargetRoiName);
      } else {
        writer.deleteData("roi", newTargetRoiName);
      }
      std::cout << "Writing " << newTargetRoiName << std::endl;
      writer.writeRoi(roi, newTargetRoiName);
    } else {
      process_error_message(
            "WARNING: no source ROI data found.", errorMsgHandler);
    }
  }
}

void FlyEmDataWriter::TransferRoiData(
      const ZDvidReader &reader,
      const std::vector<std::string> &sourceRoiNameList,
      ZDvidWriter &writer, const std::string &targetRoiName,
      std::function<void(std::string)> errorMsgHandler)
{
  if (reader.good() && writer.good() && !sourceRoiNameList.empty()) {
    std::string newTargetRoiName = targetRoiName;
    std::string sourceRoiName = sourceRoiNameList.front();
    if (!sourceRoiName.empty()) {
      if (newTargetRoiName.empty()) {
        newTargetRoiName = sourceRoiName;
      }

      ZObject3dScan roi = reader.readRoi(sourceRoiName);
      for (size_t i = 1; i < sourceRoiNameList.size(); ++i) {
        reader.readRoi(sourceRoiNameList[i], &roi, true);
      }
      roi.canonize();

      if (!roi.isEmpty()) {
        if (!writer.getDvidReader().hasData(newTargetRoiName)) {
          writer.createData("roi", newTargetRoiName);
        } else {
          writer.deleteData("roi", newTargetRoiName);
        }
        std::cout << "Writing " << newTargetRoiName << std::endl;
        writer.writeRoi(roi, newTargetRoiName);
      } else {
        process_error_message(
              "WARNING: no source ROI data found.", errorMsgHandler);
      }
    } else {
      process_error_message("WARNING: empty source ROI name", errorMsgHandler);
    }
  } else {
    process_error_message("WARNING: empty source ROI list", errorMsgHandler);
  }
}



void FlyEmDataWriter::TransferRoiRef(
    const ZDvidReader &reader, const std::string &sourceRoiName,
    ZDvidWriter &writer, const std::string &targetRoiName,
    std::function<void(std::string)> errorMsgHandler)
{
  if (reader.good() && writer.good() && !sourceRoiName.empty()) {
    std::string meshRoiEntryDataName =
        ZDvidData::GetName(ZDvidData::ERole::ROI_KEY);
    ZJsonObject roiInfo =  reader.readJsonObjectFromKey(
          meshRoiEntryDataName.c_str(), sourceRoiName.c_str());
    if (roiInfo.hasKey(neutu::json::REF_KEY)) {
      std::set<std::string> meshDataKeySet;
      ZJsonObject jsonObj(roiInfo.value(neutu::json::REF_KEY));
      std::string type = ZJsonParser::stringValue(jsonObj["type"]);
      if (type.empty() || type == "mesh") {
        if (ZJsonParser::IsArray(jsonObj["key"])) {
          ZJsonArray arrayJson(jsonObj.value("key"));
          std::vector<std::string> keyList;
          for (size_t i = 0; i < arrayJson.size(); ++i) {
            std::string key = ZJsonParser::stringValue(arrayJson.at(i));
            if (!key.empty()) {
              meshDataKeySet.insert(key);
            }
          }
        } else {
          std::string key = ZJsonParser::stringValue(jsonObj["key"]);
          if (!key.empty()) {
            meshDataKeySet.insert(key);
          }
        }
      }

      for (const std::string &key : meshDataKeySet) {
        std::string roiDataKey =
            ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY);
        TransferKeyValue(
              reader, roiDataKey,
              key, writer, "", "", errorMsgHandler);
        std::string meshInfoKey = key + ZDvidUrl::MESH_INFO_SUFFIX;
        if (reader.hasKey(roiDataKey.c_str(), meshInfoKey.c_str())) {
#ifdef _DEBUG_
          std::cout << "Transferring " << meshInfoKey << std::endl;
#endif
          TransferKeyValue(
                reader, roiDataKey, meshInfoKey, writer, "", "",
                errorMsgHandler);
        } else {
          writer.deleteKey(roiDataKey, meshInfoKey);
        }
      }
    }

    TransferKeyValue(
          reader, meshRoiEntryDataName,
          sourceRoiName, writer, "", targetRoiName, errorMsgHandler);
  }
}

void FlyEmDataWriter::TransferRoi(
    const ZDvidReader &reader, const std::string &sourceRoiName,
    ZDvidWriter &writer, const std::string &targetRoiName,
    std::function<void(std::string)> errorMsgHandler)
{
  //Transfer ROI data
  TransferRoiData(reader, sourceRoiName, writer, targetRoiName, errorMsgHandler);

  //Transfer ROI mesh
  TransferRoiRef(reader, sourceRoiName, writer, targetRoiName, errorMsgHandler);
}

void FlyEmDataWriter::WriteMeshMerge(
    ZDvidWriter &writer, uint64_t targetId,
    const std::vector<uint64_t> &bodyIdArray)
{
  if (writer.good()) {
    if (!bodyIdArray.empty()) {
      ZJsonArray json;
      json.append(targetId);
      for (uint64_t bodyId : bodyIdArray) {
        if (bodyId != targetId) {
          json.append(bodyId);
        }
      }
      writer.writeJson(
            writer.getDvidTarget().getMeshName(),
            ZDvidUrl::GetMeshKey(targetId, ZDvidUrl::EMeshType::MERGED), json);
    }
  }
}
