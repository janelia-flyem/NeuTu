#include "taskwriter.h"
#include "taskurl.h"
#include "task.h"

namespace  neutuse {

TaskWriter::TaskWriter()
{

}

void TaskWriter::uploadTask(const Task &task)
{
  post(TaskUrl::GetTaskPath(1), task.toJsonObject());
}

}
