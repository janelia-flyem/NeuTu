#include "ztaskqueue.h"
#include "ztask.h"

ZTaskQueue::ZTaskQueue(QObject *parent) : QObject(parent)
{

}

ZTaskQueue::~ZTaskQueue()
{
  foreach (ZTask *task, m_queue) {
    task->deleteLater();
  }
}

ZTask* ZTaskQueue::get()
{
  QMutexLocker locker(&m_queueLock);

  while (m_queue.isEmpty()) {
    m_queueHasItems.wait(&m_queueLock);
  }
  return m_queue.dequeue();
}

bool ZTaskQueue::isEmpty() {
    return m_queue.isEmpty();
}

void ZTaskQueue::add(ZTask *task)
{
  QMutexLocker locker(&m_queueLock);

  bool wasEmpty = m_queue.isEmpty();
  m_queue.enqueue(task);
  if (wasEmpty) {
    m_queueHasItems.wakeAll();
  }
}
