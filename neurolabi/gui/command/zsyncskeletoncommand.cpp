#include "zsyncskeletoncommand.h"

#include <memory>
#include <QDebug>

#include "zstring.h"
#include "common/zstringbuilder.h"

#include "zjsonobjectparser.h"
#include "zswctree.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"

#include "neutuse/taskwriter.h"
#include "neutuse/taskfactory.h"
#include "neutuse/task.h"

#include "flyem/flyemdatareader.h"
#include "flyem/zflyembodyannotation.h"
#include "zflyemutilities.h"

ZSyncSkeletonCommand::ZSyncSkeletonCommand()
{

}

namespace {

bool passed(
    const ZFlyEmBodyAnnotation &annot, const std::set<std::string>& statusSet)
{
  const std::string status = ZString(annot.getStatus()).lower();

  if (statusSet.empty()) {
    return true;
  } else {
    return statusSet.count(status) > 0;
  }
}

}

int ZSyncSkeletonCommand::run(
    const std::vector<std::string> &input, const std::string &output,
    const ZJsonObject &config)
{
  if(input.empty() || output.empty()) {
    return 1;
  }

  neutuse::TaskWriter writer;

  ZJsonObjectParser parser;
  std::string neutuseServer = output;
  if (neutuseServer.empty()) {
    qWarning() << "Neutuse server is not specified. Abort!";
    return 1;
  } else {
    writer.open(neutuseServer);
  }

  if (!writer.ready()) {
    qWarning() << "Cannot connect to neutuse:" << neutuseServer.c_str() << "Abort!";
    return 1;
  }

  ZDvidTarget target;
  target.setFromSourceString(input[0]);

  neutuse::TaskFactory taskFactory;
  taskFactory.setForceUpdate(true);
  taskFactory.setPriority(parser.getValue(config, "priority", 5));

  ZDvidReader reader;
  if (reader.open(target)) {
    reader.setVerbose(false);
    if (reader.hasData(target.getSkeletonName())) {
      std::set<std::string> statusSet;
      if (config.hasKey("bodyStatus")) {
        ZJsonArray statusJson(config.value("bodyStatus"));
        for (size_t i = 0; i < statusJson.size(); ++i) {
          std::string status =
              ZString(ZJsonParser::stringValue(statusJson.at(i))).lower();
          if (!status.empty()) {
            statusSet.insert(status);
          }
        }
      }

      QStringList annotList = reader.readKeys(target.getBodyAnnotationName().c_str());
      for (const QString &bodyStr : annotList) {
        uint64_t bodyId = ZString(bodyStr.toStdString()).firstUint64();
        if (bodyId > 0) {
          ZFlyEmBodyAnnotation annot =
              FlyEmDataReader::ReadBodyAnnotation(reader, bodyId);
          if (passed(annot, statusSet)) {
            if (reader.hasBody(bodyId)) {
              int64_t bodyMod = reader.readBodyMutationId(bodyId);
              std::unique_ptr<ZSwcTree> tree =
                  std::unique_ptr<ZSwcTree>(reader.readSwc(bodyId));
              bool syncing = false;
              std::string reason;
              if (!tree) {
                reason = "no skeleton";
                syncing = true;
              } else if (bodyMod >= 0) { //mutation ID must be available
                int64_t swcMod = flyem::GetMutationId(tree.get());
                if (bodyMod != swcMod) {
                  reason = ZStringBuilder("").
                      append(swcMod).append("->").append(bodyMod);
                  syncing = true;
                }
              }

              if (syncing) {
                std::cout << "To sync: " << bodyId
                          << " (" << reason << ")" << std::endl;
                writer.uploadTask(
                      taskFactory.makeDvidSkeletonizeTask(target, bodyId));
              }
            }
          }
        }
      }
    } else {
      qWarning() << "Skeleton data store does not exist. Abort!";
      return 1;
    }
  }

  return 0;
}
