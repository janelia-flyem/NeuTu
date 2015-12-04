#include "zflyemconfig.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"

const char* ZFlyEmConfig::m_dvidRepoKey = "dvid repo";

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
