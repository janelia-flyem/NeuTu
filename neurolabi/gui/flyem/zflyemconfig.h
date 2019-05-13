#ifndef ZFLYEMCONFIG_H
#define ZFLYEMCONFIG_H

#include <vector>
#include <map>
#ifdef _QT_GUI_USED_
#include <QMap>
#include <QSize>
#endif

#include "dvid/zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#ifdef _QT_GUI_USED_
#include "flyem/zneutuservice.h"
#include "neutuse/taskwriter.h"
#endif

class ZFlyEmConfig
{
public:
  ZFlyEmConfig();
  ~ZFlyEmConfig();

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
  void loadUserSettings();

  void setConfigPath(const std::string &filePath) {
    m_configPath = filePath;
  }

  std::string getConfigPath() const {
    return m_configPath;
  }

  std::string getActiveConfigPath() const;

  void setDefaultConfigPath(const std::string &path);
  std::string getDefaultConfigPath() const {
    return m_defaultConfigPath;
  }

  inline const std::vector<ZDvidTarget> &getDvidRepo() const {
    return m_dvidRepo;
  }

  std::string mapAddress(const std::string &address) const;

  std::string getDvidRootNode(const std::string &name) const;

  void setDefaultTaskServer(const std::string &taskServer);
  bool hasDefaultTaskServer() const;

  void setCustomTaskServer(const std::string &taskServer);
  std::string getTaskServer() const;
  std::string getTaskServer(bool usingDefault) const;

  std::string getDefaultLibrarian() const {
    return m_defaultLibrarian;
  }

  std::string getNeuroglancerServer() const {
    return m_neuroglancerServer;
  }

#ifdef _QT_GUI_USED_
  ZNeutuService& getNeutuService() {
    return m_neutuService;
  }

  const ZNeutuService& getNeutuService() const {
    return m_neutuService;
  }

  neutuse::TaskWriter& getNeutuseWriter() {
    return m_neutuseWriter;
  }

  const neutuse::TaskWriter& getNeutuseWriter() const {
    return m_neutuseWriter;
  }

  bool hasNormalService() const;
  void updateServiceStatus();

  void activateNeuTuServerForce();
  void activateNeuTuServer();

//  void setRemoteServer(const std::string &server);
#endif

  void setDefaultNeuTuServer(const std::string &server);
  bool hasDefaultNeuTuServer() const;

  void setCustomNeuTuServer(const std::string &server);
  std::string getNeuTuServer() const;
  std::string getNeuTuServer(bool usingDefault) const;


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

  void useDefaultNeuTuServer(bool on) {
    m_usingDefaultNeuTuServer = on;
  }

  bool usingDefaultNeuTuServer() const {
    return m_usingDefaultNeuTuServer;
  }

  void useDefaultTaskServer(bool on) {
    m_usingDefaultTaskServer = on;
  }
  bool usingDefaultTaskServer() const {
    return m_usingDefaultTaskServer;
  }

  std::pair<int,int> getCenterCut(const std::string &name) const;
  void setCenterCut(const std::string &name, int cx, int cy);

  void saveSettings() const;
  bool psdNameDetail() const {
    return m_psdNameDetail;
  }

  void setPsdNameDetail(bool on) {
    m_psdNameDetail = on;
  }

  std::string getWindowStyleSheet() const {
    return m_uiStyleSheet;
  }

public:
  const static char *DVID_REPO_KEY;
  const static char *DVID_ROOT_KEY;
  const static char *IP_KEY;
  const static char *LIBRARIAN_KEY;
  const static char *MB6_KEY;
  const static char *TASK_SERVER_KEY;
  const static char *NEUTU_SERVER_KEY;
  const static char *NEUROGLANCER_KEY;
  const static char *CENTERCUT_KEY;
  const static char *UI_KEY;
  const static char *STYLE_KEY;

private:
  void init();

private:
  ZDvidTarget m_emptyDvidTarget;
  std::vector<ZDvidTarget> m_dvidRepo;
  std::map<std::string, std::string> m_addressMap;
  std::map<std::string, std::string> m_rootMap;
  std::map<std::string, std::pair<int,int>> m_centerCut;

#ifdef _QT_GUI_USED_
  ZNeutuService m_neutuService;
  neutuse::TaskWriter m_neutuseWriter;
  bool m_neutuServerChecked = false;
#endif
//  std::string m_taskServer;
  std::string m_configPath;
  std::string m_defaultConfigPath;
  bool m_usingDefaultConfig;
  std::string m_defaultLibrarian;
  std::string m_userName;
  std::string m_neuroglancerServer;
  bool m_usingDefaultNeuTuServer = true;
  std::string m_defaultNeuTuServer;
  bool m_usingDefaultTaskServer = true;
  std::string m_defaultTaskServer;
  std::string m_uiStyleSheet;

  bool m_analyzingMb6;
  bool m_psdNameDetail = false;

//  std::string m_neutuServer;
//  std::string m_bodyLabelName;

};

#endif // ZFLYEMCONFIG_H
