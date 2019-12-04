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

  void setPriority(int p);
  void setForceUpdate(bool forceUpdate);

  static Task MakeDvidTask(
      const std::string &name,
      const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate);
  static Task MakeDvidSkeletonizeTask(
      const ZDvidTarget &target, uint64_t bodyId, bool forceUpdate);

  Task makeDvidTask(
      const std::string &name, const ZDvidTarget &target, uint64_t bodyId) const;
  Task makeDvidSkeletonizeTask(const ZDvidTarget &target, uint64_t bodyId) const;

private:
  int m_priority = 5;
  bool m_forceUpdate = true;
};

}

#endif // TASKFACTORY_H
