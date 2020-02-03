#include "ztask.h"
#include <iostream>
#include <QTimer>

#include "QsLog.h"

ZTask::ZTask(QObject *parent) : QObject(parent)
{
  setAutoDelete(false);
//  if (parent == nullptr) {
  connect(this, &ZTask::finished, this, &ZTask::deleteLater);
  connect(this, &ZTask::aborted, this, &ZTask::deleteLater);
//    connect(this, &ZTask::finished, this, &ZTask::slotTest);
//  }
}

ZTask::~ZTask()
{
#ifdef _DEBUG_2
  std::cout << "ZTask destroyed." << std::endl;
#endif
}

void ZTask::run()
{
  executeSlot();
}

void ZTask::abort()
{
  emit aborted();
}

void ZTask::executeSlot()
{
  execute();

  emit finished();
}

void ZTask::setDelay(int delay)
{
  m_delay = delay;
}

int ZTask::getDelay() const
{
  return m_delay;
}

void ZTask::slotTest()
{
  LDEBUG() << "slot test";
}

//////////////////////////////

ZFunctionTask::ZFunctionTask(QObject *parent) : ZTask(parent)
{

}

ZFunctionTask::ZFunctionTask(
    std::function<void()> f, std::function<void()> dispose, QObject *parent) :
  ZTask(parent), m_f(f), m_dispose(dispose)
{
}

/////////////Moc class for testing//////////////
ZSquareTask::ZSquareTask(QObject *parent) :
  ZTask(parent), m_value(0), m_result(0)
{
}

void ZSquareTask::run()
{
#if _DEBUG_
  std::cout << "Compute " << m_value << std::endl;
#endif

  m_result = m_value * m_value;

  emit finished();

#if _DEBUG_2
  std::cout << "finished() emitted from ZSquareTask::run() " << std::endl;
#endif
}

/*
void ZSquareTask::test()
{
  std::cout << "ZSquareTask::test" << std::endl;
}
*/
