#include "zmenufactory.h"

#include <QMenu>
#include <QWidget>

#include "neutubeconfig.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zactionfactory.h"
#include "z3dwindow.h"

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
      menu->addAction(source->getAction(action));
    }
  }
}

void ZMenuFactory::addAction(
    const QList<ZActionFactory::EAction> &actionList,
    ZStackPresenter *presenter, QMenu *menu)
{
  AddAction(actionList, presenter, menu);
}

void ZMenuFactory::addAction(
    const QList<ZActionFactory::EAction> &actionList, Z3DWindow *window, QMenu *menu)
{
  AddAction(actionList, window, menu);
}

QMenu* ZMenuFactory::makeSwcNodeContextMenu(
    ZStackDoc *doc, QWidget */*parentWidget*/, QMenu *menu)
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

  QMenu *submenu = new QMenu("Intepolate", menu);
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
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
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
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  menu->addAction(presenter->getAction(ZActionFactory::ACTION_PAINT_STROKE));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_ERASE_STROKE));

  return menu;
}

QMenu* ZMenuFactory::makeStackContextMenu(
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  switch (presenter->buddyDocument()->getTag()) {
  case NeuTube::Document::FLYEM_SPLIT:
  case NeuTube::Document::SEGMENTATION_TARGET:
    menu->addAction(presenter->getAction(ZActionFactory::ACTION_SPLIT_DATA));
    menu->addAction(presenter->getAction(
                      ZActionFactory::ACTION_ADD_SPLIT_SEED));
    break;
  case NeuTube::Document::NORMAL:
  case NeuTube::Document::BIOCYTIN_STACK:
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
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  return menu;
}

QMenu* ZMenuFactory::makeSynapseContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  return menu;
}

QMenu* ZMenuFactory::makeContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
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

    if (doc->getTag() == NeuTube::Document::FLYEM_BODY_3D ||
        doc->getTag() == NeuTube::Document::FLYEM_BODY_3D_COARSE ||
        doc->getTag() == NeuTube::Document::FLYEM_SKELETON) {
      actionList.append(ZActionFactory::ACTION_SYNAPSE_FILTER);
    }

    if (doc->getTag() == NeuTube::Document::FLYEM_BODY_3D) {
      if (doc->getSelectedSwcNodeList().size() == 1) {
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
      }

      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }
      actionList.append(ZActionFactory::ACTION_FLYEM_UPDATE_BODY);
    }

    addAction(actionList, window, menu);
  }

  return menu;
}
