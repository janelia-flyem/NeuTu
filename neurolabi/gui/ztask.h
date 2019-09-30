#ifndef ZTASK_H
#define ZTASK_H

#include <QObject>
#include <QRunnable>

class ZTask : public QObject, public QRunnable
{
  Q_OBJECT

public:
  explicit ZTask(QObject *parent = nullptr);
  virtual ~ZTask();

  void run();
  virtual void execute() = 0;
  virtual void prepare() {}

  static void ExecuteTask(ZTask *task) {
    task->execute();
  }

  void setDelay(int delay);
  int getDelay() const;

  void abort();

public slots:
  void executeSlot();
  void slotTest();

signals:
  void finished();
  void aborted();

private:
  int m_delay = 0;
};

namespace {
const std::function<void()> VOID_FUNC = []() {};
}

class ZFunctionTask : public ZTask
{
  Q_OBJECT

public:
  explicit ZFunctionTask(QObject *parent = nullptr);
  explicit ZFunctionTask(
      std::function<void()> f, std::function<void()> dispose = VOID_FUNC,
      QObject *parent = nullptr);
  ~ZFunctionTask() override {
    m_dispose();
  }

  void execute() override {
    m_f();
  }

private:
  std::function<void()> m_f = VOID_FUNC;
  std::function<void()> m_dispose = VOID_FUNC;
};

/////////////Moc class for testing//////////////
class ZSquareTask : public ZTask
{
  //Q_OBJECT

public:
  ZSquareTask(QObject *parent = nullptr);

  void run();

  inline void setValue(double value) {
    m_value = value;
  }

  inline double getResult() { return m_result; }

private:
  double m_value;
  double m_result;
};



#endif // ZTASK_H
