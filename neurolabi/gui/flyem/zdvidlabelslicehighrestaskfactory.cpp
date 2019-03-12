#include "zdvidlabelslicehighrestaskfactory.h"

#include "zdvidlabelslicehighrestask.h"

ZDvidLabelSliceHighresTaskFactory::ZDvidLabelSliceHighresTaskFactory()
{
}

ZDvidDataSliceTask* ZDvidLabelSliceHighresTaskFactory::makeTask()
{
  return new ZDvidLabelSliceHighresTask;
}
