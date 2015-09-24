#ifndef ZFLYEMKEYOPERATIONCONFIG_H
#define ZFLYEMKEYOPERATIONCONFIG_H

#include "zkeyoperationconfig.h"

class ZFlyEmKeyOperationConfig : public ZKeyOperationConfig
{
public:
  ZFlyEmKeyOperationConfig();

  virtual void configure(
      ZKeyOperationMap &map, ZKeyOperation::EGroup group);

protected:
  static void ConfigureFlyEmBookmarkMap(ZKeyOperationMap &map);
  static void ConfigureFlyEmStackMap(ZKeyOperationMap &map);
};

#endif // ZFLYEMKEYOPERATIONCONFIG_H
