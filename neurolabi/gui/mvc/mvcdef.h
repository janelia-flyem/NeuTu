#ifndef MVCDEF_H
#define MVCDEF_H

#include <QFlags>

#include "common/neutudefs.h"

namespace neutu {

namespace mvc {

enum class ERectTarget {
  PLANE_ROI, CUBOID_ROI
};

enum class ERoiRole {
  NONE, GENERAL, SPLIT
};

struct RectState {
  ERoiRole role = ERoiRole::NONE;
  ERectTarget target = ERectTarget::PLANE_ROI;
  bool appending = false;
};

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
