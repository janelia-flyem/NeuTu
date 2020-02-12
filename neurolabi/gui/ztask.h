#ifndef ZTASK_H
#define ZTASK_H

#include <functional>

#include <QObject>
#include <QRunnable>

class ZTask : public QObject, public QRunnable
{
  Q_OBJECT

public:
  explicit ZTask(QObject *parent = nullptr);
  virtual ~ZTask() override;

  void run() override;
  virtual void execute() = 0;
  virtual void prepare() {}

  static void ExecuteTask(ZTask *task) {
    task->execute();
  }

  void setDelay(int delay);
  int getDelay() const;

  bool isValid() const {
    return m_isValid;
  }

  void abort();
  void disableAutoDelete();
  void invalidate();

  QString getName() const {
    return m_name;
  }

  void setName(const QString &name) {
    m_name = name;
  }

  bool skippingUponNameDuplicate() const {
    return m_skipUponNameDuplicate;
  }

  void skipUponNameDuplicate(bool on) {
    m_skipUponNameDuplicate = on;
  }

public slots:
  void executeSlot();
  void slotTest();

signals:
  void finished(ZTask*);
  void aborted(ZTask*);

private:
  int m_delay = 0;
  bool m_isValid = true;
  QString m_name;
  bool m_skipUponNameDuplicate = false;
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
    if (m_dispose) {
      m_dispose();
    }
  }

  void execute() override {
    if (m_f) {
      m_f();
    }
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

//  void run();

  void execute() override;

  inline void setValue(double value) {
    m_value = value;
  }

  inline double getResult() { return m_result; }

private:
  double m_value;
  double m_result;
};



#endif // ZTASK_H
