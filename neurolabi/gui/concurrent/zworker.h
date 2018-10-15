#ifndef ZWORKER_H
#define ZWORKER_H

#include <QObject>

class ZTaskQueue;
class ZTask;

class ZWorker : public QObject
{
  Q_OBJECT
public:
  enum class EMode {
    QUEUE, SCHEDULE
  };

  explicit ZWorker(EMode mode, QObject *parent = nullptr);
  virtual ~ZWorker();

  EMode getMode() const {
    return m_mode;
  }

  void addTask(ZTask *task);
//  void setTaskQueue(ZTaskQueue *queue);

  void quit();

signals:
  void finished();
  void schedulingTask(ZTask *task);

public slots:
  void process();
  void processTask(ZTask *task);
  void scheduleTask(ZTask *task);

private:
  ZTaskQueue *m_taskQueue = nullptr;
  bool m_quiting =false;
  EMode m_mode = EMode::QUEUE;
};

#endif // ZWORKER_H
