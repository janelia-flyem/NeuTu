#include "zbodyprocesscommand.h"

#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zobject3dscan.h"
#include "logging/zqslog.h"

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
  int maxNumSlice = parser.getValue(config, "maxNumSlice", int(0));
  if (maxNumSlice > 0) {
    neutu::EAxis sliceAxis = neutu::EAxis::Z;
    if (config.hasKey("axis")) {
      std::string axisOption = parser.getValue(config, "axis", "Z");
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
        wholeBody.chop(currentBox.getFirstZ() + dz, sliceAxis, &remain, &subobj);
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

}

int ZBodyProcessCommand::run(
    const std::vector<std::string> &input, const std::string &/*output*/,
    const ZJsonObject &config)
{
  std::string inputPath = input.front();

  uint64_t bodyId = ZDvidUrl::GetBodyId(inputPath);

  int ret = 1;
  ZJsonObjectParser parser;
  std::string action = parser.getValue(config, "action", "");

  ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(inputPath);

  if (!action.empty()) {
    //      ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader(target);
    ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter(target);
    if (writer) {
      if (bodyId > 0) {
        if (action == "decompose") {
          ret = decompose_body(bodyId, writer);
        } else if (action == "chop") {
          ret = chop_body(bodyId, writer, config);
        }
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
            if (action == "decompose") {
              ret = decompose_body(bodyId, writer);
            } else if (action == "chop") {
              ret = chop_body(bodyId, writer, config);
            }
          }
        }
      }
    }
  }

  return ret;
}
