#ifndef Z3DDEF_H
#define Z3DDEF_H

#include <string>

namespace NeuTube3D {

enum EVolumeRenderingMode {
  VR_AUTO = 0, VR_ALPHA_BLENDING, VR_MIP, VR_MIP_OPAQUE, VR_LOCAL_MIP,
  VR_LOCAL_MIP_OPAQUE, VR_ISO_SURFACE, VR_XRAY
};

enum ESwcGeometryPrimitive {
  SWC_NORMAL, SWC_SPHERE, SWC_LINE, SWC_CYLINDER
};

enum EWindowType {
  TYPE_GENERAL, TYPE_COARSE_BODY, TYPE_BODY, TYPE_SKELETON
};

const char* GetVolumeName();
const char* GetVolumeRenderingModeName(EVolumeRenderingMode mode);

std::string GetWindowKeyString(NeuTube3D::EWindowType type);

}

#endif // Z3DDEF_H
