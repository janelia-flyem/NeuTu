#ifndef ZFLYEMPROOFDOCMENUFACTORY_H
#define ZFLYEMPROOFDOCMENUFACTORY_H

#include <QList>
#include "zactionfactory.h"
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

private:
  void addAction(
      const QList<ZActionFactory::EAction> &actionList,
      ZStackPresenter *presenter, QMenu *menu);
};

#endif // ZFLYEMPROOFDOCMENUFACTORY_H
