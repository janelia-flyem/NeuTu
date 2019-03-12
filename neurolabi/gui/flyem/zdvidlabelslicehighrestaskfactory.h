#ifndef ZDVIDLABELSLICEHIGHRESTASKFACTORY_H
#define ZDVIDLABELSLICEHIGHRESTASKFACTORY_H

#include "dvid/zdviddataslicetaskfactory.h"

class ZDvidLabelSliceHighresTaskFactory : public ZDvidDataSliceTaskFactory
{
public:
  ZDvidLabelSliceHighresTaskFactory();

  ZDvidDataSliceTask* makeTask() override;
};

#endif // ZDVIDLABELSLICEHIGHRESTASKFACTORY_H
