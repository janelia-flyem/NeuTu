#ifndef ZFLYEMCONFIG_H
#define ZFLYEMCONFIG_H

#include <vector>
#include <map>
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#ifdef _QT_GUI_USED_
#include "flyem/zneutuservice.h"
#endif

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

//  void loadConfig(const std::string &filePath);
  void loadConfig();

  void setConfigPath(const std::string &filePath) {
    m_configPath = filePath;
  }

  std::string getConfigPath() const {
    return m_configPath;
  }

  void setDefaultConfigPath(const std::string &path);
  std::string getDefaultConfigPath() const {
    return m_defaultConfigPath;
  }

  inline const std::vector<ZDvidTarget> &getDvidRepo() const {
    return m_dvidRepo;
  }

  std::string mapAddress(const std::string &address) const;

  std::string getDvidRootNode(const std::string &name) const;

  void setTaskServer(const std::string &taskServer);
  std::string getTaskServer() const;

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

  bool anayzingMb6() const {
    return m_analyzingMb6;
  }

  void setAnalyzingMb6(bool on) {
    m_analyzingMb6 = on;
  }

  void useDefaultConfig(bool on) {
    m_usingDefaultConfig = on;
  }

  bool usingDefaultConfig() const {
    return m_usingDefaultConfig;
  }

//  std::string getSplitResultUrl(const ZDvidTarget &target, uint64_t bodyId);

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
//  std::string m_taskServer;
  std::string m_configPath;
  std::string m_defaultConfigPath;
  bool m_usingDefaultConfig;
  std::string m_defaultLibrarian;
  std::string m_userName;

  bool m_analyzingMb6;
//  std::string m_neutuServer;
//  std::string m_bodyLabelName;
  const static char *DVID_REPO_KEY;
  const static char *DVID_ROOT_KEY;
  const static char *IP_KEY;
  const static char *LIBRARIAN_KEY;
  const static char *MB6_KEY;
  const static char *TASK_SERVER_KEY;
};

#endif // ZFLYEMCONFIG_H
