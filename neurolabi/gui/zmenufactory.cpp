#include "zmenufactory.h"

#include <QMenu>
#include <QWidget>

#include "neutubeconfig.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zactionfactory.h"
#include "z3dwindow.h"
#include "zmenuconfig.h"

ZMenuFactory::ZMenuFactory()
{
  init();
}

void ZMenuFactory::init()
{
  m_isAdmin = false;
}

template <typename T>
static void AddAction(const QList<ZActionFactory::EAction> &actionList,
                      T *source, QMenu *menu)
{
  foreach (ZActionFactory::EAction action, actionList) {
    if (action == ZActionFactory::ACTION_SEPARATOR) {
      menu->addSeparator();
    } else {
      QAction *actionObject = source->getAction(action);
      if (actionObject != NULL) {
        menu->addAction(actionObject);
      }
    }
  }
}

void ZMenuFactory::addAction(
    const QList<ZActionFactory::EAction> &actionList,
    ZStackPresenter *presenter, QMenu *menu)
{
  ::AddAction(actionList, presenter, menu);
}

void ZMenuFactory::addAction(
    const QList<ZActionFactory::EAction> &actionList, Z3DWindow *window, QMenu *menu)
{
  ::AddAction(actionList, window, menu);
}

QMenu* ZMenuFactory::makeSwcNodeContextMenu(
    ZStackDoc *doc, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  menu->addAction(doc->getAction(ZActionFactory::ACTION_DELETE_SWC_NODE));

  menu->addAction(doc->getAction(
                    ZActionFactory::ACTION_DELETE_UNSELECTED_SWC_NODE));

  menu->addAction(doc->getAction(ZActionFactory::ACTION_BREAK_SWC_NODE));
  menu->addAction(doc->getAction(ZActionFactory::ACTION_CONNECT_SWC_NODE));
  menu->addAction(doc->getAction(ZActionFactory::ACTION_MERGE_SWC_NODE));

  menu->addAction(doc->getAction(ZActionFactory::ACTION_INSERT_SWC_NODE));

  QMenu *submenu = new QMenu("Interpolate", menu);
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SWC_INTERPOLATION));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SWC_Z_INTERPOLATION));
  submenu->addAction(doc->getAction(
                       ZActionFactory::ACTION_SWC_POSITION_INTERPOLATION));
  submenu->addAction(doc->getAction(
                       ZActionFactory::ACTION_SWC_RADIUS_INTERPOLATION));

  menu->addMenu(submenu);

  submenu = new QMenu("Select", menu);
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SELECT_DOWNSTREAM));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SELECT_UPSTREAM));
  submenu->addAction(doc->getAction(
                       ZActionFactory::ACTION_SELECT_NEIGHBOR_SWC_NODE));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SELECT_SWC_BRANCH));
  submenu->addAction(doc->getAction(
                       ZActionFactory::ACTION_SELECT_CONNECTED_SWC_NODE));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SELECT_ALL_SWC_NODE));

  menu->addMenu(submenu);

  submenu = new QMenu("Advanced Editing", menu);

  submenu->addAction(doc->getAction(ZActionFactory::ACTION_REMOVE_TURN));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_RESOLVE_CROSSOVER));
  submenu->addAction(doc->getAction((ZActionFactory::ACTION_SET_BRANCH_POINT)));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_CONNECTED_ISOLATED_SWC));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_RESET_BRANCH_POINT));

  menu->addMenu(submenu);

  submenu = new QMenu("Change Property", menu);

  submenu->addAction(doc->getAction(ZActionFactory::ACTION_TRANSLATE_SWC_NODE));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_CHANGE_SWC_SIZE));
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SET_SWC_ROOT));

  menu->addMenu(submenu);

  submenu = new QMenu("Information", menu);
  submenu->addAction(doc->getAction(ZActionFactory::ACTION_SWC_SUMMARIZE));
  submenu->addAction(doc->getAction(
                       ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH));
  submenu->addAction(
        doc->getAction(ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH));

  menu->addMenu(submenu);

  return menu;
}

QMenu* ZMenuFactory::makeSwcNodeContextMenu(
    ZStackPresenter *presenter, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

//  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_SMART_EXTEND_SWC_NODE));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_EXTEND_SWC_NODE));
  menu->addAction(presenter->getAction(
                    ZActionFactory::ACTION_CONNECT_TO_SWC_NODE));
  menu->addAction(presenter->getAction(
                    ZActionFactory::ACTION_CHANGE_SWC_NODE_FOCUS));


  menu->addAction(presenter->getAction(
                    ZActionFactory::ACTION_MOVE_SWC_NODE));


  //  if (GET_APPLICATION_NAME == "Biocytin") {
  menu->addAction(presenter->getAction(
                    ZActionFactory::ACTION_ESTIMATE_SWC_NODE_RADIUS));
//  }

  return menu;
}

QMenu* ZMenuFactory::makeStrokePaintContextMenu(
    ZStackPresenter *presenter, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  menu->addAction(presenter->getAction(ZActionFactory::ACTION_PAINT_STROKE));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_ERASE_STROKE));

  return menu;
}

QMenu* ZMenuFactory::makeStackContextMenu(
    ZStackPresenter *presenter, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  switch (presenter->buddyDocument()->getTag()) {
  case neutube::Document::ETag::FLYEM_SPLIT:
  case neutube::Document::ETag::SEGMENTATION_TARGET:
    menu->addAction(presenter->getAction(ZActionFactory::ACTION_SPLIT_DATA));
    menu->addAction(presenter->getAction(
                      ZActionFactory::ACTION_ADD_SPLIT_SEED));
    break;
  case neutube::Document::ETag::NORMAL:
  case neutube::Document::ETag::BIOCYTIN_STACK:
    menu->addAction(presenter->getAction(ZActionFactory::ACTION_ADD_SWC_NODE));
    menu->addAction(presenter->getAction(
                      ZActionFactory::ACTION_TOGGLE_SWC_SKELETON));
    break;
  default:
    break;
  }

  return menu;
}

QMenu* ZMenuFactory::makeBodyContextMenu(
    ZStackPresenter * /*presenter*/, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  return menu;
}

QMenu* ZMenuFactory::makeSynapseContextMenu(
    ZStackPresenter * /*presenter*/, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  return menu;
}

void ZMenuFactory::AddAction(QMenu *menu, ZActionFactory::EAction actionKey)
{
  if (actionKey == ZActionFactory::ACTION_SEPARATOR) {
    menu->addSeparator();
  } else {
    QAction *action = ZActionFactory::MakeAction(actionKey, menu);
    if (action != NULL) {
      menu->addAction(action);
    }
  }
}

void ZMenuFactory::AddAction(
    QMenu *menu, QAction *action, ZActionFactory::EAction actionKey)
{
  if (actionKey == ZActionFactory::ACTION_SEPARATOR) {
    menu->addSeparator();
  } else {
    if (action != NULL) {
      menu->addAction(action);
    }
  }
}

QMenu* ZMenuFactory::InitMenu(QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  } else {
    menu->clear();
  }

  return menu;
}

QMenu* ZMenuFactory::AddSubmenu(QMenu *menu, const QString &name)
{
  QMenu *submenu = NULL;

  if (!name.isEmpty()) {
    submenu = menu->addMenu(name);
  }

  return submenu;
}

QMenu* ZMenuFactory::MakeMenu(const ZMenuConfig &config, QMenu *menu)
{
  menu = InitMenu(menu);

  QString submenuName;
  QMenu *submenu = NULL;
  for (const auto &item : config) {
    const QString &groupName = item.first;
    ZActionFactory::EAction actionKey = item.second;

    if (!groupName.isEmpty()) {
      if (submenuName != groupName) {
        submenu = new QMenu(groupName, menu);
        menu->addMenu(submenu);
        submenuName = groupName;
      }
      AddAction(submenu, actionKey);
    } else {
      AddAction(menu, actionKey);
    }
  }

  return menu;
}

#if 0
QMenu* ZMenuFactory::makeContextMenu(
    ZStackPresenter * /*presenter*/, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  return menu;
}


QMenu* ZMenuFactory::makeContextMenu(Z3DWindow *window, QMenu *menu)
{
  ZStackDoc *doc = window->getDocument();

  if (doc != NULL) {
    if (menu == NULL) {
      menu = new QMenu(NULL);
    } else {
      menu->clear();
    }

    QList<ZActionFactory::EAction> actionList;

    if (doc->getTag() == neutube::Document::FLYEM_BODY_3D ||
        doc->getTag() == neutube::Document::FLYEM_BODY_3D_COARSE ||
        doc->getTag() == neutube::Document::FLYEM_SKELETON) {
      actionList.append(ZActionFactory::ACTION_SYNAPSE_FILTER);
    }

    if (doc->getTag() == neutube::Document::FLYEM_BODY_3D) {
      actionList.append(ZActionFactory::ACTION_SHOW_NORMAL_TODO);
    }

    if (doc->getTag() == neutube::Document::FLYEM_BODY_3D ||
        doc->getTag() == neutube::Document::FLYEM_BODY_3D_COARSE ||
        doc->getTag() == neutube::Document::FLYEM_SKELETON) {
      int swcNodeCount = doc->getSelectedSwcNodeNumber();

      if (swcNodeCount == 1) {
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_MERGE);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_SPLIT);
      }

      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }

      if (swcNodeCount > 0 || doc->hasSelectedSwc()) {
        actionList.append(ZActionFactory::ACTION_DESELECT_BODY);
      }

      if (swcNodeCount == 2) {
        actionList.append(ZActionFactory::ACTION_MEASURE_SWC_NODE_DIST;
      }

      if (doc->getTag() == neutube::Document::FLYEM_BODY_3D ||
          doc->getTag() == neutube::Document::FLYEM_BODY_3D_COARSE) {
        actionList.append(ZActionFactory::ACTION_FLYEM_UPDATE_BODY);

        if (window->readyForAction(ZActionFactory::ACTION_FLYEM_COMPARE_BODY)) {
          actionList.append(ZActionFactory::ACTION_FLYEM_COMPARE_BODY);
        }
      }
    }

    if (window->readyForAction(ZActionFactory::ACTION_COPY_POSITION)) {
      actionList.append(ZActionFactory::ACTION_COPY_POSITION);
    }


    addAction(actionList, window, menu);
  }

  return menu;
}
#endif
