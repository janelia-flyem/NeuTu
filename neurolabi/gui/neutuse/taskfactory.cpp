#include "taskfactory.h"
#include "neutubeconfig.h"
#include "dvid/zdvidtarget.h"
#include "neutuse/task.h"

namespace neutuse {

TaskFactory::TaskFactory()
{

}

Task TaskFactory::MakeDvidTask(
      const std::string &name,
      const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate)
{
  Task task;
  task.setType("dvid");
  task.setName(name);
  task.setUser(NeutubeConfig::GetUserName());
  ZJsonObject config;
  config.setEntry("bodyid", bodyId);
  config.setEntry("force_update", forceUpdate);
  ZJsonObject dvidJson;
  dvidJson.setEntry("address", target.getAddress());
  dvidJson.setEntry("port", target.getPort());
  dvidJson.setEntry("uuid", target.getUuid());
  dvidJson.setEntry("body_label", target.getBodyLabelName());
  config.setEntry("dvid", dvidJson);
  task.setConfig(config);

  return task;
}

}
