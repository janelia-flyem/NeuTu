#ifndef ZSTACKDOCMENUFACTORY_H
#define ZSTACKDOCMENUFACTORY_H

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;
class QWidget;

#include "neutube.h"
#include "zactionactivator.h"

/*!
 * \brief Class of creating menus for ZStackDoc
 */
class ZStackDocMenuFactory
{
public:
  ZStackDocMenuFactory();

  inline void setSingleSwcNodeActionActivator(
      ZSingleSwcNodeActionActivator *activator) {
    m_singleSwcNodeActionActivator = activator;
  }

public:
  QMenu* makeSwcNodeContextMenu(
      ZStackDoc *doc, QWidget *parentWidget, QMenu *menu);
  QMenu* makeSwcNodeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  QMenu* makeSrokePaintContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  QMenu* makeStackContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);

private:
  ZSingleSwcNodeActionActivator *m_singleSwcNodeActionActivator;
};

#endif // ZSTACKDOCMENUFACTORY_H
