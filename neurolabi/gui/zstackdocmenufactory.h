#ifndef ZSTACKDOCMENUFACTORY_H
#define ZSTACKDOCMENUFACTORY_H

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;
class QWidget;

#include "neutube.h"

/*!
 * \brief Class of creating menus for ZStackDoc
 */
class ZStackDocMenuFactory
{
public:
  ZStackDocMenuFactory();

public:
  static QMenu* makeSwcNodeContextMenu(
      ZStackDoc *doc, QWidget *parentWidget, QMenu *menu);
  static QMenu* makeSwcNodeContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  static QMenu* makeSrokePaintContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
  static QMenu* makeStackContextMenu(
      ZStackPresenter *presenter, QWidget *parentWidget, QMenu *menu);
};

#endif // ZSTACKDOCMENUFACTORY_H
