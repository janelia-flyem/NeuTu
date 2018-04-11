#ifndef ZSTACKDOCNULLMENUFACTORY_H
#define ZSTACKDOCNULLMENUFACTORY_H

#include "zstackdocmenufactory.h"

class ZStackDocNullMenuFactory : public ZStackDocMenuFactory
{
public:
  ZStackDocNullMenuFactory();
  virtual ~ZStackDocNullMenuFactory() {}

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

#endif // ZSTACKDOCNULLMENUFACTORY_H
