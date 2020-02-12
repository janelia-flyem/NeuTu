#include "zworker.h"

#include <iostream>

#include <QTimer>

//#include "logging/zqslog.h"
#include "ztask.h"
#include "ztaskqueue.h"

ZWorker::ZWorker(EMode mode, QObject *parent) : QObject(parent),
  m_mode(mode)
{
  if (mode == EMode::QUEUE) {
    m_taskQueue = new ZTaskQueue(this);
  } else {
    connect(this, SIGNAL(schedulingTask(ZTask*)),
            this, SLOT(processTask(ZTask*)), Qt::QueuedConnection);
  }
}

ZWorker::~ZWorker()
{
//  LDEBUG() << "Worker destroyed.";
}

void ZWorker::quit()
{
  disconnect(this, SIGNAL(schedulingTask(ZTask*)),
             this, SLOT(processTask(ZTask*)));
  addTask(nullptr);
  m_quiting = true;
//  LDEBUG() << "Quit worker";
}

void ZWorker::process()
{
//  LDEBUG() <</* */"Worker started";


  while (1) {
    ZTask *task = m_taskQueue->get();
    if (m_quiting) {
      break;
    }

    if (task) {
      task->run();
    } else {
      break;
    }
  }

//  LDEBUG() << "Worker finished";

  emit finished();
}

void ZWorker::processTask(ZTask *task)
{
  if (m_quiting) {
    task->abort();
  } else {
    task->run();
  }
}

void ZWorker::scheduleTask(ZTask *task)
{
  if (m_taskQueue) {
    m_taskQueue->add(task);
  } else {
    connect(task, SIGNAL(finished(ZTask*)),
            this, SLOT(disposeTask(ZTask*)), Qt::QueuedConnection);
    connect(task, SIGNAL(aborted(ZTask*)),
            this, SLOT(disposeTask(ZTask*)), Qt::QueuedConnection);

    emit schedulingTask(task);
  }
}

void ZWorker::invalidateNamedTask(ZTask *task, QMutex *mutex)
{
  if (task) {
    if (!task->getName().isEmpty()) {
      QMutexLocker locker(mutex);
      if (m_namedTaskMap.contains(task->getName())) {
        if (m_namedTaskMap.value(task->getName()) == task) {
          task->invalidate();
          m_namedTaskMap.remove(task->getName());
        }
      }
    }
  }
}

void ZWorker::invalidateNamedTask(const QString &name, QMutex *mutex)
{
  if (!name.isEmpty()) {
    QMutexLocker locker(mutex);
    if (m_namedTaskMap.contains(name)) {
      ZTask *task = m_namedTaskMap.value(name);
      task->invalidate();
      m_namedTaskMap.remove(name);
    }
  }
}

void ZWorker::invalidateNamedTask(const QString &name)
{
  invalidateNamedTask(name, &m_nameTaskMapLock);
}

void ZWorker::disposeTask(ZTask *task)
{
#ifdef _DEBUG_
  std::cout << "ZTask dispose: " << task << std::endl;
#endif
  invalidateNamedTask(task, &m_nameTaskMapLock);
  task->deleteLater();
}

void ZWorker::addNamedTask(ZTask *task)
{
  if (task) {
    QString name = task->getName();
    if (!name.isEmpty()) {
#ifdef _DEBUG_
      std::cout << "Add named task: " << task << std::endl;
#endif
      QMutexLocker locker(&m_nameTaskMapLock);
      if (m_namedTaskMap.contains(task->getName())) {
        if (task->skippingUponNameDuplicate()) {
          task->invalidate();
        } else {
          invalidateNamedTask(name, nullptr);
        }
      }
      if (task->isValid()) {
        m_namedTaskMap[name] = task;
      }
    }
  }
}

void ZWorker::addTask(ZTask *task)
{
  if (task) {
    addNamedTask(task);
    if (m_taskQueue == nullptr) {
      task->moveToThread(thread());
      task->setParent(this);
    }
    if (task->getDelay() > 0) {
      QTimer::singleShot(task->getDelay(), this, [=]() {
        scheduleTask(task);
      });
    } else {
      scheduleTask(task);
    }
  }
}
