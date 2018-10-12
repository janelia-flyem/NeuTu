#ifndef NEUTUBE_DEF_H
#define NEUTUBE_DEF_H

#include <limits>
#include <functional>

#include "tz_stdint.h"

#define BIT_FLAG(n) (((n) <= 0) ? 0 : ((uint64_t) 1) << ((n) - 1))

namespace neutube {

static const uint64_t ONEGIGA = 1073741824;
static const uint64_t HALFGIGA = 536870912;

static const char *VERSION = "1.1";

enum class ESyncOption {
  SYNC, NO_SYNC
};

enum class EDocumentableType { //Obsolete
  SWC, PUNCTUM, OBJ3D, STROKE, LOCSEG_CHAIN, CONN, SPARSE_OBJECT, Circle
};

namespace Document {
enum class ETag {
  NORMAL, BIOCYTIN_PROJECTION, BIOCYTIN_STACK, FLYEM_BODY, FLYEM_COARSE_BODY,
  FLYEM_BODY_3D, FLYEM_BODY_3D_COARSE, FLYEM_SKELETON, FLYEM_MESH,
  FLYEM_STACK,
  FLYEM_SPLIT, FLYEM_ROI, FLYEM_MERGE, SEGMENTATION_TARGET, FLYEM_DVID,
  FLYEM_BODY_DISPLAY, FLYEM_PROOFREAD, FLYEM_ORTHO, FLYEM_ARBSLICE
};
}

namespace View {
enum class EExploreAction {
  EXPLORE_NONE, EXPLORE_MOVE, EXPLORE_ZOOM, EXPLORE_SLICE,
  EXPLORE_ZOOM_DONE, EXPLORE_MOVE_DONE,
  EXPLORE_UNKNOWN
};
}

enum class EImageBackground {
  BRIGHT, DARK
};

enum class ESizeHintOption {
  DEFAULT, CURRENT_BEST, TAKING_SPACE
};

//Must have value X=0, Y=1, Z=2 for indexing
enum class EAxis : int {
  X = 0, Y = 1, Z = 2
  , ARB //arbitrary axis
};

template <typename T>
constexpr typename std::underlying_type<T>::type EnumValue(T val)
{
    return static_cast<typename std::underlying_type<T>::type>(val);
}

enum class EPlane {
  XY = 0, XZ, YZ
};

enum class EAxisSystem {
  NORMAL, SHIFTED
};

enum class ECoordinateSystem {
  WIDGET,
  SCREEN,
  RAW_STACK, //Coordiantes relative to the first stack corner
  STACK,     //Absolute coordinates in the current stack alignment
  ORGDATA,   //Coordinates registered to the original data
  WORLD_2D, CANVAS
};

enum class EColor {
  RED, GREEN, BLUE, ALL
};

enum class ESelectOption {
  APPEND, ALONE, ALONE_TYPE
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

enum EReadStatus {
  READ_NULL, READ_OK, READ_FAILED, READ_TIMEOUT, READ_CANCELED,
  READ_BAD_RESPONSE
};

enum EToDoAction {
  TO_DO, TO_MERGE, TO_SPLIT, TO_SUPERVOXEL_SPLIT, TO_DO_IRRELEVANT
};

namespace display {
typedef uint64_t TVisualEffect;
static const TVisualEffect VE_NONE = 0;
static const TVisualEffect VE_Z_PROJ = BIT_FLAG(19);
static const TVisualEffect VE_GROUP_HIGHLIGHT = BIT_FLAG(20);

namespace image {
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

namespace Box {
static const TVisualEffect VE_GRID = BIT_FLAG(1);
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

#if defined(INT32_MIN)
static const int DIM_INVALID_INDEX = INT32_MIN;
#else
static const int DIM_INVALID_INDEX = -2147483647;
#endif

static const int DIM_PROJECTION_INDEX = DIM_INVALID_INDEX + 1;
static const int DIM_MIN_NORMAL_INDEX = DIM_INVALID_INDEX + 10;
}

namespace flyem {
enum EDvidAnnotationLoadMode {
  LOAD_NO_PARTNER, LOAD_PARTNER_LOCATION, LOAD_PARTNER_RELJSON
};

enum class EProofreadingMode {
  NORMAL, SPLIT
};

enum EBodyType {
  BODY_DEFAULT, BODY_SPHERE, BODY_SKELETON, BODY_MESH
};

enum class EBodyLabelType {
  BODY, SUPERVOXEL
};

enum EBodySplitMode {
  BODY_SPLIT_NONE, BODY_SPLIT_ONLINE, BODY_SPLIT_OFFLINE
};

enum EBodySplitRange {
  RANGE_FULL, RANGE_SEED, RANGE_LOCAL
};

enum EDataSliceUpdatePolicy {
  UPDATE_DIRECT, UPDATE_HIDDEN, UPDATE_LOWESTRES, UPDATE_LOWRES, UPDATE_SMALL
};

static const uint64_t LABEL_ID_SELECTION =
    std::numeric_limits<uint64_t>::max() - 1;
}

using TProgressFunc = std::function<void(size_t, size_t)>;

#if __cplusplus >= 201103L
#  undef NULL
#  define NULL nullptr
#endif

#if defined(SANITIZE_THREAD)
#  define STD_COUT if (1) {} else std::cout
#else
#  define STD_COUT std::cout
#endif

#if defined(_DEBUG_) && !defined(SANITIZE_THREAD)
#  define DEBUG_OUT std::cout
#else
#  define DEBUG_OUT if (1) {} else std::cout
#endif

#endif // NEUTUBE_DEF_H
