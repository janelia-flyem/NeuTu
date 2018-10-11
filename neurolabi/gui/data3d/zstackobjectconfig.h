#ifndef ZSTACKOBJECTCONFIG_H
#define ZSTACKOBJECTCONFIG_H

#include <string>

#include "zstackobjectrole.h"

class ZStackObject;

class ZStackObjectConfig
{
public:
  ZStackObjectConfig();

  void configure(ZStackObject *obj);
  void configureSeed(ZStackObject *obj, uint64_t label);

private:
  std::string m_source;
  std::string m_objectClass;
};

#endif // ZSTACKOBJECTCONFIG_H
