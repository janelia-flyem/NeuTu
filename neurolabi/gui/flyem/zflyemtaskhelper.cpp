#include "zflyemtaskhelper.h"

#include "protocols/protocoltaskfactory.h"
#include "protocols/taskbodycleave.h"

ZFlyEmTaskHelper::ZFlyEmTaskHelper()
{

}

void ZFlyEmTaskHelper::setCurrentTaskType(const QString &type)
{
  m_taskType = type;
}

/*
neutube::EToDoAction ZFlyEmTaskHelper::getPreferredTodoAction() const
{
  if (m_taskType == ProtocolTaskFactory::TASK_BODY_CLEAVE) {
    return neutube::TO_SUPERVOXEL_SPLIT;
  }

  return m_defaultTodoAction;
}
*/

void ZFlyEmTaskHelper::ResolveShortcutForSplitting(
    TaskProtocolTask *task, bool splitting)
{
  TaskBodyCleave *cleaveTask = qobject_cast<TaskBodyCleave*>(task);
  if (cleaveTask != nullptr) {
    if (splitting) {
      cleaveTask->disableCleavingShortcut();
    } else {
      cleaveTask->enableCleavingShortcut();
    }
  }
}
