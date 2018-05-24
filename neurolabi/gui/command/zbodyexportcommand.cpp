#include "zbodyexportcommand.h"

#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zglobal.h"
#include "dvid/zdvidreader.h"
#include "misc/miscutility.h"
#include "zintcuboid.h"
#include "zstack.hxx"

ZBodyExportCommand::ZBodyExportCommand()
{

}

int ZBodyExportCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &/*config*/)
{
  std::string inputPath = input.front();

  uint64_t bodyId = ZDvidUrl::GetBodyId(inputPath);

  if (bodyId > 0) {
    ZDvidReader *reader = ZGlobal::GetInstance().getDvidReaderFromUrl(inputPath);
    reader->updateMaxLabelZoom();
    if (reader != NULL) {
      if (reader->isReady()) {
        ZObject3dScan obj;
        bool needDownsampling = true;
        std::cout << "Max label zoom: "
                  << reader->getDvidTarget().getMaxLabelZoom() << std::endl;
        if (reader->getDvidTarget().getMaxLabelZoom() > 0) {
          ZObject3dScan coarseObj = reader->readCoarseBody(bodyId);
          ZDvidInfo dvidInfo = reader->readLabelInfo();
          ZIntCuboid box =coarseObj.getBoundBox();
          box.setWidth(box.getWidth() * dvidInfo.getBlockSize().getX());
          box.setHeight(box.getHeight() * dvidInfo.getBlockSize().getY());
          box.setDepth(box.getDepth() * dvidInfo.getBlockSize().getZ());
          int dsIntv = misc::getIsoDsIntvFor3DVolume(
                box, neutube::ONEGIGA, true);
          int scale = std::log2(dsIntv + 1);
          if (scale > reader->getDvidTarget().getMaxLabelZoom()) {
            scale = reader->getDvidTarget().getMaxLabelZoom();
            needDownsampling = true;
          }
          reader->readMultiscaleBody(bodyId, scale, false, &obj);
        } else {
          reader->readBody(bodyId, false, &obj);
        }

        if (!obj.isEmpty()) {
          if (needDownsampling) {
            int dsIntv = misc::getIsoDsIntvFor3DVolume(
                  obj.getBoundBox(), neutube::ONEGIGA, false);
            obj.downsampleMax(ZIntPoint(dsIntv, dsIntv, dsIntv));
          }
          ZStack *stack = obj.toStackObject();
          stack->save(output);
          delete stack;
        }
      }
    }
  }

  return 1;
}
