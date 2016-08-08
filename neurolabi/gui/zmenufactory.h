#ifndef ZMENUFACTORY_H
#define ZMENUFACTORY_H

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;
class QWidget;
class Z3DWindow;

#include "zactionfactory.h"

class ZMenuFactory
{
public:
  ZMenuFactory();
  virtual ~ZMenuFactory() {}

  void setAdminState(bool state) { m_isAdmin = state; }
  bool isAdmin() const { return m_isAdmin; }

public:
  QMenu* makeSwcNodeContextMenu(
      ZStackDoc *doc, QWidget *parentWidget, QMenu *menu);
  QMenu* makeSwcNodeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  QMenu* makeStrokePaintContextMenu(
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
  bool m_isAdmin;
};

#endif // ZMENUFACTORY_H
