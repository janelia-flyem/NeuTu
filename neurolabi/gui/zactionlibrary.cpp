#include "zactionlibrary.h"

ZActionLibrary::ZActionLibrary(QObject *parent)
{
  m_actionParent = new QObject(parent);
}

QAction* ZActionLibrary::getAction(
    ZActionFactory::EAction item, QObject *receiver, const char *slot)
{
  QAction *action = NULL;

  if (m_actionMap.contains(item)) {
    action = m_actionMap[item];
  } else {
    action = m_actionFactory.makeAction(item, m_actionParent);
    if (receiver != NULL && slot != NULL) {
      QObject::connect(action, SIGNAL(triggered()), receiver, slot);
    }

    m_actionMap[item] = action;
  }

  return action;
}
