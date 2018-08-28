#include "zflyemtaskhelper.h"

#include "protocols/protocoltaskfactory.h"

ZFlyEmTaskHelper::ZFlyEmTaskHelper()
{

}

void ZFlyEmTaskHelper::setCurrentTaskType(const QString &type)
{
  m_taskType = type;
}

neutube::EToDoAction ZFlyEmTaskHelper::getPreferredTodoAction() const
{
  if (m_taskType == ProtocolTaskFactory::TASK_BODY_CLEAVE) {
    return neutube::TO_SUPERVOXEL_SPLIT;
  }

  return m_defaultTodoAction;
}

