#include "task.h"

namespace neutuse {

const std::string Task::KEY_TYPE = "type";
const std::string Task::KEY_NAME = "name";
const std::string Task::KEY_DESCRIPTION = "description";
const std::string Task::KEY_PRIORITY = "priority";
const std::string Task::KEY_LIFE_SPAN = "life_span";
const std::string Task::KEY_MAX_TRIES = "max_tries";
const std::string Task::KEY_CONFIG = "config";
const std::string Task::KEY_USER = "user";

Task::Task()
{
}

ZJsonObject Task::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry(KEY_TYPE, m_type);
  obj.setEntry(KEY_NAME, m_name);
  obj.setEntry(KEY_PRIORITY, m_priority);
  if (m_lifeSpan > 0) {
    obj.setEntry(KEY_LIFE_SPAN, m_lifeSpan);
  }
  obj.setEntry(KEY_MAX_TRIES, m_maxTries);
  obj.setEntry(KEY_USER, m_user);
  obj.setEntry(KEY_CONFIG.c_str(), m_config.getData());

  return obj;
}

std::string Task::getType() const
{
  return m_type;
}

std::string Task::getName() const
{
  return m_name;
}

int Task::getPriority() const
{
  return m_priority;
}

void Task::setType(const std::string &type)
{
  m_type = type;
}

void Task::setName(const std::string &name)
{
  m_name = name;
}

void Task::setUser(const std::string &user)
{
  m_user = user;
}

void Task::setConfig(const ZJsonObject &config)
{
  m_config = config;
}

void Task::setPriority(int p)
{
  m_priority = p;
}

}
