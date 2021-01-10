#ifndef ZWORKERWRAPPER_H
#define ZWORKERWRAPPER_H

class ZWorker;
class ZWorkThread;
class ZTask;

class ZWorkerWrapper
{
public:
  ZWorkerWrapper();
  virtual ~ZWorkerWrapper();

  void startWorkThread();
  void endWorkThread();

protected:
  void addTask(ZTask *task);

private:
  ZWorker *m_worker = nullptr;
  ZWorkThread *m_workThread = nullptr;
};

#endif // ZWORKERWRAPPER_H
