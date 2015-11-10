#include "zflyemproofdocmenufactory.h"
#include "zstackpresenter.h"

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
        ZStackPresenter::ACTION_BODY_SPLIT_START);
  menu->addAction(action);

  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_BODY_DECOMPOSE));

  menu->addAction(presenter->getAction(
                    ZStackPresenter::ACTION_BODY_ANNOTATION));

  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_BODY_CHECKOUT));
  menu->addAction(presenter->getAction(ZStackPresenter::ACTION_BODY_CHECKIN));

  if (isAdmin()) {
    menu->addAction(presenter->getAction(
                      ZStackPresenter::ACTION_BODY_FORCE_CHECKIN));
  }

  return menu;
}
