#include "utilities.h"

std::string zstackobject::ToString(ZStackObject::ETarget target)
{
  switch (target) {
  case ZStackObject::TARGET_NULL:
    return "NULL";
  case ZStackObject::TARGET_OBJECT_CANVAS:
    return "object canvas";
  case ZStackObject::TARGET_WIDGET:
    return "widget";
  case ZStackObject::TARGET_TILE_CANVAS:
    return "tile canvas";
  case ZStackObject::TARGET_3D_ONLY:
    return "3D";
  case ZStackObject::TARGET_DYNAMIC_OBJECT_CANVAS:
    return "dynamic object canvas";
  case ZStackObject::TARGET_3D_CANVAS:
    return "3D canvas";
  case ZStackObject::TARGET_STACK_CANVAS:
    return "stack canvas";
  }

  return std::to_string(target);
}
