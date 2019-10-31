#include "zflyemconfig.h"

#include <iostream>
#include <cstdlib>
#include <regex>

#include "common/utilities.h"
#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "dvid/zdvidurl.h"
#include "zstring.h"

#ifdef _QT_GUI_USED_
#include "neutube.h"
#endif

/* Implementation Details
 *
 * ZFlyEmConfig is used to manage configuration related to flyem proofreading.
 *
 */


const char* ZFlyEmConfig::DVID_REPO_KEY = "dvid repo";
const char* ZFlyEmConfig::IP_KEY = "ip";
const char* ZFlyEmConfig::LIBRARIAN_KEY = "librarian";
const char* ZFlyEmConfig::DVID_ROOT_KEY = "dvid root";
const char* ZFlyEmConfig::MB6_KEY = "mb6_paper";
const char* ZFlyEmConfig::TASK_SERVER_KEY = "task server";
const char* ZFlyEmConfig::NEUTU_SERVER_KEY = "neutu_server";
const char* ZFlyEmConfig::NEUROGLANCER_KEY = "neuroglancer server";
const char* ZFlyEmConfig::CENTERCUT_KEY = "flyem::centercut";
const char* ZFlyEmConfig::UI_KEY = "ui";
const char* ZFlyEmConfig::STYLE_KEY = "style";

ZFlyEmConfig::ZFlyEmConfig()
{
  init();
}

ZFlyEmConfig::~ZFlyEmConfig()
{
//  saveSettings();
}

void ZFlyEmConfig::init()
{
#ifdef _QT_GUI_USED_
  m_userName = neutu::GetCurrentUserName();
#endif
//  m_neutuService.setServer("http://zhaot-ws1:8080");
//  m_neutuService.updateStatus();
//  m_neutuServer = "http://zhaot-ws1:8080";
  m_analyzingMb6 = false;
  m_usingDefaultConfig = true;
}

void ZFlyEmConfig::setDvidTarget(const std::string &repo)
{
  m_emptyDvidTarget.setFromSourceString(repo);
}

void ZFlyEmConfig::setDvidTarget(const ZDvidTarget &target)
{
  m_emptyDvidTarget = target;
}

std::string ZFlyEmConfig::getUserName() const
{
  return m_userName;
}

void ZFlyEmConfig::setDefaultConfigPath(const std::string &path)
{
  m_defaultConfigPath = path;
}

void ZFlyEmConfig::print() const
{
  std::cout << "FlyEM Configuration:" << std::endl;
  std::cout << "  Source: " << getActiveConfigPath() << std::endl;
  std::cout << "  " << m_dvidRepo.size() << " DVID repos" << std::endl;
  std::cout << "  " << "Default neutu server: " << m_defaultNeuTuServer << std::endl;
  std::cout << "  " << "Neutu server to use: " << getNeuTuServer() << std::endl;
  std::cout << "  " << "Default task server: " << m_defaultTaskServer << std::endl;
  std::cout << "  " << "Task server to use: " << getTaskServer() << std::endl;

}

std::string ZFlyEmConfig::getActiveConfigPath() const
{
  std::string filePath;
  if (usingDefaultConfig()) {
    filePath = m_defaultConfigPath;
  } else {
    filePath = m_configPath;
  }

  return filePath;
}

void ZFlyEmConfig::loadConfig()
{
  m_dvidRepo.clear();
  m_rootMap.clear();
  m_addressMap.clear();

  std::string filePath = getActiveConfigPath();

  std::cout << "Loading FlyEM config: " << filePath << std::endl;

  if (!filePath.empty()) {
    ZJsonObject obj;
    if (obj.load(filePath)) {
      if (obj.hasKey(LIBRARIAN_KEY)) {
        m_defaultLibrarian = ZJsonParser::stringValue(obj[LIBRARIAN_KEY]);
      }
      if (obj.hasKey(IP_KEY)) {
        ZJsonObject ipJson(obj.value(IP_KEY));
        std::map<std::string, json_t*> entryMap = ipJson.toEntryMap(false);
        for (std::map<std::string, json_t*>::const_iterator
             iter = entryMap.begin(); iter != entryMap.end(); ++iter) {
          std::string ip = ZJsonParser::stringValue(iter->second);
          if (!ip.empty()) {
            m_addressMap[iter->first] = ip;
          } else {
            std::cout << "Invalid ip config? " << iter->first << std::endl;
          }
        }
      }

      if (obj.hasKey(MB6_KEY)) {
        setAnalyzingMb6(ZJsonParser::booleanValue(obj[MB6_KEY]));
      }

      if (obj.hasKey(TASK_SERVER_KEY)) {
        setDefaultTaskServer(ZJsonParser::stringValue(obj[TASK_SERVER_KEY]));
      }

      if (obj.hasKey(NEUTU_SERVER_KEY)) {
        setDefaultNeuTuServer(ZJsonParser::stringValue(obj[NEUTU_SERVER_KEY]));
      }

      if (const char* setting = std::getenv("NEUROGLANCER_SERVER")) {
        m_neuroglancerServer = setting;
        m_neuroglancerServerConfigSource = neutu::EConfigSource::ENV_VAR;
      } else {
        if (obj.hasKey(NEUROGLANCER_KEY)) {
          m_neuroglancerServer =
              ZJsonParser::stringValue(obj[NEUROGLANCER_KEY]);
//          m_neuroglancerServer = m_defaultNeuroglancerServer;
          m_neuroglancerServerConfigSource = neutu::EConfigSource::CONFILG_FILE;
        }
      }

      if (obj.hasKey(DVID_ROOT_KEY)) {
        ZJsonObject rootJson(obj.value(DVID_ROOT_KEY));

        std::map<std::string, json_t*> entryMap = rootJson.toEntryMap(false);
        for (std::map<std::string, json_t*>::const_iterator
             iter = entryMap.begin(); iter != entryMap.end(); ++iter) {
          std::string root = ZJsonParser::stringValue(iter->second);
          if (!root.empty()) {
            m_rootMap[iter->first] = root;
          }
        }
      }

      if (obj.hasKey(DVID_REPO_KEY)) {
        ZJsonArray dvidArray(obj[DVID_REPO_KEY], ZJsonValue::SET_INCREASE_REF_COUNT);
        for (size_t i = 0; i < dvidArray.size(); ++i) {
          ZJsonObject dvidObj(dvidArray.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
          ZDvidTarget target;
          target.loadJsonObject(dvidObj);
          if (target.getGrayScaleName().empty()) {
            //Use default name for back compatibility
            target.setGrayScaleName(ZDvidData::GetName(ZDvidData::ERole::GRAY_SCALE));
          }
//          std::string mapped = getDvidRootNode(target.getUuid());
//          if (!mapped.empty()) {
//            target.setUuid(mapped);
//          }
          if (target.isValid()) {
            target.setEditable(false);
            m_dvidRepo.push_back(target);
          }

#ifdef _DEBUG_
          std::cout << "Name: " << target.getName() << " ";
          std::cout << "User:";
          const std::set<std::string> &userSet = target.getUserNameSet();
          std::for_each(userSet.begin(), userSet.end(),
                        [](const std::string &user) {std::cout << user << " ";});

          target.print();
#endif
        }
      }

      if (obj.hasKey(UI_KEY)) {
        ZJsonObject uiObj(obj.value(UI_KEY));
        if (uiObj.hasKey(STYLE_KEY)) {
          m_uiStyleSheet = ZJsonParser::stringValue(uiObj[STYLE_KEY]);
        }
      }
    }
  }
}

void ZFlyEmConfig::loadUserSettings()
{
#ifdef _QT_GUI_USED_
  //Load local settings
  if (NeutubeConfig::GetSettings().contains(CENTERCUT_KEY)) {
    QMap<QString, QVariant> centercutMap =
        NeutubeConfig::GetSettings().value(CENTERCUT_KEY).toMap();

    for (auto iter = centercutMap.begin(); iter != centercutMap.end();
         ++iter) {
      QSize s = iter.value().toSize();
      m_centerCut[iter.key().toStdString()] =
          std::pair<int,int>(s.width(), s.height());
    }
  }
#endif
}

std::pair<int,int> ZFlyEmConfig::getCenterCut(const std::string &name) const
{
  if (m_centerCut.count(name) > 0) {
    return m_centerCut.at(name);
  }

  return std::pair<int,int>(256, 256);
}

void ZFlyEmConfig::setCenterCut(const std::string &name, int cx, int cy)
{
  m_centerCut[name] = std::pair<int,int>(cx, cy);
}

std::string ZFlyEmConfig::mapAddress(const std::string &address) const
{
  if (m_addressMap.count(address) > 0) {
    return m_addressMap.at(address);
  }

  return address;
}

void ZFlyEmConfig::setDefaultTaskServer(const std::string &taskServer)
{
  m_defaultTaskServer = taskServer;
}

bool ZFlyEmConfig::hasDefaultTaskServer() const
{
  return !m_defaultTaskServer.empty();
}

void ZFlyEmConfig::setCustomTaskServer(const std::string &taskServer)
{
#ifdef _DEBUG_
  std::cout << "Setting task server to " << taskServer << std::endl;
#endif
//  m_taskServer = taskServer;
#ifdef _QT_GUI_USED_
  NeutubeConfig::SetTaskServer(taskServer.c_str());
#endif

#ifdef _DEBUG_
  std::cout << "Task server set to " << getTaskServer() << std::endl;
#endif
}

std::string ZFlyEmConfig::getTaskServer(bool usingDefault) const
{
  if (usingDefault) {
    return m_defaultTaskServer;
  } else {
#ifdef _QT_GUI_USED_
    return NeutubeConfig::GetTaskServer().toStdString();
#else
    return "";
#endif
  }
}

std::string ZFlyEmConfig::getTaskServer() const
{
  return getTaskServer(usingDefaultTaskServer());
}

std::string ZFlyEmConfig::getDvidRootNode(const std::string &name) const
{
  if (m_rootMap.count(name) > 0) {
    return m_rootMap.at(name);
  }

  return "";
}

void ZFlyEmConfig::saveSettings() const
{
#ifdef _QT_GUI_USED_
  QMap<QString, QVariant> setting;
  for (const auto &cc : m_centerCut) {
    setting[cc.first.c_str()] = QSize(cc.second.first, cc.second.second);
  }
  if (!setting.isEmpty()) {
    NeutubeConfig::GetSettings().setValue(CENTERCUT_KEY, setting);
  }
#endif
  /*
  for (const auto &cc : m_centerCut) {
    NeutubeConfig::GetSettings().setValue(
          CENTERCUT_KEY + "::" + cc.first,
          QSize(cc.second.first, cc.second.second));
 */
}

/*
std::string ZFlyEmConfig::getSplitResultUrl(
    const ZDvidTarget &target, uint64_t bodyId)
{
}
*/

void ZFlyEmConfig::setDefaultNeuTuServer(const std::string &server)
{
  m_defaultNeuTuServer = server;
}

bool ZFlyEmConfig::hasDefaultNeuTuServer() const
{
  return !m_defaultNeuTuServer.empty();
}

std::string ZFlyEmConfig::getNeuTuServer(bool usingDefault) const
{
  if (usingDefault) {
    return m_defaultNeuTuServer;
  } else {
#ifdef _QT_GUI_USED_
    return NeutubeConfig::GetNeuTuServer().toStdString();
#else
    return "";
#endif
  }
}

std::string ZFlyEmConfig::getNeuTuServer() const
{
  return getNeuTuServer(usingDefaultNeuTuServer());
//  return m_remoteServer;
}

void ZFlyEmConfig::setCustomNeuTuServer(const std::string &server)
{
#ifdef _QT_GUI_USED_
  NeutubeConfig::SetNeuTuServer(server.c_str());
#endif
}

#ifdef _QT_GUI_USED_

ZFlyEmConfig::EServiceStatus ZFlyEmConfig::getNeutuseStatus() const
{
  return m_neutuseStatus;
}

bool ZFlyEmConfig::isNeutuseOnline() const
{
  return (getNeutuseStatus() == EServiceStatus::ONLINE_NET) ||
      (getNeutuseStatus() == EServiceStatus::ONLINE_LOCAL);
}

/*
bool ZFlyEmConfig::hasNormalService() const
{
  if (getNeutuService().isNormal() || m_neutuseWriter.ready()) {
    return true;
  }

  return false;
}
*/

void ZFlyEmConfig::updateCheckedNeutuseStatus()
{
  if (m_neutuseWriter.ready()) {
    m_neutuseStatus =
        neutu::UsingLocalHost(m_neutuseWriter.getServerAddress())
          ? EServiceStatus::ONLINE_LOCAL
          : EServiceStatus::ONLINE_NET;
//    neutuseOpened = true;
  } else {
    m_neutuseStatus = EServiceStatus::OFFLINE;
  }
}

void ZFlyEmConfig::updateNeutuseStatus()
{
//  GET_FLYEM_CONFIG.getNeutuService().updateStatus();

  m_neutuseWriter.testConnection();
  updateCheckedNeutuseStatus();
}

void ZFlyEmConfig::activateNeutuse(bool forLocalTarget)
{
  if (m_neutuseStatus == EServiceStatus::UNCHECKED) {
    activateNeutuseForce(forLocalTarget);
  }
}

bool ZFlyEmConfig::neutuseAvailable(bool forLocalTarget) const
{
  if (forLocalTarget) {
    return m_neutuseStatus == EServiceStatus::ONLINE_LOCAL;
  }

  return isNeutuseOnline();
}

bool ZFlyEmConfig::neutuseAvailable(const ZDvidTarget &target) const
{
  return neutuseAvailable(neutu::UsingLocalHost(target.getAddress()));
}

void ZFlyEmConfig::activateNeutuseForce(bool forLocalTarget)
{
  std::string server = getNeuTuServer();
  std::vector<std::string> serverList = ZString::Tokenize(server, ';');
//  bool neutuseOpened = false;
//  bool serviceOpened = false;
  m_neutuseWriter.reset();
//  getNeutuService().reset();
  for (const std::string &server : serverList) {
    bool isLocalService = neutu::UsingLocalHost(server);
    if (ZString(server).startsWith("neutuse:")) {
      bool needOpen = false;
      if (forLocalTarget) {
        if (isLocalService) {
          needOpen = true;
        }
      } else {
        needOpen = true;
      }

      if (needOpen) {
        m_neutuseWriter.open(server.substr(8));
        updateCheckedNeutuseStatus();
      }
    }

      /* else { //obsolete
      if (!serviceOpened && !forLocalTarget) {
        getNeutuService().setServer(server);
        if (getNeutuService().isNormal()) {
          m_neutuServerStatus =
              isLocalService ? EServiceStatus::ONLINE_LOCAL
                             : EServiceStatus::ONLINE_NET;
        }
//        serviceOpened = getNeutuService().isNormal();
//        m_neutuServerChecked = true;
      }
    }*/
  }
}

/*
void ZFlyEmConfig::setRemoteServer(const std::string &server)
{
  NeutubeConfig::SetNeuTuServer(server.c_str());
  std::vector<std::string> serverList = ZString::Tokenize(server, ';');
  bool neutuseOpened = false;
  bool serviceOpened = false;
  m_neutuseWriter.reset();
  getNeutuService().reset();
  for (const std::string &server : serverList) {
    if (ZString(server).startsWith("neutuse:")) {
      if (!neutuseOpened) {
        m_neutuseWriter.open(server.substr(8));
        neutuseOpened = m_neutuseWriter.ready();
      }
    } else {
      if (!serviceOpened) {
        getNeutuService().setServer(server);
        serviceOpened = getNeutuService().isNormal();
      }
    }
  }
}
*/
#endif
