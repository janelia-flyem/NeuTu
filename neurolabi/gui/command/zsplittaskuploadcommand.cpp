#include "zsplittaskuploadcommand.h"

#include <iostream>

#include "zjsondef.h"
#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zstroke2d.h"
#include "zglobal.h"
#include "zstring.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

#include "flyem/zflyemmisc.h"

ZSplitTaskUploadCommand::ZSplitTaskUploadCommand()
{
}

int ZSplitTaskUploadCommand::run(
    const std::vector<std::string> &input, const std::string &/*output*/,
    const ZJsonObject &config)
{
  ZJsonObject dvidJson(config.value("dvid"));
  if (dvidJson.isEmpty()) {
    std::cerr << "No dvid server specified. Abort." << std::endl;
    return 1;
  }

  if (input.empty()) {
    std::cerr << "No input file specified. Abort." << std::endl;
    return 1;
  }

  ZDvidTarget target;
  target.loadJsonObject(dvidJson);
  if (!target.isValid()) {
    std::cerr << "No valide dvid server specified. Abort." << std::endl;
    return 1;
  }

  if (target.getBodyLabelName().empty()) {
    std::cerr << "No sparsevol data specified. Abort." << std::endl;
    return 1;
  }

  ZJsonObject docJson;
  docJson.load(input.front());

  ZJsonArray rootObj(docJson.value("meshReview"));
  if (rootObj.isEmpty()) {
    std::cerr << "Unrecognized input format. Abort." << std::endl;
    return 1;
  }

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());
  if (writer == NULL) {
    std::cerr << "Unable to Initialize the task server: "
              << GET_FLYEM_CONFIG.getTaskServer() << std::endl;
    std::cerr << "Abort" << std::endl;
    return 1;
  } else {
    std::cout << "Task server: " << writer->getDvidTarget().getSourceString()
              << std::endl;
  }

  ZDvidUrl dvidUrl(target);

  std::cout << std::endl;
  std::cout << "Uploading tasks: " << std::endl << std::endl;
  int count = 0;
  for (size_t i = 0; i < rootObj.size(); ++i) {
    ZJsonObject obj(rootObj.value(i));
    ZJsonArray markerJson(obj.value("pointMarkers"));
#ifdef _DEBUG_2
      std::cout << "Here";
#endif
    if (!markerJson.isEmpty()) {
      uint64_t bodyId =
          ZString(obj.value("file").toString()).firstUint64();

      if (bodyId > 0) {
        ZJsonObject taskJson;
        flyem::SetSplitTaskSignalUrl(taskJson, bodyId, target);
        for (size_t i = 0; i < markerJson.size(); ++i) {
          ZJsonObject markerObj(markerJson.value(i));
          ZStroke2d stroke =
              flyem::SyGlassSeedToStroke(markerObj);
          flyem::AddSplitTaskSeed(taskJson, stroke);
        }

        std::string location = writer->writeServiceTask("split", taskJson);
        ZJsonObject entryJson;
        entryJson.setEntry(neutu::json::REF_KEY, location);
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
        writer->writeSplitTask(taskKey, taskJson);
        std::cout << "*    Task for " << bodyId << " is saved @ "
                  << taskKey.toStdString() << "->" << location << std::endl;
        ++count;
      }
    }
  }
  std::cout << std::endl;
  std::cout << count << " tasks uploaded." << std::endl;

  return 0;
}
