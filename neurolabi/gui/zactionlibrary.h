#ifndef ZACTIONLIBRARY_H
#define ZACTIONLIBRARY_H

#include <QObject>
#include <QMap>
#include <QAction>

#include "zactionfactory.h"

class ZActionLibrary
{
public:
  ZActionLibrary(QObject *parent);
//  virtual ~ZActionLibrary();

  QAction* getAction(
      ZActionFactory::EAction item, QObject *receiver, const char *slot);

private:
  QObject *m_actionParent;
  ZActionFactory m_actionFactory;

  QMap<ZActionFactory::EAction, QAction*> m_actionMap;
};

#endif // ZACTIONLIBRARY_H
