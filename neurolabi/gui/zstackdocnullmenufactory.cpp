#include "zstackdocnullmenufactory.h"

#include <QMenu>

namespace {
QMenu* MakeEmptyMenu(QMenu *menu)
{
  if (menu == NULL) {
    menu = new QMenu(NULL);
  } else {
    menu->clear();
  }

  return menu;
}
}

ZStackDocNullMenuFactory::ZStackDocNullMenuFactory()
{

}

QMenu* ZStackDocNullMenuFactory::makeBodyContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  return MakeEmptyMenu(menu);
}

QMenu* ZStackDocNullMenuFactory::makeSynapseContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  return MakeEmptyMenu(menu);
}

QMenu* ZStackDocNullMenuFactory::makeStackContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  return MakeEmptyMenu(menu);
}

QMenu* ZStackDocNullMenuFactory::makeContextMenu(
    ZStackPresenter */*presenter*/, QWidget */*parentWidget*/, QMenu *menu)
{
  return MakeEmptyMenu(menu);
}
