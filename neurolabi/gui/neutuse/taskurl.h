#ifndef TASKURL_H
#define TASKURL_H

#include <string>

namespace neutuse {

class TaskUrl
{
public:
  TaskUrl();

public:
  static std::string GetApiPath(int version);
  static std::string GetTaskPath(int version);

  std::string getApiPath() const;
  std::string getTaskPath() const;

private:
  std::string m_server;
  int m_version;
};

}

#endif // TASKURL_H
