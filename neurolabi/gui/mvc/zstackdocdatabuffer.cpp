#include "zstackdocdatabuffer.h"

#include <iostream>

ZStackDocDataBuffer::ZStackDocDataBuffer(QObject *parent) : QObject(parent)
{
}

ZStackDocDataBuffer::~ZStackDocDataBuffer()
{
  clear();
}

void ZStackDocDataBuffer::addUpdate(ZStackDocObjectUpdate *u)
{
  QMutexLocker locker(&m_mutex);
  m_updateList.append(u);
}

void ZStackDocDataBuffer::addUpdate(
    ZStackObject *obj, ZStackDocObjectUpdate::EAction action)
{
  QMutexLocker locker(&m_mutex);
  m_updateList.append(new ZStackDocObjectUpdate(obj, action));
}

void ZStackDocDataBuffer::addUpdate(
    QList<ZStackObject *> objList, ZStackDocObjectUpdate::EAction action)
{
  QMutexLocker locker(&m_mutex);
  for (QList<ZStackObject *>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    m_updateList.append(new ZStackDocObjectUpdate(*iter, action));
  }
}

void ZStackDocDataBuffer::addUpdate(std::function<void(ZStackObject*obj)> f)
{
  addUpdate(ZStackDocObjectUpdateFactory::Make(f));
}

void ZStackDocDataBuffer::addUpdate(std::function<void()> f)
{
  addUpdate(ZStackDocObjectUpdateFactory::Make(f));
}

void ZStackDocDataBuffer::removeObjectUpdate(
    std::function<bool (ZStackDocObjectUpdate*)> pred)
{
  QMutexLocker locker(&m_mutex);

  for (QList<ZStackDocObjectUpdate*>::iterator iter = m_updateList.begin();
       iter != m_updateList.end(); /*++iter*/) {
    ZStackDocObjectUpdate *u = *iter;
    if (pred(u)) {
      iter = m_updateList.erase(iter);
      delete u;
    } else {
      ++iter;
    }
  }
}

void ZStackDocDataBuffer::deliver()
{
  emit delivering();
}

int ZStackDocDataBuffer::getActionCount(
    ZStackDocObjectUpdate::EAction action) const
{
  QMutexLocker locker(&m_mutex);
  int count = 0;
  for (const auto &u : m_updateList) {
    if (u->getAction() == action) {
      ++count;
    }
  }

  return count;
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

void ZStackDocDataBuffer::print() const
{
  QMutexLocker locker(&m_mutex);
  std::cout << m_updateList.size() << " objects to update" << std::endl;
  for (const auto &u : m_updateList) {
    u->print();
  }
}
/*
QMap<ZStackObject*, ZStackDocObjectUpdate::EAction>
ZStackDocObjectUpdate::MakeActionMap(QList<ZStackDocObjectUpdate *> updateList)
{
  //Rules for processing actions of the same object:
  //  If the last action is delete, then all the other actions will be invalidated
  QMap<ZStackObject*, ZStackDocObjectUpdate::EAction> actionMap;
  for (QList<ZStackDocObjectUpdate*>::reverse_iterator iter = updateList.rbegin();
       iter != updateList.rend(); ++iter) {
    ZStackDocObjectUpdate *u = *iter;
    if (!actionMap.contains(u->getObject())) {
      actionMap[u->getObject()] = u->getAction();
    } else {
      ZStackDocObjectUpdate::EAction laterAction = actionMap[u->getObject()];
      if (laterAction == EAction::RECYCLE ||
          laterAction == EAction::EXPEL ||
          laterAction == EAction::KILL) {
        if (laterAction > u->getAction()) {
          u->setAction(EAction::NONE);
        }
      }
    }
  }

  return actionMap;
}
*/
