#ifndef PROTOCOLTASKCONFIG_H
#define PROTOCOLTASKCONFIG_H

#include <QString>

#include "common/neutube_def.h"

class ProtocolTaskConfig
{
public:
  ProtocolTaskConfig();

  QString getTaskType() const;
  neutu::EToDoAction getDefaultTodo() const;

  void setTaskType(const QString &type);
  void setDefaultTodo(neutu::EToDoAction action);

private:
  QString m_taskType;
  neutu::EToDoAction m_defaultTodo = neutu::EToDoAction::TO_DO;
};

#endif // PROTOCOLTASKCONFIG_H
