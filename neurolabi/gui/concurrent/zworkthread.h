#ifndef ZWORKTHREAD_H
#define ZWORKTHREAD_H

#include <QThread>
class ZWorker;

class ZWorkThread : public QThread
{
  Q_OBJECT
public:
  explicit ZWorkThread(ZWorker *worker, QObject *parent = nullptr);
  virtual ~ZWorkThread();

signals:

public slots:

private:
  void setWorker(ZWorker *worker);

private:
  ZWorker *m_worker;
};

#endif // ZWORKTHREAD_H
