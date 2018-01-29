#ifndef Z3DDEF_H
#define Z3DDEF_H

#include <string>

namespace neutube3d {

enum EVolumeRenderingMode {
  VR_AUTO = 0, VR_ALPHA_BLENDING, VR_MIP, VR_MIP_OPAQUE, VR_LOCAL_MIP,
  VR_LOCAL_MIP_OPAQUE, VR_ISO_SURFACE, VR_XRAY
};

enum ESwcGeometryPrimitive {
  SWC_NORMAL, SWC_SPHERE, SWC_LINE, SWC_CYLINDER
};

enum EWindowType {
  TYPE_GENERAL, TYPE_COARSE_BODY, TYPE_BODY, TYPE_SKELETON, TYPE_NEU3,
  TYPE_MESH
};

enum ERendererLayer {
  LAYER_SWC, LAYER_PUNCTA, LAYER_GRAPH, LAYER_SURFACE, LAYER_VOLUME,
  LAYER_TODO, LAYER_MESH, LAYER_ROI, LAYER_DECORATION
};

const char* GetVolumeName();
const char* GetVolumeRenderingModeName(EVolumeRenderingMode mode);

std::string GetWindowKeyString(neutube3d::EWindowType type);

}

#endif // Z3DDEF_H
