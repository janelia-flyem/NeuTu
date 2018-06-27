#ifndef ZTASKQUEUE_H
#define ZTASKQUEUE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class ZTask;

class ZTaskQueue : public QObject
{
  Q_OBJECT
public:
  explicit ZTaskQueue(QObject *parent = nullptr);
  ~ZTaskQueue();

  ZTask* get();
  bool isEmpty();
  /*!
   * \brief Add a task to the queue.
   *
   * Note: When \a task is NULL, it can be used to indicate the end of the queue.
   *
   * \param task The task to be added.
   */
  void add(ZTask *task);

signals:

public slots:

private:
  QQueue<ZTask*> m_queue;
  QMutex m_queueLock;
  QWaitCondition m_queueHasItems;
};

#endif // ZTASKQUEUE_H
