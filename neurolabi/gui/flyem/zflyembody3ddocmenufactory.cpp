#include "zflyembody3ddocmenufactory.h"
#include "z3dwindow.h"
#include "zflyembody3ddoc.h"
#include "zmenuconfig.h"

ZFlyEmBody3dDocMenuFactory::ZFlyEmBody3dDocMenuFactory()
{

}

ZFlyEmBody3dDocMenuFactory::~ZFlyEmBody3dDocMenuFactory()
{

}


QMenu* ZFlyEmBody3dDocMenuFactory::makeContextMenu(Z3DWindow *window, QMenu *menu)
{
  ZFlyEmBody3dDoc *doc = window->getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    menu = ZStackDocMenuFactory::makeContextMenu(window, menu);
    QList<ZActionFactory::EAction> actionList;
    if (doc->hasTodoItemSelected()) {
      actionList.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
      actionList.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
      actionList.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_NORMAL);
      actionList.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_SPLIT);
    }

    if (doc->hasSelectedPuncta()) {
      actionList.append(ZActionFactory::ACTION_PUNCTA_CHANGE_COLOR);
      actionList.append(ZActionFactory::ACTION_PUNCTA_HIDE_SELECTED);
      actionList.append(ZActionFactory::ACTION_PUNCTA_SHOW_SELECTED);
    }

//    if (doc->getSelectedBodyCount() == 1) {

//    }

    addAction(actionList, window, menu);
  }

  return menu;
}

ZMenuConfig ZFlyEmBody3dDocMenuFactory::getConfig(ZFlyEmBody3dDoc *doc)
{
  ZMenuConfig config;
  if (doc != NULL) {
    if (doc->hasTodoItemSelected()) {
      config.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
      config.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
      config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_NORMAL);
      config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_SPLIT);
    }

    if (doc->hasSelectedPuncta()) {
      config.append(ZActionFactory::ACTION_PUNCTA_CHANGE_COLOR);
      config.append(ZActionFactory::ACTION_PUNCTA_HIDE_SELECTED);
      config.append(ZActionFactory::ACTION_PUNCTA_SHOW_SELECTED);
    }

    if (doc->getSelectedSingleNormalBodyId() > 0) {
      config.append(ZActionFactory::ACTION_BODY_ANNOTATION);
    }

  }

  return config;
}
