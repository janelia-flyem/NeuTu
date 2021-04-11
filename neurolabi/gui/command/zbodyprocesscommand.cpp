#include "zbodyprocesscommand.h"

#include <cstdlib>
#include <QProcess>
#include <QString>

#include "neulib/core/stringbuilder.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zobject3dscan.h"
#include "logging/zqslog.h"
#include "common/utilities.h"
#include "ztextlinecompositer.h"

#include "dvid/zdvidurl.h"
#include "dvid/zdvidtargetfactory.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

#include "zglobal.h"

ZBodyProcessCommand::ZBodyProcessCommand()
{

}

namespace {

int decompose_body(uint64_t bodyId, ZDvidWriter *writer)
{
  int ret = 0;

  ZObject3dScan wholeBody;
  writer->getDvidReader().readBody(bodyId, false, &wholeBody);

  std::vector<ZObject3dScan> objArray =
      wholeBody.getConnectedComponent(ZObject3dScan::ACTION_NONE);
  std::vector<uint64_t> updateBodyArray;

  if (objArray.size() > 1) {
    size_t maxIndex = 0;
    size_t maxSize = 0;
    size_t index = 0;
    for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter, ++index) {
      const ZObject3dScan &obj = *iter;
      if (obj.getVoxelNumber() > maxSize) {
        maxSize = obj.getVoxelNumber();
        maxIndex = index;
      }
    }

    index = 0;
    for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter, ++index) {
      if (index != maxIndex) {
        ZObject3dScan &obj = *iter;
        obj.canonize();
        wholeBody.subtractSliently(obj);
        uint64_t newBodyId =
            writer->writePartition(wholeBody, obj, bodyId);
        QString msg;
        if (newBodyId > 0) {
          size_t voxelNumber = obj.getVoxelNumber();
          msg = QString("Isolated object uploaded as %1 (%2 voxels) .").
              arg(newBodyId).arg(voxelNumber);
          if (voxelNumber >= 10) {
            updateBodyArray.push_back(newBodyId);
          }

          LINFO() << msg;
        } else {
          LWARN() << "Warning: Something wrong happened during uploading!";
          ret = 1;
        }
      }
    }
  } else {
    LINFO() << "Nothing to decompose";
  }

  return ret;
}

int chop_body(
    uint64_t bodyId, ZDvidWriter *writer, const ZJsonObject &config)
{
  int ret = 1;

  ZJsonObjectParser parser;
  int maxNumSlice = parser.GetValue(config, "maxNumSlice", int(0));
  if (maxNumSlice > 0) {
    neutu::EAxis sliceAxis = neutu::EAxis::Z;
    if (config.hasKey("axis")) {
      std::string axisOption = parser.GetValue(config, "axis", "Z");
      if (axisOption == "X") {
        sliceAxis = neutu::EAxis::X;
      } else if (axisOption == "Y") {
        sliceAxis = neutu::EAxis::Y;
      }
    }

    //Get body size
    size_t voxelCount = 0;
    size_t blockCount = 0;
    ZIntCuboid box;
    std::tie(voxelCount, blockCount, box) =
        writer->getDvidReader().readBodySizeInfo(
          bodyId, neutu::EBodyLabelType::BODY);

    int d = box.getDim(sliceAxis);
    if (d > maxNumSlice) {
      ZObject3dScan wholeBody;
      writer->getDvidReader().readBody(bodyId, false, &wholeBody);
      ZIntCuboid currentBox = wholeBody.getIntBoundBox();
      currentBox.shiftSliceAxis(sliceAxis);
      d = currentBox.getDepth();
      int numPart = d / maxNumSlice + ((d % maxNumSlice) > 0);
      int dz = d / numPart;

      while(d > dz) {
        ZObject3dScan remain;
        ZObject3dScan subobj;
        wholeBody.chop(currentBox.getMinZ() + dz, sliceAxis, &remain, &subobj);
        writer->writePartition(wholeBody, subobj, bodyId);
        wholeBody = remain;
        currentBox = wholeBody.getIntBoundBox();
        currentBox.shiftSliceAxis(sliceAxis);
        d = currentBox.getDepth();
      }
    }
    ret = 0;
  }

  return ret;
}

int process_body(
    uint64_t bodyId, const std::string &action, const ZJsonObject &config,
    ZDvidWriter *writer)
{
  int ret = 0;

  if (action == "decompose") {
    ret = decompose_body(bodyId, writer);
  } else if (action == "chop") {
    ret = chop_body(bodyId, writer, config);
  } else if (action == "external") {
    ZJsonObjectParser parser;
    std::string command = parser.GetValue(config, "script", "");
    if (!command.empty()) {
      std::system(std::string(neulib::StringBuilder(command + " ").append(bodyId)).c_str());
      /*
      //QProcess was blocked for unknown reasons
      QProcess process;
      QStringList args;
      args << QString("%1").arg(bodyId);
      process.start(command.c_str(), args);
      if (!process.waitForStarted()){
        std::cerr << process.errorString().toStdString() << std::endl;
        ret = 1;
      } else {
//        process.waitForFinished(-1);
        while(!process.waitForFinished()) {
          std::cout << process.readAllStandardOutput().toStdString() << std::endl;
          QString error = process.readAllStandardError();
          if (!error.isEmpty()) {
            std::cout << error.toStdString() << std::endl;
          }
        }

      }
      */
    }
  } else if (action == "info") {
    const ZDvidReader &reader = writer->getDvidReader();
    ZTextLineCompositer text;
    text.appendLine(neulib::StringBuilder("Info for ").append(bodyId));
    if (reader.hasBody(bodyId)) {
      size_t voxelCount = 0;
      size_t blockCount = 0;
      ZIntCuboid box;
      std::tie(voxelCount, blockCount, box) =
          reader.readBodySizeInfo(bodyId, neutu::EBodyLabelType::BODY);
      text.appendLine(neulib::StringBuilder("#Voxels: ").append(voxelCount), 1);
      text.appendLine(neulib::StringBuilder("#Blocks: ").append(blockCount), 1);
      text.appendLine("#Boundbox: " + box.toString(), 1);

      auto mergedKeys = reader.readMergedMeshKeys(bodyId);
      text.appendLine("Merged: " + neutu::ToString(mergedKeys, ", "), 1);

      if (!mergedKeys.empty() &&
          reader.readConsistentMergedMeshKeys(bodyId).empty()) {
        text.appendLine("Incomplete", 2);
      }

      {
        std::string meshDataName = reader.getDvidTarget().getMeshName();
        QStringList keyList = reader.readKeys(
              meshDataName.c_str(),
              QString("%1_0").arg(bodyId), QString("%1_z").arg(bodyId));
        text.appendLine("Mesh:", 1);
        for (const auto &key : keyList) {
          text.appendLine(key.toStdString(), 2);
        }

        std::string meshKey =
            ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::DEFAULT);
        if (reader.hasKey(meshDataName.c_str(), meshKey.c_str())) {
          text.appendLine(meshKey, 2);
        }

        meshKey = ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::DRACO);
        if (reader.hasKey(meshDataName.c_str(), meshKey.c_str())) {
          text.appendLine(meshKey, 2);
        }

        meshKey = ZDvidUrl::GetMeshKey(bodyId, ZDvidUrl::EMeshType::NG);
        if (reader.hasKey(meshDataName.c_str(), meshKey.c_str())) {
          text.appendLine(meshKey, 2);
        }
      }

      {
        std::string skeletonDataName = reader.getDvidTarget().getSkeletonName();
        std::string key = ZDvidUrl::GetSkeletonKey(bodyId);
        text.appendLine("Skeleton:", 1);
        if (reader.hasKey(skeletonDataName.c_str(), key.c_str())) {
          text.appendLine(key, 2);
        }
      }
    } else {
      text.appendLine("Does not exist", 1);
    }

    text.print(2);
  } else if (action == "remove_derived") {
    if (config.hasKey("derived")) {
      ZJsonArray derivedJson(config.value("derived"));
      derivedJson.forEachString([&](const std::string &str) {
        if (str == "mesh") {
          writer->deleteMesh(bodyId);
        } else if (str == "skeleton") {
          writer->deleteSkeleton(bodyId);
        }
      });
    }
  }

  return ret;
}

}


int ZBodyProcessCommand::run(
    const std::vector<std::string> &input, const std::string &/*output*/,
    const ZJsonObject &config)
{
  std::string inputPath = input.front();

  uint64_t bodyId = ZDvidUrl::GetBodyId(inputPath);

  int ret = 1;
  ZJsonObjectParser parser;
  std::string action = parser.GetValue(config, "action", "");

  ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(inputPath);

  if (!action.empty()) {
    //      ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader(target);
    ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter(target);
    if (writer) {
      if (bodyId > 0) {
        ret = process_body(bodyId, action, config, writer);
      } else {
        if (writer->getDvidTarget().hasMultiscaleSegmentation()) {
          ZDvidReader &reader = writer->getDvidReader();
          ZDvidInfo dvidInfo = reader.readDataInfo(
                writer->getDvidTarget().getSegmentationName());
          std::set<uint64_t> bodySet = reader.readBodyId(
                dvidInfo.getDataRange(), std::min(5, reader.getMaxLabelZoom()));
          size_t numProcessed = 0;
          size_t total = bodySet.size();
          for (uint64_t bodyId : bodySet) {
            std::cout << numProcessed++ << "/" << total << " "
                      << action << ": " << bodyId << std::endl;
            ret = process_body(bodyId, action, config, writer);
          }
        }
      }
    } else {
      std::cout << "Command Failed: Failed to open " << inputPath << std::endl;
    }
  }

  return ret;
}
