#include "zstackdocdatabuffer.h"

#include <iostream>

#include "zstackobject.h"

ZStackDocObjectUpdate::ZStackDocObjectUpdate(ZStackObject *obj, EAction action)
{
  m_obj = obj;
  m_action = action;
}

ZStackDocObjectUpdate::~ZStackDocObjectUpdate()
{
  if (m_action == ACTION_ADD_NONUNIQUE && m_action == ACTION_ADD_UNIQUE) {
    delete m_obj;
  }
}

void ZStackDocObjectUpdate::reset()
{
  m_obj = NULL;
  m_action = ACTION_NULL;
}

void ZStackDocObjectUpdate::print() const
{
  switch (m_action) {
  case ACTION_ADD_NONUNIQUE:
    std::cout << "Add nonunique";
    break;
  case ACTION_ADD_UNIQUE:
    std::cout << "Add unique";
    break;
  case ACTION_EXPEL:
    std::cout << "Expel";
    break;
  case ACTION_KILL:
    std::cout << "Kill";
    break;
  case ACTION_NULL:
    std::cout << "No action on";
    break;
  case ACTION_RECYCLE:
    std::cout << "Recycle";
    break;
  case ACTION_UPDATE:
    std::cout << "Update";
    break;
  }
  std::cout << " " << m_obj << std::endl;
}

ZStackDocDataBuffer::ZStackDocDataBuffer(QObject *parent) : QObject(parent)
{
}

ZStackDocDataBuffer::~ZStackDocDataBuffer()
{
  clear();
}

void ZStackDocDataBuffer::addUpdate(
    ZStackObject *obj, ZStackDocObjectUpdate::EAction action)
{
  QMutexLocker locker(&m_mutex);
  m_updateList.append(new ZStackDocObjectUpdate(obj, action));
}

void ZStackDocDataBuffer::deliver()
{
  emit delivering();
}

QList<ZStackDocObjectUpdate*> ZStackDocDataBuffer::take()
{
  QMutexLocker locker(&m_mutex);

  QList<ZStackDocObjectUpdate*> updateList = m_updateList;

  m_updateList.clear();

  return updateList;
}

void ZStackDocDataBuffer::clear()
{
  QMutexLocker locker(&m_mutex);

  for (QList<ZStackDocObjectUpdate*>::iterator iter = m_updateList.begin();
       iter != m_updateList.end(); ++iter) {
    ZStackDocObjectUpdate *u = *iter;

    delete u;
  }
}

void ZStackDocDataBuffer::clearList()
{
  QMutexLocker locker(&m_mutex);

  m_updateList.clear();
}

QMap<ZStackObject*, ZStackDocObjectUpdate::EAction>
ZStackDocObjectUpdate::MakeActionMap(QList<ZStackDocObjectUpdate *> updateList)
{
  //Rules for processing actions of the same object:
  //  If the last action is delete, then all the other actions
  QMap<ZStackObject*, ZStackDocObjectUpdate::EAction> actionMap;
  for (QList<ZStackDocObjectUpdate*>::reverse_iterator iter = updateList.rbegin();
       iter != updateList.rend(); ++iter) {
    ZStackDocObjectUpdate *u = *iter;
    if (!actionMap.contains(u->getObject())) {
      actionMap[u->getObject()] = u->getAction();
    } else {
      ZStackDocObjectUpdate::EAction laterAction = actionMap[u->getObject()];
      if (laterAction == ACTION_RECYCLE || laterAction == ACTION_EXPEL ||
          laterAction == ACTION_KILL) {
        if (laterAction > u->getAction()) {
          u->setAction(ACTION_NULL);
        }
      }
    }
  }

  return actionMap;
}
