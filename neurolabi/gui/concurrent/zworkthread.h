#ifndef ZWORKTHREAD_H
#define ZWORKTHREAD_H

#include <QThread>

class ZWorker;
class ZTask;

class ZWorkThread : public QThread
{
  Q_OBJECT
public:
  explicit ZWorkThread(ZWorker *worker, QObject *parent = nullptr);
  virtual ~ZWorkThread();

  void cancelAndQuit();
  void addTask(ZTask *task);

signals:

public slots:

private:
  void setWorker(ZWorker *worker);

private:


private:
  ZWorker *m_worker;
};

#endif // ZWORKTHREAD_H
