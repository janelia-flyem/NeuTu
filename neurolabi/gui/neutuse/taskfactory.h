#ifndef TASKFACTORY_H
#define TASKFACTORY_H

#include <string>

class ZDvidTarget;

namespace neutuse {

class Task;

class TaskFactory
{
public:
  TaskFactory();

  static Task MakeDvidTask(
      const std::string &name,
      const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate);
  static Task MakeDvidSkeletonizeTask(
      const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate);
};

}

#endif // TASKFACTORY_H
