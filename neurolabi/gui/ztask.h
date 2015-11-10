#ifndef ZTASK_H
#define ZTASK_H

#include <QObject>
#include <QRunnable>

class ZTask : public QObject, public QRunnable
{
  Q_OBJECT

public:
  explicit ZTask(QObject *parent = 0);
  virtual ~ZTask();

  void run();
  virtual void execute() = 0;
  virtual void prepare() {}

signals:
  void finished();
};

/////////////Moc class for testing//////////////
class ZSquareTask : public ZTask
{
  //Q_OBJECT

public:
  ZSquareTask(QObject *parent = NULL);

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
