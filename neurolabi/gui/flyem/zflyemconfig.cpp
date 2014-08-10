#include "zflyemconfig.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"

ZFlyEmConfig::ZFlyEmConfig()
{
}

void ZFlyEmConfig::setDvidTarget(const std::string &repo)
{
  m_dvidTarget.set(repo);
}

void ZFlyEmConfig::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

void ZFlyEmConfig::print() const
{
  std::cout << "FlyEM Configuration:" << std::endl;
  if (m_dvidTarget.isValid()) {
    std::cout << "  ";
    m_dvidTarget.print();
  } else {
    std::cout << "  No DVID repository." << std::endl;
  }
}
