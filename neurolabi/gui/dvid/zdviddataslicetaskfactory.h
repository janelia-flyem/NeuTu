#ifndef ZDVIDDATASLICETASKFACTORY_H
#define ZDVIDDATASLICETASKFACTORY_H

class ZDvidDataSliceTask;

class ZDvidDataSliceTaskFactory
{
public:
  ZDvidDataSliceTaskFactory();
  virtual ~ZDvidDataSliceTaskFactory();

  virtual ZDvidDataSliceTask* makeTask() = 0;

};

#endif // ZDVIDDATASLICETASKFACTORY_H
