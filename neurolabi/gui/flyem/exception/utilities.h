#ifndef _FLYEM_EXCEPTION_UTILITIES_H
#define _FLYEM_EXCEPTION_UTILITIES_H

#include "common/neutudefs.h"
#include "zwidgetmessage.h"

namespace flyem {

class SplitErrorMessageBuilder : public ZWidgetMessageBuilder
{
public:
  SplitErrorMessageBuilder();
  SplitErrorMessageBuilder& forBody(
      uint64_t bodyId, neutu::EBodyLabelType type);
};

}

#endif // UTILITIES_H
