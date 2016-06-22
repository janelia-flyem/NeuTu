#ifndef ZFLYEMCONFIG_H
#define ZFLYEMCONFIG_H

#include <vector>
#include <map>
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidtarget.h"

class ZFlyEmConfig
{
public:
  ZFlyEmConfig();

  void setDvidTarget(const std::string &repo);
  void setDvidTarget(const ZDvidTarget &target);

  void print() const;

  inline const ZDvidTarget& getDvidTarget() const {
    return m_emptyDvidTarget;
  }

  void loadConfig(const std::string &filePath);

  inline const std::vector<ZDvidTarget> &getDvidRepo() const {
    return m_dvidRepo;
  }

  std::string mapAddress(const std::string &address) const;

private:
  ZDvidTarget m_emptyDvidTarget;
  std::vector<ZDvidTarget> m_dvidRepo;
  std::map<std::string, std::string> m_addressMap;
//  std::string m_bodyLabelName;
  const static char *m_dvidRepoKey;
  const static char *m_ipKey;
};

#endif // ZFLYEMCONFIG_H
