#include "zflyemproofdocmenufactory.h"

#include <QMenu>

#include "zstackpresenter.h"
#include "zactionfactory.h"
#include "flyem/zflyemproofpresenter.h"
#include "flyem/zflyemproofdoc.h"

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

        actionList.append(ZActionFactory::ACTION_BODY_CHECKOUT);
        actionList.append(ZActionFactory::ACTION_BODY_CHECKIN);
        if (isAdmin()) {
          actionList.append(ZActionFactory::ACTION_BODY_FORCE_CHECKIN);
        }
      }
    }

    if (!actionList.isEmpty()) {
      actionList.append(ZActionFactory::ACTION_SEPARATOR);
    }

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
        actionList.append(ZActionFactory::ACTION_SYNAPSE_UNLINK);
      }
    }

    if (!actionList.isEmpty()) {
      actionList.append(ZActionFactory::ACTION_SEPARATOR);
    }
    actionList.append(ZActionFactory::ACTION_SHOW_ORTHO);

    /* Bookmark actions */
    TStackObjectSet& bookmarkSet =
        doc->getSelected(ZStackObject::TYPE_FLYEM_BOOKMARK);
    if (!bookmarkSet.isEmpty()) {
      actionList.append(ZActionFactory::ACTION_BOOKMARK_CHECK);
      actionList.append(ZActionFactory::ACTION_BOOKMARK_UNCHECK);
    }

    foreach (ZActionFactory::EAction action, actionList) {
      if (action == ZActionFactory::ACTION_SEPARATOR) {
        menu->addSeparator();
      } else {
        menu->addAction(presenter->getAction(action));
      }
    }

    //SWC actions (submenu has to be added separately)
    QList<Swc_Tree_Node*> swcNodeList = doc->getSelectedSwcNodeList();
    if (swcNodeList.size() > 1) {
      if (!actionList.isEmpty()) {
        menu->addSeparator();
      }
      QMenu *submenu = new QMenu("Path Information", menu);
      submenu->addAction(doc->getAction(
                           ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH));
      submenu->addAction(
            doc->getAction(ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH));

      menu->addMenu(submenu);
    }
  } else {
    menu = ZStackDocMenuFactory::makeContextMenu(presenter, parentWidget, menu);
  }

  return menu;
}
