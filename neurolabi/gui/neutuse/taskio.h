#ifndef TASKIO_H
#define TASKIO_H

#include <string>

namespace neutuse {

class TaskIO
{
public:
  TaskIO();
  TaskIO(const std::string &address);

protected:
  std::string m_address;
  int m_statusCode = 0;
};

}

#endif // TASKIO_H
