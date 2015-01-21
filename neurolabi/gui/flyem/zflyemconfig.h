#ifndef ZFLYEMCONFIG_H
#define ZFLYEMCONFIG_H

#include <vector>
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
    return m_dvidTarget;
  }

  void loadConfig(const std::string &filePath);

  inline const std::vector<ZDvidTarget> &getDvidRepo() const {
    return m_dvidRepo;
  }

private:
  ZDvidTarget m_dvidTarget;
  std::vector<ZDvidTarget> m_dvidRepo;
  std::string m_bodyLabelName;
  const static char *m_dvidRepoKey;
};

#endif // ZFLYEMCONFIG_H
