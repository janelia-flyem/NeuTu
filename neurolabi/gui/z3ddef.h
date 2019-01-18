#ifndef Z3DDEF_H
#define Z3DDEF_H

#include <string>

namespace neutube3d {

enum class EVolumeRenderingMode {
  VR_AUTO = 0, VR_ALPHA_BLENDING, VR_MIP, VR_MIP_OPAQUE, VR_LOCAL_MIP,
  VR_LOCAL_MIP_OPAQUE, VR_ISO_SURFACE, VR_XRAY
};

enum class ESwcGeometryPrimitive {
  SWC_NORMAL, SWC_SPHERE, SWC_LINE, SWC_CYLINDER
};

enum class EWindowType {
  GENERAL, COARSE_BODY, BODY, SKELETON, NEU3, MESH
};

enum class ERendererLayer {
  SWC, PUNCTA, GRAPH, SURFACE, VOLUME,
  TODO, MESH, ROI, DECORATION, SLICE
};

const char* GetVolumeName();
const char* GetVolumeRenderingModeName(EVolumeRenderingMode mode);

std::string GetWindowKeyString(EWindowType type);

}

#endif // Z3DDEF_H
