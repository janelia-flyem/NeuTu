#ifndef TASKWRITER_H
#define TASKWRITER_H

#include "taskio.h"

namespace  neutuse {

class Task;

class TaskWriter : public TaskIO
{
public:
  TaskWriter();

public:
  void uploadTask(const Task &task);
};

}


#endif // TASKWRITER_H
