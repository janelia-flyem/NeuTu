#ifndef ZFLYEMTASKHELPER_H
#define ZFLYEMTASKHELPER_H

#include <QString>

#include "neutube_def.h"

/*!
 * \brief The class of helping with coordinating task workflows
 */
class ZFlyEmTaskHelper
{
public:
  ZFlyEmTaskHelper();

public:
  void setCurrentTaskType(const QString &type);
  void setDefaultTodoAction(neutube::EToDoAction action);
  neutube::EToDoAction getPreferredTodoAction() const;

private:
  QString m_taskType;
  neutube::EToDoAction m_defaultTodoAction = neutube::TO_SPLIT;
};

#endif // ZFLYEMTASKHELPER_H
