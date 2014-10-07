#ifndef ZKEYEVENTMAPPER_H
#define ZKEYEVENTMAPPER_H

#include "tz_utilities.h"

class ZInteractiveContext;
class ZStackDoc;

class ZKeyEventMapper
{
public:
  ZKeyEventMapper(ZInteractiveContext *context = NULL,
                  ZStackDoc *doc = NULL);
};

#endif // ZKEYEVENTMAPPER_H
