#ifndef ZFLYEMBODY3DDOCMENUFACTORY_H
#define ZFLYEMBODY3DDOCMENUFACTORY_H

#include "zstackdocmenufactory.h"

class ZFlyEmBody3dDoc;
class ZMenuConfig;

class ZFlyEmBody3dDocMenuFactory : public ZStackDocMenuFactory
{
public:
  ZFlyEmBody3dDocMenuFactory();
  virtual ~ZFlyEmBody3dDocMenuFactory();

public:
  QMenu* makeContextMenu(Z3DWindow *window, QMenu *menu) override;

private:
  ZMenuConfig getConfig(ZFlyEmBody3dDoc *doc);
  static bool ReadyForAction(
      ZFlyEmBody3dDoc *doc, ZActionFactory::EAction action);
};

#endif // ZFLYEMBODY3DDOCMENUFACTORY_H
