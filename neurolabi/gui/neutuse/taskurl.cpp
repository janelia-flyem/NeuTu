#include "taskurl.h"

namespace neutuse {

TaskUrl::TaskUrl()
{

}

std::string TaskUrl::GetApiPath(int version)
{
  return "/api/v" + std::to_string(version);
}

std::string TaskUrl::GetTaskPath(int version)
{
  return GetApiPath(version) + "/tasks";
}

std::string TaskUrl::getApiPath() const
{
  return GetApiPath(m_version);
}

std::string TaskUrl::getTaskPath() const
{
  return GetTaskPath(m_version);
}

}
