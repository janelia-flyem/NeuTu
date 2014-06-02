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

private:
  ZDvidTarget m_dvidTarget;
};

#endif // ZFLYEMCONFIG_H
