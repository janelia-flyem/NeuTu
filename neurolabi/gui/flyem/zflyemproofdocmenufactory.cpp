#include "zflyemproofdocmenufactory.h"

#include <QMenu>

#include "zstackpresenter.h"
#include "zactionfactory.h"
#include "flyem/zflyemproofpresenter.h"
#include "flyem/zflyemproofdoc.h"
#include "z3dwindow.h"

ZFlyEmProofDocMenuFactory::ZFlyEmProofDocMenuFactory()
{
#ifdef _DEBUG_
  std::cout << "ZFlyEmProofDocMenuFactory constructed" << std::endl;
#endif
}

QMenu* ZFlyEmProofDocMenuFactory::makeBodyContextMenu(
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
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
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
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

QMenu* ZFlyEmProofDocMenuFactory::makeContextMenu(
    ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu)
{
  ZFlyEmProofPresenter *proofPresenter =
      qobject_cast<ZFlyEmProofPresenter*>(presenter);

  if (proofPresenter != NULL) {
    if (menu == NULL) {
      menu = new QMenu(NULL);
    } else {
      menu->clear();
    }

    ZFlyEmProofDoc *doc = proofPresenter->getCompleteDocument();

    QList<ZActionFactory::EAction> actionList;

    /* Split mode */
    if (proofPresenter->isSplitOn()) {
      actionList.append(ZActionFactory::ACTION_BODY_DECOMPOSE);
    } else {
      if (proofPresenter->interactiveContext().acceptingRect()) {
        actionList.append(ZActionFactory::ACTION_ZOOM_TO_RECT);
        actionList.append(ZActionFactory::ACTION_SELECT_BODY_IN_RECT);
        actionList.append(ZActionFactory::ACTION_CANCEL_RECT_ROI);
      } else {
        std::set<uint64_t> selectedOriginal =
            doc->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
        std::set<uint64_t> selectedMapped =
            doc->getSelectedBodySet(NeuTube::BODY_LABEL_MAPPED);

        if (!selectedOriginal.empty()) {
          if (selectedOriginal.size() == 1) {
            if (doc->getTag() == NeuTube::Document::FLYEM_PROOFREAD) {
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

    if (!proofPresenter->interactiveContext().acceptingRect()) {
      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }

      actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM);
      actionList.append(ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED);
      if (doc->hasTodoItemSelected()) {
        actionList.append(ZActionFactory::ACTION_CHECK_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_UNCHECK_TODO_ITEM);
        actionList.append(ZActionFactory::ACTION_REMOVE_TODO_ITEM);
      }

      actionList.append(ZActionFactory::ACTION_SEPARATOR);

      /* Synapse actions */
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

      if (!actionList.isEmpty()) {
        actionList.append(ZActionFactory::ACTION_SEPARATOR);
      }
      actionList.append(ZActionFactory::ACTION_SHOW_ORTHO);

      if (doc->hasStackData()) {
        actionList.append(ZActionFactory::ACTION_SAVE_STACK);
      }

      if (doc->getCuboidRoi().getDepth() > 1) {
        if (!actionList.isEmpty()) {
          actionList.append(ZActionFactory::ACTION_SEPARATOR);
        }
        actionList.append(ZActionFactory::ACTION_REWRITE_SEGMENTATION);
      }
    }

    addAction(actionList, presenter, menu);

    /* Bookmark actions */
    QList<ZActionFactory::EAction> bookmarkActionList;
    TStackObjectSet& bookmarkSet =
        doc->getSelected(ZStackObject::TYPE_FLYEM_BOOKMARK);
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
  } else {
    menu = ZStackDocMenuFactory::makeContextMenu(presenter, parentWidget, menu);
  }

  return menu;
}
