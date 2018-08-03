#ifndef ZFLYEMBODY3DDOCMENUFACTORY_H
#define ZFLYEMBODY3DDOCMENUFACTORY_H

#include "zstackdocmenufactory.h"

class ZFlyEmBody3dDocMenuFactory : public ZStackDocMenuFactory
{
public:
  ZFlyEmBody3dDocMenuFactory();
  virtual ~ZFlyEmBody3dDocMenuFactory();

public:
  QMenu* makeContextMenu(Z3DWindow *window, QMenu *menu) override;
};

#endif // ZFLYEMBODY3DDOCMENUFACTORY_H
