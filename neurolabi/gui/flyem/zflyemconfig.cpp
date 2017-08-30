#include "zflyemconfig.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "neutubeconfig.h"
#include "dvid/zdvidurl.h"

#ifdef _QT_GUI_USED_
#include "neutube.h"
#endif

const char* ZFlyEmConfig::DVID_REPO_KEY = "dvid repo";
const char* ZFlyEmConfig::IP_KEY = "ip";
const char* ZFlyEmConfig::LIBRARIAN_KEY = "librarian";
const char* ZFlyEmConfig::DVID_ROOT_KEY = "dvid root";
const char* ZFlyEmConfig::MB6_KEY = "mb6_paper";
const char* ZFlyEmConfig::TASK_SERVER_KEY = "task server";

ZFlyEmConfig::ZFlyEmConfig()
{
  init();
}


void ZFlyEmConfig::init()
{
#ifdef _QT_GUI_USED_
  m_userName = NeuTube::GetCurrentUserName();
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
  if (m_emptyDvidTarget.isValid()) {
    std::cout << "  ";
    m_emptyDvidTarget.print();
  } else {
    std::cout << "  No DVID repository." << std::endl;
  }
}

void ZFlyEmConfig::loadConfig()
{
  m_dvidRepo.clear();
  m_rootMap.clear();
  m_addressMap.clear();

  std::string filePath;
  if (usingDefaultConfig()) {
    filePath = m_defaultConfigPath;
  } else {
    filePath = m_configPath;
  }

  std::cout << "Loading config: " << filePath << std::endl;

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

      if (getTaskServer().empty()) {
        if (obj.hasKey(TASK_SERVER_KEY)) {
          setTaskServer(ZJsonParser::stringValue(obj[TASK_SERVER_KEY]));
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
//          std::string mapped = getDvidRootNode(target.getUuid());
//          if (!mapped.empty()) {
//            target.setUuid(mapped);
//          }
          if (target.isValid()) {
            target.setEditable(false);
            m_dvidRepo.push_back(target);
          }
        }
      }
    }
  }
}

std::string ZFlyEmConfig::mapAddress(const std::string &address) const
{
  if (m_addressMap.count(address) > 0) {
    return m_addressMap.at(address);
  }

  return address;
}

void ZFlyEmConfig::setTaskServer(const std::string &taskServer)
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

std::string ZFlyEmConfig::getTaskServer() const
{
#ifdef _QT_GUI_USED_
  return NeutubeConfig::GetTaskServer().toStdString();
#else
  return "";
#endif
}

std::string ZFlyEmConfig::getDvidRootNode(const std::string &name) const
{
  if (m_rootMap.count(name) > 0) {
    return m_rootMap.at(name);
  }

  return "";
}

/*
std::string ZFlyEmConfig::getSplitResultUrl(
    const ZDvidTarget &target, uint64_t bodyId)
{
}
*/

#ifdef _QT_GUI_USED_
void ZFlyEmConfig::setServer(const std::string &server)
{
  NeutubeConfig::SetNeuTuServer(server.c_str());
  getNeutuService().setServer(server);
}
#endif
