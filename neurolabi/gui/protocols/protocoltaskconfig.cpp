#include "protocoltaskconfig.h"

ProtocolTaskConfig::ProtocolTaskConfig()
{

}

QString ProtocolTaskConfig::getTaskType() const
{
  return m_taskType;
}

neutu::EToDoAction ProtocolTaskConfig::getDefaultTodo() const
{
  return m_defaultTodo;
}

void ProtocolTaskConfig::setTaskType(const QString &type)
{
  m_taskType = type;
}

void ProtocolTaskConfig::setDefaultTodo(neutu::EToDoAction action)
{
  m_defaultTodo = action;
}
