#ifndef ZDVIDSTORE_H
#define ZDVIDSTORE_H

#include <map>
#include "zjsonobject.h"

class ZDvidStore
{
public:
  ZDvidStore();

private:
  std::map<std::string, ZJsonObject> m_dataInfo;
};

#endif // ZDVIDSTORE_H
