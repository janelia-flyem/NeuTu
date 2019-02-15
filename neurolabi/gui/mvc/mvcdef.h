#ifndef MVCDEF_H
#define MVCDEF_H

#include <QFlags>

#include "common/neutube_def.h"

namespace neutu {

namespace mvc {

enum class ViewInfoFlag {
  NONE = 0,
  RAW_STACK_COORD = BIT_FLAG(1),
  DATA_COORD = BIT_FLAG(2),
  WINDOW_SCALE = BIT_FLAG(3),
  IMAGE_VALUE = BIT_FLAG(4),
  MASK_VALUE = BIT_FLAG(5)
};

Q_DECLARE_FLAGS(ViewInfoFlags, ViewInfoFlag)

bool HasFlag(ViewInfoFlags host, ViewInfoFlags test);


} //namespace mvc

}


#endif // MVCDEF_H
