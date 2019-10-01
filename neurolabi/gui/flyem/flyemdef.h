#ifndef FLYEMDEF_H
#define FLYEMDEF_H

#include <QChar>

namespace flyem {

enum class EProofreadingMode {
  NORMAL, SPLIT
};

enum class EBodyType {
  DEFAULT, SPHERE, SKELETON, MESH
};

enum class EBodySplitRange {
  FULL, SEED, LOCAL
};

namespace key {
static const char *GRAYSCALE = "grayscale";
static const char *SEGMENTATION = "segmentation";
}

namespace html {
static const char* COARSE_BODY_ICON = "<font color=red><b>&#9679;</b></font>";
static const char* FINE_BODY_ICON =
    "<font color=red><b>&#3894;</b></font>";
static const char* COARSE_MESH_ICON = "<font color=green>&#9650;</font>";
static const char* FINE_MESH_ICON = "<font color=green>&#9650;&#9650;&#9650;</font>";
}

}

#endif // FLYEMDEF_H
