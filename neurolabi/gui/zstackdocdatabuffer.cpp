#include "zstackdocdatabuffer.h"

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
