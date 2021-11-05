#ifndef ZDVIDDEF_H
#define ZDVIDDEF_H

#include "common/neutudefs.h"
#include "geometry/zintcuboid.h"

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

struct SparsevolConfig {
  uint64_t bodyId = 0;
  ZIntCuboid range;
  int zoom = 0;
  neutu::EBodyLabelType labelType;
  std::string format;
};

struct RequestParam {
  const char *payload = nullptr;
  int length = 0;
  bool isJson = false;
  int timeout = 120; // in s
};

}

#endif // ZDVIDDEF_H
