#ifndef ZKEYOPERATIONCONFIG_H
#define ZKEYOPERATIONCONFIG_H

#include "zkeyoperation.h"

class ZKeyOperationMap;

class ZKeyOperationConfig
{
public:
  ZKeyOperationConfig();

  static void Configure(
      ZKeyOperationMap &map, ZKeyOperation::EGroup group);

private:
  static void ConfigureSwcNodeMap(ZKeyOperationMap &map);
  static void ConfigureStackMap(ZKeyOperationMap &map);
  static void ConfigureFlyEmBookmarkMap(ZKeyOperationMap &map);
};

#endif // ZKEYOPERATIONCONFIG_H
