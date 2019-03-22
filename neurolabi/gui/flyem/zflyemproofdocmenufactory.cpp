#include "zflyemproofdocmenufactory.h"

#include <QMenu>

#include "neutubeconfig.h"

#include "zintcuboidobj.h"

#include "mvc/zstackpresenter.h"
#include "mvc/zstackdochelper.h"

#include "zmenuconfig.h"
#include "zmenufactory.h"
#include "zactionfactory.h"
#include "zflyemproofpresenter.h"
#include "zflyemproofdoc.h"
#include "z3dwindow.h"


ZFlyEmProofDocMenuFactory::ZFlyEmProofDocMenuFactory()
{
#ifdef _DEBUG_
  std::cout << "ZFlyEmProofDocMenuFactory constructed" << std::endl;
#endif
}

QMenu* ZFlyEmProofDocMenuFactory::makeBodyContextMenu(
    ZStackPresenter *presenter, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  QAction *action = presenter->getAction(
        ZActionFactory::ACTION_BODY_SPLIT_START);
  menu->addAction(action);

  menu->addAction(presenter->getAction(ZActionFactory::ACTION_BODY_DECOMPOSE));

  menu->addAction(presenter->getAction(
                    ZActionFactory::ACTION_BODY_ANNOTATION));

  menu->addAction(presenter->getAction(ZActionFactory::ACTION_BODY_CHECKOUT));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_BODY_CHECKIN));

  if (isAdmin()) {
    menu->addAction(presenter->getAction(
                      ZActionFactory::ACTION_BODY_FORCE_CHECKIN));
  }

  return menu;
}

QMenu* ZFlyEmProofDocMenuFactory::makeSynapseContextMenu(
    ZStackPresenter *presenter, QWidget * /*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  QAction *action = presenter->getAction(ZActionFactory::ACTION_SYNAPSE_ADD_PRE);
  menu->addAction(action);

  menu->addAction(presenter->getAction(ZActionFactory::ACTION_SYNAPSE_ADD_POST));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_SYNAPSE_DELETE));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_SYNAPSE_MOVE));
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_SYNAPSE_LINK));

  menu->addSeparator();
  menu->addAction(presenter->getAction(ZActionFactory::ACTION_SHOW_ORTHO));

  return menu;
}

QMenu* ZFlyEmProofDocMenuFactory::makeStackContextMenu(
    ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu)
{
  return makeSynapseContextMenu(presenter, parentWidget, menu);
}



#if 0
QMenu* ZFlyEmProofDocMenuFactory::makeContextMenu(Z3DWindow *window, QMenu *menu)
{
  ZStackDoc *doc = window->getDocument();

  if (doc != NULL) {
    if (menu == NULL) {
      menu = new QMenu(NULL);
    } else {
      menu->clear();
    }

    QList<ZActionFactory::EAction> actionList;

    if (doc->getTag() == NeuTube::Document::FLYEM_BODY ||
        doc->getTag() == NeuTube::Document::FLYEM_COARSE_BODY) {
      if (doc->getSelectedSwcNodeList().size() == 1) {
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
      }
    }

    addAction(actionList, window, menu);
  }

  return menu;
}
#endif

ZMenuConfig ZFlyEmProofDocMenuFactory::getConfig(ZFlyEmProofPresenter *presenter)
{
  ZFlyEmProofDoc *doc = presenter->getCompleteDocument();

  ZMenuConfig config;
  if (presenter->isSplitOn()) {
    config.append(ZActionFactory::ACTION_BODY_DECOMPOSE);
    config.append(ZActionFactory::ACTION_BODY_CHOP);
    ZIntCuboidObj *roi = doc->getSplitRoi();
    if (roi != NULL) {
      if (roi->isValid()) {
        config.append(ZActionFactory::ACTION_BODY_CROP);
      }
    }
  } else {
    if (presenter->interactiveContext().acceptingRect()) {
      config.append(ZActionFactory::ACTION_ZOOM_TO_RECT);
      config.append(ZActionFactory::ACTION_SELECT_BODY_IN_RECT);
      config.append(ZActionFactory::ACTION_CANCEL_RECT_ROI);
    } else {
      std::set<uint64_t> selectedOriginal =
          doc->getSelectedBodySet(neutu::ELabelSource::ORIGINAL);

      if (!selectedOriginal.empty()) {
        if (!doc->getDvidTarget().readOnly()) {
          if (selectedOriginal.size() == 1) {

            if (ZStackDocHelper::AllowingBodyAnnotation(doc)) {
//              if (neutube::IsAdminUser()) {
//                config.append(ZActionFactory::ACTION_BODY_EXPERT_STATUS);
//              }
              config.append(ZActionFactory::ACTION_BODY_ANNOTATION);
            }

            config.appendSeparator();
            if (ZStackDocHelper::AllowingBodySplit(doc)) {
              config.append(ZActionFactory::ACTION_BODY_SPLIT_START);
            }
          }

          config.appendSeparator();
          if (ZStackDocHelper::AllowingBodyMerge(doc)) {
            std::set<uint64_t> selectedMapped =
                doc->getSelectedBodySet(neutu::ELabelSource::MAPPED);

            if (selectedMapped.size() > 1) {
              config.append(ZActionFactory::ACTION_BODY_MERGE);
            }
            if (selectedMapped.size() != selectedOriginal.size()) {
              config.append(ZActionFactory::ACTION_BODY_UNMERGE);
            }
          }

          if (ZStackDocHelper::AllowingBodyLock(doc)) {
            config.append(ZActionFactory::ACTION_BODY_CHECKOUT);
            config.append(ZActionFactory::ACTION_BODY_CHECKIN);
            if (isAdmin()) {
              config.append(ZActionFactory::ACTION_BODY_FORCE_CHECKIN);
            }
          }
        }

        config.append(ZActionFactory::ACTION_BODY_PROFILE);

        if (!doc->getDvidTarget().getSynapseName().empty()) {
          config.append(ZActionFactory::ACTION_BODY_CONNECTION);
          config.appendSeparator();
        }
      }
    }
  }

  if (!presenter->interactiveContext().acceptingRect()) {
    config.appendSeparator();
    /* Synapse actions */
    if (!doc->getDvidTarget().readOnly()) {
      config.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
      config.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
      config.append(ZActionFactory::ACTION_ADD_TODO_MERGE);
      config.append(ZActionFactory::ACTION_ADD_TODO_SPLIT);
      if (doc->getDvidTarget().hasSupervoxel()) {
        config.append(ZActionFactory::ACTION_ADD_TODO_SVSPLIT);
      }
      config.append(ZActionFactory::ACTION_SEPARATOR);
      if (doc->hasTodoItemSelected()) {
        config.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
        config.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
        config.append(ZActionFactory::ACTION_REMOVE_TODO_ITEM);
      }

      if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
        config.appendSeparator();

        config.append(ZActionFactory::ACTION_SYNAPSE_ADD_PRE);
        config.append(ZActionFactory::ACTION_SYNAPSE_ADD_POST);

        std::set<ZIntPoint> synapseSet = doc->getSelectedSynapse();
        if (!synapseSet.empty()) {
          config.append(ZActionFactory::ACTION_SYNAPSE_DELETE);
          if (synapseSet.size() == 1) {
            config.append(ZActionFactory::ACTION_SYNAPSE_MOVE);
          } else {
            config.append(ZActionFactory::ACTION_SYNAPSE_LINK);
          }
          config.append(ZActionFactory::ACTION_SYNAPSE_UNLINK);
          config.append(ZActionFactory::ACTION_SYNAPSE_VERIFY);
          config.append(ZActionFactory::ACTION_SYNAPSE_UNVERIFY);
          config.append(ZActionFactory::ACTION_SYNAPSE_REPAIR);
        }
      }
    }

    if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
      config.appendSeparator();
      config.append(ZActionFactory::ACTION_SHOW_ORTHO);

      if (NeutubeConfig::IsAdvancedMode()) {
        config.append(ZActionFactory::ACTION_SHOW_ORTHO_BIG);
      }
    }

    config.appendSeparator();
    config.append(ZActionFactory::ACTION_COPY_POSITION);
    config.append(ZActionFactory::ACTION_COPY_BODY_ID);
    if (doc->getDvidTarget().hasSupervoxel()) {
      config.append(ZActionFactory::ACTION_COPY_SUPERVOXEL_ID);
      config.append(ZActionFactory::ACTION_SHOW_SUPERVOXEL_LIST);
    }

    if (doc->hasStackData()) {
      config.append(ZActionFactory::ACTION_SAVE_STACK);
    }

    if (!doc->getDvidTarget().readOnly()) {
      if (doc->getCuboidRoi().getDepth() > 1) {
        config.appendSeparator();
        config.append(ZActionFactory::ACTION_REWRITE_SEGMENTATION);
      }
    }
  }

  config.append(ZActionFactory::ACTION_REFRESH_SEGMENTATION);

//  addAction(actionList, presenter, menu);

  if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
    /* Bookmark actions */
    TStackObjectSet& bookmarkSet =
        doc->getSelected(ZStackObject::EType::FLYEM_BOOKMARK);
    if (!bookmarkSet.isEmpty()) {
      QString groupName("Bookmarks");
      config.append(groupName, ZActionFactory::ACTION_BOOKMARK_CHECK);
      config.append(groupName, ZActionFactory::ACTION_BOOKMARK_UNCHECK);
    }

    if (!presenter->interactiveContext().acceptingRect()) {
      //    QList<ZActionFactory::EAction> swcActionList;
      //SWC actions (submenu has to be added separately)
      QList<Swc_Tree_Node*> swcNodeList = doc->getSelectedSwcNodeList();
      if (swcNodeList.size() > 1) {
        config.appendSeparator();
        config << "Path Information"
               << ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH
               << ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH;
      }
    }
  }

  return config;
}

QMenu* ZFlyEmProofDocMenuFactory::makeContextMenu(
    ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu)
{
  ZFlyEmProofPresenter *proofPresenter =
      qobject_cast<ZFlyEmProofPresenter*>(presenter);

  if (proofPresenter != NULL) {
//    if (menu == NULL) {
//      menu = new QMenu(NULL);
//    } else {
//      menu->clear();
//    }

    ZMenuConfig config = getConfig(proofPresenter);

    menu = ZMenuFactory::MakeMenu(config, proofPresenter, menu);
//    menu = ZMenuFactory::MakeMenu(config, menu);

#if 0
    ZFlyEmProofDoc *doc = proofPresenter->getCompleteDocument();

    QList<ZActionFactory::EAction> actionList;

    /* Split mode */
    if (proofPresenter->isSplitOn()) {
      actionList.append(ZActionFactory::ACTION_BODY_DECOMPOSE);
      actionList.append(ZActionFactory::ACTION_BODY_CHOP);
      ZIntCuboidObj *roi = doc->getSplitRoi();
      if (roi != NULL) {
        if (roi->isValid()) {
          actionList.append(ZActionFactory::ACTION_BODY_CROP);
        }
      }
    } else {
      if (proofPresenter->interactiveContext().acceptingRect()) {
        actionList.append(ZActionFactory::ACTION_ZOOM_TO_RECT);
        actionList.append(ZActionFactory::ACTION_SELECT_BODY_IN_RECT);
        actionList.append(ZActionFactory::ACTION_CANCEL_RECT_ROI);
      } else {
        if (!doc->getDvidTarget().readOnly()) {
          std::set<uint64_t> selectedOriginal =
              doc->getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
          std::set<uint64_t> selectedMapped =
              doc->getSelectedBodySet(neutube::BODY_LABEL_MAPPED);

          if (!selectedOriginal.empty()) {
            if (selectedOriginal.size() == 1) {
              if (doc->getTag() == neutube::Document::FLYEM_PROOFREAD) {
                actionList.append(ZActionFactory::ACTION_BODY_SPLIT_START);
              }
              actionList.append(ZActionFactory::ACTION_BODY_ANNOTATION);
            }

            if (selectedMapped.size() > 1) {
              actionList.append(ZActionFactory::ACTION_BODY_MERGE);
            }
            if (selectedMapped.size() != selectedOriginal.size()) {
              actionList.append(ZActionFactory::ACTION_BODY_UNMERGE);
            }

            actionList.append(ZActionFactory::ACTION_BODY_CHECKOUT);
            actionList.append(ZActionFactory::ACTION_BODY_CHECKIN);
            if (isAdmin()) {
              actionList.append(ZActionFactory::ACTION_BODY_FORCE_CHECKIN);
            }
          }
        }
      }
    }

    if (!proofPresenter->interactiveContext().acceptingRect()) {
      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }
      /* Synapse actions */
      if (!doc->getDvidTarget().readOnly()) {
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_MERGE);
        actionList.append(ZActionFactory::ACTION_ADD_TODO_SPLIT);
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
        if (doc->hasTodoItemSelected()) {
          actionList.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
          actionList.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
          actionList.append(ZActionFactory::ACTION_REMOVE_TODO_ITEM);
        }

        actionList.append(ZActionFactory::ACTION_SEPARATOR);


        actionList.append(ZActionFactory::ACTION_SYNAPSE_ADD_PRE);
        actionList.append(ZActionFactory::ACTION_SYNAPSE_ADD_POST);

        std::set<ZIntPoint> synapseSet = doc->getSelectedSynapse();
        if (!synapseSet.empty()) {
          actionList.append(ZActionFactory::ACTION_SYNAPSE_DELETE);
          if (synapseSet.size() == 1) {
            actionList.append(ZActionFactory::ACTION_SYNAPSE_MOVE);
          } else {
            actionList.append(ZActionFactory::ACTION_SYNAPSE_LINK);
          }
          actionList.append(ZActionFactory::ACTION_SYNAPSE_UNLINK);
          actionList.append(ZActionFactory::ACTION_SYNAPSE_VERIFY);
          actionList.append(ZActionFactory::ACTION_SYNAPSE_UNVERIFY);
          actionList.append(ZActionFactory::ACTION_SYNAPSE_REPAIR);
        }
      }

      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }
      actionList.append(ZActionFactory::ACTION_SHOW_ORTHO);

      if (NeutubeConfig::IsAdvancedMode()) {
        actionList.append(ZActionFactory::ACTION_SHOW_ORTHO_BIG);
      }

      actionList.append(ZActionFactory::ACTION_SEPARATOR);
      actionList.append(ZActionFactory::ACTION_COPY_POSITION);

      if (doc->hasStackData()) {
        actionList.append(ZActionFactory::ACTION_SAVE_STACK);
      }

      if (!doc->getDvidTarget().readOnly()) {
        if (doc->getCuboidRoi().getDepth() > 1) {
          if (!actionList.isEmpty()) {
            actionList.append(ZActionFactory::ACTION_SEPARATOR);
          }
          actionList.append(ZActionFactory::ACTION_REWRITE_SEGMENTATION);
        }
      }
    }

    actionList.append(ZActionFactory::ACTION_REFRESH_SEGMENTATION);

    addAction(actionList, presenter, menu);

    /* Bookmark actions */
    QList<ZActionFactory::EAction> bookmarkActionList;
    TStackObjectSet& bookmarkSet =
        doc->getSelected(ZStackObject::EType::TYPE_FLYEM_BOOKMARK);
    if (!bookmarkSet.isEmpty()) {
      bookmarkActionList.append(ZActionFactory::ACTION_BOOKMARK_CHECK);
      bookmarkActionList.append(ZActionFactory::ACTION_BOOKMARK_UNCHECK);
    }
    if (!bookmarkActionList.isEmpty()) {
      QMenu *submenu = new QMenu("Bookmarks", menu);
      addAction(bookmarkActionList, presenter, submenu);
      menu->addMenu(submenu);
    }


    if (!proofPresenter->interactiveContext().acceptingRect()) {
      QList<ZActionFactory::EAction> swcActionList;
      //SWC actions (submenu has to be added separately)
      QList<Swc_Tree_Node*> swcNodeList = doc->getSelectedSwcNodeList();
      if (swcNodeList.size() > 1) {
        if (!actionList.isEmpty()) {
          menu->addSeparator();
        }
        swcActionList.append(ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH);
        swcActionList.append(
              ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH);
      }
      if (!swcActionList.isEmpty()) {
        QMenu *submenu = new QMenu("Path Information", menu);
        menu->addMenu(submenu);
        addAction(swcActionList, presenter, submenu);
      }
    }
#endif
  } else {
    menu = ZStackDocMenuFactory::makeContextMenu(presenter, parentWidget, menu);
  }

  return menu;
}
