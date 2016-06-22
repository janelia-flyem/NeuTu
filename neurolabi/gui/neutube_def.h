#ifndef NEUTUBE_DEF_H
#define NEUTUBE_DEF_H

#include "tz_stdint.h"

namespace NeuTube {
enum EDocumentableType {
  Documentable_SWC, Documentable_PUNCTUM, Documentable_OBJ3D,
  Documentable_STROKE, Documentable_LOCSEG_CHAIN, Documentable_CONN,
  Documentable_SPARSE_OBJECT, Documentable_Circle
};

namespace Document {
enum ETag {
  NORMAL, BIOCYTIN_PROJECTION, BIOCYTIN_STACK, FLYEM_BODY, FLYEM_COARSE_BODY,
  FLYEM_QUICK_BODY, FLYEM_QUICK_BODY_COARSE, FLYEM_SKELETON,
  FLYEM_STACK,
  FLYEM_SPLIT, FLYEM_ROI, FLYEM_MERGE, SEGMENTATION_TARGET, FLYEM_DVID,
  FLYEM_BODY_DISPLAY, FLYEM_PROOFREAD, FLYEM_ORTHO
};
}

namespace View {
enum EExploreAction {
  EXPLORE_NONE, EXPLORE_MOVE, EXPLORE_ZOOM, EXPLORE_SLICE,
  EXPLORE_ZOOM_DONE, EXPLORE_MOVE_DONE,
  EXPLORE_UNKNOWN
};
}

enum EImageBackground {
  IMAGE_BACKGROUND_BRIGHT, IMAGE_BACKGROUND_DARK
};

enum ESizeHintOption {
  SIZE_HINT_DEFAULT, SIZE_HINT_CURRENT_BEST, SIZE_HINT_TAKING_SPACE
};

enum EAxis {
  X_AXIS, Y_AXIS, Z_AXIS
};

enum EPLANE {
  PLANE_XY, PLANE_XZ, PLANE_YZ
};

enum EAxisSystem {
  AXIS_NORMAL, AXIS_SHIFTED
};

enum ECoordinateSystem {
  COORD_WIDGET, COORD_SCREEN, COORD_RAW_STACK, COORD_STACK,
  COORD_WORLD, COORD_CANVAS
};

enum EColor {
  COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_ALL
};

enum EWindowConfig {
  WINDOW_2D, WINDOW_3D
};

enum EMessageType {
  MSG_INFORMATION, MSG_WARNING, MSG_ERROR, MSG_DEBUG
};

enum EBodyLabelType {
  BODY_LABEL_ORIGINAL, BODY_LABEL_MAPPED
};

enum EBiDirection {
  DIRECTION_FORWARD, DIRECTION_BACKWARD
};

namespace Display {
typedef uint64_t TVisualEffect;
static const TVisualEffect VE_NONE = 0;
static const TVisualEffect VE_Z_PROJ = 0x0000000100000000;

namespace Image {
static const TVisualEffect VE_HIGH_CONTRAST = 1;
}

namespace Sphere {
static const TVisualEffect VE_DASH_PATTERN = 1;
static const TVisualEffect VE_BOUND_BOX = 2;
static const TVisualEffect VE_NO_CIRCLE = 4;
static const TVisualEffect VE_NO_FILL = 8;
static const TVisualEffect VE_GRADIENT_FILL = 16;
static const TVisualEffect VE_OUT_FOCUS_DIM = 32;
static const TVisualEffect VE_DOT_CENTER = 64;
static const TVisualEffect VE_RECTANGLE_SHAPE = 128;
static const TVisualEffect VE_CROSS_CENTER = 256;
static const TVisualEffect VE_FORCE_FILL = 512;
}

namespace SwcTree {
static const TVisualEffect VE_FULL_SKELETON = 1;
}

namespace Line {
static const TVisualEffect VE_LINE_PROJ = 1;
static const TVisualEffect VE_LINE_FADING_PROJ = 2;
}

namespace SparseObject {
static const TVisualEffect VE_FORCE_SOLID = 1;
}

}

namespace FlyEM {
enum ESynapseLoadMode {
  LOAD_NO_PARTNER, LOAD_PARTNER_LOCATION
};
}


}


#endif // NEUTUBE_DEF_H
