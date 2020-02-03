#include "zworker.h"

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
  emit schedulingTask(task);
}

void ZWorker::addTask(ZTask *task)
{
  if (m_taskQueue != nullptr) {
    m_taskQueue->add(task);
  } else {
    if (task != nullptr) {
      task->moveToThread(thread());
      task->setParent(this);
//      task->moveToThread(thread());
      if (task->getDelay() > 0) {
        QTimer::singleShot(task->getDelay(), this, [=]() {
          scheduleTask(task);
        });
      } else {
        emit schedulingTask(task);
      }
    }
  }
}
