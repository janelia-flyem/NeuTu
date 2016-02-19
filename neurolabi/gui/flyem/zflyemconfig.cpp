#include "zflyemconfig.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

const char* ZFlyEmConfig::m_dvidRepoKey = "dvid repo";
const char* ZFlyEmConfig::m_ipKey = "ip";

ZFlyEmConfig::ZFlyEmConfig()
{
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
  m_dvidRepo.clear();

  ZJsonObject obj;
  if (obj.load(filePath)) {
    if (obj.hasKey(m_ipKey)) {
      ZJsonObject ipJson(obj.value(m_ipKey));
      std::map<std::string, json_t*> entryMap = ipJson.toEntryMap(false);
      for (std::map<std::string, json_t*>::const_iterator iter = entryMap.begin();
           iter != entryMap.end(); ++iter) {
        std::string ip = ZJsonParser::stringValue(iter->second);
        if (!ip.empty()) {
          m_addressMap[iter->first] = ip;
        } else {
          std::cout << "Invalid ip config? " << iter->first << std::endl;
        }
      }
    }

    if (obj.hasKey(m_dvidRepoKey)) {
      ZJsonArray dvidArray(obj[m_dvidRepoKey], false);
      for (size_t i = 0; i < dvidArray.size(); ++i) {
        ZJsonObject dvidObj(dvidArray.at(i), false);
        ZDvidTarget target;
        target.loadJsonObject(dvidObj);
        if (target.isValid()) {
          target.setEditable(false);
          m_dvidRepo.push_back(target);
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
