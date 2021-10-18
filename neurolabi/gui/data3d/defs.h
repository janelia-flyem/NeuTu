#ifndef DATA3D_DEFS_H
#define DATA3D_DEFS_H

namespace neutu {

namespace data3d {

enum class ESpace  {
  MODEL, VIEW, CANVAS
};

//2D object scene:
//{
//  Settled {
//    image,
//    object{ {static data {PIXEL, HD}, dynamic data} },
//  Roaming
//}
enum class ETarget {
  STACK_CANVAS = 0,
  TILE_CANVAS,
  MASK_CANVAS,
  DYNAMIC_OBJECT_CANVAS,
  PIXEL_OBJECT_CANVAS,
  HD_OBJECT_CANVAS,
  NONBLOCKING_OBJECT_CANVAS,
  ROAMING_OBJECT_CANVAS,
  WIDGET,
  CANVAS_3D,
  ONLY_3D,
  TARGET_NONE
};

const static int TARGET_COUNT = int(ETarget::TARGET_NONE);

enum class EType { //#Review-TZ: Consider moving types to a separate file with namespace zstackobject
  UNIDENTIFIED = 0, //Unidentified type
  SWC,
  PUNCTUM,
  MESH,
  OBJ3D,
  STROKE,
  LOCSEG_CHAIN,
  CONN,
  OBJECT3D_SCAN,
  SPARSE_OBJECT,
  CIRCLE,
  STACK_BALL,
  STACK_PATCH,
  RECT2D,
  DVID_TILE,
  DVID_GRAY_SLICE,
  DVID_GRAY_SLICE_ENSEMBLE,
  DVID_TILE_ENSEMBLE,
  DVID_LABEL_SLICE,
  DVID_SPARSE_STACK,
  DVID_SPARSEVOL_SLICE,
  STACK,
  SWC_NODE,
  GRAPH_3D,
  PUNCTA,
  FLYEM_BOOKMARK,
  INT_CUBOID,
  LINE_SEGMENT,
  SLICED_PUNCTA,
  DVID_SYNAPSE,
  DVID_SYNAPE_ENSEMBLE,
  CUBE,
  DVID_ANNOTATION,
  FLYEM_TODO_ITEM,
  FLYEM_TODO_LIST,
  FLYEM_TODO_ENSEMBLE,
  FLYEM_SYNAPSE_ENSEMBLE,
  CROSS_HAIR,
  SEGMENTATION_ENCODER
};

} //namespace data3d

} //namespace neutu

#endif // DATA3D_DEFS_H
