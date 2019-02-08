#ifndef FLYEMDEF_H
#define FLYEMDEF_H

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

}

#endif // FLYEMDEF_H
