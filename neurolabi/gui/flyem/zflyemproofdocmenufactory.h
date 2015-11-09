#ifndef ZFLYEMPROOFDOCMENUFACTORY_H
#define ZFLYEMPROOFDOCMENUFACTORY_H

#include "zstackdocmenufactory.h"

class ZFlyEmProofDocMenuFactory : public ZStackDocMenuFactory
{
public:
  ZFlyEmProofDocMenuFactory();
  virtual ~ZFlyEmProofDocMenuFactory() {}

public:
  QMenu* makeBodyContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
};

#endif // ZFLYEMPROOFDOCMENUFACTORY_H
