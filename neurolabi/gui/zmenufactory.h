#ifndef ZMENUFACTORY_H
#define ZMENUFACTORY_H

#include <QVector>
#include <QString>

#include "zactionfactory.h"

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;
class QWidget;
class Z3DWindow;
class ZMenuConfig;
class QAction;

class ZMenuFactory
{
private: //disabled
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

  static QMenu* MakeMenu(const ZMenuConfig &config, QMenu *menu);

  template<typename M, typename T>
  static QMenu* MakeMenu(const M &config, T *source, QMenu *menu);
  /*
  virtual QMenu* makeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);

  virtual QMenu* makeContextMenu(Z3DWindow *window, QMenu *menu);
*/
protected:
  void addAction(
      const QList<ZActionFactory::EAction> &actionList,
      ZStackPresenter *presenter, QMenu *menu);
  void addAction(const QList<ZActionFactory::EAction> &actionList,
                 Z3DWindow *window, QMenu *menu);

  static void AddAction(QMenu *menu, ZActionFactory::EAction actionKey);
  static void AddAction(
      QMenu *menu, QAction *action, ZActionFactory::EAction actionKey);

  static QMenu* InitMenu(QMenu *menu);
  static QMenu *AddSubmenu(QMenu *menu, const QString &name);

  template<typename T>
  static void AddAction(
      QMenu *menu, ZActionFactory::EAction actionKey, T *source);

private:
  void init();

protected:
  bool m_isAdmin;
};

template<typename T>
void ZMenuFactory::AddAction(
    QMenu *menu, ZActionFactory::EAction actionKey, T *source)
{
  QAction *action = source->getAction(actionKey);
  AddAction(menu, action, actionKey);
}

template<typename M, typename T>
QMenu* ZMenuFactory::MakeMenu(const M &config, T *source, QMenu *menu)
{
  menu = InitMenu(menu);

  QString submenuName;
  QMenu *submenu = NULL;
  for (const auto &item : config) {
    const QString &groupName = item.first;
    ZActionFactory::EAction actionKey = item.second;

    if (!groupName.isEmpty()) {
      if (submenuName != groupName) {
        submenu = AddSubmenu(menu, groupName);
        submenuName = groupName;
      }
      AddAction(submenu, actionKey, source);
    } else {
      AddAction(menu, actionKey, source);
    }
  }

  return menu;
}



#endif // ZMENUFACTORY_H
