#include "zflyembody3ddocmenufactory.h"

#include <tuple>

#include "z3dwindow.h"
#include "zflyembody3ddoc.h"
#include "zmenuconfig.h"
#include "zmenufactory.h"
#include "zflyembodyenv.h"
#include "zstackdocproxy.h"
#include "zglobal.h"

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
//    menu = ZStackDocMenuFactory::makeContextMenu(window, menu);
    ZMenuConfig config = getConfig(doc, window->getBodyEnv());
    menu = ZMenuFactory::MakeMenu(config, window, menu);
#if 0
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
#endif
  }

  return menu;
}

bool ZFlyEmBody3dDocMenuFactory::ReadyForAction(
    ZFlyEmBody3dDoc *doc, ZActionFactory::EAction action)
{
  bool ready = true;

  if (doc != NULL) {
    switch (action) {
#if defined(_FLYEM_)
    case ZActionFactory::ACTION_FLYEM_COMPARE_BODY:
      if (doc->updating()) {
        ready = false;
      }
      break;
#endif
    case ZActionFactory::ACTION_COPY_POSITION:
      if (doc->getSelectedSwcNodeNumber() != 1) {
        ready = false;
      }
      break;
    default:
      break;
    }
  }

  return ready;
}

ZMenuConfig ZFlyEmBody3dDocMenuFactory::getConfig(
    ZFlyEmBody3dDoc *doc, ZFlyEmBodyEnv *bodyEnv)
{
  ZMenuConfig config;
  if (doc != NULL) {
    bool isMutable = (doc->getDvidTarget().readOnly() == false);

    if (doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D ||
        doc->getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
      config.append(ZActionFactory::ACTION_SYNAPSE_FILTER);
    } else if (doc->getTag() == neutu::Document::ETag::FLYEM_MESH) {
#if defined(_NEU3_)
      if (isMutable) {
        config.append(ZActionFactory::ACTION_REMOVE_TODO_BATCH);

        ZMesh *mesh = doc->getMeshForSplit();
        if (mesh != NULL) {
          if (doc->isSplitActivated() &&
              ZStackDocProxy::GetGeneralMeshList(doc).size() > 1) {
            config.append(ZActionFactory::ACTION_SHOW_SPLIT_MESH_ONLY);
          } else {
            bool allowingSplit = true;
            if (bodyEnv != NULL) {
              allowingSplit = bodyEnv->allowingSplit(mesh->getLabel());
            }
            if (allowingSplit) {
              config.append(ZActionFactory::ACTION_START_SPLIT);
            }
          }
        }
      }
#else
      std::ignore = bodyEnv;
#endif
    }

    config.appendSeparator();
    config.append(ZActionFactory::ACTION_SHOW_NORMAL_TODO);
    config.append(ZActionFactory::ACTION_SHOW_DONE);
    config.appendSeparator();

    if (isMutable) {
      if (doc->getSelectedSingleNormalBodyId() > 0) {
        config.append(ZActionFactory::ACTION_BODY_ANNOTATION);
      }
    }

    if (doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D ||
        doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D_COARSE ||
        doc->getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
      int swcNodeCount = doc->getSelectedSwcNodeNumber();

      if (swcNodeCount == 2) {
        config.appendSeparator();
        config.append(ZActionFactory::ACTION_MEASURE_SWC_NODE_DIST);
      }

      if (swcNodeCount >= 2) {
        if (doc->getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
          config.append(ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH);
        }
      }

      if (swcNodeCount >= 1) {
        if (doc->getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
          config.append(ZActionFactory::ACTION_SEPARATOR);
          config.append(ZActionFactory::ACTION_DELETE_SWC_NODE);
          if (swcNodeCount > 1) {
            config.append(ZActionFactory::ACTION_CONNECT_SWC_NODE);
            config.append(ZActionFactory::ACTION_BREAK_SWC_NODE);
          }
          config.append(ZActionFactory::ACTION_SEPARATOR);
        }
      }

      if (swcNodeCount == 1) {
        if (isMutable) {
          config.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
          config.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
          config.append(ZActionFactory::ACTION_ADD_TODO_MERGE);
          config.append(ZActionFactory::ACTION_ADD_TODO_SPLIT);
          config.append(ZActionFactory::ACTION_ADD_TODO_TRACE_TO_SOMA);
          config.append(ZActionFactory::ACTION_ADD_TODO_NO_SOMA);
        }
      }

      config.appendSeparator();

      if (swcNodeCount > 0 || doc->hasSelectedSwc()) {
        config.append(ZActionFactory::ACTION_DESELECT_BODY);
      }

      if (doc->hasSelectedSwc()) {
        config.append(ZActionFactory::ACTION_SAVE_OBJECT_AS);
      }

      if (doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D ||
          doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D_COARSE) {
        config.append(ZActionFactory::ACTION_FLYEM_UPDATE_BODY);

        if (ReadyForAction(doc, ZActionFactory::ACTION_FLYEM_COMPARE_BODY)) {
          config.append(ZActionFactory::ACTION_FLYEM_COMPARE_BODY);
        }
      }
    } else if (doc->getTag() == neutu::Document::ETag::FLYEM_MESH) {
#if !defined(_NEU3_)
      config.append(ZActionFactory::ACTION_FLYEM_UPDATE_BODY);
#endif
    }

    if (ReadyForAction(doc, ZActionFactory::ACTION_COPY_POSITION)) {
      config.append(ZActionFactory::ACTION_COPY_POSITION);
    }
    config.appendSeparator();

    if (doc->hasTodoItemSelected()) {
      if (isMutable) {
        config.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
        config.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
        config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_NORMAL);
        config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_SPLIT);
        config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_IRRELEVANT);
        config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_TRACE_TO_SOMA);
        config.append(ZActionFactory::ACTION_TODO_ITEM_ANNOT_NO_SOMA);
      }
    }

    config.appendSeparator();

    if (doc->hasSelectedPuncta()) {
      config.append(ZActionFactory::ACTION_PUNCTA_CHANGE_COLOR);
      config.append(ZActionFactory::ACTION_PUNCTA_HIDE_SELECTED);
      config.append(ZActionFactory::ACTION_PUNCTA_SHOW_SELECTED);
      config.append(ZActionFactory::ACTION_PUNCTA_HIDE_UNSELECTED);
      config.append(ZActionFactory::ACTION_PUNCTA_SHOW_UNSELECTED);
    }

    config.append(ZActionFactory::ACTION_PUNCTA_ADD_SELECTION);

    if (isMutable) {
      if (doc->getSelectedSingleNormalBodyId() > 0) {
        config.append(ZActionFactory::ACTION_BODY_ANNOTATION);
      }
    }
  }

  config.appendSeparator();
  if (doc->hasMesh()) {
    config.append(ZActionFactory::ACTION_SAVE_ALL_MESH);
  }

  config.appendSeparator();
  config.append(ZActionFactory::ACTION_COPY_3DCAMERA);
  if (!ZGlobal::GetInstance().get3DCamera().empty()) {
    config.append(ZActionFactory::ACTION_PASTE_3DCAMERA);
  }

#if defined(_NEU3_)
  config.appendSeparator();
  config.append(ZActionFactory::ACTION_3DWINDOW_TOGGLE_SETTING);
  config.append(ZActionFactory::ACTION_3DWINDOW_TOGGLE_OBJECTS);
#endif

  return config;
}
