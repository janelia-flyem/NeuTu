#ifndef DATA3D_DEFS_H
#define DATA3D_DEFS_H

namespace neutu {

namespace data3d {

enum class ESpace  {
  MODEL, VIEW, CANVAS
};

//2D object scene: {Settled {static data {PIXEL, HD}, dynamic data}, Roaming}
enum class ETarget {
  STACK_CANVAS,
  TILE_CANVAS,
  MASK_CANVAS,
  DYNAMIC_OBJECT_CANVAS,
  PIXEL_OBJECT_CANVAS,
  HD_OBJECT_CANVAS,
  ROAMING_OBJECT_CANVAS,
  WIDGET,
  CANVAS_3D,
  ONLY_3D,
  TARGET_NONE
};

const static int TARGET_COUNT = 11;

}

}

#endif // DATA3D_DEFS_H
