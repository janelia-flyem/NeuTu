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

Task TaskFactory::MakeDvidSkeletonizeTask(
    const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate)
{
  Task task = MakeDvidTask("skeletonize", target, bodyId, forceUpdate);
  task.setPriority(5);

  return task;
}

void TaskFactory::setPriority(int p)
{
  m_priority = p;
}

void TaskFactory::setForceUpdate(bool forceUpdate)
{
  m_forceUpdate = forceUpdate;
}

Task TaskFactory::makeDvidTask(
    const std::string &name, const ZDvidTarget &target, uint64_t bodyId) const
{
  Task task = MakeDvidTask(name, target, bodyId, m_forceUpdate);
  task.setPriority(m_priority);

  return task;
}

Task TaskFactory::makeDvidSkeletonizeTask(
    const ZDvidTarget &target, uint64_t bodyId) const
{
  return makeDvidTask("skeletonize", target, bodyId);
}

}
