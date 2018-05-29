#ifndef ZWORKER_H
#define ZWORKER_H

#include <QObject>

class ZTaskQueue;
class ZTask;

class ZWorker : public QObject
{
  Q_OBJECT
public:
  explicit ZWorker(QObject *parent = nullptr);

  void addTask(ZTask *task);
//  void setTaskQueue(ZTaskQueue *queue);

signals:
  void finished();

public slots:
  void process();

private:
  ZTaskQueue *m_taskQueue = nullptr;
  bool m_quiting =false;
};

#endif // ZWORKER_H
