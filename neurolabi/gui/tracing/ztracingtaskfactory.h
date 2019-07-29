#ifndef ZTRACINGTASKFACTORY_H
#define ZTRACINGTASKFACTORY_H

class ZTask;

class ZTracingTaskFactory
{
public:
  ZTracingTaskFactory();

  virtual ZTask* makeTask(double x, double y, double z, double r) const = 0;
};

#endif // ZTRACINGTASKFACTORY_H
