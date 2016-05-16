#ifndef ZFLYEMCONFIG_H
#define ZFLYEMCONFIG_H

#include <vector>
#include <map>
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#include "flyem/zneutuservice.h"

class ZFlyEmConfig
{
public:
  ZFlyEmConfig();

  static ZFlyEmConfig& getInstance() {
    static ZFlyEmConfig config;

    return config;
  }

  void setDvidTarget(const std::string &repo);
  void setDvidTarget(const ZDvidTarget &target);

  void print() const;

  inline const ZDvidTarget& getDvidTarget() const {
    return m_emptyDvidTarget;
  }

  void loadConfig(const std::string &filePath);

  std::string getConfigPath() const {
    return m_configPath;
  }

  inline const std::vector<ZDvidTarget> &getDvidRepo() const {
    return m_dvidRepo;
  }

  std::string mapAddress(const std::string &address) const;

  std::string getDvidRootNode(const std::string &name) const;

  std::string getDefaultLibrarian() const {
    return m_defaultLibrarian;
  }

#ifdef _QT_GUI_USED_
  ZNeutuService& getNeutuService() {
    return m_neutuService;
  }

  void setServer(const std::string &server);
#endif

  std::string getUserName() const;
  /*
  std::string getConfigPath() const {
    return m_configPath;
  }
  */

private:
  void init();

private:
  ZDvidTarget m_emptyDvidTarget;
  std::vector<ZDvidTarget> m_dvidRepo;
  std::map<std::string, std::string> m_addressMap;
  std::map<std::string, std::string> m_rootMap;
#ifdef _QT_GUI_USED_
  ZNeutuService m_neutuService;
#endif
  std::string m_configPath;
  std::string m_defaultLibrarian;
  std::string m_userName;
//  std::string m_neutuServer;
//  std::string m_bodyLabelName;
  const static char *m_dvidRepoKey;
  const static char *m_dvidRootKey;
  const static char *m_ipKey;
  const static char *m_librarianKey;
};

#endif // ZFLYEMCONFIG_H
