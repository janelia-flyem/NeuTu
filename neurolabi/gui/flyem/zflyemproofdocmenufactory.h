#ifndef ZFLYEMPROOFDOCMENUFACTORY_H
#define ZFLYEMPROOFDOCMENUFACTORY_H

#include <QList>
#include "zactionfactory.h"
#include "zstackdocmenufactory.h"

class Z3DWindow;

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

//  QMenu* makeContextMenu(Z3DWindow *window, QMenu *menu);

};

#endif // ZFLYEMPROOFDOCMENUFACTORY_H
