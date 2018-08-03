#include "ztask.h"
#include <iostream>

ZTask::ZTask(QObject *parent) : QObject(parent) {
  setAutoDelete(false);
}

ZTask::~ZTask()
{
#ifdef _DEBUG_2
  std::cout << "ZTask destroyed." << std::endl;
#endif
}

void ZTask::run()
{
  execute();

  emit finished();
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
