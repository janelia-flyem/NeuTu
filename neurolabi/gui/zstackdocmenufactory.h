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
  virtual ~ZStackDocMenuFactory() {}

  inline void setSingleSwcNodeActionActivator(
      ZSingleSwcNodeActionActivator *activator) {
    m_singleSwcNodeActionActivator = activator;
  }

  void setAdminState(bool state) { m_isAdmin = state; }
  bool isAdmin() const { return m_isAdmin; }

public:
  QMenu* makeSwcNodeContextMenu(
      ZStackDoc *doc, QWidget *parentWidget, QMenu *menu);
  QMenu* makeSwcNodeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  QMenu* makeSrokePaintContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  virtual QMenu* makeStackContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  virtual QMenu* makeBodyContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  virtual QMenu* makeSynapseContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);

  virtual QMenu* makeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);

private:
  void init();

protected:
  ZSingleSwcNodeActionActivator *m_singleSwcNodeActionActivator;
  bool m_isAdmin;
};

#endif // ZSTACKDOCMENUFACTORY_H
