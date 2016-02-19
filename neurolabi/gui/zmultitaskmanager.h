#ifndef ZMULTITASKMANAGER_H
#define ZMULTITASKMANAGER_H

#include <QVector>
#include <QRunnable>
#include <QThread>
#include <QObject>
#include "zprogressable.h"

class QThreadPool;
class ZTask;

class ZMultiTaskManager : public QObject, public ZProgressable
{
  Q_OBJECT

public:
  ZMultiTaskManager(QObject *parent = NULL);
  ~ZMultiTaskManager();

  void addTask(ZTask *task = NULL);
  void start();
  void waitForDone();

  inline bool hasActiveTask() {
    return m_activeTaskNumber > 0;
  }

  /*!
   * \brief Clear all tasks
   *
   * It does nothing and return false when there is still at least one active
   * task.
   */
  bool clear();

protected:
  QThreadPool* getThreadPool() const;
  virtual void postProcess() {}
  virtual void prepare() {}

signals:
  void finished();

public slots:
  void process();

protected:
  QVector<ZTask*> m_taskArray;
  int m_activeTaskNumber;
};

class ZSquareTaskManager : public ZMultiTaskManager
{
  //Q_OBJECT

public:
  ZSquareTaskManager(QObject *parent = NULL);

protected:
  virtual void prepare();
  virtual void postProcess();

private:
  double m_result;
};

#endif // ZMULTITASKMANAGER_H
