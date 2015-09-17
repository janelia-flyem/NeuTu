#include "zflyemneuronfiltertaskmanager.h"
#include "zflyemneuron.h"
#include "zflyemneuronfilter.h"

ZFlyEmNeuronFilterTask::ZFlyEmNeuronFilterTask() :
  m_testNeuron(NULL), m_filter(NULL), m_result(NULL)
{

}

void ZFlyEmNeuronFilterTask::execute()
{
  if (m_filter != NULL) {
    if (m_testNeuron != NULL) {
      if (m_filter->isPassed(*m_testNeuron)) {
        m_result = m_testNeuron;
      }
    }
  }
}

//////////////////////ZFlyEmNeuronFilterTaskManager///////////////////////////
ZFlyEmNeuronFilterTaskManager::ZFlyEmNeuronFilterTaskManager(QObject *parent) :
  ZMultiTaskManager(parent)
{
}

void ZFlyEmNeuronFilterTaskManager::prepare()
{
  m_filterResult.clear();
}

void ZFlyEmNeuronFilterTaskManager::postProcess()
{
  foreach (ZTask *task, m_taskArray) {
    ZFlyEmNeuronFilterTask *filterTask =
        qobject_cast<ZFlyEmNeuronFilterTask*>(task);
    if (filterTask != NULL) {
      m_filterResult.append(filterTask->getResult());
    }
  }
}
