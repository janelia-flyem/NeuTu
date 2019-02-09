#include "zglobaldvidrepo.h"
#include "neutubeconfig.h"
#include "neutube.h"

ZGlobalDvidRepo::ZGlobalDvidRepo()
{ 
}

void ZGlobalDvidRepo::setRepo(const std::string &name, const ZDvidTarget &target)
{
  if (!name.empty()) {
    m_dvidRepo[name] = target;
  }
}

void ZGlobalDvidRepo::addRepo(const ZDvidTarget &target)
{
  setRepo(target.getName(), target);
}

void ZGlobalDvidRepo::init()
{
  const std::vector<ZDvidTarget> dvidRepo = GET_FLYEM_CONFIG.getDvidRepo();

  std::string userName = neutu::GetCurrentUserName();

  for (std::vector<ZDvidTarget>::const_iterator iter = dvidRepo.begin();
           iter != dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    bool access = true;
    if (!target.getUserNameSet().empty()) {
      if (target.getUserNameSet().count(userName) == 0) {
        access = false;
      }
    }
    if (access) {
      addRepo(target);
    }
  }

  //Load locally saved targets
  QSettings &settings = NeutubeConfig::getInstance().getSettings();
  if (settings.contains("DVID")) {
    QString localDvidJsonStr = settings.value("DVID").toString();
    if (!localDvidJsonStr.isEmpty()) {
      ZJsonArray localDvidJson;
      localDvidJson.decode(localDvidJsonStr.toStdString());
      for (size_t i = 0; i < localDvidJson.size(); ++i) {
        ZJsonObject dvidTargetJson(
              localDvidJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        ZDvidTarget target;
        target.loadJsonObject(dvidTargetJson);
        addRepo(target);
      }
    }
  }
}

const ZDvidTarget& ZGlobalDvidRepo::getDvidTarget(const std::string &name)
{
  if (m_dvidRepo.count(name) > 0) {
    return m_dvidRepo[name];
  }

  return m_emptyTarget;
}

ZGlobalDvidRepo::const_iterator ZGlobalDvidRepo::begin() const
{
  return m_dvidRepo.begin();
}

ZGlobalDvidRepo::const_iterator ZGlobalDvidRepo::end() const
{
  return m_dvidRepo.end();
}
