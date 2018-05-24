#ifndef ZSTACKDOCMENUFACTORY_H
#define ZSTACKDOCMENUFACTORY_H

#include "neutube.h"
#include "zactionfactory.h"

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;
class QWidget;
class Z3DWindow;
class ZSingleSwcNodeActionActivator;


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

  virtual QMenu* makeContextMenu(Z3DWindow *window, QMenu *menu);

protected:
  void addAction(
      const QList<ZActionFactory::EAction> &actionList,
      ZStackPresenter *presenter, QMenu *menu);
  void addAction(const QList<ZActionFactory::EAction> &actionList,
                 Z3DWindow *window, QMenu *menu);

private:
  void init();

protected:
  ZSingleSwcNodeActionActivator *m_singleSwcNodeActionActivator;
  bool m_isAdmin;
};

#endif // ZSTACKDOCMENUFACTORY_H
