#ifndef ZFLYEMTASKHELPER_H
#define ZFLYEMTASKHELPER_H

#include <QString>

#include "common/neutube_def.h"

class TaskProtocolTask;

/*!
 * \brief The class of helping with coordinating task workflows
 */
class ZFlyEmTaskHelper
{
public:
  ZFlyEmTaskHelper();

public:
  void setCurrentTaskType(const QString &type);
//  void setDefaultTodoAction(neutube::EToDoAction action);
//  neutube::EToDoAction getPreferredTodoAction() const;

  static void ResolveShortcutForSplitting(
      TaskProtocolTask *task, bool splitting);

private:
  QString m_taskType;
//  neutube::EToDoAction m_defaultTodoAction = neutube::TO_SPLIT;
};

#endif // ZFLYEMTASKHELPER_H
