#ifndef NEUTUBE_DEF_H
#define NEUTUBE_DEF_H

#include <limits>
#include "tz_stdint.h"

#define BIT_FLAG(n) (((n) <= 0) ? 0 : ((uint64_t) 1) << ((n) - 1))

namespace NeuTube {

enum ESyncOption {
  SYNC, NO_SYNC
};

enum EDocumentableType {
  Documentable_SWC, Documentable_PUNCTUM, Documentable_OBJ3D,
  Documentable_STROKE, Documentable_LOCSEG_CHAIN, Documentable_CONN,
  Documentable_SPARSE_OBJECT, Documentable_Circle
};

namespace Document {
enum ETag {
  NORMAL, BIOCYTIN_PROJECTION, BIOCYTIN_STACK, FLYEM_BODY, FLYEM_COARSE_BODY,
  FLYEM_BODY_3D, FLYEM_BODY_3D_COARSE, FLYEM_SKELETON,
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

enum ECardinalDirection {
  CD_NORTH, CD_EAST, CD_SOUTH, CD_WEST
};

namespace Display {
typedef uint64_t TVisualEffect;
static const TVisualEffect VE_NONE = 0;
static const TVisualEffect VE_Z_PROJ = BIT_FLAG(19);
static const TVisualEffect VE_GROUP_HIGHLIGHT = BIT_FLAG(20);

namespace Image {
static const TVisualEffect VE_HIGH_CONTRAST = 1;
}

namespace LabelField {
static const TVisualEffect VE_HIGHLIGHT_SELECTED = 1;
}

namespace Sphere {
static const TVisualEffect VE_DASH_PATTERN = BIT_FLAG(1);
static const TVisualEffect VE_BOUND_BOX = BIT_FLAG(2);
static const TVisualEffect VE_NO_CIRCLE = BIT_FLAG(3);
static const TVisualEffect VE_NO_FILL = BIT_FLAG(4);
static const TVisualEffect VE_GRADIENT_FILL = BIT_FLAG(5);
static const TVisualEffect VE_OUT_FOCUS_DIM = BIT_FLAG(6);
static const TVisualEffect VE_DOT_CENTER = BIT_FLAG(7);
static const TVisualEffect VE_RECTANGLE_SHAPE = BIT_FLAG(8);
static const TVisualEffect VE_CROSS_CENTER = BIT_FLAG(9);
static const TVisualEffect VE_FORCE_FILL = BIT_FLAG(10);
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
static const TVisualEffect VE_PLANE_BOUNDARY = BIT_FLAG(2);
}
}
static const int INVALID_Z_INDEX = INT_MIN;

}

namespace FlyEM {
enum EDvidAnnotationLoadMode {
  LOAD_NO_PARTNER, LOAD_PARTNER_LOCATION, LOAD_PARTNER_RELJSON
};

enum EProofreadingMode {
  PR_NORMAL, PR_SPLIT
};

static const uint64_t LABEL_ID_SELECTION = std::numeric_limits<uint64_t>::max() - 1;

}


#endif // NEUTUBE_DEF_H
