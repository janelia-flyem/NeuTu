#include "ztransferskeletoncommand.h"

#include "zstring.h"

#include "dvid/zdvidtargetfactory.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

ZTransferSkeletonCommand::ZTransferSkeletonCommand()
{

}

int ZTransferSkeletonCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &/*config*/)
{
  ZDvidTarget inputTarget = ZDvidTargetFactory::MakeFromSpec(input.front());
  ZDvidTarget outputTarget = ZDvidTargetFactory::MakeFromSpec(output);

  if (ZDvidUrl(inputTarget).getSkeletonUrl() ==
      ZDvidUrl(outputTarget).getSkeletonUrl()) {
    warn("Nothing transfered", "the output is the same as the input.");
    return 0;
  }

  ZDvidWriter writer;
  if (writer.open(outputTarget)) {
    if (!writer.isSwcWrittable()) {
      error("Failed to transfer", "cannot write skeleton to the output.");
      return 1;
    }

    ZDvidReader reader;
    if (reader.open(inputTarget)) {
      int count = 0;
      auto keyList = reader.readKeys(inputTarget.getSkeletonName().c_str());
      foreach (auto key, keyList) {
        uint64_t bodyId = ZString(key.toStdString()).firstUint64();

        if (bodyId > 0) {
          bool existingSwc = writer.getDvidReader().hasKey(
                QString::fromStdString(outputTarget.getSkeletonName()), key);
          if (!existingSwc) {
            if (writer.getDvidReader().hasBody(bodyId)) {
              QByteArray data =
                  reader.readKeyValue(inputTarget.getSkeletonName().c_str(), key);
              if (!data.isEmpty()) {
                writer.writeDataToKeyValue(
                      outputTarget.getSkeletonName(), key.toStdString(), data);
                if (writer.getStatusCode() == 200) {
                  ++count;
                }
              }
            } else {
              info("Skip", "Body " + std::to_string(bodyId) + " does not exist");
            }
          }
        }
      }
      info("Done", std::to_string(count) + " skeleton(s) transferred.");
    } else {
      error("Failed to transfer", "Cannot open the input to tranfer.");
      return 1;
    }
  } else {
    error("Failed to transfer", "Cannot open the output to tranfer.");
    return 1;
  }

  return 0;
}
