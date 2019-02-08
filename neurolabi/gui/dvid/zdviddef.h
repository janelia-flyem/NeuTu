#ifndef ZDVIDDEF_H
#define ZDVIDDEF_H

namespace dvid {
enum class ELabelIndexType {
  POST_SYN, PRE_SYN, GAP, ALL_SYN, NOTE, VOXEL
};

enum class ENodeStatus {
  NORMAL, INVALID, OFFLINE, LOCKED
};

enum class EDataType {
  UNKNOWN, ANNOTATION, KEYVALUE, LABELBLK, LABELSZ,
  LABELVOL, UINT8BLK, LABELGRAPH, ROI, IMAGETILE
};

enum class EAnnotationLoadMode {
  NO_PARTNER, PARTNER_LOCATION, PARTNER_RELJSON
};

static const int DEFAULT_ROI_BLOCK_SIZE = 32;

}

#endif // ZDVIDDEF_H
