#ifndef TASK_H
#define TASK_H

#include <string>

#include "zjsonobject.h"

namespace neutuse {

class Task
{
public:
  Task();

public:
  ZJsonObject toJsonObject() const;

  void setType(const std::string &type);
  void setName(const std::string &name);
  void setUser(const std::string &user);
  void setConfig(const ZJsonObject &config);
  void setPriority(int p);

  std::string getType() const;
  std::string getName() const;
  int getPriority() const;

public:
  static const std::string KEY_TYPE;
  static const std::string KEY_NAME;
  static const std::string KEY_DESCRIPTION;
  static const std::string KEY_PRIORITY;
  static const std::string KEY_LIFE_SPAN;
  static const std::string KEY_MAX_TRIES;
  static const std::string KEY_CONFIG;
  static const std::string KEY_USER;

private:
  std::string m_type;
  std::string m_name;
  std::string m_description;
  int m_priority = 0;
  uint64_t m_lifeSpan = 0;
  int m_maxTries = 1;
  ZJsonObject m_config;
  std::string m_user;
};

}

#endif // TASK_H
