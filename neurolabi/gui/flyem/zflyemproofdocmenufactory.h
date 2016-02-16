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
  QMenu* makeSynapseContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  QMenu* makeStackContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);

  QMenu* makeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
};

#endif // ZFLYEMPROOFDOCMENUFACTORY_H
