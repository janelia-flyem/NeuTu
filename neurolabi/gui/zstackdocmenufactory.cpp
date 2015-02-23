#include "zstackdocmenufactory.h"

#include <QMenu>
#include <QWidget>

#include "neutubeconfig.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zactionfactory.h"

ZStackDocMenuFactory::ZStackDocMenuFactory() :
  m_singleSwcNodeActionActivator(NULL)
{
}

QMenu* ZStackDocMenuFactory::makeSwcNodeContextMenu(
    ZStackDoc *doc, QWidget *parentWidget, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  menu->addAction(ZActionFactory::makeAction(
                    ZActionFactory::ACTION_DELETE_SWC_NODE, doc, parentWidget));

  //menu->addAction(doc->getAction(ZStackDoc::ACTION_DELETE_SWC_NODE));
  //menu->addAction(doc->getAction(ZStackDoc::ACTION_BREAK_SWC_NODE));
  QAction *action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_BREAK_SWC_NODE, doc, parentWidget,
        m_singleSwcNodeActionActivator, false);
  menu->addAction(action);


  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_CONNECT_SWC_NODE, doc, parentWidget,
        m_singleSwcNodeActionActivator, false);
  menu->addAction(action);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, false);

  //menu->addAction(doc->getAction(ZStackDoc::ACTION_CONNECT_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_MERGE_SWC_NODE, doc, parentWidget,
        m_singleSwcNodeActionActivator, false);
  menu->addAction(action);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, false);

  //menu->addAction(doc->getAction(ZStackDoc::ACTION_MERGE_SWC_NODE));

  //menu->addAction(doc->getAction(ZStackDoc::ACTION_INSERT_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_INSERT_SWC_NODE, doc, parentWidget,
        m_singleSwcNodeActionActivator, false);
  menu->addAction(action);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, false);

  QMenu *submenu = new QMenu("Intepolate", menu);
  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SWC_INTERPOLATION));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SWC_INTERPOLATION, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SWC_Z_INTERPOLATION));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SWC_Z_INTERPOLATION, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SWC_POSITION_INTERPOLATION));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SWC_POSITION_INTERPOLATION, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SWC_RADIUS_INTERPOLATION));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SWC_RADIUS_INTERPOLATION, doc, parentWidget);
  submenu->addAction(action);

  menu->addMenu(submenu);

  submenu = new QMenu("Select", menu);
  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_DOWNSTREAM));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_DOWNSTREAM, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_UPSTREAM));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_UPSTREAM, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_NEIGHBOR_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_NEIGHBOR_SWC_NODE, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_SWC_BRANCH));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_SWC_BRANCH, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_CONNECTED_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_CONNECTED_SWC_NODE, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SELECT_ALL_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SELECT_ALL_SWC_NODE, doc, parentWidget);
  submenu->addAction(action);

  menu->addMenu(submenu);

  submenu = new QMenu("Advanced Editing", menu);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_REMOVE_TURN));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_REMOVE_TURN, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_RESOLVE_CROSSOVER));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_RESOLVE_CROSSOVER, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction((ZStackDoc::ACTION_SET_BRANCH_POINT)));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SET_BRANCH_POINT, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_CONNECTED_ISOLATED_SWC));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_CONNECTED_ISOLATED_SWC, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_RESET_BRANCH_POINT));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_RESET_BRANCH_POINT, doc, parentWidget,
        m_singleSwcNodeActionActivator, true);
  submenu->addAction(action);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, true);

  menu->addMenu(submenu);

  submenu = new QMenu("Change Property", menu);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_TRANSLATE_SWC_NODE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_TRANSLATE_SWC_NODE, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_CHANGE_SWC_SIZE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_CHANGE_SWC_SIZE, doc, parentWidget);
  submenu->addAction(action);

  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SET_SWC_ROOT));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SET_SWC_ROOT, doc, parentWidget,
        m_singleSwcNodeActionActivator, true);
  submenu->addAction(action);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, true);

  menu->addMenu(submenu);

  /*
  submenu = new QMenu("Topology", menu);
  submenu->addAction(doc->getAction(ZStackDoc::ACTION_SET_BRANCH_POINT));
  submenu->addAction(doc->getAction(ZStackDoc::ACTION_RESET_BRANCH_POINT));
  menu->addMenu(submenu);
*/
  submenu = new QMenu("Information", menu);
  //submenu->addAction(doc->getAction(ZStackDoc::ACTION_SWC_SUMMARIZE));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_SWC_SUMMARIZE, doc, parentWidget);
  submenu->addAction(action);

  submenu->addAction(doc->getAction(ZStackDoc::ACTION_MEASURE_SWC_NODE_LENGTH));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH, doc, parentWidget,
        m_singleSwcNodeActionActivator, true);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, true);
  //submenu->addAction(action);

  submenu->addAction(
        doc->getAction(ZStackDoc::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH));
  action = ZActionFactory::makeAction(
        ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH, doc, parentWidget,
        m_singleSwcNodeActionActivator, true);
  //doc->getSingleSwcNodeActionActivator()->registerAction(action, true);
  //submenu->addAction(action);

  menu->addMenu(submenu);

  return menu;
}

QMenu* ZStackDocMenuFactory::makeSwcNodeContextMenu(
    ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  //menu->addAction(presenter->getAction(ZStackPresenter::ACTION_SMART_EXTEND_SWC_NODE));
  //menu->addAction(presenter->getAction(ZStackPresenter::ACTION_EXTEND_SWC_NODE));
  menu->addAction(ZActionFactory::makeAction(
                    ZActionFactory::ACTION_EXTEND_SWC_NODE, presenter,
                    parentWidget, m_singleSwcNodeActionActivator, true));

  //menu->addAction(presenter->getAction(ZStackPresenter::ACTION_CONNECT_TO_SWC_NODE));
  menu->addAction(ZActionFactory::makeAction(
                    ZActionFactory::ACTION_CONNECT_TO_SWC_NODE, presenter,
                    parentWidget, m_singleSwcNodeActionActivator, true));

  /*
  menu->addAction(presenter->getAction(
                    ZStackPresenter::ACTION_CHANGE_SWC_NODE_FOCUS));
                    */
  menu->addAction(ZActionFactory::makeAction(
                    ZActionFactory::ACTION_CHANGE_SWC_NODE_FOCUS, presenter,
                    parentWidget));

  /*
  menu->addAction(presenter->getAction(
                    ZStackPresenter::ACTION_MOVE_SWC_NODE));
                    */
  menu->addAction(ZActionFactory::makeAction(
                    ZActionFactory::ACTION_MOVE_SWC_NODE, presenter,
                    parentWidget));

  if (GET_APPLICATION_NAME == "Biocytin") {
    menu->addAction(presenter->getAction(
                      ZStackPresenter::ACTION_ESTIMATE_SWC_NODE_RADIUS));
  }

  return menu;
}

QMenu* ZStackDocMenuFactory::makeSrokePaintContextMenu(
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_PAINT_STROKE));
  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_ERASE_STROKE));

  return menu;
}

QMenu* ZStackDocMenuFactory::makeStackContextMenu(
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  if (presenter->buddyDocument()->getTag() == NeuTube::Document::FLYEM_SPLIT ||
      presenter->buddyDocument()->getTag() ==
      NeuTube::Document::SEGMENTATION_TARGET) {
    menu->addAction(presenter->getAction(ZStackPresenter::ACTION_SPLIT_DATA));
  } else {
    menu->addAction(presenter->getAction(ZStackPresenter::ACTION_ADD_SWC_NODE));
  }

  return menu;
}

QMenu* ZStackDocMenuFactory::makeBodyContextMenu(
    ZStackPresenter *presenter, QWidget */*parentWidget*/, QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  }

  if (presenter->buddyDocument()->getTag() == NeuTube::Document::FLYEM_MERGE) {
    QAction *action = presenter->getAction(
          ZStackPresenter::ACTION_BODY_SPLIT_START);
    menu->addAction(action);
  }

  return menu;
}
