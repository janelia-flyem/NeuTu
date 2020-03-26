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

int ZBodyProcessCommand::run(
    const std::vector<std::string> &input, const std::string &/*output*/,
    const ZJsonObject &config)
{
  std::string inputPath = input.front();

  uint64_t bodyId = ZDvidUrl::GetBodyId(inputPath);

  int ret = 1;
  if (bodyId > 0) {
    ZJsonObjectParser parser;
    std::string action = parser.getValue(config, "action", "");

    ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(inputPath);

    if (!action.empty()) {
      ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader(target);

      if (reader) {
        ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter(target);
        ZObject3dScan wholeBody;
        reader->readBody(bodyId, false, &wholeBody);

        if (action == "decompose") {
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
                  LWARN() << "Warning: Something wrong happened during uploading! "
                            "Please contact the developer as soon as possible.";
                }
              }
            }
          } else {
            LINFO() << "Nothing to decompose";
          }
          ret = 0;
        }
      }
    }
  }

  return ret;
}
