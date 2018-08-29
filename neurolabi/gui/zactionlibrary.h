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

  /*!
   * \brief Get an action
   *
   * It returns an action corresponding to \a item. If the action does not exist
   * in the library priort to the call, the function will try to create it and
   * connect its \a triggered signal to \a slot of \a receiver. It returns NULL
   * if the action cannot be created. If the action already exists, \a receiver
   * and \a slot have no effect.
   */
  QAction* getAction(
      ZActionFactory::EAction item, QObject *receiver, const char *slot);

  QAction* getAction(ZActionFactory::EAction item);
  bool contains(ZActionFactory::EAction item) const;

  void setUndoStack(QUndoStack *undoStack);

  bool actionCreatedUponRetrieval() const;

private:
  QObject *m_actionParent;
  ZActionFactory m_actionFactory;
  bool m_actionCreated = false;

  QMap<ZActionFactory::EAction, QAction*> m_actionMap;
};

#endif // ZACTIONLIBRARY_H
