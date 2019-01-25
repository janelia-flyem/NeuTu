#include "utilities.h"

std::string zstackobject::ToString(ZStackObject::ETarget target)
{
  switch (target) {
  case ZStackObject::ETarget::NONE:
    return "NULL";
  case ZStackObject::ETarget::TARGET_OBJECT_CANVAS:
    return "object canvas";
  case ZStackObject::ETarget::TARGET_WIDGET:
    return "widget";
  case ZStackObject::ETarget::TARGET_TILE_CANVAS:
    return "tile canvas";
  case ZStackObject::ETarget::TARGET_3D_ONLY:
    return "3D";
  case ZStackObject::ETarget::TARGET_DYNAMIC_OBJECT_CANVAS:
    return "dynamic object canvas";
  case ZStackObject::ETarget::TARGET_3D_CANVAS:
    return "3D canvas";
  case ZStackObject::ETarget::TARGET_STACK_CANVAS:
    return "stack canvas";
  }

  return std::to_string(neutube::EnumValue(target));
}
