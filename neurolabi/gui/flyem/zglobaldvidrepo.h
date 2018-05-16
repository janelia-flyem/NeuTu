#ifndef ZGLOBALDVIDREPO_H
#define ZGLOBALDVIDREPO_H

#include <map>
#include <string>

#include "dvid/zdvidtarget.h"

class ZGlobalDvidRepo
{
public:
  ZGlobalDvidRepo();

  static ZGlobalDvidRepo& GetInstance() {
    static ZGlobalDvidRepo g;

    return g;
  }

  void init();

  void setRepo(const std::string &name, const ZDvidTarget &target);
  void addRepo(const ZDvidTarget &target);

  const ZDvidTarget& getDvidTarget(const std::string &name);

public: //iterator
  using const_iterator = std::map<std::string, ZDvidTarget>::const_iterator;

  const_iterator begin() const;
  const_iterator end() const;

private:
  std::map<std::string, ZDvidTarget> m_dvidRepo;
  ZDvidTarget m_emptyTarget;
};

#endif // ZGLOBALDVIDREPO_H
