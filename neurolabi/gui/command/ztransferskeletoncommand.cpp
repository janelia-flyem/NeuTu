#include "ztransferskeletoncommand.h"

#include "dvid/zdvidtargetfactory.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

ZTransferSkeletonCommand::ZTransferSkeletonCommand()
{

}

int ZTransferSkeletonCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  ZDvidTarget inputTarget = ZDvidTargetFactory::MakeFromSpec(input.front());

  ZDvidReader reader;
  if (reader.open(inputTarget)) {
    ZDvidTarget outputTarget = ZDvidTargetFactory::MakeFromSpec(output);
    ZDvidWriter writer;
    if (writer.open(outputTarget)) {
      auto keyList = reader.readKeys(inputTarget.getSkeletonName().c_str());
      foreach (auto key, keyList) {
        if (!writer.getDvidReader().hasKey(
              QString::fromStdString(outputTarget.getSkeletonName()), key)) {
          QByteArray data =
              reader.readKeyValue(inputTarget.getSkeletonName().c_str(), key);
          if (!data.isEmpty()) {
            writer.writeDataToKeyValue(
                  outputTarget.getSkeletonName(), key.toStdString(), data);
          }
        }
      }
    }
  }

  return 0;
}
