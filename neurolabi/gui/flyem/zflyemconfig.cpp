#include "zflyemconfig.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "neutubeconfig.h"

const char* ZFlyEmConfig::m_dvidRepoKey = "dvid repo";
const char* ZFlyEmConfig::m_ipKey = "ip";
const char* ZFlyEmConfig::m_librarianKey = "librarian";
const char* ZFlyEmConfig::m_dvidRootKey = "dvid root";

ZFlyEmConfig::ZFlyEmConfig()
{
  init();
}

void ZFlyEmConfig::init()
{
//  m_neutuService.setServer("http://zhaot-ws1:8080");
//  m_neutuService.updateStatus();
//  m_neutuServer = "http://zhaot-ws1:8080";
}

void ZFlyEmConfig::setDvidTarget(const std::string &repo)
{
  m_emptyDvidTarget.setFromSourceString(repo);
}

void ZFlyEmConfig::setDvidTarget(const ZDvidTarget &target)
{
  m_emptyDvidTarget = target;
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

void ZFlyEmConfig::loadConfig(const std::string &filePath)
{
  m_configPath = filePath;
  NeutubeConfig::SetFlyEmConfigPath(filePath.c_str());

  m_dvidRepo.clear();
  m_rootMap.clear();
  m_addressMap.clear();

  if (!filePath.empty()) {
    ZJsonObject obj;
    if (obj.load(filePath)) {
      if (obj.hasKey(m_librarianKey)) {
        m_defaultLibrarian = ZJsonParser::stringValue(obj[m_librarianKey]);
      }
      if (obj.hasKey(m_ipKey)) {
        ZJsonObject ipJson(obj.value(m_ipKey));
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

      if (obj.hasKey(m_dvidRootKey)) {
        ZJsonObject rootJson(obj.value(m_dvidRootKey));

        std::map<std::string, json_t*> entryMap = rootJson.toEntryMap(false);
        for (std::map<std::string, json_t*>::const_iterator
             iter = entryMap.begin(); iter != entryMap.end(); ++iter) {
          std::string root = ZJsonParser::stringValue(iter->second);
          if (!root.empty()) {
            m_rootMap[iter->first] = root;
          }
        }
      }

      if (obj.hasKey(m_dvidRepoKey)) {
        ZJsonArray dvidArray(obj[m_dvidRepoKey], ZJsonValue::SET_INCREASE_REF_COUNT);
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

std::string ZFlyEmConfig::getDvidRootNode(const std::string &name) const
{
  if (m_rootMap.count(name) > 0) {
    return m_rootMap.at(name);
  }

  return "";
}

#ifdef _QT_GUI_USED_
void ZFlyEmConfig::setServer(const std::string &server)
{
  NeutubeConfig::SetNeuTuServer(server.c_str());
  getNeutuService().setServer(server);
}
#endif
