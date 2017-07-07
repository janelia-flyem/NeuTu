#include "zbodyexportcommand.h"

#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zglobal.h"
#include "dvid/zdvidreader.h"
#include "misc/miscutility.h"
#include "zintcuboid.h"

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
    if (reader != NULL) {
      if (reader->isReady()) {
        ZObject3dScan obj;
        reader->readBody(bodyId, false, &obj);

        if (!obj.isEmpty()) {
          int dsIntv = misc::getIsoDsIntvFor3DVolume(
                obj.getBoundBox(), NeuTube::ONEGIGA, false);
          obj.downsampleMax(ZIntPoint(dsIntv, dsIntv, dsIntv));
          ZStack *stack = obj.toStackObject();
          stack->save(output);
          delete stack;
        }
      }
    }
  }

  return 1;
}
