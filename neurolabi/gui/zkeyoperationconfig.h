#ifndef ZKEYOPERATIONCONFIG_H
#define ZKEYOPERATIONCONFIG_H

#include "zkeyoperation.h"

class ZKeyOperationMap;

class ZKeyOperationConfig
{
public:
  ZKeyOperationConfig();
  virtual ~ZKeyOperationConfig() {}

  virtual void configure(
      ZKeyOperationMap &map, ZKeyOperation::EGroup group);

protected:
  static void ConfigureActiveStrokeMap(ZKeyOperationMap &map);
  static void ConfigureSwcNodeMap(ZKeyOperationMap &map);
  static void ConfigureObjectMap(ZKeyOperationMap &map);
  static void ConfigureStackMap(ZKeyOperationMap &map);
};

#endif // ZKEYOPERATIONCONFIG_H
