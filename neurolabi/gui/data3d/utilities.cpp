#include "utilities.h"

std::string zstackobject::ToString(ZStackObject::ETarget target)
{
  switch (target) {
  case ZStackObject::ETarget::NONE:
    return "NULL";
  case ZStackObject::ETarget::OBJECT_CANVAS:
    return "object canvas";
  case ZStackObject::ETarget::WIDGET:
    return "widget";
  case ZStackObject::ETarget::TILE_CANVAS:
    return "tile canvas";
  case ZStackObject::ETarget::ONLY_3D:
    return "3D";
  case ZStackObject::ETarget::DYNAMIC_OBJECT_CANVAS:
    return "dynamic object canvas";
  case ZStackObject::ETarget::CANVAS_3D:
    return "3D canvas";
  case ZStackObject::ETarget::STACK_CANVAS:
    return "stack canvas";
  }

  return std::to_string(neutube::EnumValue(target));
}
