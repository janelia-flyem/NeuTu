#ifndef ZWORKER_H
#define ZWORKER_H

#include <QObject>
#include <QMap>
#include <QString>

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
  void invalidateNamedTask(const QString &name);
  void invalidateNamedTask(ZTask *task, QMutex *mutex);
  void invalidateNamedTask(const QString &name, QMutex *mutex);
  void addNamedTask(ZTask *task);

private slots:
  void disposeTask(ZTask *task);

private:
  ZTaskQueue *m_taskQueue = nullptr;
  QMap<QString, ZTask*> m_namedTaskMap;
  QMutex m_nameTaskMapLock;
  bool m_quiting =false;
  EMode m_mode = EMode::QUEUE;
};

#endif // ZWORKER_H
